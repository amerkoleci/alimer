// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_AUDIO)
#include "alimer_internal.h"
#include "alimer_audio.h"

ALIMER_DISABLE_WARNINGS()
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
#include "third_party/miniaudio.h"
ALIMER_ENABLE_WARNINGS()

#include <mutex>
#include <atomic>

namespace
{
    static_assert(sizeof(AudioEngineState) == sizeof(ma_device_state));
    static_assert(AudioEngineState_Uninitialized == (int)ma_device_state_uninitialized);
    static_assert(AudioEngineState_Stopped == (int)ma_device_state_stopped);
    static_assert(AudioEngineState_Started == (int)ma_device_state_started);
    static_assert(AudioEngineState_Starting == (int)ma_device_state_starting);
    static_assert(AudioEngineState_Stopping == (int)ma_device_state_stopping);

    static_assert(sizeof(ma_vec3f) == sizeof(Vector3));

    constexpr AudioDeviceType FromMiniaudio(ma_device_type value)
    {
        switch (value)
        {
            case ma_device_type_playback:   return AudioDeviceType_Playback;
            case ma_device_type_capture:    return AudioDeviceType_Capture;

            default:
                ALIMER_UNREACHABLE();
                return _AudioDeviceType_Count;
        }
    }

    constexpr AudioFormat FromMiniaudio(ma_format value)
    {
        switch (value)
        {
            case ma_format_unknown: return AudioFormat_Unknown;
            case ma_format_u8:      return AudioFormat_Unsigned8;
            case ma_format_s16:     return AudioFormat_Signed16;
            case ma_format_s24:     return AudioFormat_Signed24;
            case ma_format_s32:     return AudioFormat_Signed32;
            case ma_format_f32:     return AudioFormat_Float32;

            default:
                ALIMER_UNREACHABLE();
                return AudioFormat_Unknown;
        }
    }

    static void FromMiniaudio(const ma_vec3f& value, Vector3* result)
    {
        result->x = value.x;
        result->y = value.y;
        result->z = value.z;
    }

    constexpr ma_vec3f ToMiniaudio(const Vector3& value)
    {
        return ma_vec3f{ value.x, value.y, value.z };
    }

    static void log_callback(void* pUserData, ma_uint32 level, const char* message)
    {
        ALIMER_UNUSED(pUserData);

        switch (level)
        {
            case MA_LOG_LEVEL_DEBUG:
                alimerLogFormat(LogCategory_Audio, LogLevel_Debug, "[MiniAudio] {}", message);
                break;

            case MA_LOG_LEVEL_INFO:
                alimerLogFormat(LogCategory_Audio, LogLevel_Info, "[MiniAudio] {}", message);
                break;

            case MA_LOG_LEVEL_WARNING:
                alimerLogFormat(LogCategory_Audio, LogLevel_Warn, "[MiniAudio] {}", message);
                break;

            case MA_LOG_LEVEL_ERROR:
                alimerLogFormat(LogCategory_Audio, LogLevel_Error, "[MiniAudio] {}", message);
                break;
        }
    }
}

struct AudioContext final
{
    std::atomic_uint32_t refCount;
    ma_context handle;
};

struct AudioEngine final
{
    std::atomic_uint32_t refCount;
    std::mutex readMutex;
    ma_device device;
    ma_engine handle;
    ma_node* endpointNode = nullptr;
};

AudioContext* alimerAudioContextInit(void)
{
    AudioContext* context = new AudioContext();
    context->refCount.store(1);

    ma_result result = ma_log_init(nullptr, &context->handle.log);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_log_init failed: %s", ma_result_description(result));
        return nullptr;
    }

    result = ma_log_register_callback(&context->handle.log, ma_log_callback_init(log_callback, nullptr));
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&context->handle.log);
        alimerLogError(LogCategory_Audio, "ma_log_register_callback failed: %s", ma_result_description(result));
        return nullptr;
    }

    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.pLog = &context->handle.log;

    ma_backend nullBackend = ma_backend_null;

    result = ma_context_init(nullptr, 0, &contextConfig, &context->handle);
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&context->handle.log);
        alimerLogError(LogCategory_Audio, "ma_context_init failed: %s", ma_result_description(result));
        return nullptr;
    }

    return context;
}

void alimerAudioContextDestroy(AudioContext* context)
{
    uint32_t newCount = --context->refCount;
    if (newCount == 0)
    {
        ma_result result = ma_context_uninit(&context->handle);
        if (result != MA_SUCCESS)
        {
            alimerLogError(LogCategory_Audio, "ma_context_uninit failed: %s", ma_result_description(result));
        }

        delete context;
    }
}

static AudioDeviceCallback* s_enumerateCallback = nullptr;

static ma_bool32 enumDevicesCallback(ma_context* context, ma_device_type type, const ma_device_info* info, void* userdata)
{
    const AudioDeviceType deviceType = FromMiniaudio(type);
    AudioDevice device = { deviceType, sizeof(info->id), &info->id, info->name, info->isDefault };
    s_enumerateCallback(&device, userdata);
    return MA_TRUE;
}

