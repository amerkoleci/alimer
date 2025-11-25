// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include <stdio.h>

ALIMER_DISABLE_WARNINGS()
#define STBI_ASSERT(x) ALIMER_ASSERT(x)
//#define STBI_MALLOC(sz) alimer_alloc(sz)
//#define STBI_REALLOC(p,newsz) alimer_realloc(p, newsz)
//#define STBI_FREE(p) alimer_free(p)
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBIW_ASSERT(x) ALIMER_ASSERT(x)
//#define STBIW_MALLOC(sz) alimer_alloc(sz)
//#define STBIW_REALLOC(p, newsz) alimer_realloc(p, newsz)
//#define STBIW_FREE(p) alimer_free(p)
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
//#define QOI_MALLOC(sz) STBI_MALLOC(sz)
//#define QOI_FREE(p) STBI_FREE(p) 
#include "third_party/qoi.h"

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include "third_party/tinyexr.h"

#if defined(ALIMER_IMAGE_KTX)
//#include <gl_format.h>
#include <ktx.h>
#endif

ALIMER_ENABLE_WARNINGS()

struct Image final
{
    ImageDesc desc;
    size_t dataSize;
    void* pData;
};

static Image* DDS_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    ALIMER_UNUSED(pData);
    ALIMER_UNUSED(dataSize);

    return nullptr;
}

static Image* ASTC_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    ALIMER_UNUSED(pData);
    ALIMER_UNUSED(dataSize);

    return nullptr;
}

#if defined(ALIMER_IMAGE_KTX)
static Image* KTX_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    ktxTexture* ktx_texture = 0;
    KTX_error_code ktx_result = ktxTexture_CreateFromMemory(
        pData,
        dataSize,
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        &ktx_texture
    );

    // Not ktx texture.
    if (ktx_result != KTX_SUCCESS)
    {
        return NULL;
    }

    PixelFormat format = PixelFormat_RGBA8Unorm;
    if (ktx_texture->classId == ktxTexture2_c)
    {
        ktxTexture2* ktx_texture2 = (ktxTexture2*)ktx_texture;

        if (ktxTexture2_NeedsTranscoding(ktx_texture2))
        {
            // Once transcoded, the ktxTexture object contains the texture data in a native GPU format (e.g. BC7)
            // Handle other formats (textureCompressionBC) See: https://raw.githubusercontent.com/KhronosGroup/Vulkan-Samples/main/samples/performance/texture_compression_basisu/texture_compression_basisu.cpp
            ktx_result = ktxTexture2_TranscodeBasis(ktx_texture2, KTX_TTF_BC7_RGBA, 0);
            //result->format = TEXTURE_FORMAT_BC7;
        }

        format = alimerPixelFormatFromVkFormat(ktx_texture2->vkFormat);
    }
    else
    {
        // KTX1
        ktxTexture1* ktx_texture1 = (ktxTexture1*)ktx_texture;

        //format = FromVkFormat(vkGetFormatFromOpenGLInternalFormat(ktx_texture1->glInternalformat));

        // KTX-1 files don't contain color space information. Color data is normally
        // in sRGB, but the format we get back won't report that, so this will adjust it
        // if necessary.
        //format = LinearToSrgbFormat(format);
    }

    Image* result = nullptr;
    if (ktx_texture->baseDepth > 1)
    {
        //Initialize3D(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->baseDepth, ktxTexture->numLevels);
    }
    else if (ktx_texture->isCubemap && !ktx_texture->isArray)
    {
        //InitializeCube(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->numFaces, ktxTexture->numLevels);
    }
    else
    {
        result = alimerImageCreate2D(format,
            ktx_texture->baseWidth,
            ktx_texture->baseHeight,
            ktx_texture->isArray ? ktx_texture->numLayers : 1u,
            ktx_texture->numLevels);
    }

    // If the texture contains more than one layer, then populate the offsets otherwise take the mipmap level offsets
    if (ktx_texture->isCubemap || ktx_texture->isArray)
    {
        uint32_t layerCount = ktx_texture->isCubemap ? ktx_texture->numFaces : ktx_texture->numLayers;

        for (uint32_t layer = 0; layer < layerCount; layer++)
        {
            for (uint32_t miplevel = 0; miplevel < ktx_texture->numLevels; miplevel++)
            {
                ktx_size_t     offset;
                KTX_error_code result;
                if (ktx_texture->isCubemap)
                {
                    result = ktxTexture_GetImageOffset(ktx_texture, miplevel, 0, layer, &offset);
                }
                else
                {
                    result = ktxTexture_GetImageOffset(ktx_texture, miplevel, layer, 0, &offset);
                }

                if (result != KTX_SUCCESS)
                {
                    //LOGF("Error loading KTX texture");
                }

                auto levelSize = ktxTexture_GetImageSize(ktx_texture, miplevel);
                //auto levelData = GetLevel(miplevel, layer);
                //memcpy(levelData->pixels, ktx_texture->pData + offset, levelSize);
            }
        }
    }
    else
    {
        for (uint32_t miplevel = 0; miplevel < ktx_texture->numLevels; miplevel++)
        {
            ktx_size_t     offset;
            KTX_error_code result;
            result = ktxTexture_GetImageOffset(ktx_texture, miplevel, 0, 0, &offset);
            if (result != KTX_SUCCESS)
            {
                //LOGF("Error loading KTX texture");
            }

            //auto levelSize = ktxTexture_GetImageSize(ktx_texture, miplevel);
            //auto levelData = GetLevel(miplevel);
            //memcpy(levelData->pixels, ktx_texture->pData + offset, levelSize);
        }
    }

    ktxTexture_Destroy(ktx_texture);
    return result;
}
#endif /* defined(ALIMER_KTX) */

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
    image->dataSize = width * height * 4 * sizeof(float);
    image->pData = malloc(image->dataSize);
    memcpy(image->pData, pixelData, image->dataSize);
    free(pixelData);

    return image;
}

