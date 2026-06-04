// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Core/Memory.h"
//#include "Alimer/Core/Profiler.h"
#include "Alimer/IO/FileSystem.h"
#include "Alimer/IO/MemoryStream.h"
#include "Alimer/Assets/Image.h"

#include <stdio.h>

using namespace Alimer;

ALIMER_DISABLE_WARNINGS()
#define STBI_ASSERT(x) ALIMER_ASSERT(x)
#define STBI_MALLOC(sz) Memory::Alloc(sz)
#define STBI_REALLOC(p,newsz) Memory::Realloc(p, newsz)
#define STBI_FREE(p) Memory::Free(p)
//#define STB_IMAGE_STATIC
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBIW_ASSERT(x) ALIMER_ASSERT(x)
#define STBIW_MALLOC(sz) Memory::Alloc(sz)
#define STBIW_REALLOC(p, newsz) Memory::Realloc(p, newsz)
#define STBIW_FREE(p) Memory::Free(p)
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#if TODO_IMAGE
#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#define QOI_MALLOC(sz) STBI_MALLOC(sz)
#define QOI_FREE(p) STBI_FREE(p) 
#include "Alimer/ThirdParty/qoi.h"

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include "Alimer/ThirdParty/tinyexr.h"

#include <gl_format.h>
#include <ktx.h>  
#endif // TODO_IMAGE

ALIMER_ENABLE_WARNINGS()

#ifndef KTX_IDENTIFIER_REF
#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }
#endif

#ifndef KTX2_IDENTIFIER_REF
#define KTX2_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }
#endif 

namespace
{
    constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "
#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE
#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME
    enum DDS_RESOURCE_DIMENSION : uint32_t
    {
        DDS_DIMENSION_TEXTURE1D = 2,
        DDS_DIMENSION_TEXTURE2D = 3,
        DDS_DIMENSION_TEXTURE3D = 4,
    };

    // Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
    enum DDS_RESOURCE_MISC_FLAG : uint32_t
    {
        DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
    };

    enum DDS_MISC_FLAGS2 : uint32_t
    {
        DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
    };

    enum DDS_ALPHA_MODE : uint32_t
    {
        DDS_ALPHA_MODE_UNKNOWN = 0,
        DDS_ALPHA_MODE_STRAIGHT = 1,
        DDS_ALPHA_MODE_PREMULTIPLIED = 2,
        DDS_ALPHA_MODE_OPAQUE = 3,
        DDS_ALPHA_MODE_CUSTOM = 4,
    };

    struct DDS_PIXELFORMAT
    {
        uint32_t    size;
        uint32_t    flags;
        uint32_t    fourCC;
        uint32_t    RGBBitCount;
        uint32_t    RBitMask;
        uint32_t    GBitMask;
        uint32_t    BBitMask;
        uint32_t    ABitMask;
    };

    struct DDS_HEADER
    {
        uint32_t        size;
        uint32_t        flags;
        uint32_t        height;
        uint32_t        width;
        uint32_t        pitchOrLinearSize;
        uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        uint32_t        mipMapCount;
        uint32_t        reserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t        caps;
        uint32_t        caps2;
        uint32_t        caps3;
        uint32_t        caps4;
        uint32_t        reserved2;
    };

    struct DDS_HEADER_DXT10
    {
        uint32_t        dxgiFormat; /* DXGI_FORMAT */
        uint32_t        resourceDimension;
        uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        uint32_t        arraySize;
        uint32_t        miscFlags2; // see DDS_MISC_FLAGS2
    };

    static constexpr size_t MAX_DDS_HEADER_SIZE = sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);

    static_assert(sizeof(DDS_PIXELFORMAT) == 32, "DDS pixel format size mismatch");
    static_assert(sizeof(DDS_HEADER) == 124, "DDS Header size mismatch");
    static_assert(sizeof(DDS_HEADER_DXT10) == 20, "DDS DX10 Extended Header size mismatch");

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
#define DDS_PAL8A       0x00000021  // DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV
    // DDS_BUMPLUMINANCE 0x00040000

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
                (static_cast<uint32_t>(static_cast<uint8_t>(ch0)) \
                | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8) \
                | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16) \
                | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24))
