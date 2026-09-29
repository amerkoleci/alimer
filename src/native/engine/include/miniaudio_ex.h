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

    /* Context */
    MA_API ma_result ma_ex_context_init_default(ma_context* pContext);

#if defined(__cplusplus)
}
#endif

#endif  /* MINIAUDIOEX_H_ */
