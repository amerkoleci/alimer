// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef MINIAUDIOEX_H_
#define MINIAUDIOEX_H_ 1

#include "miniaudio.h"

#if defined(__cplusplus)
extern "C" {
#endif

    MA_API size_t ma_device_id_sizeof(void);
    MA_API size_t ma_device_info_sizeof(void);
    MA_API size_t ma_device_sizeof(void);
    MA_API size_t ma_engine_sizeof(void);
    MA_API size_t ma_sound_sizeof(void);
    MA_API size_t ma_sound_group_sizeof(void);
    MA_API size_t ma_decoder_sizeof(void);

    /* context */
    MA_API ma_result ma_ex_context_init_default(ma_context* pContext);

    /* device */
    MA_API ma_result ma_ex_device_init_default(ma_context* pContext, ma_device_type deviceType, ma_device* pDevice);

    /* engine */
    typedef struct ma_engine_ex_config {
        const ma_device_id* playbackDeviceID;
        /// Audio output channel count.
        ma_uint32 channelCount;
        /// Audio output sample rate.
        ma_uint32 sampleRate;
    } ma_engine_ex_config;

    MA_API ma_result ma_ex_engine_init_default(ma_engine* pEngine);
    MA_API ma_result ma_ex_engine_init_with_config(ma_context* pContext, const ma_engine_ex_config* config, ma_engine* pEngine);

#if defined(__cplusplus)
}
#endif

#endif  /* MINIAUDIOEX_H_ */
