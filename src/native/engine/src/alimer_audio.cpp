// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_AUDIO)
#include "alimer_internal.h"
#include "alimer_audio.h"

ALIMER_DISABLE_WARNINGS()
#define STB_VORBIS_HEADER_ONLY 
#include "third_party/stb_vorbis.c"

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

#undef STB_VORBIS_HEADER_ONLY 
#include "third_party/stb_vorbis.c"

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

    constexpr uint32_t FormatSize(AudioFormat value)
    {
        switch (value)
        {
            case AudioFormat_Unknown: return 0;
            case AudioFormat_Unsigned8: return 1;
            case AudioFormat_Signed16: return 2;
            case AudioFormat_Signed24: return 3;
            case AudioFormat_Signed32: return 4;
            case AudioFormat_Float32: return 4;

            default:
                ALIMER_UNREACHABLE();
                return 0;
        }
    }

    constexpr AudioPanMode FromMiniaudio(ma_pan_mode value)
    {
        switch (value)
        {
            case ma_pan_mode_balance: return AudioPanMode_Balance;
            case ma_pan_mode_pan:     return AudioPanMode_Pan;

            default:
                ALIMER_UNREACHABLE();
                return AudioPanMode_Balance;
        }
    }


    static void FromMiniaudio(const ma_vec3f& value, Vector3* result)
    {
        ALIMER_ASSERT(result);

        result->x = value.x;
        result->y = value.y;
        result->z = value.z;
    }

    constexpr ma_vec3f ToMiniaudio(const Vector3& value)
    {
        return ma_vec3f{ value.x, value.y, value.z };
    }

    constexpr ma_pan_mode ToMiniaudio(AudioPanMode value)
    {
        switch (value)
        {
            case AudioPanMode_Balance: return ma_pan_mode_balance;
            case AudioPanMode_Pan:     return ma_pan_mode_pan;

            default:
                ALIMER_UNREACHABLE();
                return ma_pan_mode_balance;
        }
    }

    static void log_callback(void* pUserData, ma_uint32 level, const char* message)
    {
        ALIMER_UNUSED(pUserData);

        switch (level)
        {
            case MA_LOG_LEVEL_DEBUG:
                alimerLogFormat(LogCategory_Audio, LogLevel_Debug, "[MiniAudio] %s", message);
                break;

            case MA_LOG_LEVEL_INFO:
                alimerLogFormat(LogCategory_Audio, LogLevel_Info, "[MiniAudio] %s", message);
                break;

            case MA_LOG_LEVEL_WARNING:
                alimerLogFormat(LogCategory_Audio, LogLevel_Warn, "[MiniAudio] %s", message);
                break;

            case MA_LOG_LEVEL_ERROR:
                alimerLogFormat(LogCategory_Audio, LogLevel_Error, "[MiniAudio] %s", message);
                break;
        }
    }
}

struct AudioDevice final
{
    AudioDeviceType deviceType;
    const ma_device_info* info;
};

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

struct AudioClip final
{
    std::atomic_uint32_t refCount;
    ma_decoder* decoder = nullptr;
    AudioFormat format = AudioFormat_Unknown;
    uint32_t channels = 0;
    uint32_t sampleRate = 0;
    uint64_t frameCount = 0;
};

struct AudioSource final
{
    std::atomic_uint32_t refCount;
    AudioClip* clip = nullptr;
    ma_sound* handle = nullptr;
};

static struct
{
    std::atomic_uint32_t refCount;
    ma_log log;
    ma_context context;
} state;

bool alimerAudioInit(void)
{
    if (state.refCount > 0)
        return true;

    ma_result result = ma_log_init(nullptr, &state.log);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_log_init failed: %s", ma_result_description(result));
        return false;
    }

    result = ma_log_register_callback(&state.log, ma_log_callback_init(log_callback, nullptr));
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&state.log);
        alimerLogError(LogCategory_Audio, "ma_log_register_callback failed: %s", ma_result_description(result));
        return false;
    }

    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.pLog = &state.log;

    result = ma_context_init(nullptr, 0, &contextConfig, &state.context);
    if (result != MA_SUCCESS)
    {
        ma_log_uninit(&state.log);
        alimerLogError(LogCategory_Audio, "ma_context_init failed: %s", ma_result_description(result));
        return false;
    }

    state.refCount.store(1);
    return true;
}

