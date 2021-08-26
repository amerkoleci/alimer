// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"

namespace alimer
{
    /* Constants */
    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxSimultaneousRenderTargets = 8;
    static constexpr uint32_t kMaxFrameCommandBuffers = 32;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 4;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxUniformBufferBindings = 14;
    static constexpr uint32_t kMaxDescriptorBindings = 32;
    static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;
    static constexpr uint32_t kInvalidBindlessIndex = static_cast<uint32_t>(-1);

    static constexpr uint32_t KnownVendorId_AMD = 0x1002;
    static constexpr uint32_t KnownVendorId_Intel = 0x8086;
    static constexpr uint32_t KnownVendorId_Nvidia = 0x10DE;
    static constexpr uint32_t KnownVendorId_Microsoft = 0x1414;
    static constexpr uint32_t KnownVendorId_ARM = 0x13B5;
    static constexpr uint32_t KnownVendorId_ImgTec = 0x1010;
    static constexpr uint32_t KnownVendorId_Qualcomm = 0x5143;

    /* Enums */
    enum class Format : uint32_t
    {
        Undefined = 0,
        // 8-bit formats
        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,
        // 16-bit formats
        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16Float,
        RG8UNorm,
        RG8SNorm,
        RG8UInt,
        RG8SInt,
        // Packed 16-Bit Pixel Formats
        BGRA4UNorm,
        B5G6R5UNorm,
        B5G5R5A1UNorm,
        // 32-bit formats
        R32UInt,
        R32SInt,
        R32Float,
        RG16UNorm,
        RG16SNorm,
        RG16UInt,
        RG16SInt,
        RG16Float,
        RGBA8UNorm,
        RGBA8UNormSrgb,
        RGBA8SNorm,
        RGBA8UInt,
        RGBA8SInt,
        BGRA8UNorm,
        BGRA8UNormSrgb,
        // Packed 32-Bit formats
        RGB10A2UNorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit formats
        RG32UInt,
        RG32SInt,
        RG32Float,
        RGBA16UNorm,
        RGBA16SNorm,
        RGBA16UInt,
        RGBA16SInt,
        RGBA16Float,
        // 128-Bit formats
        RGBA32UInt,
        RGBA32SInt,
        RGBA32Float,
        // Depth-stencil formats
        Depth16UNorm,
        Depth32Float,
        Depth24UNormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1UNorm,
        BC1UNormSrgb,
        BC2UNorm,
        BC2UNormSrgb,
        BC3UNorm,
        BC3UNormSrgb,
        BC4UNorm,
        BC4SNorm,
        BC5UNorm,
        BC5SNorm,
        BC6HUFloat,
        BC6HSFloat,
        BC7UNorm,
        BC7UNormSrgb,
        Count,
    };

    enum class CompareFunction : uint32_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    /* Helper methods */
    /// Pixel format kind
    enum class PixelFormatKind
    {
        Integer,
        Normalized,
        Float,
        DepthStencil
    };

    struct PixelFormatInfo
    {
        Format format;
        const std::string name;
        uint8_t bytesPerBlock;
        uint8_t blockSize;
        PixelFormatKind kind;
        bool hasRed : 1;
        bool hasGreen : 1;
        bool hasBlue : 1;
        bool hasAlpha : 1;
        bool hasDepth : 1;
        bool hasStencil : 1;
        bool isSigned : 1;
        bool isSRGB : 1;
    };

    ALIMER_API extern const PixelFormatInfo kFormatDesc[];
    ALIMER_API const PixelFormatInfo& GetFormatInfo(Format format);

