// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/Audio/Audio.h"
#include "Alimer/Audio/AudioClip.h"
#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Audio/Components/AudioSourceComponent.h"
#include "Alimer/Audio/Components/AudioListenerComponent.h"

ALIMER_DISABLE_WARNINGS()
#define STB_VORBIS_HEADER_ONLY
//#define STB_VORBIS_NO_STDIO
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_COREAUDIO
#define MA_ENABLE_OPENSL
#define MA_ENABLE_WEBAUDIO
//#define MA_NO_DECODING
#define MA_NO_ENCODING
//#define MA_NO_RESOURCE_MANAGER
#define MA_NO_GENERATION
//#define MA_NO_NODE_GRAPH
//#define MA_NO_ENGINE
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
ALIMER_ENABLE_WARNINGS()

#include <tuple>

using namespace Alimer;

namespace Alimer
{
    struct AudioEngineImpl final
    {
        std::mutex readMutex;
        ma_device device;
        ma_engine handle;
        ma_node* endpointNode = nullptr;
        ma_node_graph* nodeGraph = nullptr;
    };
}

namespace
{
    static constexpr AudioDeviceType FromMiniaudio(ma_device_type type)
    {
        switch (type)
        {
            case ma_device_type_playback:
                return AudioDeviceType::Playback;
            case ma_device_type_capture:
                return AudioDeviceType::Capture;
            default:
                assert(false && "Unknown ma_device_type");
                return AudioDeviceType::Playback;
        }
    }

    static constexpr AudioEngineState FromMiniaudio(ma_device_state value)
    {
        switch (value)
        {
            case ma_device_state_uninitialized:
                return AudioEngineState::Uninitialized;
            case ma_device_state_stopped:
                return AudioEngineState::Stopped;
            case ma_device_state_started:
                return AudioEngineState::Started;
            case ma_device_state_starting:
                return AudioEngineState::Starting;
            case ma_device_state_stopping:
                return AudioEngineState::Stopping;
            default:
                assert(false && "Unknown ma_device_state");
                return AudioEngineState::Uninitialized;
        }
    }

    static inline Vector3 ToVector3(const ma_vec3f& v)
    {
        return { v.x, v.y, v.z };
    }

    static void MiniAudioLogCallback(void* pUserData, ma_uint32 level, const char* pMessage)
    {
        ALIMER_UNUSED(pUserData);

        switch (level)
        {
            case MA_LOG_LEVEL_DEBUG:
                LOGD("[MiniAudio] {}", pMessage);
                break;

            case MA_LOG_LEVEL_INFO:
                LOGI("[MiniAudio] {}", pMessage);
                break;

            case MA_LOG_LEVEL_WARNING:
                LOGW("[MiniAudio] {}", pMessage);
                break;

            case MA_LOG_LEVEL_ERROR:
                LOGE("[MiniAudio] {}", pMessage);
                break;
        }
    }

    static void DataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
    {
        AudioEngineImpl& thisEngine = *static_cast<AudioEngineImpl*>(pDevice->pUserData);
        std::unique_lock callbackLock(thisEngine.readMutex);

        if (thisEngine.handle.pResourceManager != nullptr)
        {
            if ((thisEngine.handle.pResourceManager->config.flags & MA_RESOURCE_MANAGER_FLAG_NO_THREADING) != 0)
            {
                ma_resource_manager_process_next_job(thisEngine.handle.pResourceManager);
            }
        }

        ma_engine_read_pcm_frames(&thisEngine.handle, pOutput, frameCount, nullptr);
    }
}

static struct
{
    std::atomic_bool initialized;
    ma_context* context = nullptr;
} g_audio;

