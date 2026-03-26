// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    /// Enum describing pixel format.
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
        // Packed 16-Bit formats
        B5G6R5Unorm,
        BGR5A1Unorm,
        BGRA4Unorm,
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
        // Packed 32-Bit Pixel Formats
        RGB10A2Unorm,
        RGB10A2Uint,
        RG11B10Float,
        RGB9E5Float,
        // 64-bit formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-bit formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth24UnormStencil8,
        Depth32Float,
        Depth32FloatStencil8,
        Stencil8,
        // BC compressed formats
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
        BC6HRGBUfloat,
        BC6HRGBFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        // ETC2/EAC compressed formats
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
        // ASTC HDR compressed formats
        ASTC4x4HDR,
        ASTC5x4HDR,
        ASTC5x5HDR,
        ASTC6x5HDR,
        ASTC6x6HDR,
        ASTC8x5HDR,
        ASTC8x6HDR,
        ASTC8x8HDR,
        ASTC10x5HDR,
        ASTC10x6HDR,
        ASTC10x8HDR,
        ASTC10x10HDR,
        ASTC12x10HDR,
        ASTC12x12HDR,

        Count
    };

    enum class PixelFormatKind : uint8_t
    {
        /// Unsigned normalized formats
        Unorm,
        /// Unsigned normalized sRGB formats
        UnormSrgb,
        /// Signed normalized formats
        Snorm,
        /// Unsigned integer formats
        Uint,
        /// Unsigned integer formats
        Sint,
        /// Floating-point formats
        Float,
        /// HDR formats.
        HDR,
    };

    struct PixelFormatInfo
    {
        PixelFormat format;
        const char* name;
        uint8_t bytesPerBlock; 
        uint8_t blockWidth;
        uint8_t blockHeight;
        PixelFormatKind kind;
    };
    extern const ALIMER_API PixelFormatInfo kFormatDesc[];

    ALIMER_API const PixelFormatInfo& GetPixelFormatInfo(PixelFormat format);

    /// Get the number of bytes per format.
    ALIMER_API uint32_t GetFormatBytesPerBlock(PixelFormat format);

    /// Check if the format has a depth component
    ALIMER_API bool IsDepthFormat(PixelFormat format);

    /// Check if the format has a stencil component
    ALIMER_API bool IsStencilFormat(PixelFormat format);

    /// Check if the format has depth or stencil components
    ALIMER_API bool IsDepthStencilFormat(PixelFormat format);

    /// Check if the format has a depth only component.
    ALIMER_API bool IsDepthOnlyFormat(PixelFormat format);

    /// Check if the format has a stencil only component.
    ALIMER_API bool IsStencilOnlyFormat(PixelFormat format);

    /// Check if the format is a compressed format.
    ALIMER_API bool IsCompressedFormat(PixelFormat format);

    /// Check if the format is a BC-compressed format.
    ALIMER_API bool IsCompressedBCFormat(PixelFormat format);

    /// Check if the format is a ASTC-compressed format.
    ALIMER_API bool IsCompressedASTCFormat(PixelFormat format);

    /// Get the pixel format kind
    ALIMER_API PixelFormatKind GetPixelFormatKind(PixelFormat format);

    /// Check if a format is an integer type.
    ALIMER_API bool IsIntegerFormat(PixelFormat format);

    /// Check if a format represents sRGB color space
    ALIMER_API bool IsSrgbFormat(PixelFormat format);

    /// Convert an SRGB format to linear. If the format is already linear, will return it
    ALIMER_API PixelFormat SrgbToLinearFormat(PixelFormat format);

    /// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format, will return the original
    ALIMER_API PixelFormat LinearToSrgbFormat(PixelFormat format);

    /// Get format name
    ALIMER_API const char* ToString(PixelFormat format);

    ALIMER_API uint32_t BitsPerPixel(PixelFormat format) noexcept;
    ALIMER_API void GetSurfaceInfo(PixelFormat format, uint32_t width, uint32_t height,
        _Out_opt_ uint32_t* pRowPitch, _Out_opt_ uint32_t* pSlicePitch,
        _Out_opt_ uint32_t* pWidthCount = nullptr, _Out_opt_ uint32_t* pHeightCount = nullptr);

    ALIMER_API uint32_t ToDxgiFormat(PixelFormat format);
    ALIMER_API PixelFormat FromDxgiFormat(uint32_t dxgiFormat);

    ALIMER_API uint32_t ToVkFormat(PixelFormat format);
    ALIMER_API PixelFormat FromVkFormat(uint32_t vkFormat);
}