bool AlimerImage_TestQOI(const uint8_t* data, size_t size)
{
    if (size < QOI_HEADER_SIZE)
        return false;

    int p = 0;
    unsigned int magic = qoi_read_32(data, &p);
    if (magic != QOI_MAGIC)
        return false;

    return true;
}

static Image* QOI_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    if (!AlimerImage_TestQOI(pData, dataSize))
        return nullptr;

    int channels = 4;
    qoi_desc qoi_desc;
    void* result = qoi_decode(pData, (int)dataSize, &qoi_desc, channels);

    if (result != nullptr)
    {
        Image* image = alimerImageCreate2D(PixelFormat_RGBA8Unorm, qoi_desc.width, qoi_desc.height, 1u, 1u);
        image->dataSize = qoi_desc.width * qoi_desc.height * channels * sizeof(uint8_t);
        image->pData = malloc(image->dataSize);
        memcpy(image->pData, result, image->dataSize);

        free(result);
        return image;
    }

    return nullptr;
}

static Image* STB_LoadFromMemory(const uint8_t* pData, size_t dataSize)
{
    int width, height, channels;
    PixelFormat format = PixelFormat_RGBA8Unorm;
    void* image_data;
    uint32_t memorySize = 0;
    if (stbi_is_16_bit_from_memory(pData, (int)dataSize))
    {
        image_data = stbi_load_16_from_memory(pData, (int)dataSize, &width, &height, &channels, 0);
        switch (channels)
        {
            case 1:
                format = PixelFormat_R16Uint;
                memorySize = width * height * sizeof(uint16_t);
                break;
            case 2:
                format = PixelFormat_RG16Uint;
                memorySize = width * height * 2 * sizeof(uint16_t);
                break;
            case 4:
                format = PixelFormat_RGBA16Uint;
                memorySize = width * height * 4 * sizeof(uint16_t);
                break;
            default:
                ALIMER_UNREACHABLE();
        }
    }
    else if (stbi_is_hdr_from_memory(pData, (int)dataSize))
    {
        // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
        image_data = stbi_loadf_from_memory(pData, (int)dataSize, &width, &height, NULL, 4);
        format = PixelFormat_RGBA32Float;
        memorySize = width * height * 4 * sizeof(float);
    }
    else
    {
        image_data = stbi_load_from_memory(pData, (int)dataSize, &width, &height, NULL, 4);
        format = PixelFormat_RGBA8Unorm;
        memorySize = width * height * 4 * sizeof(uint8_t);
    }

    if (!image_data)
        return nullptr;

    Image* result = alimerImageCreate2D(format, width, height, 1u, 1u);
    result->dataSize = memorySize;
    result->pData = malloc(memorySize);
    memcpy(result->pData, image_data, memorySize);
    stbi_image_free(image_data);
    return result;
}