AudioEngine::AudioEngine(const AudioConfig* config)
    : _impl(new AudioEngineImpl())
{

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    if (config && config->playbackDeviceId)
    {
        deviceConfig.playback.pDeviceID = reinterpret_cast<const ma_device_id*>(&config->playbackDeviceId->data[0]);
    }
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 2;
    deviceConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 48000;
    deviceConfig.dataCallback = DataCallback;
    deviceConfig.pUserData = _impl;

    ma_result result = ma_device_init(g_audio.context, &deviceConfig, &_impl->device);
    if (result != MA_SUCCESS)
    {
        LOGE("Failed to initialize audio device");
        return;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &_impl->device;
    engineConfig.pProcessUserData = _impl;
    engineConfig.listenerCount = 1;
    result = ma_engine_init(&engineConfig, &_impl->handle);
    if (result != MA_SUCCESS)
    {
        LOGE("Failed to initialize audio engine");
        return;
    }

    char name[512];
    ma_device_get_name(&_impl->device, ma_device_type_playback, name, 512, nullptr);
    LOGI("Audio engine created with success using playback device '{}'", name);

    _impl->endpointNode = ma_engine_get_endpoint(&_impl->handle);
    _impl->nodeGraph = ma_engine_get_node_graph(&_impl->handle);
    _listenerCount = ma_engine_get_listener_count(&_impl->handle);
}

AudioEngine::~AudioEngine()
{
    if (_impl)
    {
        ma_engine_uninit(&_impl->handle);
        ma_device_uninit(&_impl->device);
        delete _impl;
        _impl = nullptr;
    }
}

Vector<AudioDevice> Audio::PlaybackDevices;
Vector<AudioDevice> Audio::CaptureDevices;

bool Audio::Initialize()
{
    if (g_audio.initialized.load())
        return true;

    g_audio.context = new ma_context();

    ma_result result = ma_log_init(nullptr, &g_audio.context->log);
    if (result != MA_SUCCESS)
    {
        LOGE("ma_log_init failed: {}", ma_result_description(result));
        return false;
    }

    result = ma_log_register_callback(&g_audio.context->log, ma_log_callback_init(MiniAudioLogCallback, nullptr));
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&g_audio.context->log);
        LOGE("ma_log_register_callback failed: {}", ma_result_description(result));
        return false;
    }

    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.pLog = &g_audio.context->log;

    result = ma_context_init(nullptr, 0, &contextConfig, g_audio.context);
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&g_audio.context->log);
        LOGE("ma_context_init failed: {}", ma_result_description(result));
        return false;
    }

    // Enumerate devices
    result = ma_context_enumerate_devices(g_audio.context, [](ma_context* /*context*/, ma_device_type deviceType, const ma_device_info* info, void* userdata) -> ma_bool32
        {
            const AudioDeviceType engineDeviceType = FromMiniaudio(deviceType);

            static_assert(sizeof(AudioDeviceId) >= sizeof(ma_device_id));
            static_assert(alignof(AudioDeviceId) >= alignof(ma_device_id));
            static_assert(std::tuple_size_v<decltype(AudioDevice::deviceName)> >= MA_MAX_DEVICE_NAME_LENGTH + 1);

            std::vector<AudioDevice>& deviceList = (deviceType == ma_device_type_playback) ? Audio::PlaybackDevices : Audio::CaptureDevices;
            auto& device = deviceList.emplace_back();
            std::memcpy(&device.deviceId.data[0], &info->id, sizeof(ma_device_id));
            std::strcpy(&device.deviceName[0], &info->name[0]);

            device.isDefault = info->isDefault == MA_TRUE;
            if (deviceType == ma_device_type_playback)
            {
                device.deviceType = AudioDeviceType::Playback;
            }
            else
            {
                assert(deviceType == ma_device_type_capture);
                device.deviceType = AudioDeviceType::Capture;
            }

            return MA_TRUE;
        }, nullptr);
    if (result != MA_SUCCESS)
    {
        LOGE("ma_context_enumerate_devices failed: {}", ma_result_description(result));
        ma_context_uninit(g_audio.context);
        delete g_audio.context;
        g_audio.context = nullptr;
        return false;
    }

    AudioClip::Register();
    AudioSource::Register();
    AudioSourceComponent::Register();
    AudioListenerComponent::Register();

    g_audio.initialized.store(true);

    LOGI("Audio Initialized");
    return true;
}

void Audio::Shutdown()
{
    if (!g_audio.initialized.load())
        return;

    if (g_audio.context)
    {
        ma_result result = ma_context_uninit(g_audio.context);
        if (result != MA_SUCCESS)
        {
            LOGE("ma_context_uninit failed: {}", ma_result_description(result));
        }

        delete g_audio.context;
        g_audio.context = nullptr;
    }

    g_audio.initialized.store(false);
    memset(&g_audio, 0, sizeof(g_audio));
}

uint32_t AudioEngine::GetOutputChannelCount() const
{
    return ma_engine_get_channels(&_impl->handle);
}

uint32_t AudioEngine::GetOutputSampleRate() const
{
    return ma_engine_get_sample_rate(&_impl->handle);
}

void AudioEngine::Suspend()
{
    if (ma_engine_stop(&_impl->handle) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to suspend");
    }
}

void AudioEngine::Resume()
{
    if (ma_engine_start(&_impl->handle) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to resume");
    }
}

AudioEngineState AudioEngine::GetState() const
{
    ma_device_state state = ma_device_get_state(&_impl->device);
    return FromMiniaudio(state);
}

