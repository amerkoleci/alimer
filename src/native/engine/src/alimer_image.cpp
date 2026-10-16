// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer_image.h"
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

ALIMER_ENABLE_WARNINGS()

struct Image final
{
    ImageDesc desc;
    /// Image levels count.
    uint32_t levelsCount;
    /// Image mip levels.
    ImageLevel* levels;
    /// Image pixel data.
    uint8_t* pixels;
    /// Image pixel memory size.
    size_t pixelsSize;
};


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

namespace
{
    static uint32_t CountMips(uint32_t width, uint32_t height) noexcept
    {
        uint32_t mipLevels = 1;

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

    static uint32_t CountMips3D(uint32_t width, uint32_t height, uint32_t depth) noexcept
    {
        uint32_t mipLevels = 1;

        while (height > 1 || width > 1 || depth > 1)
        {
            if (height > 1)
                height >>= 1;

            if (width > 1)
                width >>= 1;

            if (depth > 1)
                depth >>= 1;

            ++mipLevels;
        }

        return mipLevels;
    }

    static bool CalculateMipLevels(uint32_t width, uint32_t height, uint32_t& mipLevels) noexcept
    {
        if (mipLevels > 1)
        {
            const uint32_t maxMips = CountMips(width, height);
            if (mipLevels > maxMips)
                return false;
        }
        else if (mipLevels == 0)
        {
            mipLevels = CountMips(width, height);
        }
        else
        {
            mipLevels = 1;
        }

        return true;
    }

    static bool CalculateMipLevels3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t& mipLevels) noexcept
    {
        if (mipLevels > 1)
        {
            const uint32_t maxMips = CountMips3D(width, height, depth);
            if (mipLevels > maxMips)
                return false;
        }
        else if (mipLevels == 0)
        {
            mipLevels = CountMips3D(width, height, depth);
        }
        else
        {
            mipLevels = 1;
        }
        return true;
    }

