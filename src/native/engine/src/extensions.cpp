// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include <stdio.h>

ALIMER_DISABLE_WARNINGS()

#define STBIDEF ALIMER_API
#define STBI_ASSERT(x) ALIMER_ASSERT(x)
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBIWDEF ALIMER_API
#define STBIW_ASSERT(x) ALIMER_ASSERT(x)
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include "third_party/tinyexr.h"

#define STBTT_DEF ALIMER_API
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

ALIMER_ENABLE_WARNINGS()

ALIMER_API void stbi_free(void* ptr)
{
    STBI_FREE(ptr);
}

ALIMER_API void stbi_write_free(void* ptr)
{
    STBIW_FREE(ptr);
}

ALIMER_API int alimerIsEXRFromMemory(const uint8_t* pData, size_t dataSize) // TINYEXR_SUCCESS
{
    return IsEXRFromMemory(pData, dataSize);
}

ALIMER_API int alimerLoadEXRFromMemory(float** out_rgba, int* width, int* height, const uint8_t* memory, size_t size, const char** err)
{
    return LoadEXRFromMemory(out_rgba, width, height, memory, size, err);
}

ALIMER_API void alimerFreeEXRErrorMessage(const char* msg)
{
    FreeEXRErrorMessage(msg);
}
