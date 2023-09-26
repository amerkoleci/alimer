// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_IMAGE_H
#define _ALIMER_IMAGE_H

#if defined(ALIMER_IMAGE_SHARED_LIBRARY)
#   if defined(_WIN32) || defined(__CYGWIN__)
#       if defined(ALIMER_IMAGE_IMPLEMENTATION)
#           define _ALIMER_IMAGE_EXPORT __declspec(dllexport)
#       else
#           define _ALIMER_IMAGE_EXPORT __declspec(dllimport)
#       endif
#   define ALIMER_IMAGE_CALL __cdecl
#   else
#   define ALIMER_IMAGE_CALL
#       if defined(ALIMER_IMAGE_IMPLEMENTATION)
#           define _ALIMER_IMAGE_EXPORT __attribute__((visibility("default")))
#       else
#           define _ALIMER_IMAGE_EXPORT
#       endif
#   endif
#else
#   define ALIMER_IMAGE_CALL
#   define _ALIMER_IMAGE_EXPORT
#endif

#ifdef __cplusplus
#   define ALIMER_IMAGE_API extern "C" _ALIMER_IMAGE_EXPORT 
#else
#   define ALIMER_IMAGE_API extern _ALIMER_IMAGE_EXPORT 
#endif

#include <stdint.h>

typedef uint32_t Bool32;

/* Similar to WebGPU Format (WGPUTextureFormat) with some additions */
typedef enum ImageFormat {
    ImageFormat_Undefined = 0,
    // 8-bit formats
    ImageFormat_R8Unorm,
    ImageFormat_R8Snorm,
    ImageFormat_R8Uint,
    ImageFormat_R8Sint,
    // 16-bit formats
    ImageFormat_R16Unorm,
    ImageFormat_R16Snorm,
    ImageFormat_R16Uint,
    ImageFormat_R16Sint,
    ImageFormat_R16Float,
    ImageFormat_RG8Unorm,
    ImageFormat_RG8Snorm,
    ImageFormat_RG8Uint,
    ImageFormat_RG8Sint,
    // Packed 16-Bit formats
    ImageFormat_BGRA4Unorm,
    ImageFormat_B5G6R5Unorm,
    ImageFormat_BGR5A1Unorm,
    // 32-bit formats
    ImageFormat_R32Uint,
    ImageFormat_R32Sint,
    ImageFormat_R32Float,
    ImageFormat_RG16Unorm,
    ImageFormat_RG16Snorm,
    ImageFormat_RG16Uint,
    ImageFormat_RG16Sint,
    ImageFormat_RG16Float,
    ImageFormat_RGBA8Unorm,
    ImageFormat_RGBA8UnormSrgb,
    ImageFormat_RGBA8Snorm,
    ImageFormat_RGBA8Uint,
    ImageFormat_RGBA8Sint,
    ImageFormat_BGRA8Unorm,
    ImageFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    ImageFormat_RGB10A2Unorm,
    ImageFormat_RGB10A2Uint,
    ImageFormat_RG11B10Float,
    ImageFormat_RGB9E5Float,
    // 64-bit formats
    ImageFormat_RG32Uint,
    ImageFormat_RG32Sint,
    ImageFormat_RG32Float,
    ImageFormat_RGBA16Unorm,
    ImageFormat_RGBA16Snorm,
    ImageFormat_RGBA16Uint,
    ImageFormat_RGBA16Sint,
    ImageFormat_RGBA16Float,
    // 128-bit formats
    ImageFormat_RGBA32Uint,
    ImageFormat_RGBA32Sint,
    ImageFormat_RGBA32Float,
    // Depth-stencil formats
    ImageFormat_Depth16Unorm,
    ImageFormat_Depth24UnormStencil8,
    ImageFormat_Depth32Float,
    ImageFormat_Depth32FloatStencil8,
    // Bc compressed formats
    ImageFormat_BC1RGBAUnorm,
    ImageFormat_BC1RGBAUnormSrgb,
    ImageFormat_BC2RGBAUnorm,
    ImageFormat_BC2RGBAUnormSrgb,
    ImageFormat_BC3RGBAUnorm,
    ImageFormat_BC3RGBAUnormSrgb,
    ImageFormat_BC4RUnorm,
    ImageFormat_BC4RSnorm,
    ImageFormat_BC5RGUnorm,
    ImageFormat_BC5RGSnorm,
    ImageFormat_BC6HRGBUfloat,
    ImageFormat_BC6HRGBFloat,
    ImageFormat_BC7RGBAUnorm,
    ImageFormat_BC7RGBAUnormSrgb,
    // Etc2/Eac compressed formats
    ImageFormat_ETC2RGB8Unorm,
    ImageFormat_ETC2RGB8UnormSrgb,
    ImageFormat_ETC2RGB8A1Unorm,
    ImageFormat_ETC2RGB8A1UnormSrgb,
    ImageFormat_ETC2RGBA8Unorm,
    ImageFormat_ETC2RGBA8UnormSrgb,
    ImageFormat_EACR11Unorm,
    ImageFormat_EACR11Snorm,
    ImageFormat_EACRG11Unorm,
    ImageFormat_EACRG11Snorm,
    // Astc compressed formats
    ImageFormat_ASTC4x4Unorm ,
    ImageFormat_ASTC4x4UnormSrgb,
    ImageFormat_ASTC5x4Unorm,
    ImageFormat_ASTC5x4UnormSrgb,
    ImageFormat_ASTC5x5Unorm,
    ImageFormat_ASTC5x5UnormSrgb,
    ImageFormat_ASTC6x5Unorm,
    ImageFormat_ASTC6x5UnormSrgb,
    ImageFormat_ASTC6x6Unorm,
    ImageFormat_ASTC6x6UnormSrgb,
    ImageFormat_ASTC8x5Unorm,
    ImageFormat_ASTC8x5UnormSrgb,
    ImageFormat_ASTC8x6Unorm,
    ImageFormat_ASTC8x6UnormSrgb,
    ImageFormat_ASTC8x8Unorm,
    ImageFormat_ASTC8x8UnormSrgb,
    ImageFormat_ASTC10x5Unorm,
    ImageFormat_ASTC10x5UnormSrgb,
    ImageFormat_ASTC10x6Unorm,
    ImageFormat_ASTC10x6UnormSrgb,
    ImageFormat_ASTC10x8Unorm,
    ImageFormat_ASTC10x8UnormSrgb,
    ImageFormat_ASTC10x10Unorm,
    ImageFormat_ASTC10x10UnormSrgb,
    ImageFormat_ASTC12x10Unorm,
    ImageFormat_ASTC12x10UnormSrgb,
    ImageFormat_ASTC12x12Unorm,
    ImageFormat_ASTC12x12UnormSrgb,

    ImageFormat_Force32 = 0x7FFFFFFF
} ImageFormat;

