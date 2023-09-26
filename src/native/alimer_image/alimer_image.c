// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_image.h"
#include <stdbool.h>
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <assert.h>

#define _min(a,b) (((a)<(b))?(a):(b))
#define _max(a,b) (((a)>(b))?(a):(b))

#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"
//#include <ktx.h>

typedef struct AlimerImage {
    ImageDimension  dimension;
    ImageFormat     format;
    uint32_t        width;
    uint32_t        height;
    uint32_t        depthOrArraySize;
    uint32_t        mipLevels;
    bool            isCubemap;

    size_t          dataSize;
    void* pData;
} AlimerImage;

static AlimerImage* dds_load_from_memory(const uint8_t* data, size_t size)
{
    return NULL;
}

static AlimerImage* astc_load_from_memory(const uint8_t* data, size_t size)
{
    return NULL;
}

static AlimerImage* ktx_load_from_memory(const uint8_t* data, size_t size)
{
    return NULL;

#if TODO_KTX
    ktxTexture* ktx_texture = 0;
    KTX_error_code ktx_result = ktxTexture_CreateFromMemory(
        data,
        size,
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        &ktx_texture
    );

    // Not ktx texture.
    if (ktx_result != KTX_SUCCESS)
    {
        return NULL;
    }

    ImageFormat format = IMAGE_FORMAT_RGBA8;
    if (ktx_texture->classId == ktxTexture2_c)
    {
        ktxTexture2* ktx_texture2 = (ktxTexture2*)ktx_texture;

        if (ktxTexture2_NeedsTranscoding(ktx_texture2))
        {
            // Once transcoded, the ktxTexture object contains the texture data in a native GPU format (e.g. BC7)
            ktx_result = ktxTexture2_TranscodeBasis(ktx_texture2, KTX_TTF_BC7_RGBA, 0);
            //result->format = TEXTURE_FORMAT_BC7;
        }
        else
        {
        }
    }

    Image* result = image_create_new(ktx_texture->baseWidth, ktx_texture->baseHeight, format);
    if (ktx_texture->baseDepth > 1)
    {
        result->dimension = IMAGE_DIMENSION_3D;
        result->depthOrArrayLayers = ktx_texture->baseDepth;
    }
    else
    {
        result->dimension = IMAGE_DIMENSION_2D;
        result->depthOrArrayLayers = ktx_texture->numLayers;
    }

    result->numLevels = ktx_texture->numLevels;
    result->numFaces = ktx_texture->numFaces;
    result->isArray = ktx_texture->isArray;
    result->isCubemap = ktx_texture->isCubemap;

    result->dataSize = (uint32_t)ktx_texture->dataSize;
    result->pData = malloc(ktx_texture->dataSize);
    memcpy(result->pData, ktx_texture->pData, ktx_texture->dataSize);
    ktxTexture_Destroy(ktx_texture);
    return result;
#endif // TODO_KTX

}

static AlimerImage* stb_load_from_memory(const uint8_t* data, size_t size)
{
    int width, height, channels;
    ImageFormat format = ImageFormat_RGBA8Unorm;
    void* image_data;
    uint32_t memorySize = 0;
    if (stbi_is_16_bit_from_memory(data, (int)size))
    {
        image_data = stbi_load_16_from_memory(data, (int)size, &width, &height, &channels, 0);
        switch (channels)
        {
            case 1:
                format = ImageFormat_R16Uint;
                memorySize = width * height * sizeof(uint16_t);
                break;
            case 2:
                format = ImageFormat_RG16Uint;
                memorySize = width * height * 2 * sizeof(uint16_t);
                break;
            case 4:
                format = ImageFormat_RGBA16Uint;
                memorySize = width * height * 4 * sizeof(uint16_t);
                break;
            default:
                assert(0);
        }
    }
    else if (stbi_is_hdr_from_memory(data, (int)size))
    {
        image_data = stbi_loadf_from_memory(data, (int)size, &width, &height, &channels, 4);
        format = ImageFormat_RGBA32Float;
        memorySize = width * height * 4 * sizeof(float);
    }
    else
    {
        image_data = stbi_load_from_memory(data, (int)size, &width, &height, &channels, 4);
        format = ImageFormat_RGBA8Unorm;
        memorySize = width * height * 4 * sizeof(uint8_t);
    }

    if (!image_data) {
        return NULL;
    }

    AlimerImage* result = AlimerImageCreate2D(format, width, height, 1, 1);
    result->dataSize = memorySize;
    result->pData = malloc(memorySize);
    memcpy(result->pData, image_data, memorySize);
    stbi_image_free(image_data);
    return result;
}

