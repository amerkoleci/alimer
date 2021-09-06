// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"
#include "Window.h"
#include "Core/Log.h"

namespace Alimer::rhi
{
    const FormatInfo kFormatDesc[] = {
        //        format                    name                bytes blk         kind               red   green   blue  alpha  depth  stencl signed  srgb
        { Format::Undefined,            "Undefined",        0,   0, FormatKind::Integer,      false, false, false, false, false, false, false, false },
        { Format::R8UNorm,              "R8UNorm",          1,   1, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R8SNorm,              "R8SNorm",          1,   1, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R8UInt,               "R8UInt",           1,   1, FormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R8SInt,               "R8SInt",           1,   1, FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R16UNorm,             "R16UNorm",         2,   1, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16SNorm,             "R16SNorm",         2,   1, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16UInt,              "R16UInt",          2,   1, FormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R16SInt,              "R16SInt",          2,   1, FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R16Float,             "R16Float",         2,   1, FormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { Format::RG8UNorm,             "RG8UNorm",         2,   1, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG8SNorm,             "RG8SNorm",         2,   1, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG8UInt,              "RG8UInt",          2,   1, FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG8SInt,              "RG8SInt",          2,   1, FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::BGRA4UNorm,           "BGRA4UNorm",       2,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::B5G6R5UNorm,          "B5G6R5UNorm",      2,   1, FormatKind::Normalized,   true,  true,  true,  false, false, false, false, false },
        { Format::B5G5R5A1UNorm,        "B5G5R5A1UNorm",    2,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::R32UInt,              "R32UInt",          4,   1, FormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R32SInt,              "R32SInt",          4,   1, FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R32Float,             "R32Float",         4,   1, FormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { Format::RG16UNorm,            "RG16UNorm",        4,   1, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16SNorm,            "RG16SNorm",        4,   1, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16UInt,             "RG16UInt",         4,   1, FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG16SInt,             "RG16SInt",         4,   1, FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG16Float,            "RG16Float",        4,   1, FormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::RGBA8UNorm,           "RGBA8UNorm",       4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8UNormSrgb,       "RGBA8UNormSrgb",   4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::RGBA8SNorm,           "RGBA8SNorm",       4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8UInt,            "RGBA8UInt",        4,   1, FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8SInt,            "RGBA8SInt",        4,   1, FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::BGRA8UNorm,           "BGRA8UNorm",       4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BGRA8UNormSrgb,       "BGRA8UNormSrgb",   4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGB10A2UNorm,         "RGB10A2UNorm",     4,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RG11B10Float,         "RG11B10Float",     4,   1, FormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::RGB9E5Float,          "RGB9E5Float",      4,   1, FormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::RG32UInt,             "RG32UInt",         8,   1, FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG32SInt,             "RG32SInt",         8,   1, FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG32Float,            "RG32Float",        8,   1, FormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::RGBA16UNorm,          "RGBA16UNorm",      8,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16SNorm,          "RGBA16SNorm",      8,   1, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16UInt,           "RGBA16UInt",       8,   1, FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16SInt,           "RGBA16SInt",       8,   1, FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA16Float,          "RGBA16Float",      8,   1, FormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        { Format::RGB32UInt,            "RGB32UInt",        12,  1, FormatKind::Integer,      true,  true,  true,  false, false, false, false, false },
        { Format::RGB32SInt,            "RGB32SInt",        12,  1, FormatKind::Integer,      true,  true,  true,  false, false, false, true,  false },
        { Format::RGB32Float,           "RGB32Float",       12,  1, FormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::RGBA32UInt,           "RGBA32UInt",       16,  1, FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA32SInt,           "RGBA32SInt",       16,  1, FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA32Float,          "RGBA32Float",      16,  1, FormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        { Format::Depth16UNorm,         "Depth16UNorm",     2,   1, FormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::Depth32Float,         "Depth32Float",     4,   1, FormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::Depth24UNormStencil8, "Depth24UNormStencil8",    4,   1, FormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        { Format::Depth32FloatStencil8, "Depth32FloatStencil8",    8,   1, FormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        //{ Format::X24G8_UINT,        "X24G8_UINT",        4,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        //{ Format::X32G8_UINT,        "X32G8_UINT",        8,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        { Format::BC1UNorm,             "BC1UNorm",         8,   4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC1UNormSrgb,         "BC1UNormSrgb",     8,   4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC2UNorm,             "BC2UNorm",         16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC2UNormSrgb,         "BC2UNormSrgb",     16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC3UNorm,             "BC3UNorm",         16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC3UNormSrgb,         "BC3UNormSrgb",     16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC4UNorm,             "BC4UNorm",         8,   4, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC4SNorm,             "BC4SNorm",         8,   4, FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC5UNorm,             "BC5UNorm",         16,  4, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC5SNorm,             "BC5SNorm",         16,  4, FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC6HUFloat,           "BC6HUFloat",       16,  4, FormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::BC6HSFloat,           "BC6HSFloat",       16,  4, FormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::BC7UNorm,             "BC7UNorm",         16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC7UNormSrgb,         "BC7UNormSrgb",     16,  4, FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
    };

    const FormatInfo& GetFormatInfo(Format format)
    {
        static_assert(sizeof(kFormatDesc) / sizeof(FormatInfo) == size_t(Format::Count),
            "The format info table doesn't have the right number of elements");

        if (uint32_t(format) >= uint32_t(Format::Count))
            return kFormatDesc[0]; // UNKNOWN

        const FormatInfo& info = kFormatDesc[uint32_t(format)];
        ALIMER_ASSERT(info.format == format);
        return info;
    }

    const char* ToString(CompareFunction func)
    {
        switch (func)
        {
            case CompareFunction::Never:        return "Never";
            case CompareFunction::Less:         return "Less";
            case CompareFunction::Equal:        return "Equal";
            case CompareFunction::LessEqual:    return "LessEqual";
            case CompareFunction::Greater:      return "Greater";
            case CompareFunction::NotEqual:     return "NotEqual";
            case CompareFunction::GreaterEqual: return "GreaterEqual";
            case CompareFunction::Always:       return "Always";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    const char* ToString(TextureDimension dimension)
    {
        switch (dimension)
        {
            case TextureDimension::Texture1D:           return "Texture1D";
            case TextureDimension::Texture1DArray:      return "Texture1DArray";
            case TextureDimension::Texture2D:           return "Texture2D";
            case TextureDimension::Texture2DArray:      return "Texture2DArray";
            case TextureDimension::Texture2DMS:         return "Texture2DMS";
            case TextureDimension::Texture2DMSArray:    return "Texture2DMSArray";
            case TextureDimension::TextureCube:         return "TextureCube";
            case TextureDimension::TextureCubeArray:    return "TextureCubeArray";
            case TextureDimension::Texture3D:           return "Texture3D";
            default:                                    return "<INVALID>";
        }
    }

    /* IDevice */
#if defined(ALIMER_RHI_D3D11)
    extern DeviceHandle CreateD3D11Device(Alimer::Window* window, const PresentationParameters& presentationParameters);
#endif

#if defined(ALIMER_RHI_D3D12)
    extern DeviceHandle CreateD3D12Device(Alimer::Window* window, const PresentationParameters& presentationParameters);
#endif

    DeviceHandle IDevice::Create(_In_ Alimer::Window* window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_RHI_D3D11)
        //return CreateD3D11Device(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_D3D12)
        return CreateD3D12Device(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
        //return Vulkan_Initialize(window, presentationParameters);
#endif

        return nullptr;
    }

    TextureHandle IDevice::CreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        if (!VerifyTextureDesc(desc))
        {
            return nullptr;
        }

        return CreateTextureCore(desc, nullptr, initialData);
    }

    TextureHandle IDevice::CreateExternalTexture(void* nativeHandle, const TextureDesc& desc)
    {
        if (!VerifyTextureDesc(desc))
        {
            return nullptr;
        }

        return CreateTextureCore(desc, nativeHandle, nullptr);
    }

    BufferHandle IDevice::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        static constexpr uint64_t c_maxBytes = 128 * 1024u * 1024u;

        if (desc.size > c_maxBytes)
        {
            LOGE("Buffer size too large (size {})", desc.size);
            return nullptr;
        }

        return CreateBufferCore(desc, nullptr, initialData);
    }

    PipelineHandle IDevice::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        RenderPipelineDesc descDef = desc;

        uint32_t autoOffsets[kMaxVertexBufferBindings] = {};

        bool useAutoOffset = true;
        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            // To use computed offsets, all attribute offsets must be 0.
            if (desc.vertexLayout.attributes[index].offset != 0) {
                useAutoOffset = false;
                break;
            }
        }

        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            VertexAttribute* attribute = &descDef.vertexLayout.attributes[index];
            if (attribute->format == Format::Undefined) {
                continue;
            }

            ALIMER_ASSERT(attribute->bufferIndex < kMaxVertexBufferBindings);
            if (useAutoOffset) {
                attribute->offset = autoOffsets[attribute->bufferIndex];
            }
            autoOffsets[attribute->bufferIndex] += GetVertexFormatSize(attribute->format);
        }

        // Compute vertex strides if needed.
        for (uint32_t index = 0; index < kMaxVertexBufferBindings; index++)
        {
            VertexBufferLayout* layout = &descDef.vertexLayout.buffers[index];
            if (layout->stride == 0) {
                layout->stride = autoOffsets[index];
            }
        }

        return CreateRenderPipelineCore(descDef);
    }

    bool IDevice::VerifyTextureDesc(const TextureDesc& desc)
    {
        ALIMER_ASSERT(desc.width >= 1);
        ALIMER_ASSERT(desc.height >= 1);
        ALIMER_ASSERT(desc.depth >= 1);
        ALIMER_ASSERT(desc.arraySize >= 1);
        ALIMER_ASSERT(desc.usage != TextureUsage::None);

        if ((desc.usage & TextureUsage::ShaderWrite) != 0)
        {
            // Check storage support
            //if (!Any(gGraphics().GetCaps().formatProperties[(uint32_t)desc.format].features, PixelFormatFeatures::Storage))
            //{
            //    LOGE("PixelFormat doesn't support shader write");
            //    return nullptr;
            //}

            if (CheckBitsAny(desc.usage, TextureUsage::RenderTarget) && IsDepthStencilFormat(desc.format))
            {
                LOGE("Cannot create DepthStencil texture with ShaderWrite usage");
                return false;
            }
        }

        return true;
    }

    uint32_t GetVertexFormatNumComponents(Format format)
    {
        switch (format)
        {
            case Format::R32Float:
            case Format::R32UInt:
            case Format::R32SInt:
                return 1;

            case Format::RG8UInt:
            case Format::RG8SInt:
            case Format::RG8UNorm:
            case Format::RG8SNorm:
            case Format::RG16UInt:
            case Format::RG16SInt:
            case Format::RG16UNorm:
            case Format::RG16SNorm:
            case Format::RG16Float:
            case Format::RG32Float:
            case Format::RG32UInt:
            case Format::RG32SInt:
                return 2;

            case Format::RGB32UInt:
            case Format::RGB32SInt:
            case Format::RGB32Float:
                return 3;

            case Format::RGBA8UInt:
            case Format::RGBA8SInt:
            case Format::RGBA8UNorm:
            case Format::RGBA8SNorm:
            case Format::RGBA16UInt:
            case Format::RGBA16SInt:
            case Format::RGBA16UNorm:
            case Format::RGBA16SNorm:
            case Format::RGBA16Float:
            case Format::RGBA32Float:
            case Format::RGBA32UInt:
            case Format::RGBA32SInt:
            case Format::RGB10A2UNorm:
                return 4;

            default:
                ALIMER_UNREACHABLE();
                return 0;
        }
    }

    uint32_t GetVertexFormatComponentSize(Format format)
    {
        switch (format)
        {
            case Format::RG8UInt:
            case Format::RGBA8UInt:
            case Format::RG8SInt:
            case Format::RGBA8SInt:
            case Format::RG8UNorm:
            case Format::RGBA8UNorm:
            case Format::RG8SNorm:
            case Format::RGBA8SNorm:
                return sizeof(char);

            case Format::RG16UInt:
            case Format::RGBA16UInt:
            case Format::RG16UNorm:
            case Format::RGBA16UNorm:
            case Format::RG16SInt:
            case Format::RGBA16SInt:
            case Format::RG16SNorm:
            case Format::RGBA16SNorm:
            case Format::RG16Float:
            case Format::RGBA16Float:
                return sizeof(uint16_t);

            case Format::R32Float:
            case Format::RG32Float:
            case Format::RGB32Float:
            case Format::RGBA32Float:
                return sizeof(float);
            case Format::R32UInt:
            case Format::RG32UInt:
            case Format::RGB32UInt:
            case Format::RGBA32UInt:
            case Format::R32SInt:
            case Format::RG32SInt:
            case Format::RGB32SInt:
            case Format::RGBA32SInt:
            case Format::RGB10A2UNorm:
                return sizeof(int32_t);

            default:
                ALIMER_UNREACHABLE();
        }
    }

    uint32_t GetVertexFormatSize(Format format)
    {
        return GetVertexFormatNumComponents(format) * GetVertexFormatComponentSize(format);
    }

    bool StencilTestEnabled(const DepthStencilState* depthStencil)
    {
        return depthStencil->backFace.compare != CompareFunction::Always ||
            depthStencil->backFace.failOp != StencilOperation::Keep ||
            depthStencil->backFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->backFace.passOp != StencilOperation::Keep ||
            depthStencil->frontFace.compare != CompareFunction::Always ||
            depthStencil->frontFace.failOp != StencilOperation::Keep ||
            depthStencil->frontFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->frontFace.passOp != StencilOperation::Keep;
    }

    uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth)
    {
        uint32_t numMips = 0;
        uint32_t size = std::max(std::max(width, height), depth);
        while (1u << numMips <= size)
        {
            ++numMips;
        }

        if (1u << numMips < size)
        {
            ++numMips;
        }

        return numMips;
    }

    const char* ToString(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::Point:  return "Point";
            case SamplerFilter::Linear: return "Linear";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    const char* ToString(SamplerAddressMode mode)
    {
        switch (mode)
        {
            case SamplerAddressMode::Wrap:          return "Wrap";
            case SamplerAddressMode::Mirror:        return "Mirror";
            case SamplerAddressMode::Clamp:         return "Clamp";
            case SamplerAddressMode::Border:        return "Border";
            case SamplerAddressMode::MirrorOnce:    return "MirrorOnce";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    const char* ToString(SamplerBorderColor borderColor)
    {
        switch (borderColor)
        {
            case SamplerBorderColor::TransparentBlack:  return "TransparentBlack";
            case SamplerBorderColor::OpaqueBlack:       return "OpaqueBlack";
            case SamplerBorderColor::OpaqueWhite:       return "OpaqueWhite";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

}
