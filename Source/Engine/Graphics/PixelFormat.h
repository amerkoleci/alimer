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
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        // 16-bit formats
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        // Packed 16-Bit Pixel Formats
        BGRA4Unorm,
        B5G6R5Unorm,
        B5G5R5A1Unorm,
        // 32-bit formats
        R32Uint,
        R32Sint,
        R32Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-Bit formats
        RGB10A2Unorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-Bit formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth32Float,
        Depth24UnormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBFloat,
        BC6HRGBUFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        // EAC/ETC compressed formats
        ETC2RGB8Unorm,
        ETC2RGB8UnormSrgb,
        ETC2RGB8A1Unorm,
        ETC2RGB8A1UnormSrgb,
        ETC2RGBA8Unorm,
        ETC2RGBA8UnormSrgb,
        EACR11Unorm,
        EACR11Snorm,
        EACRG11Unorm,
        EACRG11Snorm,
        // ASTC compressed formats
        ASTC4x4Unorm,
        ASTC4x4UnormSrgb,
        ASTC5x4Unorm,
        ASTC5x4UnormSrgb,
        ASTC5x5Unorm,
        ASTC5x5UnormSrgb,
        ASTC6x5Unorm,
        ASTC6x5UnormSrgb,
        ASTC6x6Unorm,
        ASTC6x6UnormSrgb,
        ASTC8x5Unorm,
        ASTC8x5UnormSrgb,
        ASTC8x6Unorm,
        ASTC8x6UnormSrgb,
        ASTC8x8Unorm,
        ASTC8x8UnormSrgb,
        ASTC10x5Unorm,
        ASTC10x5UnormSrgb,
        ASTC10x6Unorm,
        ASTC10x6UnormSrgb,
        ASTC10x8Unorm,
        ASTC10x8UnormSrgb,
        ASTC10x10Unorm,
        ASTC10x10UnormSrgb,
        ASTC12x10Unorm,
        ASTC12x10UnormSrgb,
        ASTC12x12Unorm,
        ASTC12x12UnormSrgb,
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
        case PixelFormat::BC1RGBAUnorm:
        case PixelFormat::BC1RGBAUnormSrgb:
        case PixelFormat::BC2RGBAUnorm:
        case PixelFormat::BC2RGBAUnormSrgb:
        case PixelFormat::BC3RGBAUnorm:
        case PixelFormat::BC3RGBAUnormSrgb:
        case PixelFormat::BC4RUnorm:
        case PixelFormat::BC4RSnorm:
        case PixelFormat::BC5RGUnorm:
        case PixelFormat::BC5RGSnorm:
        case PixelFormat::BC6HRGBFloat:
        case PixelFormat::BC6HRGBUFloat:
        case PixelFormat::BC7RGBAUnorm:
        case PixelFormat::BC7RGBAUnormSrgb:
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
        case PixelFormat::BC1RGBAUnormSrgb:
            return PixelFormat::BC1RGBAUnorm;
        case PixelFormat::BC2RGBAUnormSrgb:
            return PixelFormat::BC2RGBAUnorm;
        case PixelFormat::BC3RGBAUnormSrgb:
            return PixelFormat::BC3RGBAUnorm;
        case PixelFormat::BGRA8UnormSrgb:
            return PixelFormat::BGRA8Unorm;
        case PixelFormat::RGBA8UnormSrgb:
            return PixelFormat::RGBA8Unorm;
        case PixelFormat::BC7RGBAUnormSrgb:
            return PixelFormat::BC7RGBAUnorm;
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
        case PixelFormat::BC1RGBAUnorm:
            return PixelFormat::BC1RGBAUnormSrgb;
        case PixelFormat::BC2RGBAUnorm:
            return PixelFormat::BC2RGBAUnormSrgb;
        case PixelFormat::BC3RGBAUnorm:
            return PixelFormat::BC3RGBAUnormSrgb;
        case PixelFormat::BGRA8Unorm:
            return PixelFormat::BGRA8UnormSrgb;
        case PixelFormat::RGBA8Unorm:
            return PixelFormat::RGBA8UnormSrgb;
        case PixelFormat::BC7RGBAUnorm:
            return PixelFormat::BC7RGBAUnormSrgb;
        default:
            return format;
        }
    }
}