bool IsPow2(size_t x)
{
    return ((x != 0) && !(x & (x - 1)));
}


uint32_t CountMips(uint32_t width, uint32_t height)
{
    size_t mipLevels = 1;

    while (height > 1 || width > 1)
    {
        if (height > 1)
            height >>= 1;

        if (width > 1)
            width >>= 1;

        ++mipLevels;
    }

    return mipLevels;
}


bool CalculateMipLevels(uint32_t width, uint32_t height, uint32_t* mipLevels)
{
    if (*mipLevels > 1)
    {
        const uint32_t maxMips = CountMips(width, height);
        if (*mipLevels > maxMips)
            return false;
    }
    else if (*mipLevels == 0)
    {
        *mipLevels = CountMips(width, height);
    }
    else
    {
        *mipLevels = 1;
    }
    return true;
}

uint32_t BitsPerPixel(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat_RGBA32Uint:
        case ImageFormat_RGBA32Sint:
        case ImageFormat_RGBA32Float:
            return 128;

        case ImageFormat_RG32Uint:
        case ImageFormat_RG32Sint:
        case ImageFormat_RG32Float:
        case ImageFormat_RGBA16Unorm:
        case ImageFormat_RGBA16Snorm:
        case ImageFormat_RGBA16Uint:
        case ImageFormat_RGBA16Sint:
        case ImageFormat_RGBA16Float:
            return 64;

        case ImageFormat_R32Uint:
        case ImageFormat_R32Sint:
        case ImageFormat_R32Float:
        case ImageFormat_RG16Unorm:
        case ImageFormat_RG16Snorm:
        case ImageFormat_RG16Uint:
        case ImageFormat_RG16Sint:
        case ImageFormat_RG16Float:
        case ImageFormat_RGBA8Unorm:
        case ImageFormat_RGBA8UnormSrgb:
        case ImageFormat_RGBA8Snorm:
        case ImageFormat_RGBA8Uint:
        case ImageFormat_RGBA8Sint:
        case ImageFormat_BGRA8Unorm:
        case ImageFormat_BGRA8UnormSrgb:
        case ImageFormat_RGB10A2Unorm:
        case ImageFormat_RGB10A2Uint:
        case ImageFormat_RG11B10Float:
        case ImageFormat_RGB9E5Float:
            return 32;

        case ImageFormat_R16Unorm:
        case ImageFormat_R16Snorm:
        case ImageFormat_R16Uint:
        case ImageFormat_R16Sint:
        case ImageFormat_R16Float:
        case ImageFormat_RG8Unorm:
        case ImageFormat_RG8Snorm:
        case ImageFormat_RG8Uint:
        case ImageFormat_RG8Sint:
            // Packed 16-Bit formats
        case ImageFormat_BGRA4Unorm:
        case ImageFormat_B5G6R5Unorm:
        case ImageFormat_BGR5A1Unorm:
            return 16;

        case ImageFormat_R8Unorm:
        case ImageFormat_R8Snorm:
        case ImageFormat_R8Uint:
        case ImageFormat_R8Sint:
        case ImageFormat_BC2RGBAUnorm:
        case ImageFormat_BC2RGBAUnormSrgb:
        case ImageFormat_BC3RGBAUnorm:
        case ImageFormat_BC3RGBAUnormSrgb:
        case ImageFormat_BC5RGUnorm:
        case ImageFormat_BC5RGSnorm:
        case ImageFormat_BC6HRGBUfloat:
        case ImageFormat_BC6HRGBFloat:
        case ImageFormat_BC7RGBAUnorm:
        case ImageFormat_BC7RGBAUnormSrgb:
            return 8;

        case ImageFormat_BC1RGBAUnorm:
        case ImageFormat_BC1RGBAUnormSrgb:
        case ImageFormat_BC4RUnorm:
        case ImageFormat_BC4RSnorm:
            return 4;

        default:
            return 0;
    }
}