typedef enum ImageDimension {
    ImageDimension_1D = 0,
    ImageDimension_2D = 1,
    ImageDimension_3D = 2,
    ImageDimension_Force32 = 0x7FFFFFFF
} ImageDimension;

typedef struct AlimerImage AlimerImage;

/* Callbacks */
typedef void (ALIMER_IMAGE_CALL* SaveCallback)(void* data, size_t size);

ALIMER_IMAGE_API AlimerImage* AlimerImageCreate2D(ImageFormat format, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t mipLevels);
ALIMER_IMAGE_API AlimerImage* alimerImageCreateFromMemory(const void* data, size_t size);
ALIMER_IMAGE_API void AlimerImageDestroy(AlimerImage* image);

ALIMER_IMAGE_API ImageDimension alimerImageGetDimension(AlimerImage* image);
ALIMER_IMAGE_API ImageFormat alimerImageGetFormat(AlimerImage* image);
ALIMER_IMAGE_API uint32_t alimerImageGetWidth(AlimerImage* image, uint32_t level);
ALIMER_IMAGE_API uint32_t alimerImageGetHeight(AlimerImage* image, uint32_t level);
ALIMER_IMAGE_API uint32_t alimerImageGetDepth(AlimerImage* image, uint32_t level);
ALIMER_IMAGE_API uint32_t alimerImageGetArraySize(AlimerImage* image);
ALIMER_IMAGE_API uint32_t alimerImageGetMipLevels(AlimerImage* image);
ALIMER_IMAGE_API Bool32 alimerImageIsCubemap(AlimerImage* image);

ALIMER_IMAGE_API size_t alimerImageGetDataSize(AlimerImage* image);
ALIMER_IMAGE_API void* alimerImageGetData(AlimerImage* image);

ALIMER_IMAGE_API Bool32 AlimerImageSavePngMemory(AlimerImage* image, SaveCallback callback);

#endif /* _ALIMER_IMAGE_H */
