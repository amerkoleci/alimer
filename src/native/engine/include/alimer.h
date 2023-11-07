// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_H
#define ALIMER_H

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define ALIMER_EXPORT __declspec(dllexport)
#        else
#            define ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define ALIMER_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined(ALIMER_SHARED_LIBRARY)
#    define ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_API _ALIMER_EXTERN ALIMER_EXPORT 

/* Defines */
#define ALIMER_VERSION_MAJOR	1
#define ALIMER_VERSION_MINOR	0
#define ALIMER_VERSION_PATCH	0

#include <stddef.h>
#include <stdint.h>

/* Types */
typedef uint32_t Flags;
typedef uint32_t Bool32;
typedef struct AlimerImage AlimerImage;

/* Enums */
typedef enum AlimerLogLevel {
    AlimerLogLevel_Trace = 0,
    AlimerLogLevel_Debug = 1,
    AlimerLogLevel_Info = 2,
    AlimerLogLevel_Warn = 3,
    AlimerLogLevel_Error = 4,
    AlimerLogLevel_Critical = 5,
    AlimerLogLevel_Off = 6,

    _AlimerLogLevel_Force32 = 0x7FFFFFFF
} AlimerLogLevel;

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
    PixelFormat_BGRA4Unorm,
    PixelFormat_B5G6R5Unorm,
    PixelFormat_BGR5A1Unorm,
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
    PixelFormat_RG11B10UFloat,
    PixelFormat_RGB9E5UFloat,
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

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _PixelFormat_Count,
    _PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum TextureDimension
{
    /// One-dimensional Texture.
    TextureDimension_1D,
    /// Two-dimensional Texture.
    TextureDimension_2D,
    /// Three-dimensional Texture.
    TextureDimension_3D,
    /// Cubemap Texture.
    TextureDimension_Cube,

    _TextureDimension_Count,
    _TextureDimension_Force32 = 0x7FFFFFFF
} TextureDimension;

/* Methods*/
ALIMER_API void Alimer_GetVersion(int* major, int* minor, int* patch);

typedef void (*AlimerLogCallback)(AlimerLogLevel level, const char* message, void* userData);

ALIMER_API AlimerLogLevel AlimerGetLogLevel(void);
ALIMER_API void AlimerSetLogLevel(AlimerLogLevel level);
ALIMER_API void AlimerSetLogCallback(AlimerLogCallback callback, void* userData);

/* Image methods */
ALIMER_API AlimerImage* AlimerImage_Create2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevels);
ALIMER_API AlimerImage* AlimerImage_CreateFromMemory(const void* data, size_t size);
ALIMER_API void AlimerImage_Destroy(AlimerImage* image);

ALIMER_API TextureDimension AlimerImage_GetDimension(AlimerImage* image);
ALIMER_API PixelFormat AlimerImage_GetFormat(AlimerImage* image);
ALIMER_API uint32_t AlimerImage_GetWidth(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetHeight(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetDepth(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetArrayLayers(AlimerImage* image);
ALIMER_API uint32_t AlimerImage_GetMipLevels(AlimerImage* image);
ALIMER_API Bool32 AlimerImage_ImageIsCubemap(AlimerImage* image);

ALIMER_API void* AlimerImage_GetData(AlimerImage* image, size_t* size);

typedef void (*AlimerImageSaveCallback)(AlimerImage* image, void* pData, uint32_t dataSize);

ALIMER_API Bool32 AlimerImage_SaveBmp(AlimerImage* image, AlimerImageSaveCallback callback);
ALIMER_API Bool32 AlimerImage_SavePng(AlimerImage* image, AlimerImageSaveCallback callback);
ALIMER_API Bool32 AlimerImage_SaveJpg(AlimerImage* image, int quality, AlimerImageSaveCallback callback);
ALIMER_API Bool32 AlimerImage_SaveTga(AlimerImage* image, AlimerImageSaveCallback callback);
ALIMER_API Bool32 AlimerImage_SaveHdr(AlimerImage* image, AlimerImageSaveCallback callback);

#endif /* ALIMER_H */