bool GetSurfaceInfo(ImageFormat format, uint32_t width, uint32_t height, uint32_t* pRowPitch, uint32_t* pSlicePitch, uint32_t* pRowCount)
{
    uint32_t rowPitch = 0;
    uint32_t slicePitch = 0;
    uint32_t rowCount = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;

    switch (format)
    {
        case ImageFormat_BC1RGBAUnorm:
        case ImageFormat_BC1RGBAUnormSrgb:
        case ImageFormat_BC4RUnorm:
        case ImageFormat_BC4RSnorm:
            bc = true;
            bpe = 8;
            break;

        case ImageFormat_BC2RGBAUnorm:
        case ImageFormat_BC2RGBAUnormSrgb:
        case ImageFormat_BC3RGBAUnorm:
        case ImageFormat_BC3RGBAUnormSrgb:
        case ImageFormat_BC5RGUnorm:
        case ImageFormat_BC5RGSnorm:
        case ImageFormat_BC6HRGBUfloat:
        case ImageFormat_BC6HRGBFloat:
        case ImageFormat_BC7RGBAUnorm:
        case ImageFormat_BC7RGBAUnormSrgb:
            bc = true;
            bpe = 16;
            break;

        default:
            break;
    }

    if (bc)
    {
        uint32_t numBlocksWide = 0;
        if (width > 0)
        {
            numBlocksWide = _max(1u, (width + 3u) / 4u);
        }
        uint32_t numBlocksHigh = 0;
        if (height > 0)
        {
            numBlocksHigh = _max(1u, (height + 3u) / 4u);
        }
        rowPitch = numBlocksWide * bpe;
        slicePitch = rowPitch * numBlocksHigh;
        rowCount = numBlocksHigh;
    }
    else if (packed)
    {
        rowPitch = ((width + 1) >> 1) * bpe;
        rowCount = height;
        slicePitch = rowPitch * height;
    }
    else if (planar)
    {
        rowPitch = ((width + 1) >> 1) * bpe;
        slicePitch = (rowPitch * height) + ((rowPitch * height + 1) >> 1);
        rowCount = height + ((height + 1u) >> 1);
    }
    else
    {
        uint32_t bpp = BitsPerPixel(format);
        rowPitch = (width * bpp + 7) / 8; // round up to nearest byte
        rowCount = height;
        slicePitch = rowPitch * height;
    }

    if (pRowPitch)
    {
        *pRowPitch = rowPitch;
    }

    if (pSlicePitch)
    {
        *pSlicePitch = slicePitch;
    }

    if (pRowCount)
    {
        *pRowCount = rowCount;
    }

    return true;
}