    static bool DetermineImageArray(Image* image)
    {
        image->pixelsSize = 0;
        image->levelsCount = 0;

        switch (image->desc.type)
        {
            case ImageType1D:
            case ImageType2D:
            case ImageTypeCube:
                for (uint32_t item = 0; item < image->desc.depthOrArrayLayers; ++item)
                {
                    uint32_t mipWidth = image->desc.width;
                    uint32_t mipHeight = image->desc.height;

                    for (uint32_t level = 0; level < image->desc.mipLevelCount; ++level)
                    {
                        uint32_t rowPitch, slicePitch, widthCount, heightCount;
                        alimerGetSurfaceInfo(image->desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                        image->pixelsSize += slicePitch;
                        ++image->levelsCount;

                        if (mipHeight > 1)
                            mipHeight >>= 1;

                        if (mipWidth > 1)
                            mipWidth >>= 1;
                    }
                }
                break;

            case ImageType3D:
            {
                uint32_t mipWidth = image->desc.width;
                uint32_t mipHeight = image->desc.height;
                uint32_t mipDepth = image->desc.depthOrArrayLayers;

                for (uint32_t level = 0; level < image->desc.mipLevelCount; ++level)
                {
                    uint32_t rowPitch, slicePitch, widthCount, heightCount;
                    alimerGetSurfaceInfo(image->desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                    for (uint32_t slice = 0; slice < mipDepth; ++slice)
                    {
                        image->pixelsSize += slicePitch;
                        ++image->levelsCount;
                    }

                    if (mipHeight > 1)
                        mipHeight >>= 1;

                    if (mipWidth > 1)
                        mipWidth >>= 1;

                    if (mipDepth > 1)
                        mipDepth >>= 1;
                }
            }
            break;

            default:
                return false;
        }

        return true;
    }

    static bool SetupImageArray(Image* image) noexcept
    {
        ALIMER_ASSERT(image);
        ALIMER_ASSERT(image->pixels);
        ALIMER_ASSERT(image->pixelsSize > 0);
        ALIMER_ASSERT(image->levelsCount > 0);

        if (!image->levels)
            return false;

        size_t index = 0;
        uint8_t* pixels = image->pixels;
        const uint8_t* pEndBits = image->pixels + image->pixelsSize;
        //size_t offset = 0;

        const ImageDesc& desc = image->desc;
        switch (desc.type)
        {
            case ImageType1D:
            case ImageType2D:
            case ImageTypeCube:
                if (desc.depthOrArrayLayers == 0 || desc.mipLevelCount == 0)
                {
                    return false;
                }

                for (uint32_t arrayIndex = 0; arrayIndex < desc.depthOrArrayLayers; ++arrayIndex)
                {
                    uint32_t mipWidth = desc.width;
                    uint32_t mipHeight = desc.height;

                    for (uint32_t level = 0; level < desc.mipLevelCount; ++level)
                    {
                        if (index >= image->levelsCount)
                        {
                            return false;
                        }

                        uint32_t rowPitch, slicePitch, widthCount, heightCount;
                        alimerGetSurfaceInfo(desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                        image->levels[index].width = mipWidth;
                        image->levels[index].height = mipHeight;
                        image->levels[index].format = desc.format;
                        image->levels[index].rowPitch = rowPitch;
                        image->levels[index].slicePitch = slicePitch;
                        image->levels[index].pixels = pixels;
                        ++index;

                        //offset += slicePitch;
                        pixels += slicePitch;
                        if (pixels > pEndBits)
                        {
                            return false;
                        }

                        if (mipWidth > 1)
                            mipWidth >>= 1;

                        if (mipHeight > 1)
                            mipHeight >>= 1;
                    }
                }
                return true;

            case ImageType3D:
            {
                if (desc.mipLevelCount == 0 || desc.depthOrArrayLayers == 0)
                {
                    return false;
                }

                uint32_t mipWidth = desc.width;
                uint32_t mipHeight = desc.height;
                uint32_t mipDepth = desc.depthOrArrayLayers;

                for (uint32_t level = 0; level < desc.mipLevelCount; ++level)
                {
                    uint32_t rowPitch, slicePitch, widthCount, heightCount;
                    alimerGetSurfaceInfo(desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                    for (uint32_t slice = 0; slice < mipDepth; ++slice)
                    {
                        if (index >= image->levelsCount)
                        {
                            return false;
                        }

                        // We use the same memory organization that Direct3D 11 needs for D3D11_SUBRESOURCE_DATA
                        // with all slices of a given miplevel being continuous in memory
                        image->levels[index].width = mipWidth;
                        image->levels[index].height = mipHeight;
                        image->levels[index].format = desc.format;
                        image->levels[index].rowPitch = rowPitch;
                        image->levels[index].slicePitch = slicePitch;
                        image->levels[index].pixels = pixels;
                        ++index;

                        pixels += slicePitch;
                        if (pixels > pEndBits)
                        {
                            return false;
                        }
                    }

                    if (mipWidth > 1)
                        mipWidth >>= 1;

                    if (mipHeight > 1)
                        mipHeight >>= 1;

                    if (mipDepth > 1)
                        mipDepth >>= 1;
                }
            }
            return true;

            default:
                return false;
        }
    }

    static bool InitializeImage(Image* image)
    {
        if (!DetermineImageArray(image))
            return false;

        image->levels = (ImageLevel*)alimerCalloc(image->levelsCount, sizeof(ImageLevel));
        if (!image->levels)
            return false;

        image->pixels = (uint8_t*)alimerMalloc(image->pixelsSize);
        //pixels = static_cast<uint8_t*>(NativeMemory::AlignedAlloc(_memorySize, 16));
        if (!image->pixels)
        {
            alimerImageDestroy(image);
            return false;
        }

        //memset(pixels, 0, _memorySize);

        if (!SetupImageArray(image))
        {
            alimerImageDestroy(image);
            return false;
        }

        return true;
    }
}

static Image* EXR_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    if (!IsEXRFromMemory(pData, dataSize))
        return nullptr;

    float* pixelData;
    int width, height;
    const char* err = NULL;
    int ret = LoadEXRFromMemory(&pixelData, &width, &height, pData, dataSize, &err);
    if (ret != TINYEXR_SUCCESS)
    {
        if (err)
        {
            //std::cerr << "Could not load EXR file '" << path << "': " << err << std::endl;
            FreeEXRErrorMessage(err); // release memory of error message.
        }

        return nullptr;
    }

    // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
    Image* image = alimerImageCreate2D(PixelFormat_RGBA32Float, width, height, 1, 1);
    memcpy(image->pixels, pixelData, image->pixelsSize);
    free(pixelData);

    return image;
}

static Image* STB_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    int width, height, channels;
    PixelFormat format = PixelFormat_RGBA8Unorm;
    void* image_data;
    if (stbi_is_16_bit_from_memory(pData, (int)dataSize))
    {
        image_data = stbi_load_16_from_memory(pData, (int)dataSize, &width, &height, &channels, 0);
        switch (channels)
        {
            case 1:
                format = PixelFormat_R16Uint;
                break;
            case 2:
                format = PixelFormat_RG16Uint;
                break;
            case 4:
                format = PixelFormat_RGBA16Uint;
                break;
            default:
                ALIMER_UNREACHABLE();
        }
    }
    else if (stbi_is_hdr_from_memory(pData, (int)dataSize))
    {
        // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
        image_data = stbi_loadf_from_memory(pData, (int)dataSize, &width, &height, &channels, 4);
        format = PixelFormat_RGBA32Float;
    }
    else
    {
        image_data = stbi_load_from_memory(pData, (int)dataSize, &width, &height, &channels, 4);
        format = PixelFormat_RGBA8Unorm;
    }

    if (!image_data)
        return nullptr;

    Image* result = alimerImageCreate2D(format, width, height, 1u, 1u);
    memcpy(result->pixels, image_data, result->pixelsSize);
    stbi_image_free(image_data);
    return result;
}

Image* alimerImageCreate1D(PixelFormat format, uint32_t width, uint32_t arrayLayers, uint32_t mipLevelCount)
{
    if (format == PixelFormat_Undefined || !width || !arrayLayers)
        return nullptr;


    // 1D is a special case of the 2D case
    Image* image = alimerImageCreate2D(format, width, 1u, arrayLayers, mipLevelCount);
    if (!image)
        return image;

    image->desc.type = ImageType1D;
    return image;
}

Image* alimerImageCreate2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount)
{
    if (format == PixelFormat_Undefined || !width || !height || !arrayLayers)
        return nullptr;

    if (!CalculateMipLevels(width, height, mipLevelCount))
    {
        return nullptr;
    }

    Image* image = ALIMER_ALLOC(Image);
    ALIMER_ASSERT(image);

    image->desc.type = ImageType2D;
    image->desc.format = format;
    image->desc.width = width;
    image->desc.height = height;
    image->desc.depthOrArrayLayers = arrayLayers;
    image->desc.mipLevelCount = mipLevelCount;

    // Already calls ImageDestroy on failure.
    if (!InitializeImage(image))
    {
        return nullptr;
    }

    return image;
}

Image* alimerImageCreate3D(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount)
{
    if (format == PixelFormat_Undefined || !width || !height || !depth)
    {
        return nullptr;
    }

    if (!CalculateMipLevels3D(width, height, depth, mipLevelCount))
    {
        return nullptr;
    }

    Image* image = ALIMER_ALLOC(Image);
    ALIMER_ASSERT(image);

    image->desc.type = ImageType3D;
    image->desc.format = format;
    image->desc.width = width;
    image->desc.height = height;
    image->desc.depthOrArrayLayers = depth;
    image->desc.mipLevelCount = mipLevelCount;

    // Already calls ImageDestroy on failure.
    if (!InitializeImage(image))
    {
        return nullptr;
    }

    return image;
}

Image* alimerImageCreateCube(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount)
{
    if (!width || !height || !arrayLayers)
        return nullptr;

    // A cubemap is just a 2D texture array that is a multiple of 6 for each cube
    Image* image = alimerImageCreate2D(format, width, height, arrayLayers * 6, mipLevelCount);
    if (!image)
    {
        return nullptr;
    }

    image->desc.type = ImageTypeCube;
    return image;
}

Image* alimerImageCreateFromMemory(const uint8_t* pData, size_t dataSize)
{
    Image* image = nullptr;

    if ((image = EXR_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    if ((image = STB_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    return nullptr;
}

void alimerImageDestroy(Image* image)
{
    if (!image)
        return;

    if (image->levels)
    {
        alimerFree(image->levels);
    }

    if (image->pixels)
    {
        // TODO: AlignedFree
        alimerFree(image->pixels);
    }

    alimerFree(image);
}

void alimerImageGetDesc(Image* image, ImageDesc* pDesc)
{
    ALIMER_ASSERT(image);
    ALIMER_ASSERT(pDesc);

    *pDesc = image->desc;
}

ImageType alimerImageGetType(Image* image)
{
    return image->desc.type;
}

PixelFormat alimerImageGetFormat(Image* image)
{
    return image->desc.format;
}

uint32_t alimerImageGetWidth(Image* image, uint32_t level)
{
    return std::max(image->desc.width >> level, 1u);
}

uint32_t alimerImageGetHeight(Image* image, uint32_t level)
{
    return std::max(image->desc.height >> level, 1u);
}

uint32_t alimerImageGetDepth(Image* image, uint32_t level)
{
    if (image->desc.type != ImageType3D) {
        return 1u;
    }

    return std::max(image->desc.depthOrArrayLayers >> level, 1u);
}

uint32_t alimerImageGetArrayLayers(Image* image)
{
    if (image->desc.type == ImageType3D) {
        return 1u;
    }

    return image->desc.depthOrArrayLayers;
}

uint32_t alimerImageGetMipLevelCount(Image* image)
{
    return image->desc.mipLevelCount;
}

uint8_t* alimerImageGetPixels(Image* image, size_t* pixelsSize)
{
    if (pixelsSize)
        *pixelsSize = image->pixelsSize;

    return image->pixels;
}

ImageLevel* alimerImageGetLevel(Image* image, uint32_t mipLevel, uint32_t arrayOrDepthSlice)
{
    ALIMER_ASSERT(image);

    if (mipLevel >= image->desc.mipLevelCount)
        return nullptr;

    uint32_t index = 0;

    switch (image->desc.type)
    {
        case ImageType1D:
        case ImageType2D:
        case ImageTypeCube:
        {
            if (arrayOrDepthSlice >= image->desc.depthOrArrayLayers)
                return nullptr;

            index = arrayOrDepthSlice * (image->desc.mipLevelCount) + mipLevel;
            break;
        }

        case ImageType3D:
        {
            uint32_t mipDepth = image->desc.depthOrArrayLayers;

            for (uint32_t level = 0; level < mipLevel; ++level)
            {
                index += mipDepth;
                if (mipDepth > 1)
                    mipDepth >>= 1;
            }

            if (arrayOrDepthSlice >= mipDepth)
                return nullptr;

            index += arrayOrDepthSlice;
            break;
        }

        default:
            return nullptr;
    }

    return &image->levels[index];
}
