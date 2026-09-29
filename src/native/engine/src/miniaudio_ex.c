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
