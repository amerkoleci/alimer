// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_AUDIO)
#include "alimer_internal.h"
#include "alimer_audio.h"

ALIMER_DISABLE_WARNINGS()
#include "miniaudio_ex.h"
ALIMER_ENABLE_WARNINGS()

#include <mutex>
#include <atomic>

namespace
{
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

    constexpr AudioAttenuationModel FromMiniaudio(ma_attenuation_model value)
    {
        switch (value)
        {
            case ma_attenuation_model_none: return AudioAttenuationModel_None;
            case ma_attenuation_model_inverse: return AudioAttenuationModel_Inverse;
            case ma_attenuation_model_linear: return AudioAttenuationModel_Linear;
            case ma_attenuation_model_exponential: return AudioAttenuationModel_Exponential;

            default:
                ALIMER_UNREACHABLE();
                return AudioAttenuationModel_None;
        }
    }

    constexpr AudioPositioning FromMiniaudio(ma_positioning value)
    {
        switch (value)
        {
            case ma_positioning_absolute: return AudioPositioning_Absolute;
            case ma_positioning_relative: return AudioPositioning_Relative;
            default:
                ALIMER_UNREACHABLE();
                return AudioPositioning_Absolute;
        }
    }

    static void FromMiniaudio(const ma_vec3f& value, float result[3])
    {
        ALIMER_ASSERT(result);

        result[0] = value.x;
        result[1] = value.y;
        result[2] = value.z;
    }

    constexpr ma_vec3f ToMiniaudio(const float value[3])
    {
        return ma_vec3f{ value[0], value[1], value[2] };
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

    constexpr ma_attenuation_model ToMiniaudio(AudioAttenuationModel value)
    {
        switch (value)
        {
            case AudioAttenuationModel_None: return ma_attenuation_model_none;
            case AudioAttenuationModel_Inverse: return ma_attenuation_model_inverse;
            case AudioAttenuationModel_Linear: return ma_attenuation_model_linear;
            case AudioAttenuationModel_Exponential: return ma_attenuation_model_exponential;

            default:
                ALIMER_UNREACHABLE();
                return ma_attenuation_model_none;
        }
    }

    constexpr ma_positioning ToMiniaudio(AudioPositioning value)
    {
        switch (value)
        {
            case AudioPositioning_Relative: return ma_positioning_relative;
            case AudioPositioning_Absolute: return ma_positioning_absolute;
            default:
                ALIMER_UNREACHABLE();
                return ma_positioning_relative;
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

AudioEngine* alimerAudioEngineCreate(const AudioEngineConfig* config)
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

    ma_result result = ma_device_init(nullptr, &deviceConfig, &engine->device);
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

static bool alimerAudioClipInitFromDecoder(AudioClip* clip)
{
    ma_format format = ma_format_unknown;
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

void alimerAudioClipAddRef(AudioClip* clip)
{
    ++clip->refCount;
}

void alimerAudioClipRelease(AudioClip* clip)
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

#endif /* defined(ALIMER_AUDIO) */