float AudioEngine::GetMasterVolume(VolumeUnit unit) const
{
    float volume;
    ma_result result = ma_device_get_master_volume(&_impl->device, &volume);
    if (result != MA_SUCCESS)
    {
        LOGE( "ma_device_get_master_volume failed: {}", ma_result_description(result));
        return 0.f;
    }

    return (unit == VolumeUnit::Linear) ? volume : ma_volume_linear_to_db(volume);
}

void AudioEngine::SetMasterVolume(float volume, VolumeUnit unit)
{
    if (unit == VolumeUnit::Decibels)
        volume = ma_volume_db_to_linear(volume);

    ma_result result = ma_device_set_master_volume(&_impl->device, volume);
    if (result != MA_SUCCESS)
    {
        LOGE("ma_device_set_master_volume failed: {}", ma_result_description(result));
    }
}

float AudioEngine::GetVolume(VolumeUnit unit) const
{
    const float volume = ma_engine_get_volume(&_impl->handle);
    return (unit == VolumeUnit::Linear) ? volume : ma_volume_linear_to_db(volume);
}

void AudioEngine::SetVolume(float volume, VolumeUnit unit)
{
    if (unit == VolumeUnit::Decibels)
        volume = ma_volume_db_to_linear(volume);

    ma_result result = ma_engine_set_volume(&_impl->handle, volume);
    if (result != MA_SUCCESS)
    {
        LOGE("Audio: Failed to set master volume");
    }
}


uint64_t AudioEngine::GetTimeInPCMFrames() const
{
    return (uint64_t)ma_engine_get_time_in_pcm_frames(&_impl->handle);
}

uint64_t AudioEngine::GetTimeInMilliseconds() const
{
    return (uint64_t)ma_engine_get_time_in_milliseconds(&_impl->handle);
}

void AudioEngine::SetTimeInPCMFrames(uint64_t value)
{
    ma_result result = ma_engine_set_time_in_pcm_frames(&_impl->handle, (ma_uint64)value);
    if (result != MA_SUCCESS)
    {
        LOGE("ma_engine_set_time_in_pcm_frames failed: {}", ma_result_description(result));
    }
}

void AudioEngine::SetTimeInMilliseconds(uint64_t value)
{
    ma_result result = ma_engine_set_time_in_milliseconds(&_impl->handle, (ma_uint64)value);
    if (result != MA_SUCCESS)
    {
        LOGE("ma_engine_set_time_in_milliseconds failed: {}", ma_result_description(result));
    }
}

bool AudioEngine::IsListenerEnabled(uint32_t listenerIndex) const
{
    return ma_engine_listener_is_enabled(&_impl->handle, listenerIndex);
}

void AudioEngine::SetListenerEnabled(uint32_t listenerIndex, bool enabled)
{
    ma_engine_listener_set_enabled(&_impl->handle, listenerIndex, (enabled) ? MA_TRUE : MA_FALSE);
}

Vector3 AudioEngine::GetListenerPosition(uint32_t listenerIndex) const
{
    return ToVector3(ma_engine_listener_get_position(&_impl->handle, listenerIndex));
}

void AudioEngine::SetListenerPosition(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_position(&_impl->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 AudioEngine::GetListenerDirection(uint32_t listenerIndex) const
{
    return ToVector3(ma_engine_listener_get_direction(&_impl->handle, listenerIndex));
}

void AudioEngine::SetListenerDirection(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_direction(&_impl->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 AudioEngine::GetListenerVelocity(uint32_t listenerIndex) const
{
    return ToVector3(ma_engine_listener_get_velocity(&_impl->handle, listenerIndex));
}

void AudioEngine::SetListenerVelocity(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_velocity(&_impl->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 AudioEngine::GetListenerWorldUp(uint32_t listenerIndex) const
{
    return ToVector3(ma_engine_listener_get_world_up(&_impl->handle, listenerIndex));
}

void AudioEngine::SetListenerWorldUp(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_world_up(&_impl->handle, listenerIndex, value.x, value.y, value.z);
}

void AudioEngine::GetListenerCone(uint32_t listenerIndex, float& innerAngleInRadians, float& outerAngleInRadians, float& outerGain) const
{
    ma_engine_listener_get_cone(&_impl->handle, listenerIndex, &innerAngleInRadians, &outerAngleInRadians, &outerGain);
}

void AudioEngine::SetListenerCone(uint32_t listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain)
{
    ma_engine_listener_set_cone(&_impl->handle, listenerIndex, innerAngleInRadians, outerAngleInRadians, outerGain);
}

ma_engine* AudioEngine::GetEngine()  const
{
    return &_impl->handle;
}