    /// Get the number of bits per format.
    constexpr uint32_t GetFormatBytesPerBlock(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].bytesPerBlock;
    }

    constexpr uint32_t GetFormatBlockSize(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].blockSize;
    }

    /// Check if the format has a depth component
    constexpr bool IsDepthFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasDepth;
    }

    /// Check if the format has a stencil component
    constexpr bool IsStencilFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasStencil;
    }

    /// Check if the format has depth or stencil components
    constexpr bool IsDepthStencilFormat(Format format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    /// Check if the format is a compressed format
    constexpr bool IsBlockCompressedFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);

        switch (format)
        {
        case Format::BC1UNorm:
        case Format::BC1UNormSrgb:
        case Format::BC2UNorm:
        case Format::BC2UNormSrgb:
        case Format::BC3UNorm:
        case Format::BC3UNormSrgb:
        case Format::BC4UNorm:
        case Format::BC4SNorm:
        case Format::BC5UNorm:
        case Format::BC5SNorm:
        case Format::BC6HUFloat:
        case Format::BC6HSFloat:
        case Format::BC7UNorm:
        case Format::BC7UNormSrgb:
            return true;
        }

        return false;
    }

    /// Get the format Type
    constexpr PixelFormatKind GetFormatKind(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].kind;
    }

    constexpr const std::string& ToString(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].name;
    }

    /// Check if a format represents sRGB color space.
    constexpr bool IsSrgbFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].isSRGB;
    }

    /// Convert a SRGB format to linear. If the format is already linear no conversion will be made.
    constexpr Format SRGBToLinearFormat(Format format)
    {
        switch (format)
        {
        case Format::BC1UNormSrgb:
            return Format::BC1UNorm;
        case Format::BC2UNormSrgb:
            return Format::BC2UNorm;
        case Format::BC3UNormSrgb:
            return Format::BC3UNorm;
        case Format::BGRA8UNormSrgb:
            return Format::BGRA8UNorm;
        case Format::RGBA8UNormSrgb:
            return Format::RGBA8UNorm;
        case Format::BC7UNormSrgb:
            return Format::BC7UNorm;
        default:
            ALIMER_ASSERT(IsSrgbFormat(format) == false);
            return format;
        }
    }

    /// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format no conversion will be made.
    constexpr Format LinearToSRGBFormat(Format format)
    {
        switch (format)
        {
        case Format::BC1UNorm:
            return Format::BC1UNormSrgb;
        case Format::BC2UNorm:
            return Format::BC2UNormSrgb;
        case Format::BC3UNorm:
            return Format::BC3UNormSrgb;
        case Format::BGRA8UNorm:
            return Format::BGRA8UNormSrgb;
        case Format::RGBA8UNorm:
            return Format::RGBA8UNormSrgb;
        case Format::BC7UNorm:
            return Format::BC7UNormSrgb;
        default:
            return format;
        }
    }

    ALIMER_API const char* ToString(CompareFunction func);

    inline const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
        case KnownVendorId_AMD:
            return "AMD";
        case KnownVendorId_ImgTec:
            return "IMAGINATION";
        case KnownVendorId_Nvidia:
            return "Nvidia";
        case KnownVendorId_ARM:
            return "ARM";
        case KnownVendorId_Qualcomm:
            return "Qualcom";
        case KnownVendorId_Intel:
            return "Intel";
        default:
            return "Unknown";
        }
    }
}

#undef RHI_ENUM_CLASS_FLAG_OPERATORS

namespace std
{
#if TODO
    template<> struct hash<Alimer::RHI::TextureSubresourceSet>
    {
        std::size_t operator()(const Alimer::RHI::TextureSubresourceSet& set) const noexcept
        {
            size_t hash = 0;
            Alimer::RHI::hash_combine(hash, set.baseMipLevel);
            Alimer::RHI::HashCombine(hash, set.numMipLevels);
            Alimer::RHI::HashCombine(hash, set.baseArraySlice);
            Alimer::RHI::HashCombine(hash, set.numArraySlices);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::StencilFaceState>
    {
        std::size_t operator()(const Alimer::RHI::StencilFaceState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, (uint32_t)state.failOp);
            Alimer::HashCombine(hash, (uint32_t)state.passOp);
            Alimer::HashCombine(hash, (uint32_t)state.depthFailOp);
            Alimer::HashCombine(hash, (uint32_t)state.compare);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::DepthStencilState>
    {
        std::size_t operator()(const Alimer::RHI::DepthStencilState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.depthWriteEnabled);
            Alimer::HashCombine(hash, (uint32_t)state.depthCompare);
            Alimer::HashCombine(hash, state.frontFace);
            Alimer::HashCombine(hash, state.backFace);
            Alimer::HashCombine(hash, state.stencilReadMask);
            Alimer::HashCombine(hash, state.stencilWriteMask);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::RenderTargetBlendState>
    {
        std::size_t operator()(const Alimer::RHI::RenderTargetBlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.blendEnable);
            Alimer::HashCombine(hash, (uint32_t)state.srcBlend);
            Alimer::HashCombine(hash, (uint32_t)state.destBlend);
            Alimer::HashCombine(hash, (uint32_t)state.blendOp);
            Alimer::HashCombine(hash, (uint32_t)state.srcBlendAlpha);
            Alimer::HashCombine(hash, (uint32_t)state.destBlendAlpha);
            Alimer::HashCombine(hash, (uint32_t)state.blendOpAlpha);
            Alimer::HashCombine(hash, (uint8_t)state.writeMask);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::BlendState>
    {
        std::size_t operator()(const Alimer::RHI::BlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.alphaToCoverageEnable);
            Alimer::HashCombine(hash, state.independentBlendEnable);
            for (const auto& target : state.renderTargets)
            {
                Alimer::HashCombine(hash, target);
            }
            return hash;
        }
    };
#endif // TODO
}