bool DetermineImageArray(AlimerImage* image)
{
    assert(image->width > 0 && image->height > 0 && image->depthOrArraySize > 0);
    assert(image->mipLevels > 0);

    size_t totalPixelSize = 0;
    size_t nimages = 0;

    switch (image->dimension)
    {
        case ImageDimension_1D:
        case ImageDimension_2D:
            for (uint32_t arrayIndex = 0; arrayIndex < image->depthOrArraySize; ++arrayIndex)
            {
                uint32_t w = image->width;
                uint32_t h = image->height;

                for (uint32_t mipLevel = 0; mipLevel < image->mipLevels; ++mipLevel)
                {
                    uint32_t rowPitch, slicePitch;
                    if (!GetSurfaceInfo(image->format, w, h, &rowPitch, &slicePitch, NULL))
                    {
                        return false;
                    }

                    totalPixelSize += slicePitch;
                    ++nimages;

                    if (h > 1)
                        h >>= 1;

                    if (w > 1)
                        w >>= 1;
                }
            }
            break;

        case ImageDimension_3D:
        {
            size_t w = image->width;
            size_t h = image->height;
            size_t d = image->depthOrArraySize;

            for (uint32_t mipLevel = 0; mipLevel < image->mipLevels; ++mipLevel)
            {
                uint32_t rowPitch, slicePitch;
                if (!GetSurfaceInfo(image->format, w, h, &rowPitch, &slicePitch, NULL))
                {
                    return false;
                }

                for (size_t slice = 0; slice < d; ++slice)
                {
                    totalPixelSize += slicePitch;
                    ++nimages;
                }

                if (h > 1)
                    h >>= 1;

                if (w > 1)
                    w >>= 1;

                if (d > 1)
                    d >>= 1;
            }
        }
        break;

        default:
            return false;
    }

    //nImages = nimages;
    image->dataSize = totalPixelSize;

    return true;
}

AlimerImage* AlimerImageCreate2D(ImageFormat format, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t mipLevels)
{
    if (format == ImageFormat_Undefined || !width || !height || !arraySize)
        return NULL;

    if (!CalculateMipLevels(width, height, &mipLevels))
        return NULL;

    AlimerImage* image = (AlimerImage*)malloc(sizeof(AlimerImage));
    assert(image);
    memset(image, 0, sizeof(AlimerImage));

    image->dimension = ImageDimension_2D;
    image->format = format;
    image->width = width;
    image->height = height;
    image->depthOrArraySize = arraySize;
    image->mipLevels = mipLevels;
    image->isCubemap = false;
    if (!DetermineImageArray(image))
    {
        free(image);
        return NULL;
    }

    return image;
}

AlimerImage* alimerImageCreateFromMemory(const void* data, size_t size)
{
    AlimerImage* image = NULL;

    if ((image = dds_load_from_memory(data, size)) != NULL) {
        return image;
    }

    if ((image = astc_load_from_memory(data, size)) != NULL) {
        return image;
    }

    if ((image = ktx_load_from_memory(data, size)) != NULL) {
        return image;
    }

    if ((image = stb_load_from_memory(data, size)) != NULL) {
        return image;
    }

    return NULL;
}

void AlimerImageDestroy(AlimerImage* image)
{
    if (!image)
        return;

    if (image->pData) {
        free(image->pData);
    }

    free(image);
}

ImageDimension alimerImageGetDimension(AlimerImage* image)
{
    return image->dimension;
}

ImageFormat alimerImageGetFormat(AlimerImage* image)
{
    return image->format;
}

uint32_t alimerImageGetWidth(AlimerImage* image, uint32_t level)
{
    return _max(image->width >> level, 1);
}

uint32_t alimerImageGetHeight(AlimerImage* image, uint32_t level)
{
    return _max(image->height >> level, 1);
}

uint32_t alimerImageGetDepth(AlimerImage* image, uint32_t level)
{
    if (image->dimension != ImageDimension_3D) {
        return 1u;
    }

    return _max(image->depthOrArraySize >> level, 1);
}

uint32_t alimerImageGetArraySize(AlimerImage* image)
{
    if (image->dimension == ImageDimension_3D) {
        return 1u;
    }

    return image->depthOrArraySize;
}

uint32_t alimerImageGetMipLevels(AlimerImage* image)
{
    return image->mipLevels;
}

Bool32 alimerImageIsCubemap(AlimerImage* image)
{
    return image->isCubemap;
}

size_t alimerImageGetDataSize(AlimerImage* image)
{
    return image->dataSize;
}

void* alimerImageGetData(AlimerImage* image)
{
    return image->pData;
}

Bool32 AlimerImageSavePngMemory(AlimerImage* image, SaveCallback callback)
{
    int len;
    unsigned char* data = stbi_write_png_to_mem(image->pData, image->width * 4, image->width, image->height, 4, &len);
    if (data == NULL)
        return false;

    callback(data, len);
    STBIW_FREE(data);
    return true;
}