void alimerAudioContextEnumerateDevices(AudioContext* context, AudioDeviceCallback* callback, void* userdata)
{
    s_enumerateCallback = callback;
    ma_result result = ma_context_enumerate_devices(&context->handle, enumDevicesCallback, userdata);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_context_enumerate_devices failed: %s", ma_result_description(result));
        return;
    }
}

/* AudioEngine */
AudioEngine* alimerAudioEngineCreate(AudioContext* context, const AudioConfig* config)
{
    AudioEngine* engine = new AudioEngine();
    engine->refCount.store(1);

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    //if (playbackDevice)
    //    deviceConfig.playback.pDeviceID = reinterpret_cast<const ma_device_id*>(&playbackDevice->data[0]);

    // Device config for engines (taken from ma_engine_init)
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.noPreSilencedOutputBuffer = MA_TRUE;
    deviceConfig.noClip = MA_TRUE;
    deviceConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 0;
    deviceConfig.pUserData = engine;
    deviceConfig.dataCallback = [](ma_device* device, void* pOutput, const void* pInput, ma_uint32 frameCount)
    {
        AudioEngine& thisEngine = *static_cast<AudioEngine*>(device->pUserData);
        std::unique_lock callbackLock(thisEngine.readMutex);
        ma_engine_read_pcm_frames(&thisEngine.handle, pOutput, frameCount, nullptr);
    };
    deviceConfig.notificationCallback = [](const ma_device_notification* pNotification)
    {
        ALIMER_UNUSED(pNotification);
    };
    ma_result result = ma_device_init(&context->handle, &deviceConfig, &engine->device);
    if (result != MA_SUCCESS)
    {
        delete engine;
        alimerLogError(LogCategory_Audio, "ma_device_init failed: {}", ma_result_description(result));
        return nullptr;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &engine->device;
    engineConfig.pProcessUserData = engine;
    engineConfig.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 0;
    engineConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 0;
    result = ma_engine_init(&engineConfig, &engine->handle);
    if (result != MA_SUCCESS)
    {
        delete engine;
        alimerLogError(LogCategory_Audio, "ma_engine_init failed: {}", ma_result_description(result));
        return nullptr;
    }

    engine->endpointNode = ma_engine_get_endpoint(&engine->handle);

    return engine;
}

void alimerAudioEngineDestroy(AudioEngine* engine)
{
    uint32_t newCount = --engine->refCount;
    if (newCount == 0)
    {
        ma_engine_uninit(&engine->handle);
        delete engine;
    }
}

void alimerAudioEngineStart(AudioEngine* engine)
{
    ma_result result = ma_device_start(&engine->device);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_device_start failed: {}", ma_result_description(result));
    }
}

void alimerAudioEngineStop(AudioEngine* engine)
{
    ma_result result = ma_device_stop(&engine->device);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_device_stop failed: {}", ma_result_description(result));
    }
}

AudioEngineState alimerAudioEngineGetState(AudioEngine* engine)
{
    ma_device_state state = ma_device_get_state(&engine->device);
    return static_cast<AudioEngineState>(state);
}

float alimerAudioEngineGetMasterVolume(AudioEngine* engine, VolumeUnit unit)
{
    float volume;
    ma_result result = ma_device_get_master_volume(&engine->device, &volume);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_device_get_master_volume failed: {}", ma_result_description(result));
        return 0.f;
    }

    return (unit == VolumeUnit_Linear) ? volume : ma_volume_linear_to_db(volume);
}

void alimerAudioEngineSetMasterVolume(AudioEngine* engine, float value, VolumeUnit unit)
{
    if (unit == VolumeUnit_Decibels)
        value = ma_volume_db_to_linear(value);

    ma_result result = ma_device_set_master_volume(&engine->device, value);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_device_set_master_volume failed: {}", ma_result_description(result));
    }
}

float alimerAudioEngineGetVolume(AudioEngine* engine, VolumeUnit unit)
{
    const float volume = ma_engine_get_volume(&engine->handle);
    return (unit == VolumeUnit_Linear) ? volume : ma_volume_linear_to_db(volume);
}

void alimerAudioEngineSetVolume(AudioEngine* engine, float value, VolumeUnit unit)
{
    if (unit == VolumeUnit_Decibels)
        value = ma_volume_db_to_linear(value);

    ma_engine_set_volume(&engine->handle, value);
}

uint32_t alimerAudioEngineGetChannelCount(AudioEngine* engine)
{
    return ma_engine_get_channels(&engine->handle);
}

uint32_t alimerAudioEngineGetSampleRate(AudioEngine* engine)
{
    return ma_engine_get_sample_rate(&engine->handle);
}

#endif /* defined(ALIMER_AUDIO) */