Image* alimerImageCreate2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount)
{
    if (format == PixelFormat_Undefined || !width || !height || !arrayLayers)
        return nullptr;

    Image* image = ALIMER_ALLOC(Image);
    ALIMER_ASSERT(image);

    image->desc.dimension = TextureDimension_2D;
    image->desc.format = format;
    image->desc.width = width;
    image->desc.height = height;
    image->desc.depthOrArrayLayers = arrayLayers;
    image->desc.mipLevelCount = mipLevelCount;

    return image;
}

Image* alimerImageCreateFromMemory(const uint8_t* pData, size_t dataSize)
{
    Image* image = nullptr;

    if ((image = DDS_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    if ((image = ASTC_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

#if defined(ALIMER_IMAGE_KTX)
    if ((image = KTX_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;
#endif

    if ((image = EXR_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    if ((image = QOI_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    if ((image = STB_LoadFromMemory(pData, dataSize)) != nullptr)
        return image;

    return nullptr;
}

void alimerImageDestroy(Image* image)
{
    if (!image)
        return;

    if (image->pData)
        alimerFree(image->pData);

    alimerFree(image);
}

void alimerImageGetDesc(Image* image, ImageDesc* pDesc)
{
    ALIMER_ASSERT(image);
    ALIMER_ASSERT(pDesc);

    *pDesc = image->desc;
}

TextureDimension alimerImageGetDimension(Image* image)
{
    return image->desc.dimension;
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
    if (image->desc.dimension != TextureDimension_3D) {
        return 1u;
    }

    return std::max(image->desc.depthOrArrayLayers >> level, 1u);
}

uint32_t alimerImageGetArrayLayers(Image* image)
{
    if (image->desc.dimension == TextureDimension_3D) {
        return 1u;
    }

    return image->desc.depthOrArrayLayers;
}

uint32_t alimerImageGetMipLevelCount(Image* image)
{
    return image->desc.mipLevelCount;
}

void* alimerImageGetData(Image* image, size_t* size)
{
    if (size)
        *size = image->dataSize;

    return image->pData;
}

struct ImageMemory
{
    static const size_t grow = 4096;

    ImageMemory()
    {
        size = grow;
        data = (uint8_t*)alimerMalloc(grow);
        offset = 0;
    }

    uint8_t* data;
    size_t size;
    size_t offset;
};

static void alimer_stbi_write(void* context, void* data, int size)
{
    ImageMemory* image = (ImageMemory*)context;

    size_t offset = image->offset + size;


    if (offset > image->size)
    {
        // TODO:  Need a better strategy for memory growth.
        // TODO:  Need to handle allocation failure gracefully.
        image->size = offset + ImageMemory::grow; // Need a better algo!
        image->data = (uint8_t*)alimerRealloc(image->data, image->size);
    }

    memcpy(image->data + image->offset, data, size);
    image->offset = offset;
}

Blob* alimerImageEncodeJPG(Image* image, int quality)
{
    //if (IsCompressedFormat(desc.format))
    //{
    //    alimerLogError("Cannot save compressed image as JPG");
    //    return false;
    //}

    ImageMemory memory;
    if (stbi_write_jpg_to_func(alimer_stbi_write, &memory,
        image->desc.width,
        image->desc.height,
        4,
        image->pData,
        quality) != 0)
    {
        return alimerBlobCreate(memory.data, memory.offset, nullptr);
    }

    return nullptr;
}