#endif

    static const DDS_PIXELFORMAT DDSPF_DXT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_DXT2 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_DXT3 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_DXT4 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_DXT5 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_BC4_UNORM =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','U'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_BC4_SNORM =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','S'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_BC5_UNORM =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','U'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_BC5_SNORM =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','S'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('R','G','B','G'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('G','R','G','B'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_YUY2 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_UYVY =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('U','Y','V','Y'), 0, 0, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

    static const DDS_PIXELFORMAT DDSPF_X8R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0 };

    static const DDS_PIXELFORMAT DDSPF_A8B8G8R8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

    static const DDS_PIXELFORMAT DDSPF_X8B8G8R8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0 };

    static const DDS_PIXELFORMAT DDSPF_G16R16 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_R5G6B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0xf800, 0x07e0, 0x001f, 0 };

    static const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x7c00, 0x03e0, 0x001f, 0x8000 };

    static const DDS_PIXELFORMAT DDSPF_X1R5G5B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x7c00, 0x03e0, 0x001f, 0 };

    static const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x0f00, 0x00f0, 0x000f, 0xf000 };

    static const DDS_PIXELFORMAT DDSPF_X4R4G4B4 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0f00, 0x00f0, 0x000f, 0 };

    static const DDS_PIXELFORMAT DDSPF_R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0xff0000, 0x00ff00, 0x0000ff, 0 };

    static const DDS_PIXELFORMAT DDSPF_A8R3G3B2 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00e0, 0x001c, 0x0003, 0xff00 };

    static const DDS_PIXELFORMAT DDSPF_R3G3B2 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 8, 0xe0, 0x1c, 0x03, 0 };

    static const DDS_PIXELFORMAT DDSPF_A4L4 =
    { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 8, 0x0f, 0, 0, 0xf0 };

    static const DDS_PIXELFORMAT DDSPF_L8 =
    { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 8, 0xff, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_L16 =
    { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 16, 0xffff, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_A8L8 =
    { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 16, 0x00ff, 0, 0, 0xff00 };

    static const DDS_PIXELFORMAT DDSPF_A8L8_ALT =
    { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 8, 0x00ff, 0, 0, 0xff00 };

    static const DDS_PIXELFORMAT DDSPF_L8_NVTT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 8, 0xff, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_L16_NVTT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0xffff, 0, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_A8L8_NVTT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00ff, 0, 0, 0xff00 };

    static const DDS_PIXELFORMAT DDSPF_A8 =
    { sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0, 0, 0, 0xff };

    static const DDS_PIXELFORMAT DDSPF_V8U8 =
    { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0, 0 };

    static const DDS_PIXELFORMAT DDSPF_Q8W8V8U8 =
    { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

    static const DDS_PIXELFORMAT DDSPF_V16U16 =
    { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0, 0 };

    // D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue
    static const DDS_PIXELFORMAT DDSPF_A2R10G10B10 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000 };
    static const DDS_PIXELFORMAT DDSPF_A2B10G10R10 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 };

    // We do not support the following legacy Direct3D 9 formats:
    // DDSPF_A2W10V10U10 = { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 };
    // DDSPF_L6V5U5 = { sizeof(DDS_PIXELFORMAT), DDS_BUMPLUMINANCE, 0, 16, 0x001f, 0x03e0, 0xfc00, 0 };
    // DDSPF_X8L8V8U8 = { sizeof(DDS_PIXELFORMAT), DDS_BUMPLUMINANCE, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0 };

    static const DDS_PIXELFORMAT DDSPF_DX10 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','1','0'), 0, 0, 0, 0, 0 };

    void alimer_stbi_write(void* context, void* data, int size)
    {
        ((Stream*)context)->Write(data, size);
    }

    uint32_t CountMips(uint32_t width, uint32_t height) noexcept
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

    uint32_t CountMips3D(uint32_t width, uint32_t height, uint32_t depth) noexcept
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

    bool SetupImageArray(
        uint8_t* pMemory,
        size_t memorySize,
        const ImageDesc& metadata,
        ImageLevel* images,
        uint32_t nImages) noexcept
    {
        ALIMER_ASSERT(pMemory);
        ALIMER_ASSERT(memorySize > 0);
        ALIMER_ASSERT(nImages > 0);

        if (!images)
            return false;

        size_t index = 0;
        uint8_t* pixels = pMemory;
        const uint8_t* pEndBits = pMemory + memorySize;
        //size_t offset = 0;

        switch (metadata.dimension)
        {
            case ImageDimension::Texture1D:
            case ImageDimension::Texture2D:
            case ImageDimension::TextureCube:
                if (metadata.depthOrArrayLayers == 0 || metadata.mipLevelCount == 0)
                {
                    return false;
                }

                for (uint32_t arrayIndex = 0; arrayIndex < metadata.depthOrArrayLayers; ++arrayIndex)
                {
                    uint32_t mipWidth = metadata.width;
                    uint32_t mipHeight = metadata.height;

                    for (uint32_t level = 0; level < metadata.mipLevelCount; ++level)
                    {
                        if (index >= nImages)
                        {
                            return false;
                        }

                        uint32_t rowPitch, slicePitch, widthCount, heightCount;
                        GetSurfaceInfo(metadata.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                        images[index].width = mipWidth;
                        images[index].height = mipHeight;
                        images[index].format = metadata.format;
                        images[index].rowPitch = rowPitch;
                        images[index].slicePitch = slicePitch;
                        images[index].pixels = pixels;
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

            case ImageDimension::Texture3D:
            {
                if (metadata.mipLevelCount == 0 || metadata.depthOrArrayLayers == 0)
                {
                    return false;
                }

                uint32_t mipWidth = metadata.width;
                uint32_t mipHeight = metadata.height;
                uint32_t mipDepth = metadata.depthOrArrayLayers;

                for (uint32_t level = 0; level < metadata.mipLevelCount; ++level)
                {
                    uint32_t rowPitch, slicePitch, widthCount, heightCount;
                    GetSurfaceInfo(metadata.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                    for (uint32_t slice = 0; slice < mipDepth; ++slice)
                    {
                        if (index >= nImages)
                        {
                            return false;
                        }

                        // We use the same memory organization that Direct3D 11 needs for D3D11_SUBRESOURCE_DATA
                        // with all slices of a given miplevel being continuous in memory
                        images[index].width = mipWidth;
                        images[index].height = mipHeight;
                        images[index].format = metadata.format;
                        images[index].rowPitch = rowPitch;
                        images[index].slicePitch = slicePitch;
                        images[index].pixels = pixels;
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

#if TODO_IMAGE
    inline VkFormat vkGetFormatFromOpenGLInternalFormat(const GLenum internalFormat)
    {
        switch (internalFormat)
        {
            //
            // 8 bits per component
            //
            case GL_R8:												return VK_FORMAT_R8_UNORM;					// 1-component, 8-bit unsigned normalized
            case GL_RG8:											return VK_FORMAT_R8G8_UNORM;				// 2-component, 8-bit unsigned normalized
            case GL_RGB8:											return VK_FORMAT_R8G8B8_UNORM;				// 3-component, 8-bit unsigned normalized
            case GL_RGBA8:											return VK_FORMAT_R8G8B8A8_UNORM;			// 4-component, 8-bit unsigned normalized

            case GL_R8_SNORM:										return VK_FORMAT_R8_SNORM;					// 1-component, 8-bit signed normalized
            case GL_RG8_SNORM:										return VK_FORMAT_R8G8_SNORM;				// 2-component, 8-bit signed normalized
            case GL_RGB8_SNORM:										return VK_FORMAT_R8G8B8_SNORM;				// 3-component, 8-bit signed normalized
            case GL_RGBA8_SNORM:									return VK_FORMAT_R8G8B8A8_SNORM;			// 4-component, 8-bit signed normalized

            case GL_R8UI:											return VK_FORMAT_R8_UINT;					// 1-component, 8-bit unsigned integer
            case GL_RG8UI:											return VK_FORMAT_R8G8_UINT;					// 2-component, 8-bit unsigned integer
            case GL_RGB8UI:											return VK_FORMAT_R8G8B8_UINT;				// 3-component, 8-bit unsigned integer
            case GL_RGBA8UI:										return VK_FORMAT_R8G8B8A8_UINT;				// 4-component, 8-bit unsigned integer

            case GL_R8I:											return VK_FORMAT_R8_SINT;					// 1-component, 8-bit signed integer
            case GL_RG8I:											return VK_FORMAT_R8G8_SINT;					// 2-component, 8-bit signed integer
            case GL_RGB8I:											return VK_FORMAT_R8G8B8_SINT;				// 3-component, 8-bit signed integer
            case GL_RGBA8I:											return VK_FORMAT_R8G8B8A8_SINT;				// 4-component, 8-bit signed integer

            case GL_SR8:											return VK_FORMAT_R8_SRGB;					// 1-component, 8-bit sRGB
            case GL_SRG8:											return VK_FORMAT_R8G8_SRGB;					// 2-component, 8-bit sRGB
            case GL_SRGB8:											return VK_FORMAT_R8G8B8_SRGB;				// 3-component, 8-bit sRGB
            case GL_SRGB8_ALPHA8:									return VK_FORMAT_R8G8B8A8_SRGB;				// 4-component, 8-bit sRGB

                //
                // 16 bits per component
                //
            case GL_R16:											return VK_FORMAT_R16_UNORM;					// 1-component, 16-bit unsigned normalized
            case GL_RG16:											return VK_FORMAT_R16G16_UNORM;				// 2-component, 16-bit unsigned normalized
            case GL_RGB16:											return VK_FORMAT_R16G16B16_UNORM;			// 3-component, 16-bit unsigned normalized
            case GL_RGBA16:											return VK_FORMAT_R16G16B16A16_UNORM;		// 4-component, 16-bit unsigned normalized

            case GL_R16_SNORM:										return VK_FORMAT_R16_SNORM;					// 1-component, 16-bit signed normalized
            case GL_RG16_SNORM:										return VK_FORMAT_R16G16_SNORM;				// 2-component, 16-bit signed normalized
            case GL_RGB16_SNORM:									return VK_FORMAT_R16G16B16_SNORM;			// 3-component, 16-bit signed normalized
            case GL_RGBA16_SNORM:									return VK_FORMAT_R16G16B16A16_SNORM;		// 4-component, 16-bit signed normalized

            case GL_R16UI:											return VK_FORMAT_R16_UINT;					// 1-component, 16-bit unsigned integer
            case GL_RG16UI:											return VK_FORMAT_R16G16_UINT;				// 2-component, 16-bit unsigned integer
            case GL_RGB16UI:										return VK_FORMAT_R16G16B16_UINT;			// 3-component, 16-bit unsigned integer
            case GL_RGBA16UI:										return VK_FORMAT_R16G16B16A16_UINT;			// 4-component, 16-bit unsigned integer

            case GL_R16I:											return VK_FORMAT_R16_SINT;					// 1-component, 16-bit signed integer
            case GL_RG16I:											return VK_FORMAT_R16G16_SINT;				// 2-component, 16-bit signed integer
            case GL_RGB16I:											return VK_FORMAT_R16G16B16_SINT;			// 3-component, 16-bit signed integer
            case GL_RGBA16I:										return VK_FORMAT_R16G16B16A16_SINT;			// 4-component, 16-bit signed integer

            case GL_R16F:											return VK_FORMAT_R16_SFLOAT;				// 1-component, 16-bit floating-point
            case GL_RG16F:											return VK_FORMAT_R16G16_SFLOAT;				// 2-component, 16-bit floating-point
            case GL_RGB16F:											return VK_FORMAT_R16G16B16_SFLOAT;			// 3-component, 16-bit floating-point
            case GL_RGBA16F:										return VK_FORMAT_R16G16B16A16_SFLOAT;		// 4-component, 16-bit floating-point

                //
                // 32 bits per component
                //
            case GL_R32UI:											return VK_FORMAT_R32_UINT;					// 1-component, 32-bit unsigned integer
            case GL_RG32UI:											return VK_FORMAT_R32G32_UINT;				// 2-component, 32-bit unsigned integer
            case GL_RGB32UI:										return VK_FORMAT_R32G32B32_UINT;			// 3-component, 32-bit unsigned integer
            case GL_RGBA32UI:										return VK_FORMAT_R32G32B32A32_UINT;			// 4-component, 32-bit unsigned integer

            case GL_R32I:											return VK_FORMAT_R32_SINT;					// 1-component, 32-bit signed integer
            case GL_RG32I:											return VK_FORMAT_R32G32_SINT;				// 2-component, 32-bit signed integer
            case GL_RGB32I:											return VK_FORMAT_R32G32B32_SINT;			// 3-component, 32-bit signed integer
            case GL_RGBA32I:										return VK_FORMAT_R32G32B32A32_SINT;			// 4-component, 32-bit signed integer

            case GL_R32F:											return VK_FORMAT_R32_SFLOAT;				// 1-component, 32-bit floating-point
            case GL_RG32F:											return VK_FORMAT_R32G32_SFLOAT;				// 2-component, 32-bit floating-point
            case GL_RGB32F:											return VK_FORMAT_R32G32B32_SFLOAT;			// 3-component, 32-bit floating-point
            case GL_RGBA32F:										return VK_FORMAT_R32G32B32A32_SFLOAT;		// 4-component, 32-bit floating-point

                //
                // Packed
                //
            case GL_R3_G3_B2:										return VK_FORMAT_UNDEFINED;					// 3-component 3:3:2,       unsigned normalized
            case GL_RGB4:											return VK_FORMAT_UNDEFINED;					// 3-component 4:4:4,       unsigned normalized
            case GL_RGB5:											return VK_FORMAT_R5G5B5A1_UNORM_PACK16;		// 3-component 5:5:5,       unsigned normalized
            case GL_RGB565:											return VK_FORMAT_R5G6B5_UNORM_PACK16;		// 3-component 5:6:5,       unsigned normalized
            case GL_RGB10:											return VK_FORMAT_A2R10G10B10_UNORM_PACK32;	// 3-component 10:10:10,    unsigned normalized
            case GL_RGB12:											return VK_FORMAT_UNDEFINED;					// 3-component 12:12:12,    unsigned normalized
            case GL_RGBA2:											return VK_FORMAT_UNDEFINED;					// 4-component 2:2:2:2,     unsigned normalized
            case GL_RGBA4:											return VK_FORMAT_R4G4B4A4_UNORM_PACK16;		// 4-component 4:4:4:4,     unsigned normalized
            case GL_RGBA12:											return VK_FORMAT_UNDEFINED;					// 4-component 12:12:12:12, unsigned normalized
            case GL_RGB5_A1:										return VK_FORMAT_A1R5G5B5_UNORM_PACK16;		// 4-component 5:5:5:1,     unsigned normalized
            case GL_RGB10_A2:										return VK_FORMAT_A2R10G10B10_UNORM_PACK32;	// 4-component 10:10:10:2,  unsigned normalized
            case GL_RGB10_A2UI:										return VK_FORMAT_A2R10G10B10_UINT_PACK32;	// 4-component 10:10:10:2,  unsigned integer
            case GL_R11F_G11F_B10F:									return VK_FORMAT_B10G11R11_UFLOAT_PACK32;	// 3-component 11:11:10,    floating-point
            case GL_RGB9_E5:										return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;	// 3-component/exp 9:9:9/5, floating-point

                //
                // S3TC/DXT/BC
                //

            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:					return VK_FORMAT_BC1_RGB_UNORM_BLOCK;		// line through 3D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:					return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;		// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:					return VK_FORMAT_BC2_UNORM_BLOCK;			// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:					return VK_FORMAT_BC3_UNORM_BLOCK;			// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:					return VK_FORMAT_BC1_RGB_SRGB_BLOCK;		// line through 3D space, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;		// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return VK_FORMAT_BC2_SRGB_BLOCK;			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return VK_FORMAT_BC3_SRGB_BLOCK;			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

            case GL_COMPRESSED_LUMINANCE_LATC1_EXT:					return VK_FORMAT_BC4_UNORM_BLOCK;			// line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return VK_FORMAT_BC5_UNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return VK_FORMAT_BC4_SNORM_BLOCK;			// line through 1D space, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return VK_FORMAT_BC5_SNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, signed normalized

            case GL_COMPRESSED_RED_RGTC1:							return VK_FORMAT_BC4_UNORM_BLOCK;			// line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RG_RGTC2:							return VK_FORMAT_BC5_UNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_RED_RGTC1:					return VK_FORMAT_BC4_SNORM_BLOCK;			// line through 1D space, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_RG_RGTC2:						return VK_FORMAT_BC5_SNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, signed normalized

            case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return VK_FORMAT_BC6H_UFLOAT_BLOCK;			// 3-component, 4x4 blocks, unsigned floating-point
            case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return VK_FORMAT_BC6H_SFLOAT_BLOCK;			// 3-component, 4x4 blocks, signed floating-point
            case GL_COMPRESSED_RGBA_BPTC_UNORM:						return VK_FORMAT_BC7_UNORM_BLOCK;			// 4-component, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return VK_FORMAT_BC7_SRGB_BLOCK;			// 4-component, 4x4 blocks, sRGB

                //
                // ETC
                //
            case GL_ETC1_RGB8_OES:									return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;	// 3-component ETC1, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_RGB8_ETC2:							return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;	// 3-component ETC2, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA8_ETC2_EAC:						return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;	// 4-component ETC2, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB8_ETC2:							return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;	// 3-component ETC2, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;	// 4-component ETC2, 4x4 blocks, sRGB

            case GL_COMPRESSED_R11_EAC:								return VK_FORMAT_EAC_R11_UNORM_BLOCK;		// 1-component ETC, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RG11_EAC:							return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;	// 2-component ETC, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_R11_EAC:						return VK_FORMAT_EAC_R11_SNORM_BLOCK;		// 1-component ETC, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_RG11_EAC:						return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;	// 2-component ETC, 4x4 blocks, signed normalized

                //
                // PVRTC
                //
            case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;	// 3-component PVRTC, 16x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;	// 3-component PVRTC,  8x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;	// 4-component PVRTC, 16x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;	// 4-component PVRTC,  8x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;	// 4-component PVRTC,  8x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;	// 4-component PVRTC,  4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;	// 3-component PVRTC, 16x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;	// 3-component PVRTC,  8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;	// 4-component PVRTC, 16x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;	// 4-component PVRTC,  8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;	// 4-component PVRTC,  8x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;	// 4-component PVRTC,  4x4 blocks, sRGB

                //
                // ASTC
                //
            case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:					return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;		// 4-component ASTC, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:					return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;		// 4-component ASTC, 5x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:					return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;		// 4-component ASTC, 5x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:					return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;		// 4-component ASTC, 6x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:					return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;		// 4-component ASTC, 6x6 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:					return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;		// 4-component ASTC, 8x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:					return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;		// 4-component ASTC, 8x6 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:					return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;		// 4-component ASTC, 8x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:					return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;		// 4-component ASTC, 10x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:					return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;		// 4-component ASTC, 10x6 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:					return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;		// 4-component ASTC, 10x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:					return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;	// 4-component ASTC, 10x10 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:					return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;	// 4-component ASTC, 12x10 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:					return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;	// 4-component ASTC, 12x12 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;		// 4-component ASTC, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;		// 4-component ASTC, 5x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;		// 4-component ASTC, 5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;		// 4-component ASTC, 6x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;		// 4-component ASTC, 6x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;		// 4-component ASTC, 8x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;		// 4-component ASTC, 8x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;		// 4-component ASTC, 8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;		// 4-component ASTC, 10x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;		// 4-component ASTC, 10x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;		// 4-component ASTC, 10x8 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;		// 4-component ASTC, 10x10 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;		// 4-component ASTC, 12x10 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;		// 4-component ASTC, 12x12 blocks, sRGB

            case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:					return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return VK_FORMAT_UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, sRGB

                //
                // ATC
                //
            case GL_ATC_RGB_AMD:									return VK_FORMAT_UNDEFINED;					// 3-component, 4x4 blocks, unsigned normalized
            case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:					return VK_FORMAT_UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized
            case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return VK_FORMAT_UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized

                //
                // Palletized
                //
            case GL_PALETTE4_RGB8_OES:								return VK_FORMAT_UNDEFINED;					// 3-component 8:8:8,   4-bit palette, unsigned normalized
            case GL_PALETTE4_RGBA8_OES:								return VK_FORMAT_UNDEFINED;					// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
            case GL_PALETTE4_R5_G6_B5_OES:							return VK_FORMAT_UNDEFINED;					// 3-component 5:6:5,   4-bit palette, unsigned normalized
            case GL_PALETTE4_RGBA4_OES:								return VK_FORMAT_UNDEFINED;					// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
            case GL_PALETTE4_RGB5_A1_OES:							return VK_FORMAT_UNDEFINED;					// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
            case GL_PALETTE8_RGB8_OES:								return VK_FORMAT_UNDEFINED;					// 3-component 8:8:8,   8-bit palette, unsigned normalized
            case GL_PALETTE8_RGBA8_OES:								return VK_FORMAT_UNDEFINED;					// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
            case GL_PALETTE8_R5_G6_B5_OES:							return VK_FORMAT_UNDEFINED;					// 3-component 5:6:5,   8-bit palette, unsigned normalized
            case GL_PALETTE8_RGBA4_OES:								return VK_FORMAT_UNDEFINED;					// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
            case GL_PALETTE8_RGB5_A1_OES:							return VK_FORMAT_UNDEFINED;					// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

                //
                // Depth/stencil
                //
            case GL_DEPTH_COMPONENT16:								return VK_FORMAT_D16_UNORM;
            case GL_DEPTH_COMPONENT24:								return VK_FORMAT_X8_D24_UNORM_PACK32;
            case GL_DEPTH_COMPONENT32:								return VK_FORMAT_UNDEFINED;
            case GL_DEPTH_COMPONENT32F:								return VK_FORMAT_D32_SFLOAT;
            case GL_DEPTH_COMPONENT32F_NV:							return VK_FORMAT_D32_SFLOAT;
            case GL_STENCIL_INDEX1:									return VK_FORMAT_UNDEFINED;
            case GL_STENCIL_INDEX4:									return VK_FORMAT_UNDEFINED;
            case GL_STENCIL_INDEX8:									return VK_FORMAT_S8_UINT;
            case GL_STENCIL_INDEX16:								return VK_FORMAT_UNDEFINED;
            case GL_DEPTH24_STENCIL8:								return VK_FORMAT_D24_UNORM_S8_UINT;
            case GL_DEPTH32F_STENCIL8:								return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case GL_DEPTH32F_STENCIL8_NV:							return VK_FORMAT_D32_SFLOAT_S8_UINT;

            default:												return VK_FORMAT_UNDEFINED;
        }
    }
#endif /* TODO_IMAGE */
}

void Image::Register()
{
    RegisterFactory<Image>();
}

Image::Image(std::string_view name)
    : Asset(name)
{
}

Image::~Image()
{
    Destroy();
}

void Image::Destroy() noexcept
{
    levelsCount = 0;
    _memorySize = 0;

    if (levels)
    {
        delete[] levels;
        levels = nullptr;
    }

    if (pixels)
    {
        Memory::AlignedFree(pixels);
        pixels = nullptr;
    }

    desc = {};
}

bool Image::Initialize1D(PixelFormat format_, uint32_t width_, uint32_t arrayLayers, uint32_t mipLevelCount) noexcept
{
    if (format_ == PixelFormat::Undefined || !width_ || !arrayLayers)
    {
        return false;
    }

    // 1D is a special case of the 2D case
    bool result = Initialize2D(format_, width_, 1, arrayLayers, mipLevelCount);
    if (!result)
        return result;

    desc.dimension = ImageDimension::Texture1D;
    return true;
}

bool Image::Initialize2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount) noexcept
{
    if (format == PixelFormat::Undefined || !width || !height || !arrayLayers)
    {
        return false;
    }

    if (!CalculateMipLevels(width, height, mipLevelCount))
    {
        return false;
    }

    Destroy();

    desc.dimension = ImageDimension::Texture2D;
    desc.format = format;
    desc.width = width;
    desc.height = height;
    desc.depthOrArrayLayers = arrayLayers;
    desc.mipLevelCount = mipLevelCount;

    return Initialize();
}

bool Image::Initialize3D(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount) noexcept
{
    if (format == PixelFormat::Undefined || !width || !height || !depth)
    {
        return false;
    }

    if (!CalculateMipLevels3D(width, height, depth, mipLevelCount))
    {
        return false;
    }

    Destroy();

    desc.dimension = ImageDimension::Texture3D;
    desc.format = format;
    desc.width = width;
    desc.height = height;
    desc.depthOrArrayLayers = depth;
    desc.mipLevelCount = mipLevelCount;

    return Initialize();
}

bool Image::InitializeCube(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount) noexcept
{
    if (!width || !height || !arrayLayers)
        return false;

    // A cubemap is just a 2D texture array that is a multiple of 6 for each cube
    bool result = Initialize2D(format, width, height, arrayLayers * 6, mipLevelCount);
    if (!result)
    {
        return result;
    }

    desc.dimension = ImageDimension::TextureCube;
    return true;
}

const ImageLevel* Image::GetLevel(uint32_t mipLevel, uint32_t arrayOrDepthSlice) const noexcept
{
    if (mipLevel >= desc.mipLevelCount)
        return nullptr;

    uint32_t index = 0;

    switch (desc.dimension)
    {
        case ImageDimension::Texture1D:
        case ImageDimension::Texture2D:
        case ImageDimension::TextureCube:
        {
            if (arrayOrDepthSlice >= desc.depthOrArrayLayers)
                return nullptr;

            index = arrayOrDepthSlice * (desc.mipLevelCount) + mipLevel;
            break;
        }

        case ImageDimension::Texture3D:
        {
            uint32_t mipDepth = desc.depthOrArrayLayers;

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

    return &levels[index];
}

bool Image::Save(Stream& dest) const
{
    return SavePNG(dest);
}

bool Image::Save(std::string_view fileName, ImageFileType fileType) const
{
    FileStream stream(fileName, FileMode::Write);
    if (stream.IsOpen() && stream.CanWrite())
    {
        return Image::Save(stream, fileType);
    }

    return false;
}

bool Image::SavePNG(const std::string& fileName) const
{
    if (IsCompressedFormat(desc.format))
    {
        LOGE("Cannot save compressed image as JPG");
        return false;
    }

    FileStream stream(fileName, FileMode::Write);
    if (stream.IsOpen() && stream.CanWrite())
    {
        return SavePNG(stream);
    }

    return false;
}

bool Image::SavePNG(Stream& stream) const
{
    if (IsCompressedFormat(desc.format))
    {
        LOGE("Can not save compressed image");
        return false;
    }
    stbi_write_force_png_filter = 0;
    stbi_write_png_compression_level = 0;

    if (stbi_write_png_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels, desc.width * 4) != 0)
        return true;

    return false;
}

bool Image::SaveJPG(const std::string& fileName, int quality) const
{
    if (IsCompressedFormat(desc.format))
    {
        LOGE("Cannot save compressed image as JPG");
        return false;
    }

    FileStream stream(fileName, FileMode::Write);
    if (stream.IsOpen() && stream.CanWrite())
    {
        return SaveJPG(stream, quality);
    }

    return false;
}

bool Image::Save(Stream& stream, ImageFileType fileType) const
{
    if (fileType != ImageFileType::DDS &&
        fileType != ImageFileType::Alimer &&
        IsCompressedFormat(desc.format))
    {
        LOGE("Can not save compressed image");
        return false;
    }

    switch (fileType)
    {
        case ImageFileType::Bmp:
            if (stbi_write_bmp_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels) != 0)
                return true;
            break;
        case ImageFileType::Png:
            stbi_write_force_png_filter = 0;
            stbi_write_png_compression_level = 0;

            if (stbi_write_png_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels, desc.width * 4) != 0)
                return true;
            break;
        case ImageFileType::Jpg:
            if (stbi_write_jpg_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels, 0) != 0)
                return true;
            break;
        case ImageFileType::Tga:
            if (stbi_write_tga_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels) != 0)
                return true;
            break;
        case ImageFileType::Hdr:
            if (stbi_write_hdr_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, (const float*)pixels) != 0)
                return true;
            break;
        case ImageFileType::DDS:
            return SaveDDS(stream);
        default:
            break;
    }

    return false;
}

bool Image::SaveJPG(Stream& stream, int quality) const
{
    if (IsCompressedFormat(desc.format))
    {
        LOGE("Can not save compressed image");
        return false;
    }

    if (stbi_write_jpg_to_func(alimer_stbi_write, &stream, desc.width, desc.height, 4, pixels, quality) != 0)
    {
        return true;
    }

    return false;
}

bool Image::SaveDDS(Stream& stream) const
{
    //ALIMER_PROFILE_CPU("Image::SaveDDS");

    // see: https://raw.githubusercontent.com/microsoft/DirectXTex/main/DirectXTex/DirectXTexDDS.cpp
    bool DDS_FLAGS_FORCE_DX10_EXT_MISC2 = false;
    bool DDS_FLAGS_FORCE_DX10_EXT = false;
    bool DDS_FLAGS_FORCE_DXT5_RXGB = false;

    if (desc.depthOrArrayLayers > 1)
    {
        if ((desc.depthOrArrayLayers != 6) ||
            (desc.dimension != ImageDimension::Texture2D) ||
            (desc.dimension != ImageDimension::TextureCube))
        {
            // Texture1D arrays, Texture2D arrays, and Cubemap arrays must be stored using 'DX10' extended header
            DDS_FLAGS_FORCE_DX10_EXT = true;
        }
    }

    if (DDS_FLAGS_FORCE_DX10_EXT_MISC2)
    {
        DDS_FLAGS_FORCE_DX10_EXT = true;
    }

    DDS_PIXELFORMAT ddpf = {};
    if (!DDS_FLAGS_FORCE_DX10_EXT)
    {
        bool DDS_FLAGS_FORCE_DX9_LEGACY = false;
        bool isPMAlpha = false;
        switch (desc.format)
        {
            case PixelFormat::RGBA8Unorm:        memcpy(&ddpf, &DDSPF_A8B8G8R8, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::RG16Unorm:          memcpy(&ddpf, &DDSPF_G16R16, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::RG8Unorm:            memcpy(&ddpf, &DDSPF_A8L8, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::R16Unorm:             memcpy(&ddpf, &DDSPF_L16, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::R8Unorm:              memcpy(&ddpf, &DDSPF_L8, sizeof(DDS_PIXELFORMAT)); break;
            //case DXGI_FORMAT_A8_UNORM:              memcpy(&ddpf, &DDSPF_A8, sizeof(DDS_PIXELFORMAT)); break;
            //case DXGI_FORMAT_R8G8_B8G8_UNORM:       memcpy(&ddpf, &DDSPF_R8G8_B8G8, sizeof(DDS_PIXELFORMAT)); break;
            //case DXGI_FORMAT_G8R8_G8B8_UNORM:       memcpy(&ddpf, &DDSPF_G8R8_G8B8, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BC1RGBAUnorm:             memcpy(&ddpf, &DDSPF_DXT1, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BC2RGBAUnorm:             memcpy(&ddpf, isPMAlpha ? (&DDSPF_DXT2) : (&DDSPF_DXT3), sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BC4RUnorm:             memcpy(&ddpf, &DDSPF_BC4_SNORM, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BC5RGSnorm:             memcpy(&ddpf, &DDSPF_BC5_SNORM, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::B5G6R5Unorm:          memcpy(&ddpf, &DDSPF_R5G6B5, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BGR5A1Unorm:        memcpy(&ddpf, &DDSPF_A1R5G5B5, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::RG8Snorm:            memcpy(&ddpf, &DDSPF_V8U8, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::RGBA8Snorm:        memcpy(&ddpf, &DDSPF_Q8W8V8U8, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::RG16Snorm:          memcpy(&ddpf, &DDSPF_V16U16, sizeof(DDS_PIXELFORMAT)); break;
            case PixelFormat::BGRA8Unorm:        memcpy(&ddpf, &DDSPF_A8R8G8B8, sizeof(DDS_PIXELFORMAT)); break; // DXGI 1.1
            //case DXGI_FORMAT_B8G8R8X8_UNORM:        memcpy(&ddpf, &DDSPF_X8R8G8B8, sizeof(DDS_PIXELFORMAT)); break; // DXGI 1.1
            case PixelFormat::BGRA4Unorm:        memcpy(&ddpf, &DDSPF_A4R4G4B4, sizeof(DDS_PIXELFORMAT)); break; // DXGI 1.2
            //case DXGI_FORMAT_YUY2:                  memcpy(&ddpf, &DDSPF_YUY2, sizeof(DDS_PIXELFORMAT)); break; // DXGI 1.2

            case PixelFormat::BC3RGBAUnorm:
                memcpy(&ddpf, isPMAlpha ? (&DDSPF_DXT4) : (&DDSPF_DXT5), sizeof(DDS_PIXELFORMAT));
                if (DDS_FLAGS_FORCE_DXT5_RXGB)
                {
                    ddpf.fourCC = MAKEFOURCC('R', 'X', 'G', 'B');
                }
                break;

                // Legacy D3DX formats using D3DFMT enum value as FourCC
            case PixelFormat::RGBA32Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 116;  // D3DFMT_A32B32G32R32F
                break;
            case PixelFormat::RGBA16Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 113;  // D3DFMT_A16B16G16R16F
                break;
            case PixelFormat::RGBA16Unorm:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 36;  // D3DFMT_A16B16G16R16
                break;
            case PixelFormat::RGBA16Snorm:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 110;  // D3DFMT_Q16W16V16U16
                break;
            case PixelFormat::RG32Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 115;  // D3DFMT_G32R32F
                break;
            case PixelFormat::RG16Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 112;  // D3DFMT_G16R16F
                break;
            case PixelFormat::R32Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 114;  // D3DFMT_R32F
                break;
            case PixelFormat::R16Float:
                ddpf.size = sizeof(DDS_PIXELFORMAT); ddpf.flags = DDS_FOURCC; ddpf.fourCC = 111;  // D3DFMT_R16F
                break;

                // DX9 legacy pixel formats
            case PixelFormat::RGB10A2Unorm:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    // Write using the 'incorrect' mask version to match D3DX bug
                    memcpy(&ddpf, &DDSPF_A2B10G10R10, sizeof(DDS_PIXELFORMAT));
                }
                break;

            case PixelFormat::RGBA8UnormSrgb:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    memcpy(&ddpf, &DDSPF_A8B8G8R8, sizeof(DDS_PIXELFORMAT));
                }
                break;

            case PixelFormat::BC1RGBAUnormSrgb:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    memcpy(&ddpf, &DDSPF_DXT1, sizeof(DDS_PIXELFORMAT));
                }
                break;

            case PixelFormat::BC2RGBAUnormSrgb:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    memcpy(&ddpf, isPMAlpha ? (&DDSPF_DXT2) : (&DDSPF_DXT3), sizeof(DDS_PIXELFORMAT));
                }
                break;

            case PixelFormat::BC3RGBAUnormSrgb:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    memcpy(&ddpf, isPMAlpha ? (&DDSPF_DXT4) : (&DDSPF_DXT5), sizeof(DDS_PIXELFORMAT));
                }
                break;

            case PixelFormat::BC4RSnorm:
                memcpy(&ddpf, &DDSPF_BC4_UNORM, sizeof(DDS_PIXELFORMAT));
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    ddpf.fourCC = MAKEFOURCC('A', 'T', 'I', '1');
                }
                break;

            case PixelFormat::BC5RGUnorm:
                memcpy(&ddpf, &DDSPF_BC5_UNORM, sizeof(DDS_PIXELFORMAT));
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    ddpf.fourCC = MAKEFOURCC('A', 'T', 'I', '2');
                }
                break;

            case PixelFormat::BGRA8UnormSrgb:
                if (DDS_FLAGS_FORCE_DX9_LEGACY)
                {
                    memcpy(&ddpf, &DDSPF_A8R8G8B8, sizeof(DDS_PIXELFORMAT));
                }
                break;

            //case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            //    if (DDS_FLAGS_FORCE_DX9_LEGACY)
            //    {
            //        memcpy(&ddpf, &DDSPF_X8R8G8B8, sizeof(DDS_PIXELFORMAT));
            //    }
            //    break;

            default:
                break;
        }
    }

    DDS_HEADER header = {};
    header.size = sizeof(DDS_HEADER);
    header.flags = DDS_HEADER_FLAGS_TEXTURE;
    header.caps = DDS_SURFACE_FLAGS_TEXTURE;

    if (desc.mipLevelCount > 0)
    {
        header.flags |= DDS_HEADER_FLAGS_MIPMAP;

        if (desc.mipLevelCount > Limits<uint16_t>::Max)
            return false;

        header.mipMapCount = static_cast<uint32_t>(desc.mipLevelCount);

        if (header.mipMapCount > 1)
            header.caps |= DDS_SURFACE_FLAGS_MIPMAP;
    }

    switch (desc.dimension)
    {
        case ImageDimension::Texture1D:
            if (desc.width > UINT32_MAX)
                return false;

            header.width = static_cast<uint32_t>(desc.width);
            header.height = header.depth = 1;
            break;

        case ImageDimension::Texture2D:
        case ImageDimension::TextureCube:
            if (desc.height > UINT32_MAX || desc.width > UINT32_MAX)
                return false;

            header.height = static_cast<uint32_t>(desc.height);
            header.width = static_cast<uint32_t>(desc.width);
            header.depth = 1;

            if (desc.dimension == ImageDimension::TextureCube)
            {
                header.caps |= DDS_SURFACE_FLAGS_CUBEMAP;
                header.caps2 |= DDS_CUBEMAP_ALLFACES;
            }
            break;

        case ImageDimension::Texture3D:
            if (desc.height > UINT32_MAX || desc.width > UINT32_MAX || desc.depthOrArrayLayers > UINT16_MAX)
            {
                return false;
            }

            header.flags |= DDS_HEADER_FLAGS_VOLUME;
            header.caps2 |= DDS_FLAGS_VOLUME;
            header.height = static_cast<uint32_t>(desc.height);
            header.width = static_cast<uint32_t>(desc.width);
            header.depth = static_cast<uint32_t>(desc.depthOrArrayLayers);
            break;

        default:
            return false;
    }

    // TODO: PixelFormat
    uint32_t rowPitch, slicePitch;
    GetSurfaceInfo(desc.format, desc.width, desc.height, &rowPitch, &slicePitch);

    if (slicePitch == 0 || rowPitch == 0)
    {
        return false;
    }

    if (IsCompressedFormat(desc.format))
    {
        header.flags |= DDS_HEADER_FLAGS_LINEARSIZE;
        header.pitchOrLinearSize = static_cast<uint32_t>(slicePitch);
    }
    else
    {
        header.flags |= DDS_HEADER_FLAGS_PITCH;
        header.pitchOrLinearSize = static_cast<uint32_t>(rowPitch);
    }

    DDS_HEADER_DXT10 dxtHeader = {};
    if (ddpf.size == 0)
    {
        memcpy(&header.ddspf, &DDSPF_DX10, sizeof(DDS_PIXELFORMAT));

        dxtHeader.dxgiFormat = ToDxgiFormat(desc.format);
        dxtHeader.resourceDimension = DDS_DIMENSION_TEXTURE2D; // metadata.dimension;

        if (desc.depthOrArrayLayers > UINT16_MAX)
            return false;

        //static_assert(static_cast<int>(TEX_MISC_TEXTURECUBE) == static_cast<int>(DDS_RESOURCE_MISC_TEXTURECUBE), "DDS header mismatch");

        dxtHeader.miscFlag = 0; // metadata.miscFlags & ~static_cast<uint32_t>(TEX_MISC_TEXTURECUBE);

        if (desc.dimension == ImageDimension::TextureCube)
        {
            dxtHeader.miscFlag |= DDS_RESOURCE_MISC_TEXTURECUBE;
            assert((desc.depthOrArrayLayers % 6) == 0);
            dxtHeader.arraySize = desc.depthOrArrayLayers / 6;
        }
        else
        {
            dxtHeader.arraySize = desc.depthOrArrayLayers;
        }

        if (DDS_FLAGS_FORCE_DX10_EXT_MISC2)
        {
            // This was formerly 'reserved'. D3DX10 and D3DX11 will fail if this value is anything other than 0
            //ext->miscFlags2 = metadata.miscFlags2;
        }
    }
    else
    {
        memcpy(&header.ddspf, &ddpf, sizeof(ddpf));
    }

    stream.Write(DDS_MAGIC);
    stream.Write(&header, sizeof(header));
    if (ddpf.size == 0)
    {
        stream.Write(&dxtHeader, sizeof(dxtHeader));
    }
    for (uint32_t i = 0; i < levelsCount; ++i)
    {
        stream.Write(levels[i].pixels, levels[i].slicePitch);
    }

    return true;
}

