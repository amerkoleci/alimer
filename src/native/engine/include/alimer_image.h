// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_IMAGE_H_
#define ALIMER_IMAGE_H_ 1

#include "alimer.h"

/* Forward */
typedef struct Image Image;

/* Enums */
typedef enum ImageType
{
    ImageType2D = 0,
    ImageType1D,
    ImageType3D,
    ImageTypeCube,

    _ImageType_Force32 = 0x7FFFFFFF
} ImageType;

typedef enum ImageFileType
{
    ImageFileType_Unknown = 0,
    ImageFileType_BMP,
    ImageFileType_PNG,
    ImageFileType_JPEG,
    ImageFileType_EXR,
    ImageFileType_DDS,
    ImageFileType_KTX1,
    ImageFileType_KTX2,
} ImageFileType;

/* Structs */
typedef struct ImageLevel {
    uint32_t      width;
    uint32_t      height;
    PixelFormat   format;
    uint32_t      rowPitch;
    uint32_t      slicePitch;
    uint8_t* pixels;
} ImageLevel;

typedef struct ImageDesc {
    ImageType           type;
    PixelFormat         format;
    uint32_t            width;
    uint32_t            height;
    uint32_t            depthOrArrayLayers;
    uint32_t            mipLevelCount;
} ImageDesc;

ALIMER_API Image* alimerImageCreate1D(PixelFormat format, uint32_t width, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreate2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreate3D(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreateCube(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);

ALIMER_API ImageFileType alimerImageDetectFileType(const void* pData, size_t dataSize);
ALIMER_API Image* alimerImageCreateFromMemory(const uint8_t* pData, size_t dataSize);
ALIMER_API void alimerImageDestroy(Image* image);

ALIMER_API void alimerImageGetDesc(Image* image, ImageDesc* pDesc);
ALIMER_API ImageType alimerImageGetType(Image* image);
ALIMER_API PixelFormat alimerImageGetFormat(Image* image);
ALIMER_API uint32_t alimerImageGetWidth(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetHeight(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetDepth(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetArrayLayers(Image* image);
ALIMER_API uint32_t alimerImageGetMipLevelCount(Image* image);
ALIMER_API uint8_t* alimerImageGetPixels(Image* image, size_t* pixelsSize);
ALIMER_API ImageLevel* alimerImageGetLevel(Image* image, uint32_t mipLevel, uint32_t arrayOrDepthSlice /* = 0*/);

/// Save in JPG format to file with specified quality. Return true if successful.
ALIMER_API Blob* alimerImageEncodeJPG(Image* image, int quality);

#endif /* ALIMER_IMAGE_H_ */
