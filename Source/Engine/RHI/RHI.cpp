// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"
#include "Window.h"

namespace alimer::rhi
{
    const FormatInfo kFormatDesc[] = {
        //        format                    name                bytes blk         kind               red   green   blue  alpha  depth  stencl signed  srgb
        { Format::Undefined,           "Undefined",        0,   0, PixelFormatKind::Integer,      false, false, false, false, false, false, false, false },
        { Format::R8UNorm,             "R8UNorm",          1,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R8SNorm,             "R8SNorm",          1,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R8UInt,              "R8UInt",           1,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R8SInt,              "R8SInt",           1,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R16UNorm,            "R16UNorm",         2,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16SNorm,            "R16SNorm",         2,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::R16UInt,             "R16UInt",          2,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R16SInt,             "R16SInt",          2,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R16Float,            "R16Float",         2,   1, PixelFormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { Format::RG8UNorm,            "RG8UNorm",         2,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG8SNorm,            "RG8SNorm",         2,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG8UInt,             "RG8UInt",          2,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG8SInt,             "RG8SInt",          2,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::BGRA4UNorm,          "BGRA4UNorm",       2,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::B5G6R5UNorm,         "B5G6R5UNorm",      2,   1, PixelFormatKind::Normalized,   true,  true,  true,  false, false, false, false, false },
        { Format::B5G5R5A1UNorm,       "B5G5R5A1UNorm",    2,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::R32UInt,             "R32UInt",          4,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { Format::R32SInt,             "R32SInt",          4,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { Format::R32Float,            "R32Float",         4,   1, PixelFormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { Format::RG16UNorm,           "RG16UNorm",        4,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16SNorm,           "RG16SNorm",        4,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::RG16UInt,            "RG16UInt",         4,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG16SInt,            "RG16SInt",         4,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG16Float,           "RG16Float",        4,   1, PixelFormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::RGBA8UNorm,          "RGBA8UNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8UNormSrgb,      "RGBA8UNormSrgb",   4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::RGBA8SNorm,          "RGBA8SNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8UInt,           "RGBA8UInt",        4,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA8SInt,           "RGBA8SInt",        4,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::BGRA8UNorm,          "BGRA8UNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BGRA8UNormSrgb,      "BGRA8UNormSrgb",   4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGB10A2UNorm,        "RGB10A2UNorm",     4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RG11B10Float,        "RG11B10Float",     4,   1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::RGB9E5Float,         "RGB9E5Float",      4,   1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::RG32UInt,            "RG32UInt",         8,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { Format::RG32SInt,            "RG32SInt",         8,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { Format::RG32Float,           "RG32Float",        8,   1, PixelFormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { Format::RGBA16UNorm,         "RGBA16UNorm",      8,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16SNorm,         "RGBA16SNorm",      8,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16UInt,          "RGBA16UInt",       8,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA16SInt,          "RGBA16SInt",       8,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA16Float,         "RGBA16Float",      8,   1, PixelFormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        //{ Format::RGB32_UINT,        "RGB32_UINT",        12,  1, PixelFormatKind::Integer,      true,  true,  true,  false, false, false, false, false },
        //{ Format::RGB32_SINT,        "RGB32_SINT",        12,  1, PixelFormatKind::Integer,      true,  true,  true,  false, false, false, true,  false },
        //{ Format::RGB32_FLOAT,       "RGB32_FLOAT",       12,  1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::RGBA32UInt,          "RGBA32UInt",       16,  1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { Format::RGBA32SInt,          "RGBA32SInt",       16,  1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { Format::RGBA32Float,         "RGBA32Float",      16,  1, PixelFormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        { Format::Depth16UNorm,        "Depth16UNorm",     2,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::Depth32Float,        "Depth32Float",     4,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { Format::Depth24UNormStencil8, "Depth24UNormStencil8",    4,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        { Format::Depth32FloatStencil8, "Depth32FloatStencil8",    8,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        //{ Format::X24G8_UINT,        "X24G8_UINT",        4,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        //{ Format::X32G8_UINT,        "X32G8_UINT",        8,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        { Format::BC1UNorm,            "BC1UNorm",         8,   4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC1UNormSrgb,        "BC1UNormSrgb",     8,   4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC2UNorm,            "BC2UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC2UNormSrgb,        "BC2UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC3UNorm,            "BC3UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC3UNormSrgb,        "BC3UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { Format::BC4UNorm,            "BC4UNorm",         8,   4, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC4SNorm,            "BC4SNorm",         8,   4, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { Format::BC5UNorm,            "BC5UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC5SNorm,            "BC5SNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { Format::BC6HUFloat,          "BC6HUFloat",       16,  4, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { Format::BC6HSFloat,          "BC6HSFloat",       16,  4, PixelFormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { Format::BC7UNorm,            "BC7UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { Format::BC7UNormSrgb,        "BC7UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
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
#if defined(ALIMER_RHI_D3D12)
    extern DeviceHandle CreateD3D12Device(alimer::Window* window, const PresentationParameters& presentationParameters);
#endif

    DeviceHandle IDevice::Create(_In_ alimer::Window* window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_RHI_D3D12)
        return CreateD3D12Device(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
        //return Vulkan_Initialize(window, presentationParameters);
#endif

        return nullptr;
    }

#if TODO
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
#endif // TODO

}
