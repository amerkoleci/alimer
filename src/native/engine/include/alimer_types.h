// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_TYPES_H_
#define ALIMER_TYPES_H_ 1

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __declspec(dllexport)
#        else
#            define _ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define _ALIMER_EXPORT
#        endif
#    endif
#else
#    define _ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_API _ALIMER_EXTERN _ALIMER_EXPORT

#if !defined(ALIMER_NULLABLE)
#define ALIMER_NULLABLE
#endif

#ifdef __cplusplus
#   define DEFAULT_INITIALIZER(x) = x
#else
#   define DEFAULT_INITIALIZER(x)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Types */
typedef uint32_t Flags;
typedef uint32_t Bool32;

/* Enums */
typedef enum PixelFormat {
    PixelFormat_Undefined = 0,
    // 8-bit formats
    PixelFormat_R8Unorm,
    PixelFormat_R8Snorm,
    PixelFormat_R8Uint,
    PixelFormat_R8Sint,
    // 16-bit formats
    PixelFormat_R16Unorm,
    PixelFormat_R16Snorm,
    PixelFormat_R16Uint,
    PixelFormat_R16Sint,
    PixelFormat_R16Float,
    PixelFormat_RG8Unorm,
    PixelFormat_RG8Snorm,
    PixelFormat_RG8Uint,
    PixelFormat_RG8Sint,
    // Packed 16-Bit formats
    PixelFormat_B5G6R5Unorm,
    PixelFormat_BGR5A1Unorm,
    PixelFormat_BGRA4Unorm,
    // 32-bit formats
    PixelFormat_R32Uint,
    PixelFormat_R32Sint,
    PixelFormat_R32Float,
    PixelFormat_RG16Unorm,
    PixelFormat_RG16Snorm,
    PixelFormat_RG16Uint,
    PixelFormat_RG16Sint,
    PixelFormat_RG16Float,
    PixelFormat_RGBA8Unorm,
    PixelFormat_RGBA8UnormSrgb,
    PixelFormat_RGBA8Snorm,
    PixelFormat_RGBA8Uint,
    PixelFormat_RGBA8Sint,
    PixelFormat_BGRA8Unorm,
    PixelFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    PixelFormat_RGB10A2Unorm,
    PixelFormat_RGB10A2Uint,
    PixelFormat_RG11B10Ufloat,
    PixelFormat_RGB9E5Ufloat,
    // 64-bit formats
    PixelFormat_RG32Uint,
    PixelFormat_RG32Sint,
    PixelFormat_RG32Float,
    PixelFormat_RGBA16Unorm,
    PixelFormat_RGBA16Snorm,
    PixelFormat_RGBA16Uint,
    PixelFormat_RGBA16Sint,
    PixelFormat_RGBA16Float,
    // 128-bit formats
    PixelFormat_RGBA32Uint,
    PixelFormat_RGBA32Sint,
    PixelFormat_RGBA32Float,
    // Depth-stencil formats
    PixelFormat_Depth16Unorm,
    PixelFormat_Depth24UnormStencil8,
    PixelFormat_Depth32Float,
    PixelFormat_Depth32FloatStencil8,
    // BC compressed formats
    PixelFormat_BC1RGBAUnorm,
    PixelFormat_BC1RGBAUnormSrgb,
    PixelFormat_BC2RGBAUnorm,
    PixelFormat_BC2RGBAUnormSrgb,
    PixelFormat_BC3RGBAUnorm,
    PixelFormat_BC3RGBAUnormSrgb,
    PixelFormat_BC4RUnorm,
    PixelFormat_BC4RSnorm,
    PixelFormat_BC5RGUnorm,
    PixelFormat_BC5RGSnorm,
    PixelFormat_BC6HRGBUfloat,
    PixelFormat_BC6HRGBFloat,
    PixelFormat_BC7RGBAUnorm,
    PixelFormat_BC7RGBAUnormSrgb,
    // ETC2/EAC compressed formats
    PixelFormat_ETC2RGB8Unorm,
    PixelFormat_ETC2RGB8UnormSrgb,
    PixelFormat_ETC2RGB8A1Unorm,
    PixelFormat_ETC2RGB8A1UnormSrgb,
    PixelFormat_ETC2RGBA8Unorm,
    PixelFormat_ETC2RGBA8UnormSrgb,
    PixelFormat_EACR11Unorm,
    PixelFormat_EACR11Snorm,
    PixelFormat_EACRG11Unorm,
    PixelFormat_EACRG11Snorm,
    // ASTC compressed formats
    PixelFormat_ASTC4x4Unorm,
    PixelFormat_ASTC4x4UnormSrgb,
    PixelFormat_ASTC5x4Unorm,
    PixelFormat_ASTC5x4UnormSrgb,
    PixelFormat_ASTC5x5Unorm,
    PixelFormat_ASTC5x5UnormSrgb,
    PixelFormat_ASTC6x5Unorm,
    PixelFormat_ASTC6x5UnormSrgb,
    PixelFormat_ASTC6x6Unorm,
    PixelFormat_ASTC6x6UnormSrgb,
    PixelFormat_ASTC8x5Unorm,
    PixelFormat_ASTC8x5UnormSrgb,
    PixelFormat_ASTC8x6Unorm,
    PixelFormat_ASTC8x6UnormSrgb,
    PixelFormat_ASTC8x8Unorm,
    PixelFormat_ASTC8x8UnormSrgb,
    PixelFormat_ASTC10x5Unorm,
    PixelFormat_ASTC10x5UnormSrgb,
    PixelFormat_ASTC10x6Unorm,
    PixelFormat_ASTC10x6UnormSrgb,
    PixelFormat_ASTC10x8Unorm,
    PixelFormat_ASTC10x8UnormSrgb,
    PixelFormat_ASTC10x10Unorm,
    PixelFormat_ASTC10x10UnormSrgb,
    PixelFormat_ASTC12x10Unorm,
    PixelFormat_ASTC12x10UnormSrgb,
    PixelFormat_ASTC12x12Unorm,
    PixelFormat_ASTC12x12UnormSrgb,
    // ASTC HDR compressed formats
    PixelFormat_ASTC4x4HDR,
    PixelFormat_ASTC5x4HDR,
    PixelFormat_ASTC5x5HDR,
    PixelFormat_ASTC6x5HDR,
    PixelFormat_ASTC6x6HDR,
    PixelFormat_ASTC8x5HDR,
    PixelFormat_ASTC8x6HDR,
    PixelFormat_ASTC8x8HDR,
    PixelFormat_ASTC10x5HDR,
    PixelFormat_ASTC10x6HDR,
    PixelFormat_ASTC10x8HDR,
    PixelFormat_ASTC10x10HDR,
    PixelFormat_ASTC12x10HDR,
    PixelFormat_ASTC12x12HDR,

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _PixelFormat_Count,
    _PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

/* Common types */
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Blob {
    uint32_t ref;
    void* data;
    size_t size;
    char* name;
} Blob;

#endif /* ALIMER_TYPES_H_ */
