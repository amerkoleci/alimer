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
}

struct AudioEngine final
{
    std::atomic_uint32_t refCount;
    std::mutex readMutex;
    ma_device device;
    ma_engine handle;
    ma_node* endpointNode = nullptr;
    ma_node_graph* nodeGraph = nullptr;
    uint32_t listenerCount = 0;
};

static void DataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    AudioEngine& thisEngine = *static_cast<AudioEngine*>(pDevice->pUserData);
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

static struct
{
    std::atomic_bool initialized;
    ma_context* context = nullptr;
    AudioEngine* engine = nullptr;
} g_audio;

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

    if (g_audio.engine)
    {
        ShutdownEngine();
    }

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

bool Audio::InitEngine(const AudioConfig* config)
{
    if (g_audio.engine)
        return true;

    g_audio.engine = new AudioEngine();

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    if (config && config->playbackDevice)
    {
        //deviceConfig.playback.pDeviceID = &config->playbackDevice->info->id;
    }
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 2;
    deviceConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 48000;
    deviceConfig.dataCallback = DataCallback;
    deviceConfig.pUserData = g_audio.engine;

    ma_result result = ma_device_init(g_audio.context, &deviceConfig, &g_audio.engine->device);
    if (result != MA_SUCCESS)
    {
        LOGE("Failed to initialize audio device");
        delete g_audio.engine;
        g_audio.engine = nullptr;
        return false;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &g_audio.engine->device;
    engineConfig.pProcessUserData = g_audio.engine;
    engineConfig.listenerCount = 1;
    result = ma_engine_init(&engineConfig, &g_audio.engine->handle);
    if (result != MA_SUCCESS)
    {
        LOGE("Failed to initialize audio engine");
        delete g_audio.engine;
        g_audio.engine = nullptr;
        return false;
    }

    char name[512];
    ma_device_get_name(&g_audio.engine->device, ma_device_type_playback, name, 512, nullptr);
    LOGI("Audio engine created with success using playback device {}", name);

    g_audio.engine->endpointNode = ma_engine_get_endpoint(&g_audio.engine->handle);
    g_audio.engine->nodeGraph = ma_engine_get_node_graph(&g_audio.engine->handle);

    // Init listeners
    g_audio.engine->listenerCount = ma_engine_get_listener_count(&g_audio.engine->handle);

    LOGI("Audio Engine Initialized");
    return true;
}

void Audio::ShutdownEngine()
{
    if (!g_audio.engine)
        return;

    ma_engine_uninit(&g_audio.engine->handle);
    ma_device_uninit(&g_audio.engine->device);
    delete g_audio.engine;
    g_audio.engine = nullptr;
}

void Audio::Suspend()
{
    if (ma_engine_stop(&g_audio.engine->handle) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to suspend");
    }
}

void Audio::Resume()
{
    if (ma_engine_start(&g_audio.engine->handle) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to resume");
    }
}

float Audio::GetVolume(VolumeUnit unit)
{
    const float volume = ma_engine_get_volume(&g_audio.engine->handle);
    return (unit == VolumeUnit::Linear) ? volume : ma_volume_linear_to_db(volume);
}

void Audio::SetVolume(float volume, VolumeUnit unit)
{
    if (unit == VolumeUnit::Decibels)
        volume = ma_volume_db_to_linear(volume);

    if (ma_engine_set_volume(&g_audio.engine->handle, volume) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to set master volume");
    }
}

uint32_t Audio::GetOutputChannelCount()
{
    return ma_engine_get_channels(&g_audio.engine->handle);
}

uint32_t Audio::GetOutputSampleRate()
{
    return ma_engine_get_sample_rate(&g_audio.engine->handle);
}

uint32_t Audio::GetListenerCount()
{
    return g_audio.engine->listenerCount;
}

bool Audio::IsListenerEnabled(uint32_t listenerIndex)
{
    return ma_engine_listener_is_enabled(&g_audio.engine->handle, listenerIndex);
}

void Audio::SetListenerEnabled(uint32_t listenerIndex, bool enabled)
{
    ma_engine_listener_set_enabled(&g_audio.engine->handle, listenerIndex, (enabled) ? MA_TRUE : MA_FALSE);
}

Vector3 Audio::GetListenerPosition(uint32_t listenerIndex)
{
    return ToVector3(ma_engine_listener_get_position(&g_audio.engine->handle, listenerIndex));
}

void Audio::SetListenerPosition(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_position(&g_audio.engine->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 Audio::GetListenerDirection(uint32_t listenerIndex)
{
    return ToVector3(ma_engine_listener_get_direction(&g_audio.engine->handle, listenerIndex));
}

void Audio::SetListenerDirection(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_direction(&g_audio.engine->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 Audio::GetListenerVelocity(uint32_t listenerIndex)
{
    return ToVector3(ma_engine_listener_get_velocity(&g_audio.engine->handle, listenerIndex));
}

void Audio::SetListenerVelocity(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_velocity(&g_audio.engine->handle, listenerIndex, value.x, value.y, value.z);
}

Vector3 Audio::GetListenerWorldUp(uint32_t listenerIndex)
{
    return ToVector3(ma_engine_listener_get_world_up(&g_audio.engine->handle, listenerIndex));
}

void Audio::SetListenerWorldUp(uint32_t listenerIndex, const Vector3& value)
{
    ma_engine_listener_set_world_up(&g_audio.engine->handle, listenerIndex, value.x, value.y, value.z);
}

void Audio::SetListenerCone(uint32_t listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain)
{
    ma_engine_listener_set_cone(&g_audio.engine->handle, listenerIndex, innerAngleInRadians, outerAngleInRadians, outerGain);
}

void Audio::GetListenerCone(uint32_t listenerIndex, float& innerAngleInRadians, float& outerAngleInRadians, float& outerGain)
{
    ma_engine_listener_get_cone(&g_audio.engine->handle, listenerIndex, &innerAngleInRadians, &outerAngleInRadians, &outerGain);
}

ma_engine* Audio::GetEngine()
{
    return &g_audio.engine->handle;
}
