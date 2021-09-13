// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"

namespace Alimer
{
    /// Defines pixel format.
    enum class PixelFormat : uint32_t
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
        Count
    };

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
        PixelFormat format;
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
    ALIMER_API const PixelFormatInfo& GetFormatInfo(PixelFormat format);

    /// Get the number of bits per format.
    constexpr uint32_t GetFormatBytesPerBlock(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].bytesPerBlock;
    }

    constexpr uint32_t GetFormatBlockSize(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].blockSize;
    }

    /// Check if the format has a depth component
    constexpr bool IsDepthFormat(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasDepth;
    }

    /// Check if the format has a stencil component
    constexpr bool IsStencilFormat(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasStencil;
    }

    /// Check if the format has depth or stencil components
    constexpr bool IsDepthStencilFormat(PixelFormat format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    /// Check if the format is a compressed format
    constexpr bool IsBlockCompressedFormat(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);

        switch (format)
        {
        case PixelFormat::BC1UNorm:
        case PixelFormat::BC1UNormSrgb:
        case PixelFormat::BC2UNorm:
        case PixelFormat::BC2UNormSrgb:
        case PixelFormat::BC3UNorm:
        case PixelFormat::BC3UNormSrgb:
        case PixelFormat::BC4UNorm:
        case PixelFormat::BC4SNorm:
        case PixelFormat::BC5UNorm:
        case PixelFormat::BC5SNorm:
        case PixelFormat::BC6HUFloat:
        case PixelFormat::BC6HSFloat:
        case PixelFormat::BC7UNorm:
        case PixelFormat::BC7UNormSrgb:
            return true;
        }

        return false;
    }

    /// Get the format Type
    constexpr PixelFormatKind GetFormatKind(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].kind;
    }

    constexpr const std::string& ToString(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].name;
    }

    /// Check if a format represents sRGB color space.
    constexpr bool IsSrgbFormat(PixelFormat format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].isSRGB;
    }

    /// Convert a SRGB format to linear. If the format is already linear no conversion will be made.
    constexpr PixelFormat SRGBToLinearFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::BC1UNormSrgb:
            return PixelFormat::BC1UNorm;
        case PixelFormat::BC2UNormSrgb:
            return PixelFormat::BC2UNorm;
        case PixelFormat::BC3UNormSrgb:
            return PixelFormat::BC3UNorm;
        case PixelFormat::BGRA8UNormSrgb:
            return PixelFormat::BGRA8UNorm;
        case PixelFormat::RGBA8UNormSrgb:
            return PixelFormat::RGBA8UNorm;
        case PixelFormat::BC7UNormSrgb:
            return PixelFormat::BC7UNorm;
        default:
            ALIMER_ASSERT(IsSrgbFormat(format) == false);
            return format;
        }
    }

    /// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format no conversion will be made.
    constexpr PixelFormat LinearToSRGBFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::BC1UNorm:
            return PixelFormat::BC1UNormSrgb;
        case PixelFormat::BC2UNorm:
            return PixelFormat::BC2UNormSrgb;
        case PixelFormat::BC3UNorm:
            return PixelFormat::BC3UNormSrgb;
        case PixelFormat::BGRA8UNorm:
            return PixelFormat::BGRA8UNormSrgb;
        case PixelFormat::RGBA8UNorm:
            return PixelFormat::RGBA8UNormSrgb;
        case PixelFormat::BC7UNorm:
            return PixelFormat::BC7UNormSrgb;
        default:
            return format;
        }
    }
}
