// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct PixelFormatInfo(
    PixelFormat Format,
    uint BytesPerBlock,
    uint BlockWidth,
    uint BlockHeight,
    FormatKind Kind
    );

public static class PixelFormatUtils
{
    private static readonly PixelFormatInfo[] s_formatInfos =
    [
        new(PixelFormat.Undefined,              0, 0, 0, FormatKind.Unorm),
        // 8-bit pixel formats
        new(PixelFormat.R8Unorm,                1, 1, 1, FormatKind.Unorm),
        new(PixelFormat.R8Snorm,                1, 1, 1, FormatKind.Snorm),
        new(PixelFormat.R8Uint,                 1, 1, 1, FormatKind.Uint),
        new(PixelFormat.R8Sint,                 1, 1, 1, FormatKind.Sint),
        // 16-bit pixel formats
        new(PixelFormat.R16Unorm,               2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.R16Snorm,               2, 1, 1, FormatKind.Snorm),
        new(PixelFormat.R16Uint,                2, 1, 1, FormatKind.Uint),
        new(PixelFormat.R16Sint,                2, 1, 1, FormatKind.Sint),
        new(PixelFormat.R16Float,               2, 1, 1, FormatKind.Float),
        new(PixelFormat.RG8Unorm,               2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.RG8Snorm,               2, 1, 1, FormatKind.Snorm),
        new(PixelFormat.RG8Uint,                2, 1, 1, FormatKind.Uint),
        new(PixelFormat.RG8Sint,                2, 1, 1, FormatKind.Sint),
        // Packed 16-Bit Pixel Formats
        new(PixelFormat.BGRA4Unorm,             2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.B5G6R5Unorm,            2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.BGR5A1Unorm,            2, 1, 1, FormatKind.Unorm),
        // 32-bit pixel formats
        new(PixelFormat.R32Uint,                4, 1, 1, FormatKind.Uint),
        new(PixelFormat.R32Sint,                4, 1, 1, FormatKind.Sint),
        new(PixelFormat.R32Float,               4, 1, 1, FormatKind.Float),
        new(PixelFormat.RG16Unorm,              4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.RG16Snorm,              4, 1, 1, FormatKind.Snorm),
        new(PixelFormat.RG16Uint,               4, 1, 1, FormatKind.Uint),
        new(PixelFormat.RG16Sint,               4, 1, 1, FormatKind.Sint),
        new(PixelFormat.RG16Float,              4, 1, 1, FormatKind.Float),
        new(PixelFormat.RGBA8Unorm,             4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.RGBA8UnormSrgb,         4, 1, 1, FormatKind.UnormSrgb),
        new(PixelFormat.RGBA8Snorm,             4, 1, 1, FormatKind.Snorm),
        new(PixelFormat.RGBA8Uint,              4, 1, 1, FormatKind.Uint),
        new(PixelFormat.RGBA8Sint,              4, 1, 1, FormatKind.Uint),
        new(PixelFormat.BGRA8Unorm,             4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.BGRA8UnormSrgb,         4, 1, 1, FormatKind.UnormSrgb),
        // Packed 32-Bit Pixel formats
        new(PixelFormat.RGB10A2Unorm,           4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.RGB10A2Uint,            4, 1, 1, FormatKind.Uint),
        new(PixelFormat.RG11B10Float,           4, 1, 1, FormatKind.Float),
        new(PixelFormat.RGB9E5Float,            4, 1, 1, FormatKind.Float),
        // 64-Bit Pixel Formats
        new(PixelFormat.RG32Uint,               8, 1, 1, FormatKind.Uint),
        new(PixelFormat.RG32Sint,               8, 1, 1, FormatKind.Sint),
        new(PixelFormat.RG32Float,              8, 1, 1, FormatKind.Float),
        new(PixelFormat.RGBA16Unorm,            8, 1, 1, FormatKind.Unorm),
        new(PixelFormat.RGBA16Snorm,            8, 1, 1, FormatKind.Snorm),
        new(PixelFormat.RGBA16Uint,             8, 1, 1, FormatKind.Uint),
        new(PixelFormat.RGBA16Sint,             8, 1, 1, FormatKind.Sint),
        new(PixelFormat.RGBA16Float,            8, 1, 1, FormatKind.Float),
        // 128-Bit Pixel Formats
        new(PixelFormat.Rgba32Uint,            16, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rgba32Sint,            16, 1, 1, FormatKind.Sint),
        new(PixelFormat.Rgba32Float,           16, 1, 1, FormatKind.Float),
        // Depth-stencil formats
        new(PixelFormat.Depth16Unorm,          2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Depth24UnormStencil8,  4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Depth32Float,          4, 1, 1, FormatKind.Float),
        new(PixelFormat.Depth32FloatStencil8,  8, 1, 1, FormatKind.Float),
        // BC compressed formats
        new(PixelFormat.Bc1RgbaUnorm,          8, 4, 4,  FormatKind.Unorm),
        new(PixelFormat.Bc1RgbaUnormSrgb,      8, 4, 4,  FormatKind.UnormSrgb),
        new(PixelFormat.Bc2RgbaUnorm,          16, 4, 4, FormatKind.Unorm),
        new(PixelFormat.Bc2RgbaUnormSrgb,      16, 4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Bc3RgbaUnorm,          16, 4, 4, FormatKind.Unorm),
        new(PixelFormat.Bc3RgbaUnormSrgb,      16, 4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Bc4RUnorm,             8,  4, 4, FormatKind.Unorm),
        new(PixelFormat.Bc4RSnorm,             8,  4, 4, FormatKind.Snorm),
        new(PixelFormat.Bc5RgUnorm,            16, 4, 4, FormatKind.Unorm),
        new(PixelFormat.Bc5RgSnorm,            16, 4, 4, FormatKind.Snorm),
        new(PixelFormat.Bc6hRgbUfloat,         16, 4, 4, FormatKind.Float),
        new(PixelFormat.Bc6hRgbFloat,          16, 4, 4, FormatKind.Float),
        new(PixelFormat.Bc7RgbaUnorm,          16, 4, 4, FormatKind.Unorm),
        new(PixelFormat.Bc7RgbaUnormSrgb,      16, 4, 4, FormatKind.UnormSrgb),
        // ETC2/EAC compressed formats
        new(PixelFormat.Etc2Rgb8Unorm,        8,   4, 4, FormatKind.Unorm),
        new(PixelFormat.Etc2Rgb8UnormSrgb,    8,   4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Etc2Rgb8A1Unorm,     16,   4, 4, FormatKind.Unorm),
        new(PixelFormat.Etc2Rgb8A1UnormSrgb, 16,   4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Etc2Rgba8Unorm,      16,   4, 4, FormatKind.Unorm),
        new(PixelFormat.Etc2Rgba8UnormSrgb,  16,   4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.EacR11Unorm,         8,    4, 4, FormatKind.Unorm),
        new(PixelFormat.EacR11Snorm,         8,    4, 4, FormatKind.Snorm),
        new(PixelFormat.EacRg11Unorm,        16,   4, 4, FormatKind.Unorm),
        new(PixelFormat.EacRg11Snorm,        16,   4, 4, FormatKind.Snorm),

        // ASTC compressed formats
        new(PixelFormat.Astc4x4Unorm,        16,   4, 4, FormatKind.Unorm),
        new(PixelFormat.Astc4x4UnormSrgb,    16,   4, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Astc5x4Unorm,        16,   5, 4, FormatKind.Unorm),
        new(PixelFormat.Astc5x4UnormSrgb,    16,   5, 4, FormatKind.UnormSrgb),
        new(PixelFormat.Astc5x5Unorm,        16,   5, 5, FormatKind.Unorm),
        new(PixelFormat.Astc5x5UnormSrgb,    16,   5, 5, FormatKind.UnormSrgb),
        new(PixelFormat.Astc6x5Unorm,        16,   6, 5, FormatKind.Unorm),
        new(PixelFormat.Astc6x5UnormSrgb,    16,   6, 5, FormatKind.UnormSrgb),
        new(PixelFormat.Astc6x6Unorm,        16,   6, 6, FormatKind.Unorm),
        new(PixelFormat.Astc6x6UnormSrgb,    16,   6, 6, FormatKind.UnormSrgb),
        new(PixelFormat.Astc8x5Unorm,        16,   8, 5, FormatKind.Unorm),
        new(PixelFormat.Astc8x5UnormSrgb,    16,   8, 5, FormatKind.UnormSrgb),
        new(PixelFormat.Astc8x6Unorm,        16,   8, 6, FormatKind.Unorm),
        new(PixelFormat.Astc8x6UnormSrgb,    16,   8, 6, FormatKind.UnormSrgb),
        new(PixelFormat.Astc8x8Unorm,        16,   8, 8, FormatKind.Unorm),
        new(PixelFormat.Astc8x8UnormSrgb,    16,   8, 8, FormatKind.UnormSrgb),
        new(PixelFormat.Astc10x5Unorm,       16,   10, 5, FormatKind.Unorm),
        new(PixelFormat.Astc10x5UnormSrgb,   16,   10, 5, FormatKind.UnormSrgb),
        new(PixelFormat.Astc10x6Unorm,       16,   10, 6, FormatKind.Unorm),
        new(PixelFormat.Astc10x6UnormSrgb,   16,   10, 6, FormatKind.UnormSrgb ),
        new(PixelFormat.Astc10x8Unorm,       16,   10, 8, FormatKind.Unorm),
        new(PixelFormat.Astc10x8UnormSrgb,   16,   10, 8, FormatKind.UnormSrgb),
        new(PixelFormat.Astc10x10Unorm,      16,   10, 10, FormatKind.Unorm ),
        new(PixelFormat.Astc10x10UnormSrgb,  16,   10, 10, FormatKind.UnormSrgb),
        new(PixelFormat.Astc12x10Unorm,      16,   12, 10, FormatKind.Unorm),
        new(PixelFormat.Astc12x10UnormSrgb,  16,   12, 10, FormatKind.UnormSrgb),
        new(PixelFormat.Astc12x12Unorm,      16,   12, 12, FormatKind.Unorm),
        new(PixelFormat.Astc12x12UnormSrgb,  16,   12, 12, FormatKind.UnormSrgb),
        // ASTC HDR compressed formats
        new(PixelFormat.Astc4x4Hdr,             16,   4, 4, FormatKind.Hdr),
        new(PixelFormat.Astc5x4Hdr,             16,   5, 4, FormatKind.Hdr),
        new(PixelFormat.Astc5x5Hdr,             16,   5, 5, FormatKind.Hdr),
        new(PixelFormat.Astc6x5Hdr,             16,   6, 5, FormatKind.Hdr),
        new(PixelFormat.Astc6x6Hdr,             16,   6, 6, FormatKind.Hdr),
        new(PixelFormat.Astc8x5Hdr,             16,   8, 5, FormatKind.Hdr),
        new(PixelFormat.Astc8x6Hdr,             16,   8, 6, FormatKind.Hdr),
        new(PixelFormat.Astc8x8Hdr,             16,   8, 6, FormatKind.Hdr),
        new(PixelFormat.Astc10x5Hdr,            16,   10, 5, FormatKind.Hdr),
        new(PixelFormat.Astc10x6Hdr,            16,   10, 6, FormatKind.Hdr),
        new(PixelFormat.Astc10x8Hdr,            16,   10, 8, FormatKind.Hdr),
        new(PixelFormat.Astc10x10Hdr,           16,   10, 10, FormatKind.Hdr),
        new(PixelFormat.Astc12x10Hdr,           16,   12, 10, FormatKind.Hdr),
        new(PixelFormat.Astc12x12Hdr,           16,   12, 12, FormatKind.Hdr),
    ];