void alimerAudioShutdown(void)
{
    uint32_t newCount = --state.refCount;
    if (newCount == 0)
    {
        ma_result result = ma_context_uninit(&state.context);
        if (result != MA_SUCCESS)
        {
            alimerLogError(LogCategory_Audio, "ma_context_uninit failed: %s", ma_result_description(result));
        }

        memset(&state, 0, sizeof(state));
    }
}

static AudioDeviceCallback* s_enumerateCallback = nullptr;

static ma_bool32 enumDevicesCallback(ma_context* context, ma_device_type type, const ma_device_info* info, void* userdata)
{
    const AudioDeviceType deviceType = FromMiniaudio(type);
    AudioDevice device = { deviceType, info };
    s_enumerateCallback(&device, userdata);
    return MA_TRUE;
}

void alimerAudioEnumerateDevices(AudioDeviceCallback* callback, void* userdata)
{
    s_enumerateCallback = callback;
    ma_result result = ma_context_enumerate_devices(&state.context, enumDevicesCallback, userdata);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_context_enumerate_devices failed: %s", ma_result_description(result));
        return;
    }
}

/* AudioDevice */
AudioDeviceType alimerAudioDeviceGetType(AudioDevice* device)
{
    return device->deviceType;
}

const char* alimerAudioDeviceGetName(AudioDevice* device)
{
    return device->info->name;
}

bool alimerAudioDeviceIsDefault(AudioDevice* device)
{
    return device->info->isDefault == MA_TRUE;
}

/* AudioEngine */
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

AudioEngine* alimerAudioEngineCreate(const AudioConfig* config)
{
    AudioEngine* engine = new AudioEngine();
    engine->refCount.store(1);

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    if (config && config->playbackDevice)
    {
        deviceConfig.playback.pDeviceID = &config->playbackDevice->info->id;
    }
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 2;
    deviceConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 48000;
    deviceConfig.dataCallback = DataCallback;
    deviceConfig.pUserData = engine;

    ma_result result = ma_device_init(&state.context, &deviceConfig, &engine->device);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "Failed to initialize audio device");
        delete engine;
        return nullptr;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &engine->device;
    engineConfig.pProcessUserData = engine;
    engineConfig.listenerCount = 1;
    result = ma_engine_init(&engineConfig, &engine->handle);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "Failed to initialize audio engine");
        delete engine;
        return nullptr;
    }

    char name[512];
    ma_device_get_name(&engine->device, ma_device_type_playback, name, 512, nullptr);
    alimerLogInfo(LogCategory_Audio, "Audio engine created with success using playback device %s", name);

    engine->endpointNode = ma_engine_get_endpoint(&engine->handle);
    engine->nodeGraph = ma_engine_get_node_graph(&engine->handle);
    engine->listenerCount = ma_engine_get_listener_count(&engine->handle);

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
    ma_result result = ma_engine_start(&engine->handle);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_engine_start failed: %s", ma_result_description(result));
    }
}

void alimerAudioEngineStop(AudioEngine* engine)
{
    ma_result result = ma_device_stop(&engine->device);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_device_stop failed: %s", ma_result_description(result));
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
        alimerLogError(LogCategory_Audio, "ma_device_get_master_volume failed: %s", ma_result_description(result));
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
        alimerLogError(LogCategory_Audio, "ma_device_set_master_volume failed: %s", ma_result_description(result));
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

uint64_t alimerAudioEngineGetTimeInPCMFrames(AudioEngine* engine)
{
    return ma_engine_get_time_in_pcm_frames(&engine->handle);
}

uint64_t alimerAudioEngineGetTimeInMilliseconds(AudioEngine* engine)
{
    return ma_engine_get_time_in_milliseconds(&engine->handle);
}

void alimerAudioEngineSetTimeInPCMFrames(AudioEngine* engine, uint64_t value)
{
    ma_result result = ma_engine_set_time_in_pcm_frames(&engine->handle, value);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_engine_set_time_in_pcm_frames failed: %s", ma_result_description(result));
    }
}

void alimerAudioEngineSetTimeInMilliseconds(AudioEngine* engine, uint64_t value)
{
    ma_result result = ma_engine_set_time_in_milliseconds(&engine->handle, value);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_engine_set_time_in_milliseconds failed: %s", ma_result_description(result));
    }
}

