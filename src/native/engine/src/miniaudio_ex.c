// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"

#define STB_VORBIS_HEADER_ONLY 
#include "third_party/stb_vorbis.c"

ALIMER_DISABLE_WARNINGS()
#define MA_DLL
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_COREAUDIO
#define MA_ENABLE_OPENSL
#define MA_ENABLE_WEBAUDIO
//#define MA_NO_DECODING
//#define MA_NO_ENCODING
//#define MA_NO_RESOURCE_MANAGER
//#define MA_NO_GENERATION
//#define MA_NO_NODE_GRAPH
//#define MA_NO_ENGINE
#include "miniaudio_ex.h"

#undef STB_VORBIS_HEADER_ONLY 
#include "third_party/stb_vorbis.c"
ALIMER_ENABLE_WARNINGS()

size_t ma_device_id_sizeof(void)
{
    return sizeof(ma_device_id);
}

size_t ma_device_info_sizeof(void)
{
    return sizeof(ma_device_info);
}

size_t ma_device_sizeof(void)
{
    return sizeof(ma_device);
}

size_t ma_engine_sizeof(void)
{
    return sizeof(ma_engine);
}

size_t ma_sound_sizeof(void)
{
    return sizeof(ma_sound);
}

size_t ma_sound_group_sizeof(void)
{
    return sizeof(ma_sound_group);
}

size_t ma_decoder_sizeof(void)
{
    return sizeof(ma_decoder);
}

ma_result ma_ex_context_init_default(ma_context* pContext)
{
    ma_context_config config = ma_context_config_init();
    return ma_context_init(NULL, 0, &config, pContext);
}

ma_result ma_ex_device_init_default(ma_context* pContext, ma_device_type deviceType, ma_device* pDevice)
{
    ma_device_config config = ma_device_config_init(deviceType);
    return ma_device_init(pContext, &config, pDevice);
}

ma_result ma_ex_engine_init_default(ma_engine* pEngine)
{
    ma_engine_config config = ma_engine_config_init();
    return ma_engine_init(&config, pEngine);
}

static void DataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_engine* engine = (ma_engine*)pDevice->pUserData;

    if (engine->pResourceManager)
    {
        if ((engine->pResourceManager->config.flags & MA_RESOURCE_MANAGER_FLAG_NO_THREADING) != 0)
        {
            ma_resource_manager_process_next_job(engine->pResourceManager);
        }
    }

    ma_engine_read_pcm_frames(engine, pOutput, frameCount, NULL);
}

ma_result ma_ex_engine_init_with_config(ma_context* pContext, const ma_engine_ex_config* config, ma_engine* pEngine)
{
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    if (config && config->playbackDeviceID)
    {
        deviceConfig.playback.pDeviceID = config->playbackDeviceID;
    }
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = (config != NULL && config->channelCount > 0) ? config->channelCount : 2;
    deviceConfig.sampleRate = (config != NULL && config->sampleRate > 0) ? config->sampleRate : 48000;
    deviceConfig.dataCallback = DataCallback;
    deviceConfig.pUserData = pEngine;

    ma_device* device = (ma_device*)ma_malloc(sizeof(ma_device), &pEngine->allocationCallbacks);
    ma_result result = ma_device_init(pContext, &deviceConfig, device);
    if (result != MA_SUCCESS)
    {
        ma_free(device, &pEngine->allocationCallbacks);
        return result;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = device;
    engineConfig.pProcessUserData = pEngine;
    engineConfig.listenerCount = 1;
    return ma_engine_init(&engineConfig, pEngine);
}