    public static ref readonly PixelFormatInfo GetFormatInfo(this PixelFormat format)
    {
        if (format >= PixelFormat.Count)
        {
            return ref s_formatInfos[0]; // UNKNOWN
        }

        Guard.IsTrue(s_formatInfos[(int)format].Format == format);
        return ref s_formatInfos[(int)format];
    }

    /// <summary>
    /// Get the number of bytes per format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatBytesPerBlock(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(uint)format].BytesPerBlock;
    }

    public static uint GetFormatPixelsPerBlock(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(uint)format].BlockWidth * s_formatInfos[(uint)format].BlockHeight;
    }

    public static FormatKind GetKind(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(uint)format].Kind;
    }

    public static bool IsInteger(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].Kind == FormatKind.Uint || s_formatInfos[(int)format].Kind == FormatKind.Sint;
    }

    public static bool IsSigned(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].Kind == FormatKind.Sint;
    }

    public static bool IsSrgb(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].Kind == FormatKind.UnormSrgb;
    }

    /// <summary>
    /// Check if the format is a compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsCompressedFormat(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].BlockWidth > 1;
    }

    /// <summary>
    /// Check if the format is a BC compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsBcCompressedFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.Bc1RgbaUnorm:
            case PixelFormat.Bc1RgbaUnormSrgb:
            case PixelFormat.Bc2RgbaUnorm:
            case PixelFormat.Bc2RgbaUnormSrgb:
            case PixelFormat.Bc3RgbaUnorm:
            case PixelFormat.Bc3RgbaUnormSrgb:
            case PixelFormat.Bc4RUnorm:
            case PixelFormat.Bc4RSnorm:
            case PixelFormat.Bc5RgUnorm:
            case PixelFormat.Bc5RgSnorm:
            case PixelFormat.Bc6hRgbUfloat:
            case PixelFormat.Bc6hRgbFloat:
            case PixelFormat.Bc7RgbaUnorm:
            case PixelFormat.Bc7RgbaUnormSrgb:
                return true;
            default:
                return false;
        }
    }

    /// <summary>
    /// Check if the format is a ETC compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsEtcCompressedFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.Etc2Rgb8Unorm:
            case PixelFormat.Etc2Rgb8UnormSrgb:
            case PixelFormat.Etc2Rgb8A1Unorm:
            case PixelFormat.Etc2Rgb8A1UnormSrgb:
            case PixelFormat.Etc2Rgba8Unorm:
            case PixelFormat.Etc2Rgba8UnormSrgb:
                return true;
            default:
                return false;
        }
    }

    /// <summary>
    /// Check if the format is a EAC compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsEacCompressedFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.EacR11Unorm:
            case PixelFormat.EacR11Snorm:
            case PixelFormat.EacRg11Unorm:
            case PixelFormat.EacRg11Snorm:
                return true;
            default:
                return false;
        }
    }

    /// <summary>
    /// Check if the format is a ASTC compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsAstcCompressedFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.Astc4x4Unorm:
            case PixelFormat.Astc4x4UnormSrgb:
            case PixelFormat.Astc5x4Unorm:
            case PixelFormat.Astc5x4UnormSrgb:
            case PixelFormat.Astc5x5Unorm:
            case PixelFormat.Astc5x5UnormSrgb:
            case PixelFormat.Astc6x5Unorm:
            case PixelFormat.Astc6x5UnormSrgb:
            case PixelFormat.Astc6x6Unorm:
            case PixelFormat.Astc6x6UnormSrgb:
            case PixelFormat.Astc8x5Unorm:
            case PixelFormat.Astc8x5UnormSrgb:
            case PixelFormat.Astc8x6Unorm:
            case PixelFormat.Astc8x6UnormSrgb:
            case PixelFormat.Astc8x8Unorm:
            case PixelFormat.Astc8x8UnormSrgb:
            case PixelFormat.Astc10x5Unorm:
            case PixelFormat.Astc10x5UnormSrgb:
            case PixelFormat.Astc10x6Unorm:
            case PixelFormat.Astc10x6UnormSrgb:
            case PixelFormat.Astc10x8Unorm:
            case PixelFormat.Astc10x8UnormSrgb:
            case PixelFormat.Astc10x10Unorm:
            case PixelFormat.Astc10x10UnormSrgb:
            case PixelFormat.Astc12x10Unorm:
            case PixelFormat.Astc12x10UnormSrgb:
            case PixelFormat.Astc12x12Unorm:
                return true;

            case PixelFormat.Astc4x4Hdr:
            case PixelFormat.Astc5x4Hdr:
            case PixelFormat.Astc5x5Hdr:
            case PixelFormat.Astc6x5Hdr:
            case PixelFormat.Astc6x6Hdr:
            case PixelFormat.Astc8x5Hdr:
            case PixelFormat.Astc8x6Hdr:
            case PixelFormat.Astc8x8Hdr:
            case PixelFormat.Astc10x5Hdr:
            case PixelFormat.Astc10x6Hdr:
            case PixelFormat.Astc10x8Hdr:
            case PixelFormat.Astc10x10Hdr:
            case PixelFormat.Astc12x10Hdr:
            case PixelFormat.Astc12x12Hdr:
                return true;

            default:
                return false;
        }
    }

    /// <summary>
    /// Get the format compression ration along the x-axis.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatWidthCompressionRatio(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].BlockWidth;
    }

    /// <summary>
    /// Get the format compression ration along the y-axis.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatHeightCompressionRatio(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);

        return s_formatInfos[(int)format].BlockHeight;
    }

    /// <summary>
    /// Check if the format has a depth component.
    /// </summary>
    /// <param name="format">The <see cref="PixelFormat"/> to check.</param>
    /// <returns>True if format has depth component, false otherwise.</returns>
    public static bool IsDepthFormat(this PixelFormat format)
    {
        return format switch
        {
            PixelFormat.Depth16Unorm or PixelFormat.Depth24UnormStencil8 or PixelFormat.Depth32Float or PixelFormat.Depth32FloatStencil8 => true,
            _ => false,
        };
    }

    /// <summary>
    /// Check if the format has a stencil component.
    /// </summary>
    /// <param name="format">The <see cref="PixelFormat"/> to check.</param>
    /// <returns>True if format has stencil component, false otherwise.</returns>
    public static bool IsStencilFormat(this PixelFormat format)
    {
        return format switch
        {
            PixelFormat.Depth24UnormStencil8 or PixelFormat.Depth32FloatStencil8 => true,
            _ => false,
        };
    }

    /// <summary>
    /// Check if the format has depth or stencil components.
    /// </summary>
    /// <param name="format">The <see cref="PixelFormat"/> to check.</param>
    /// <returns>True if format has depth or stencil component, false otherwise.</returns>
    public static bool IsDepthStencilFormat(this PixelFormat format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    /// <summary>
    /// Check if the format has depth only components.
    /// </summary>
    /// <param name="format">The <see cref="PixelFormat"/> to check.</param>
    /// <returns>True if format has depth or stencil component, false otherwise.</returns>
    public static bool IsDepthOnlyFormat(this PixelFormat format)
    {
        return format switch
        {
            PixelFormat.Depth16Unorm or PixelFormat.Depth32Float => true,
            _ => false,
        };
    }

    /// <summary>
    /// Get the number of bytes per row. If format is compressed, width should be evenly divisible by the compression ratio.
    /// </summary>
    /// <param name="format"></param>
    /// <param name="width"></param>
    /// <returns></returns>
    public static uint GetFormatRowPitch(this PixelFormat format, uint width)
    {
        Guard.IsTrue(width % GetFormatWidthCompressionRatio(format) == 0);

        return (width / GetFormatWidthCompressionRatio(format)) * GetFormatBytesPerBlock(format);
    }

    /// <summary>
    /// Convert an SRGB format to linear. If the format is already linear, will return it
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static PixelFormat SrgbToLinearFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.RGBA8UnormSrgb:
                return PixelFormat.RGBA8Unorm;
            case PixelFormat.BGRA8UnormSrgb:
                return PixelFormat.BGRA8Unorm;
            // Bc compressed formats
            case PixelFormat.Bc1RgbaUnormSrgb:
                return PixelFormat.Bc1RgbaUnorm;
            case PixelFormat.Bc2RgbaUnormSrgb:
                return PixelFormat.Bc2RgbaUnorm;
            case PixelFormat.Bc3RgbaUnormSrgb:
                return PixelFormat.Bc3RgbaUnorm;
            case PixelFormat.Bc7RgbaUnormSrgb:
                return PixelFormat.Bc7RgbaUnorm;

            // Etc2/Eac compressed formats
            case PixelFormat.Etc2Rgb8UnormSrgb:
                return PixelFormat.Etc2Rgb8Unorm;
            case PixelFormat.Etc2Rgb8A1UnormSrgb:
                return PixelFormat.Etc2Rgb8A1Unorm;
            case PixelFormat.Etc2Rgba8UnormSrgb:
                return PixelFormat.Etc2Rgba8Unorm;

            // Astc compressed formats
            case PixelFormat.Astc4x4UnormSrgb:
                return PixelFormat.Astc4x4Unorm;
            case PixelFormat.Astc5x4UnormSrgb:
                return PixelFormat.Astc5x4Unorm;
            case PixelFormat.Astc5x5UnormSrgb:
                return PixelFormat.Astc5x5Unorm;
            case PixelFormat.Astc6x5UnormSrgb:
                return PixelFormat.Astc6x5Unorm;
            case PixelFormat.Astc6x6UnormSrgb:
                return PixelFormat.Astc6x6Unorm;
            case PixelFormat.Astc8x5UnormSrgb:
                return PixelFormat.Astc8x5Unorm;
            case PixelFormat.Astc8x6UnormSrgb:
                return PixelFormat.Astc8x6Unorm;
            case PixelFormat.Astc8x8UnormSrgb:
                return PixelFormat.Astc8x8Unorm;
            case PixelFormat.Astc10x5UnormSrgb:
                return PixelFormat.Astc10x5Unorm;
            case PixelFormat.Astc10x6UnormSrgb:
                return PixelFormat.Astc10x6Unorm;
            case PixelFormat.Astc10x8UnormSrgb:
                return PixelFormat.Astc10x8Unorm;
            case PixelFormat.Astc10x10UnormSrgb:
                return PixelFormat.Astc10x10Unorm;
            case PixelFormat.Astc12x10UnormSrgb:
                return PixelFormat.Astc12x10Unorm;
            case PixelFormat.Astc12x12UnormSrgb:
                return PixelFormat.Astc12x12Unorm;

            default:
                Guard.IsFalse(IsSrgb(format));
                return format;
        }
    }

    /// <summary>
    /// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format, will return the original
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static PixelFormat LinearToSrgbFormat(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.RGBA8Unorm:
                return PixelFormat.RGBA8UnormSrgb;
            case PixelFormat.BGRA8Unorm:
                return PixelFormat.BGRA8UnormSrgb;

            // Bc compressed formats
            case PixelFormat.Bc1RgbaUnorm:
                return PixelFormat.Bc1RgbaUnormSrgb;
            case PixelFormat.Bc2RgbaUnorm:
                return PixelFormat.Bc2RgbaUnormSrgb;
            case PixelFormat.Bc3RgbaUnorm:
                return PixelFormat.Bc3RgbaUnormSrgb;
            case PixelFormat.Bc7RgbaUnorm:
                return PixelFormat.Bc7RgbaUnormSrgb;

            // Etc2/Eac compressed formats
            case PixelFormat.Etc2Rgb8Unorm:
                return PixelFormat.Etc2Rgb8UnormSrgb;
            case PixelFormat.Etc2Rgb8A1Unorm:
                return PixelFormat.Etc2Rgb8A1UnormSrgb;
            case PixelFormat.Etc2Rgba8Unorm:
                return PixelFormat.Etc2Rgba8UnormSrgb;

            // Astc compressed formats
            case PixelFormat.Astc4x4Unorm:
                return PixelFormat.Astc4x4UnormSrgb;
            case PixelFormat.Astc5x4Unorm:
                return PixelFormat.Astc5x4UnormSrgb;
            case PixelFormat.Astc5x5Unorm:
                return PixelFormat.Astc5x5UnormSrgb;
            case PixelFormat.Astc6x5Unorm:
                return PixelFormat.Astc6x5UnormSrgb;
            case PixelFormat.Astc6x6Unorm:
                return PixelFormat.Astc6x6UnormSrgb;
            case PixelFormat.Astc8x5Unorm:
                return PixelFormat.Astc8x5UnormSrgb;
            case PixelFormat.Astc8x6Unorm:
                return PixelFormat.Astc8x6UnormSrgb;
            case PixelFormat.Astc8x8Unorm:
                return PixelFormat.Astc8x8UnormSrgb;
            case PixelFormat.Astc10x5Unorm:
                return PixelFormat.Astc10x5UnormSrgb;
            case PixelFormat.Astc10x6Unorm:
                return PixelFormat.Astc10x6UnormSrgb;
            case PixelFormat.Astc10x8Unorm:
                return PixelFormat.Astc10x8UnormSrgb;
            case PixelFormat.Astc10x10Unorm:
                return PixelFormat.Astc10x10UnormSrgb;
            case PixelFormat.Astc12x10Unorm:
                return PixelFormat.Astc12x10UnormSrgb;
            case PixelFormat.Astc12x12Unorm:
                return PixelFormat.Astc12x12UnormSrgb;

            default:
                return format;
        }
    }

    /// <summary>
    /// Return the BPP for a given <see cref="PixelFormat"/>.
    /// </summary>
    /// <param name="format">The <see cref="PixelFormat"/>.</param>
    /// <returns>Bits per pixel of the format</returns>
    public static uint BitsPerPixel(this PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat.Rgba32Uint:
            case PixelFormat.Rgba32Sint:
            case PixelFormat.Rgba32Float:
                return 128;

            //case PixelFormat.Rgb32Uint:
            //case PixelFormat.Rgb32Sint:
            //case PixelFormat.Rgb32Float:
            //    return 96;

            case PixelFormat.RGBA16Unorm:
            case PixelFormat.RGBA16Snorm:
            case PixelFormat.RGBA16Uint:
            case PixelFormat.RGBA16Sint:
            case PixelFormat.RGBA16Float:
            case PixelFormat.RG32Uint:
            case PixelFormat.RG32Sint:
            case PixelFormat.RG32Float:
            case PixelFormat.Depth32FloatStencil8:
                return 64;

            case PixelFormat.RGB10A2Unorm:
            case PixelFormat.RGB10A2Uint:
            case PixelFormat.RG11B10Float:
            case PixelFormat.RGB9E5Float:
            case PixelFormat.RGBA8Unorm:
            case PixelFormat.RGBA8UnormSrgb:
            case PixelFormat.RGBA8Snorm:
            case PixelFormat.RGBA8Uint:
            case PixelFormat.RGBA8Sint:
            case PixelFormat.RG16Unorm:
            case PixelFormat.RG16Snorm:
            case PixelFormat.RG16Uint:
            case PixelFormat.RG16Sint:
            case PixelFormat.RG16Float:
            case PixelFormat.Depth32Float:
            case PixelFormat.R32Uint:
            case PixelFormat.R32Sint:
            case PixelFormat.R32Float:
            case PixelFormat.Depth24UnormStencil8:
            case PixelFormat.BGRA8Unorm:
            case PixelFormat.BGRA8UnormSrgb:
                return 32;

            case PixelFormat.RG8Unorm:
            case PixelFormat.RG8Snorm:
            case PixelFormat.RG8Uint:
            case PixelFormat.RG8Sint:
            case PixelFormat.R16Unorm:
            case PixelFormat.R16Snorm:
            case PixelFormat.R16Uint:
            case PixelFormat.R16Sint:
            case PixelFormat.R16Float:
            case PixelFormat.Depth16Unorm:
            case PixelFormat.B5G6R5Unorm:
            case PixelFormat.BGR5A1Unorm:
            case PixelFormat.BGRA4Unorm:
                return 16;

            case PixelFormat.R8Unorm:
            case PixelFormat.R8Snorm:
            case PixelFormat.R8Uint:
            case PixelFormat.R8Sint:
            case PixelFormat.Bc2RgbaUnorm:
            case PixelFormat.Bc2RgbaUnormSrgb:
            case PixelFormat.Bc3RgbaUnorm:
            case PixelFormat.Bc3RgbaUnormSrgb:
            case PixelFormat.Bc5RgUnorm:
            case PixelFormat.Bc5RgSnorm:
            case PixelFormat.Bc6hRgbUfloat:
            case PixelFormat.Bc6hRgbFloat:
            case PixelFormat.Bc7RgbaUnorm:
            case PixelFormat.Bc7RgbaUnormSrgb:
            case PixelFormat.Etc2Rgb8A1Unorm:
            case PixelFormat.Etc2Rgb8A1UnormSrgb:
            case PixelFormat.Etc2Rgba8Unorm:
            case PixelFormat.Etc2Rgba8UnormSrgb:
            case PixelFormat.EacRg11Unorm:
            case PixelFormat.EacRg11Snorm:
                return 8;

            case PixelFormat.Bc1RgbaUnorm:
            case PixelFormat.Bc1RgbaUnormSrgb:
            case PixelFormat.Bc4RUnorm:
            case PixelFormat.Bc4RSnorm:
            case PixelFormat.Etc2Rgb8Unorm:
            case PixelFormat.Etc2Rgb8UnormSrgb:
            case PixelFormat.EacR11Unorm:
            case PixelFormat.EacR11Snorm:
                return 4;

            default:
                return 0;
        }
    }

    public static void GetSurfaceInfo(in PixelFormat format, in uint width, in uint height,
        out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount)
    {
        Guard.IsTrue(s_formatInfos[(int)format].Format == format);
        ref PixelFormatInfo formatInfo = ref s_formatInfos[(int)format];

        widthCount = width;
        heightCount = height;

        switch (format)
        {
            case PixelFormat.Bc1RgbaUnorm:
            case PixelFormat.Bc1RgbaUnormSrgb:
            case PixelFormat.Bc2RgbaUnorm:
            case PixelFormat.Bc2RgbaUnormSrgb:
            case PixelFormat.Bc3RgbaUnorm:
            case PixelFormat.Bc3RgbaUnormSrgb:
            case PixelFormat.Bc4RUnorm:
            case PixelFormat.Bc4RSnorm:
            case PixelFormat.Bc5RgUnorm:
            case PixelFormat.Bc5RgSnorm:
            case PixelFormat.Bc6hRgbUfloat:
            case PixelFormat.Bc6hRgbFloat:
            case PixelFormat.Bc7RgbaUnorm:
            case PixelFormat.Bc7RgbaUnormSrgb:
                widthCount = Math.Max(1, (width + 3) / 4);
                heightCount = Math.Max(1, (height + 3) / 4);
                rowPitch = widthCount * formatInfo.BytesPerBlock; // BytesPerBlock is 8 or 16
                slicePitch = rowPitch * heightCount;
                break;

            // ETC2/EAC compressed formats
            case PixelFormat.Etc2Rgb8Unorm:
            case PixelFormat.Etc2Rgb8UnormSrgb:
            case PixelFormat.Etc2Rgb8A1Unorm:
            case PixelFormat.Etc2Rgb8A1UnormSrgb:
            case PixelFormat.Etc2Rgba8Unorm:
            case PixelFormat.Etc2Rgba8UnormSrgb:
            case PixelFormat.EacR11Unorm:
            case PixelFormat.EacR11Snorm:
            case PixelFormat.EacRg11Unorm:
            case PixelFormat.EacRg11Snorm:
                widthCount = Math.Max(1, (width + formatInfo.BlockWidth - 1) / formatInfo.BlockWidth);
                heightCount = Math.Max(1, (height + formatInfo.BlockHeight - 1) / formatInfo.BlockHeight);
                rowPitch = widthCount * formatInfo.BytesPerBlock; // BytesPerBlock is 8 or 16
                slicePitch = rowPitch * heightCount;
                break;

            // ASTC compressed formats
            case PixelFormat.Astc4x4Unorm:
            case PixelFormat.Astc4x4UnormSrgb:
            case PixelFormat.Astc5x4Unorm:
            case PixelFormat.Astc5x4UnormSrgb:
            case PixelFormat.Astc5x5Unorm:
            case PixelFormat.Astc5x5UnormSrgb:
            case PixelFormat.Astc6x5Unorm:
            case PixelFormat.Astc6x5UnormSrgb:
            case PixelFormat.Astc6x6Unorm:
            case PixelFormat.Astc6x6UnormSrgb:
            case PixelFormat.Astc8x5Unorm:
            case PixelFormat.Astc8x5UnormSrgb:
            case PixelFormat.Astc8x6Unorm:
            case PixelFormat.Astc8x6UnormSrgb:
            case PixelFormat.Astc8x8Unorm:
            case PixelFormat.Astc8x8UnormSrgb:
            case PixelFormat.Astc10x5Unorm:
            case PixelFormat.Astc10x5UnormSrgb:
            case PixelFormat.Astc10x6Unorm:
            case PixelFormat.Astc10x6UnormSrgb:
            case PixelFormat.Astc10x8Unorm:
            case PixelFormat.Astc10x8UnormSrgb:
            case PixelFormat.Astc10x10Unorm:
            case PixelFormat.Astc10x10UnormSrgb:
            case PixelFormat.Astc12x10Unorm:
            case PixelFormat.Astc12x10UnormSrgb:
            case PixelFormat.Astc12x12Unorm:
                widthCount = Math.Max(1, (width + formatInfo.BlockWidth - 1) / formatInfo.BlockWidth);
                heightCount = Math.Max(1, (height + formatInfo.BlockHeight - 1) / formatInfo.BlockHeight);
                rowPitch = widthCount * formatInfo.BytesPerBlock;  // BytesPerBlock is always 16
                slicePitch = rowPitch * heightCount;
                break;


            // ASTC HDR compressed formats
            case PixelFormat.Astc4x4Hdr:
            case PixelFormat.Astc5x4Hdr:
            case PixelFormat.Astc5x5Hdr:
            case PixelFormat.Astc6x5Hdr:
            case PixelFormat.Astc6x6Hdr:
            case PixelFormat.Astc8x5Hdr:
            case PixelFormat.Astc8x6Hdr:
            case PixelFormat.Astc8x8Hdr:
            case PixelFormat.Astc10x5Hdr:
            case PixelFormat.Astc10x6Hdr:
            case PixelFormat.Astc10x8Hdr:
            case PixelFormat.Astc10x10Hdr:
            case PixelFormat.Astc12x10Hdr:
            case PixelFormat.Astc12x12Hdr:
                throw new NotImplementedException();
                //widthCount = Math.Max(1, (width + formatInfo.BlockWidth - 1) / formatInfo.BlockWidth);
                //heightCount = Math.Max(1, (height + formatInfo.BlockHeight - 1) / formatInfo.BlockHeight);
                //rowPitch = widthCount * formatInfo.BytesPerBlock;  // BytesPerBlock is always 16
                //slicePitch = rowPitch * heightCount;
                //break;

            //case Format.R8G8_B8G8_UNorm:
            //case Format.G8R8_G8B8_UNorm:
            //case Format.YUY2:
            //    packed = true;
            //    bpe = 4;
            //    break;
            //
            //case Format.Y210:
            //case Format.Y216:
            //    packed = true;
            //    bpe = 8;
            //    break;
            //
            //case Format.NV12:
            //case Format.Opaque420:
            //case Format.P208:
            //    planar = true;
            //    bpe = 2;
            //    break;
            //
            //case Format.P010:
            //case Format.P016:
            //    planar = true;
            //    bpe = 4;
            //    break;

            default:
                uint bpp = BitsPerPixel(format);
                rowPitch = (width * bpp + 7) / 8; // round up to nearest byte
                slicePitch = rowPitch * height;
                return;
        }
    }

    public static void GetSurfaceInfo(in PixelFormat format, in uint width, in uint height, out uint rowPitch, out uint slicePitch)
    {
        GetSurfaceInfo(format, width, height, out rowPitch, out slicePitch, out _, out _);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DxgiFormat ToDxgiFormat(this PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case PixelFormat.R8Unorm: return DxgiFormat.R8Unorm;
            case PixelFormat.R8Snorm: return DxgiFormat.R8Snorm;
            case PixelFormat.R8Uint: return DxgiFormat.R8Uint;
            case PixelFormat.R8Sint: return DxgiFormat.R8Sint;
            // 16-bit formats
            case PixelFormat.R16Unorm: return DxgiFormat.R16Unorm;
            case PixelFormat.R16Snorm: return DxgiFormat.R16Snorm;
            case PixelFormat.R16Uint: return DxgiFormat.R16Uint;
            case PixelFormat.R16Sint: return DxgiFormat.R16Sint;
            case PixelFormat.R16Float: return DxgiFormat.R16Float;
            case PixelFormat.RG8Unorm: return DxgiFormat.R8G8Unorm;
            case PixelFormat.RG8Snorm: return DxgiFormat.R8G8Snorm;
            case PixelFormat.RG8Uint: return DxgiFormat.R8G8Uint;
            case PixelFormat.RG8Sint: return DxgiFormat.R8G8Sint;
            // Packed 16-Bit Pixel Formats
            case PixelFormat.BGRA4Unorm: return DxgiFormat.B4G4R4A4Unorm;
            case PixelFormat.B5G6R5Unorm: return DxgiFormat.B5G6R5Unorm;
            case PixelFormat.BGR5A1Unorm: return DxgiFormat.B5G5R5A1Unorm;
            // 32-bit formats
            case PixelFormat.R32Uint: return DxgiFormat.R32Uint;
            case PixelFormat.R32Sint: return DxgiFormat.R32Sint;
            case PixelFormat.R32Float: return DxgiFormat.R32Float;
            case PixelFormat.RG16Unorm: return DxgiFormat.R16G16Unorm;
            case PixelFormat.RG16Snorm: return DxgiFormat.R16G16Snorm;
            case PixelFormat.RG16Uint: return DxgiFormat.R16G16Uint;
            case PixelFormat.RG16Sint: return DxgiFormat.R16G16Sint;
            case PixelFormat.RG16Float: return DxgiFormat.R16G16Float;
            case PixelFormat.RGBA8Unorm: return DxgiFormat.R8G8B8A8Unorm;
            case PixelFormat.RGBA8UnormSrgb: return DxgiFormat.R8G8B8A8UnormSrgb;
            case PixelFormat.RGBA8Snorm: return DxgiFormat.R8G8B8A8Snorm;
            case PixelFormat.RGBA8Uint: return DxgiFormat.R8G8B8A8Uint;
            case PixelFormat.RGBA8Sint: return DxgiFormat.R8G8B8A8Sint;
            case PixelFormat.BGRA8Unorm: return DxgiFormat.B8G8R8A8Unorm;
            case PixelFormat.BGRA8UnormSrgb: return DxgiFormat.B8G8R8A8UnormSrgb;
            // Packed 32-Bit formats
            case PixelFormat.RGB10A2Unorm: return DxgiFormat.R10G10B10A2Unorm;
            case PixelFormat.RGB10A2Uint: return DxgiFormat.R10G10B10A2Uint;
            case PixelFormat.RG11B10Float: return DxgiFormat.R11G11B10Float;
            case PixelFormat.RGB9E5Float: return DxgiFormat.R9G9B9E5SharedExp;
            // 64-Bit formats
            case PixelFormat.RG32Uint: return DxgiFormat.R32G32Uint;
            case PixelFormat.RG32Sint: return DxgiFormat.R32G32Sint;
            case PixelFormat.RG32Float: return DxgiFormat.R32G32Float;
            case PixelFormat.RGBA16Unorm: return DxgiFormat.R16G16B16A16Unorm;
            case PixelFormat.RGBA16Snorm: return DxgiFormat.R16G16B16A16Snorm;
            case PixelFormat.RGBA16Uint: return DxgiFormat.R16G16B16A16Uint;
            case PixelFormat.RGBA16Sint: return DxgiFormat.R16G16B16A16Sint;
            case PixelFormat.RGBA16Float: return DxgiFormat.R16G16B16A16Float;
            // 128-Bit formats
            case PixelFormat.Rgba32Uint: return DxgiFormat.R32G32B32A32Uint;
            case PixelFormat.Rgba32Sint: return DxgiFormat.R32G32B32A32Sint;
            case PixelFormat.Rgba32Float: return DxgiFormat.R32G32B32A32Float;
            // Depth-stencil formats
            //case PixelFormat.Stencil8:              return DxgiFormat.D24UnormS8Uint;
            case PixelFormat.Depth16Unorm: return DxgiFormat.D16Unorm;
            case PixelFormat.Depth32Float: return DxgiFormat.D32Float;
            case PixelFormat.Depth24UnormStencil8: return DxgiFormat.D24UnormS8Uint;
            case PixelFormat.Depth32FloatStencil8: return DxgiFormat.D32FloatS8X24Uint;
            // Compressed BC formats
            case PixelFormat.Bc1RgbaUnorm: return DxgiFormat.BC1Unorm;
            case PixelFormat.Bc1RgbaUnormSrgb: return DxgiFormat.BC1UnormSrgb;
            case PixelFormat.Bc2RgbaUnorm: return DxgiFormat.BC2Unorm;
            case PixelFormat.Bc2RgbaUnormSrgb: return DxgiFormat.BC2UnormSrgb;
            case PixelFormat.Bc3RgbaUnorm: return DxgiFormat.BC3Unorm;
            case PixelFormat.Bc3RgbaUnormSrgb: return DxgiFormat.BC3UnormSrgb;
            case PixelFormat.Bc4RUnorm: return DxgiFormat.BC4Unorm;
            case PixelFormat.Bc4RSnorm: return DxgiFormat.BC4Snorm;
            case PixelFormat.Bc5RgUnorm: return DxgiFormat.BC5Unorm;
            case PixelFormat.Bc5RgSnorm: return DxgiFormat.BC5Snorm;
            case PixelFormat.Bc6hRgbUfloat: return DxgiFormat.BC6HUF16;
            case PixelFormat.Bc6hRgbFloat: return DxgiFormat.BC6HSF16;
            case PixelFormat.Bc7RgbaUnorm: return DxgiFormat.BC7Unorm;
            case PixelFormat.Bc7RgbaUnormSrgb: return DxgiFormat.BC7UnormSrgb;

            default:
                return DxgiFormat.Unknown;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static PixelFormat FromDxgiFormat(this DxgiFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case DxgiFormat.R8Unorm: return PixelFormat.R8Unorm;
            case DxgiFormat.R8Snorm: return PixelFormat.R8Snorm;
            case DxgiFormat.R8Uint: return PixelFormat.R8Uint;
            case DxgiFormat.R8Sint: return PixelFormat.R8Sint;
            // 16-bit formats
            case DxgiFormat.R16Unorm: return PixelFormat.R16Unorm;
            case DxgiFormat.R16Snorm: return PixelFormat.R16Snorm;
            case DxgiFormat.R16Uint: return PixelFormat.R16Uint;
            case DxgiFormat.R16Sint: return PixelFormat.R16Sint;
            case DxgiFormat.R16Float: return PixelFormat.R16Float;
            case DxgiFormat.R8G8Unorm: return PixelFormat.RG8Unorm;
            case DxgiFormat.R8G8Snorm: return PixelFormat.RG8Snorm;
            case DxgiFormat.R8G8Uint: return PixelFormat.RG8Uint;
            case DxgiFormat.R8G8Sint: return PixelFormat.RG8Sint;
            // Packed 16-Bit Pixel Formats
            case DxgiFormat.B4G4R4A4Unorm: return PixelFormat.BGRA4Unorm;
            case DxgiFormat.B5G6R5Unorm: return PixelFormat.B5G6R5Unorm;
            case DxgiFormat.B5G5R5A1Unorm: return PixelFormat.BGR5A1Unorm;
            // 32-bit formats
            case DxgiFormat.R32Uint: return PixelFormat.R32Uint;
            case DxgiFormat.R32Sint: return PixelFormat.R32Sint;
            case DxgiFormat.R32Float: return PixelFormat.R32Float;
            case DxgiFormat.R16G16Unorm: return PixelFormat.RG16Unorm;
            case DxgiFormat.R16G16Snorm: return PixelFormat.RG16Snorm;
            case DxgiFormat.R16G16Uint: return PixelFormat.RG16Uint;
            case DxgiFormat.R16G16Sint: return PixelFormat.RG16Sint;
            case DxgiFormat.R16G16Float: return PixelFormat.RG16Float;
            case DxgiFormat.R8G8B8A8Unorm: return PixelFormat.RGBA8Unorm;
            case DxgiFormat.R8G8B8A8UnormSrgb: return PixelFormat.RGBA8UnormSrgb;
            case DxgiFormat.R8G8B8A8Snorm: return PixelFormat.RGBA8Snorm;
            case DxgiFormat.R8G8B8A8Uint: return PixelFormat.RGBA8Uint;
            case DxgiFormat.R8G8B8A8Sint: return PixelFormat.RGBA8Sint;
            case DxgiFormat.B8G8R8A8Unorm: return PixelFormat.BGRA8Unorm;
            case DxgiFormat.B8G8R8A8UnormSrgb: return PixelFormat.BGRA8UnormSrgb;
            // Packed 32-Bit formats
            case DxgiFormat.R10G10B10A2Unorm: return PixelFormat.RGB10A2Unorm;
            case DxgiFormat.R10G10B10A2Uint: return PixelFormat.RGB10A2Uint;
            case DxgiFormat.R11G11B10Float: return PixelFormat.RG11B10Float;
            case DxgiFormat.R9G9B9E5SharedExp: return PixelFormat.RGB9E5Float;
            // 64-Bit formats
            case DxgiFormat.R32G32Uint: return PixelFormat.RG32Uint;
            case DxgiFormat.R32G32Sint: return PixelFormat.RG32Sint;
            case DxgiFormat.R32G32Float: return PixelFormat.RG32Float;
            case DxgiFormat.R16G16B16A16Unorm: return PixelFormat.RGBA16Unorm;
            case DxgiFormat.R16G16B16A16Snorm: return PixelFormat.RGBA16Snorm;
            case DxgiFormat.R16G16B16A16Uint: return PixelFormat.RGBA16Uint;
            case DxgiFormat.R16G16B16A16Sint: return PixelFormat.RGBA16Sint;
            case DxgiFormat.R16G16B16A16Float: return PixelFormat.RGBA16Float;
            // 128-Bit formats
            case DxgiFormat.R32G32B32A32Uint: return PixelFormat.Rgba32Uint;
            case DxgiFormat.R32G32B32A32Sint: return PixelFormat.Rgba32Sint;
            case DxgiFormat.R32G32B32A32Float: return PixelFormat.Rgba32Float;
            // Depth-stencil formats
            case DxgiFormat.D16Unorm: return PixelFormat.Depth16Unorm;
            case DxgiFormat.D32Float: return PixelFormat.Depth32Float;
            //case DxgiFormat.D24UnormS8Uint: return PixelFormat.Stencil8;
            case DxgiFormat.D24UnormS8Uint: return PixelFormat.Depth24UnormStencil8;
            case DxgiFormat.D32FloatS8X24Uint: return PixelFormat.Depth32FloatStencil8;
            // Compressed BC formats
            case DxgiFormat.BC1Unorm: return PixelFormat.Bc1RgbaUnorm;
            case DxgiFormat.BC1UnormSrgb: return PixelFormat.Bc1RgbaUnormSrgb;
            case DxgiFormat.BC2Unorm: return PixelFormat.Bc2RgbaUnorm;
            case DxgiFormat.BC2UnormSrgb: return PixelFormat.Bc2RgbaUnormSrgb;
            case DxgiFormat.BC3Unorm: return PixelFormat.Bc3RgbaUnorm;
            case DxgiFormat.BC3UnormSrgb: return PixelFormat.Bc3RgbaUnormSrgb;
            case DxgiFormat.BC4Unorm: return PixelFormat.Bc4RUnorm;
            case DxgiFormat.BC4Snorm: return PixelFormat.Bc4RSnorm;
            case DxgiFormat.BC5Unorm: return PixelFormat.Bc5RgUnorm;
            case DxgiFormat.BC5Snorm: return PixelFormat.Bc5RgSnorm;
            case DxgiFormat.BC6HUF16: return PixelFormat.Bc6hRgbUfloat;
            case DxgiFormat.BC6HSF16: return PixelFormat.Bc6hRgbFloat;
            case DxgiFormat.BC7Unorm: return PixelFormat.Bc7RgbaUnorm;
            case DxgiFormat.BC7UnormSrgb: return PixelFormat.Bc7RgbaUnormSrgb;

            default:
                return PixelFormat.Undefined;
        }
    }
}