/* AudioListener */
uint32_t alimerAudioEngineGetListenerCount(AudioEngine* engine)
{
    return engine->listenerCount;
}

void alimerAudioEngineListenerSetPosition(AudioEngine* engine, uint32_t listenerIndex, const Vector3* position)
{
    ALIMER_ASSERT(position);

    ma_engine_listener_set_position(&engine->handle, listenerIndex, position->x, position->y, position->z);
}

void alimerAudioEngineListenerGetPosition(const AudioEngine* engine, uint32_t listenerIndex, Vector3* result)
{
    FromMiniaudio(ma_engine_listener_get_position(&engine->handle, listenerIndex), result);
}

bool alimerAudioEngineListenerIsEnabled(AudioEngine* engine, uint32_t listenerIndex)
{
    return ma_engine_listener_is_enabled(&engine->handle, listenerIndex) == MA_TRUE;
}

void alimerAudioEngineListenerSetEnabled(AudioEngine* engine, uint32_t listenerIndex, bool enabled)
{
    ma_engine_listener_set_enabled(&engine->handle, listenerIndex, enabled ? MA_TRUE : MA_FALSE);
}

static bool alimerAudioClipInitFromDecoder(AudioClip* clip)
{
    ma_format format;
    ma_decoder_get_data_format(clip->decoder, &format, &clip->channels, &clip->sampleRate, nullptr, 0);
    clip->format = FromMiniaudio(format);

    ma_uint64 frames = 0;
    ma_result result = ma_decoder_get_length_in_pcm_frames(clip->decoder, &frames);
    if (result != MA_SUCCESS)
        return false;

    clip->frameCount = static_cast<uint64_t>(frames);

    //ma_uint64 availableFrames;
    //ma_decoder_get_available_frames(clip->decoder, &availableFrames);
    return true;
}

AudioClip* alimerAudioClipCreate(const char* filepath)
{
    AudioClip* clip = new AudioClip();
    clip->refCount.store(1);
    clip->decoder = (ma_decoder*)ma_malloc(sizeof(ma_decoder), nullptr);

    ma_result result = ma_decoder_init_file(filepath, nullptr, clip->decoder);
    if (result != MA_SUCCESS)
    {
        ma_free(clip->decoder, nullptr);
        delete clip;
        return nullptr;
    }

    if (!alimerAudioClipInitFromDecoder(clip))
    {
        ma_free(clip->decoder, nullptr);
        delete clip;
        return nullptr;
    }

    return clip;
}

AudioClip* alimerAudioClipCreateFromMemory(const void* pData, size_t dataSize)
{
    AudioClip* clip = new AudioClip();
    clip->refCount.store(1);
    clip->decoder = (ma_decoder*)ma_malloc(sizeof(ma_decoder), nullptr);

    ma_result result = ma_decoder_init_memory(pData, dataSize, nullptr, clip->decoder);
    if (result != MA_SUCCESS)
    {
        ma_free(clip->decoder, nullptr);
        delete clip;
        return nullptr;
    }

    if (!alimerAudioClipInitFromDecoder(clip))
    {
        ma_free(clip->decoder, nullptr);
        delete clip;
        return nullptr;
    }

    return clip;

}

uint32_t alimerAudioClipAddRef(AudioClip* clip)
{
    return ++clip->refCount;
}

uint32_t alimerAudioClipRelease(AudioClip* clip)
{
    uint32_t newCount = --clip->refCount;
    if (newCount == 0)
    {
        if (clip->decoder)
        {
            ma_decoder_uninit(clip->decoder);
            ma_free(clip->decoder, nullptr);
        }

        delete clip;
    }
    return newCount;
}

AudioFormat alimerAudioClipGetFormat(AudioClip* clip)
{
    return clip->format;
}

uint32_t alimerAudioClipGetChannelCount(AudioClip* clip)
{
    return clip->channels;
}

uint32_t alimerAudioClipGetSampleRate(AudioClip* clip)
{
    return clip->sampleRate;
}

uint64_t alimerAudioClipGetFrameCount(AudioClip* clip)
{
    return clip->frameCount;
}

uint32_t alimerAudioClipGetStride(AudioClip* clip)
{
    return clip->channels * FormatSize(clip->format);
}