ImageRef Image::FromFile(std::string_view path)
{
    if (!File::Exists(path))
    {
        LOGW("File '{}' doesn't exists", path);
        return nullptr;
    }

    FileStream stream(path, FileMode::Read);
    return FromStream(stream);
}

ImageRef Image::FromStream(Stream& stream)
{
    ImageRef image(new Image());
    if (!image->Load(stream))
    {
        return nullptr;
    }

    return image;
}

ImageRef Image::FromMemory(const void* data, size_t size)
{
    MemoryStream stream(data, size);
    ImageRef image(new Image());
    if (!image->Load(stream))
    {
        return nullptr;
    }

    return image;
}

bool Image::Initialize()
{
    if (!DetermineImageArray())
        return false;

    levels = new ImageLevel[levelsCount];
    if (!levels)
        return false;

    memset(levels, 0, sizeof(ImageLevel) * levelsCount);
    pixels = static_cast<uint8_t*>(Memory::AlignedAlloc(_memorySize, 16));
    if (!pixels)
    {
        Destroy();
        return false;
    }

    memset(pixels, 0, _memorySize);

    if (!SetupImageArray(pixels, _memorySize, desc, levels, levelsCount))
    {
        Destroy();
        return false;
    }

    return true;
}

bool Image::DetermineImageArray()
{
    _memorySize = 0;
    levelsCount = 0;

    switch (desc.dimension)
    {
        case ImageDimension::Texture1D:
        case ImageDimension::Texture2D:
        case ImageDimension::TextureCube:
            for (uint32_t item = 0; item < desc.depthOrArrayLayers; ++item)
            {
                uint32_t mipWidth = desc.width;
                uint32_t mipHeight = desc.height;

                for (uint32_t level = 0; level < desc.mipLevelCount; ++level)
                {
                    uint32_t rowPitch, slicePitch, widthCount, heightCount;
                    GetSurfaceInfo(desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                    _memorySize += slicePitch;
                    ++levelsCount;

                    if (mipHeight > 1)
                        mipHeight >>= 1;

                    if (mipWidth > 1)
                        mipWidth >>= 1;
                }
            }
            break;

        case ImageDimension::Texture3D:
        {
            uint32_t mipWidth = desc.width;
            uint32_t mipHeight = desc.height;
            uint32_t mipDepth = desc.depthOrArrayLayers;

            for (uint32_t level = 0; level < desc.mipLevelCount; ++level)
            {
                uint32_t rowPitch, slicePitch, widthCount, heightCount;
                GetSurfaceInfo(desc.format, mipWidth, mipHeight, &rowPitch, &slicePitch, &widthCount, &heightCount);

                for (uint32_t slice = 0; slice < mipDepth; ++slice)
                {
                    _memorySize += slicePitch;
                    ++levelsCount;
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


bool Image::BeginLoad(Stream& source)
{
    size_t dataSize = source.GetSize();
    std::unique_ptr<uint8_t> buffer(new uint8_t[dataSize]);
    source.Read(buffer.get(), dataSize);

    if (IsEngine(buffer.get(), dataSize))
    {
        source.Seek(4);
        const uint16_t version = source.ReadUInt16();
        if (version != 1)
        {
            LOGE("Image version is wrong");
            return false;
        }

        const ImageDimension type = (ImageDimension)source.ReadUInt8();
        const PixelFormat format = (PixelFormat)source.ReadUInt8();
        const uint32_t width = source.ReadUInt32();
        const uint32_t height = source.ReadUInt32();
        const uint32_t depthOrArrayLayers = source.ReadUInt32();
        const uint32_t mipLevels = source.ReadUInt32();

        // Write texture data
        const uint32_t dataSize = source.ReadUInt32();
        Vector<uint8_t> pixelData = source.ReadBytes(dataSize);

        ALIMER_UNUSED(type);
        Initialize2D(format, width, height, depthOrArrayLayers, mipLevels);
        memcpy(pixels, pixelData.data(), _memorySize);
        return true;
    }
    else if (IsDDS(buffer.get(), dataSize))
    {
    }
#if TODO_IMAGE
    else if (IsQOI(buffer.get(), dataSize))
    {
        qoi_desc qoiDesc;
        void* pixelData = qoi_decode(buffer.get(), (int)dataSize, &qoiDesc, 4);

        if (pixelData != nullptr)
        {
            Initialize2D(PixelFormat::RGBA8UnormSrgb, qoiDesc.width, qoiDesc.height, 1u, 1u);
            memcpy(pixels, pixelData, _memorySize);
            QOI_FREE(pixelData);
            return true;
        }
    }
    else if (IsEXRFromMemory(buffer.get(), dataSize) == TINYEXR_SUCCESS)
    {
        float* pixelData;
        int width, height;
        const char* err = NULL;
        int ret = LoadEXRFromMemory(&pixelData, &width, &height, buffer.get(), dataSize, &err);
        if (ret != TINYEXR_SUCCESS)
        {
            if (err)
            {
                LOGE("Could not load EXR file '{}': {}", source.GetName(), err);
                FreeEXRErrorMessage(err); // release memory of error message.
            }

            return false;
        }

        // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
        Initialize2D(PixelFormat::RGBA32Float, width, height, 1u, 1u);
        memcpy(pixels, pixelData, _memorySize);
        free(pixelData);
    }
    else if (IsKTX1(buffer.get(), dataSize) || IsKTX2(buffer.get(), dataSize))
    {
        ktxTexture* ktxTexture;
        KTX_error_code result = ktxTexture_CreateFromMemory(buffer.get(),
            dataSize,
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &ktxTexture);

        // Not ktx texture.
        if (result != KTX_SUCCESS)
        {
            return false;
        }

        // We detect format from VkFormat
        PixelFormat format = PixelFormat::Undefined;
        if (ktxTexture->classId == ktxTexture2_c)
        {
            ktxTexture2* ktx_texture2 = (ktxTexture2*)ktxTexture;
            if (ktxTexture2_NeedsTranscoding(ktx_texture2))
            {
                // Once transcoded, the ktxTexture object contains the texture data in a native GPU format (e.g. BC7)
                // Handle other formats (textureCompressionBC) See: https://raw.githubusercontent.com/KhronosGroup/Vulkan-Samples/main/samples/performance/texture_compression_basisu/texture_compression_basisu.cpp
                result = ktxTexture2_TranscodeBasis(ktx_texture2, KTX_TTF_BC7_RGBA, 0);
            }

            format = FromVkFormat(ktx_texture2->vkFormat);
        }
        else
        {
            // KTX1
            ktxTexture1* ktx_texture1 = (ktxTexture1*)ktxTexture;

            format = FromVkFormat(vkGetFormatFromOpenGLInternalFormat(ktx_texture1->glInternalformat));

            // KTX-1 files don't contain color space information. Color data is normally
            // in sRGB, but the format we get back won't report that, so this will adjust it
            // if necessary.
            format = LinearToSrgbFormat(format);
        }

        if (ktxTexture->baseDepth > 1)
        {
            Initialize3D(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->baseDepth, ktxTexture->numLevels);
        }
        else if (ktxTexture->isCubemap && !ktxTexture->isArray)
        {
            InitializeCube(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->numFaces / 6u, ktxTexture->numLevels);
        }
        else
        {
            Initialize2D(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->isArray ? ktxTexture->numLayers : 1u, ktxTexture->numLevels);
        }

        // If the texture contains more than one layer, then populate the offsets otherwise take the mipmap level offsets
        if (ktxTexture->isCubemap || ktxTexture->isArray)
        {
            uint32_t layerCount = ktxTexture->isCubemap ? ktxTexture->numFaces : ktxTexture->numLayers;

            for (uint32_t layer = 0; layer < layerCount; layer++)
            {
                for (uint32_t miplevel = 0; miplevel < ktxTexture->numLevels; miplevel++)
                {
                    ktx_size_t     offset;
                    KTX_error_code result;
                    if (ktxTexture->isCubemap)
                    {
                        result = ktxTexture_GetImageOffset(ktxTexture, miplevel, 0, layer, &offset);
                    }
                    else
                    {
                        result = ktxTexture_GetImageOffset(ktxTexture, miplevel, layer, 0, &offset);
                    }

                    if (result != KTX_SUCCESS)
                    {
                        LOGF("Error loading KTX texture");
                    }

                    auto levelSize = ktxTexture_GetImageSize(ktxTexture, miplevel);
                    auto levelData = GetLevel(miplevel, layer);
                    memcpy(levelData->pixels, ktxTexture->pData + offset, levelSize);
                }
            }
        }
        else
        {
            for (uint32_t miplevel = 0; miplevel < ktxTexture->numLevels; miplevel++)
            {
                ktx_size_t     offset;
                KTX_error_code result;
                result = ktxTexture_GetImageOffset(ktxTexture, miplevel, 0, 0, &offset);
                if (result != KTX_SUCCESS)
                {
                    LOGF("Error loading KTX texture");
                }

                auto levelSize = ktxTexture_GetImageSize(ktxTexture, miplevel);
                auto levelData = GetLevel(miplevel);
                memcpy(levelData->pixels, ktxTexture->pData + offset, levelSize);
            }
        }

        //result = (ktxTexture->isCubemap && !ktxTexture->isArray) ? ktxTexture_IterateLevelFaces(ktxTexture, KtxLevelCallback, this) : ktxTexture_IterateLevels(ktxTexture, KtxLevelCallback, this);
        //if (result != KTX_SUCCESS)
        //{
        //    LOGF("Error loading KTX texture");
        //}

        ktxTexture_Destroy(ktxTexture);
    }
    else
#endif /* TODO_IMAGE */
    {
        int width, height, channels;
        PixelFormat format = PixelFormat::Undefined;
        void* pixelData = nullptr;
        if (stbi_is_16_bit_from_memory(buffer.get(), (int)dataSize))
        {
            pixelData = stbi_load_16_from_memory(buffer.get(), (int)dataSize, &width, &height, &channels, 0);

            switch (channels)
            {
                case 1:
                    format = PixelFormat::R16Unorm;
                    break;
                case 2:
                    format = PixelFormat::RG16Unorm;
                    break;
                case 4:
                    format = PixelFormat::RGBA16Unorm;
                    break;
                default:
                    ALIMER_ASSERT_FAIL("Unsupported channel count for 16 bit image: %d", channels);
            }
        }
        else if (stbi_is_hdr_from_memory(buffer.get(), (int)dataSize))
        {
            pixelData = stbi_loadf_from_memory(buffer.get(), (int)dataSize, &width, &height, &channels, 4);
            format = PixelFormat::RGBA32Float;
        }
        else
        {
            pixelData = stbi_load_from_memory(buffer.get(), (int)dataSize, &width, &height, &channels, 4);
            channels = 4;
            switch (channels)
            {
                case STBI_rgb_alpha:
                    format = PixelFormat::RGBA8UnormSrgb;
                    break;
                case STBI_rgb:
                    // Not supported
                    //format = PixelFormat::RGB8;
                    break;
                case STBI_grey_alpha:
                    format = PixelFormat::RG8Unorm;
                    break;
                case STBI_grey:
                    format = PixelFormat::R8Unorm;
                    break;
                default:
                    LOGE("Invalid channels count");
                    return false;
            }
        }

        if (!pixelData)
        {
            LOGE("Could not load image {} : {}", source.GetName().c_str(), stbi_failure_reason());
            return false;
        }

        Initialize2D(format, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u, 1u);
        memcpy(pixels, pixelData, _memorySize);
        stbi_image_free(pixelData);
    }

    return true;
}

bool Image::IsEngine(const uint8_t* data, size_t size)
{
    if (size < 4)
        return false;

    return data[0] == 'T' && data[1] == 'E' && data[2] == 'X' && data[3] == 'B';
}

bool Image::IsKTX1(const uint8_t* data, size_t size)
{
    if (size <= 12) 
        return false;
    

    static const uint8_t ktx_ident_ref[12] = KTX_IDENTIFIER_REF;
    return memcmp(ktx_ident_ref, data, 12) == 0;
}

bool Image::IsKTX2(const uint8_t* data, size_t size)
{
    if (size <= 12) 
        return false;
    

    static const uint8_t ktx_ident_ref[12] = KTX2_IDENTIFIER_REF;
    return memcmp(ktx_ident_ref, data, 12) == 0;
}

bool Image::IsDDS(const uint8_t* data, size_t size)
{
    if (size < 4) 
        return false;

    return data[0] == 'D' && data[1] == 'D' && data[2] == 'S';
}

bool Image::IsQOI(const uint8_t* data, size_t size)
{
#if TODO_IMAGE
    if (size < QOI_HEADER_SIZE)
        return false;

    int p = 0;
    unsigned int magic = qoi_read_32(data, &p);
    if (magic != QOI_MAGIC)
        return false;

    return true;
#else
    return false;
#endif
}

bool Alimer::CalculateMipLevels(uint32_t width, uint32_t height, uint32_t& mipLevels) noexcept
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

bool Alimer::CalculateMipLevels3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t& mipLevels) noexcept
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