/* AudioSource */
AudioSource* alimerAudioSourceCreate(AudioEngine* engine, AudioClip* clip)
{
    AudioSource* source = new AudioSource();
    source->refCount.store(1);
    source->clip = clip;
    alimerAudioClipAddRef(clip);
    source->handle = (ma_sound*)ma_malloc(sizeof(ma_sound), nullptr);

    ma_uint32 soundFlags = 0; // MA_SOUND_FLAG_STREAM
    ma_result result = ma_sound_init_from_data_source(&engine->handle, &clip->decoder, soundFlags, nullptr, source->handle);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "Failed to initialize audio source!");
        ma_free(source->handle, nullptr);
        delete source;
        return nullptr;
    }

    return source;
}

uint32_t alimerAudioSourceAddRef(AudioSource* source)
{
    return ++source->refCount;
}

uint32_t alimerAudioSourceRelease(AudioSource* source)
{
    uint32_t newCount = --source->refCount;
    if (newCount == 0)
    {
        if (source->handle)
        {
            ma_sound_uninit(source->handle);
            ma_free(source->handle, nullptr);
            source->handle = nullptr;
        }

        alimerAudioClipRelease(source->clip);
        delete source;
    }
    return newCount;
}

void alimerAudioSourcePlay(AudioSource* source)
{
    ma_result result = ma_sound_start(source->handle);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_sound_start failed: %s", ma_result_description(result));
    }
}

void alimerAudioSourceStop(AudioSource* source)
{
    ma_result result = ma_sound_stop(source->handle);
    if (result != MA_SUCCESS)
    {
        alimerLogError(LogCategory_Audio, "ma_sound_stop failed: %s", ma_result_description(result));
    }
}

float alimerAudioSourceGetVolume(AudioSource* source, VolumeUnit unit)
{
    const float volume = ma_sound_get_volume(source->handle);
    return (unit == VolumeUnit_Linear) ? volume : ma_volume_linear_to_db(volume);
}

void alimerAudioSourceSetVolume(AudioSource* source, float value, VolumeUnit unit)
{
    const float volume = (unit == VolumeUnit_Linear) ? value : ma_volume_db_to_linear(value);
    ma_sound_set_volume(source->handle, volume);
}

void alimerAudioSourceSetPan(AudioSource* source, float value)
{
    ma_sound_set_pan(source->handle, value);
}

float alimerAudioSourceGetPan(const AudioSource* source)
{
    return ma_sound_get_pan(source->handle);
}

void alimerAudioSourceSetPanMode(AudioSource* source, AudioPanMode value)
{
    ma_sound_set_pan_mode(source->handle, ToMiniaudio(value));
}

AudioPanMode alimerAudioSourceGetPanMode(const AudioSource* source)
{
    return FromMiniaudio(ma_sound_get_pan_mode(source->handle));
}

void alimerAudioSourceSetPitch(AudioSource* source, float value)
{
    ma_sound_set_pitch(source->handle, value);
}

float alimerAudioSourceGetPitch(const AudioSource* source)
{
    return ma_sound_get_pitch(source->handle);
}

void alimerAudioSourceSetSpatializationEnabled(AudioSource* source, bool enabled)
{
    ma_sound_set_spatialization_enabled(source->handle, enabled ? MA_TRUE : MA_FALSE);
}

bool alimerAudioSourceIsSpatializationEnabled(const AudioSource* source)
{
    return ma_sound_is_spatialization_enabled(source->handle) == MA_TRUE;
}

bool alimerAudioSourceIsPlaying(AudioSource* source)
{
    return ma_sound_is_playing(source->handle) == MA_TRUE;
}

uint64_t alimerAudioSourceGetTimeInPCMFrames(AudioSource* source)
{
    return (uint64_t)ma_sound_get_time_in_pcm_frames(source->handle);
}

uint64_t alimerAudioSourceGetTimeInMilliseconds(AudioSource* source)
{
    return (uint64_t)ma_sound_get_time_in_milliseconds(source->handle);
}

void alimerAudioSourceSetLooping(AudioSource* source, bool looping)
{
    ma_sound_set_looping(source->handle, looping ? MA_TRUE : MA_FALSE);
}

bool alimerAudioSourceIsLooping(const AudioSource* source)
{
    return ma_sound_is_looping(source->handle) == MA_TRUE;
}

bool alimerAudioSourceIsAtEnd(const AudioSource* source)
{
    return ma_sound_at_end(source->handle) == MA_TRUE;
}

#endif /* defined(ALIMER_AUDIO) */
