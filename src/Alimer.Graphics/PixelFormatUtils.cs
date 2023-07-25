// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct PixelFormatInfo(
    PixelFormat DxgiFormat,
    uint BytesPerBlock,
    uint BlockWidth,
    uint BlockHeight,
    FormatKind Kind
    );

public static class PixelFormatUtils
{
    private static readonly PixelFormatInfo[] s_formatInfos = new PixelFormatInfo[]
    {
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
        new(PixelFormat.Rg8Unorm,               2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Rg8Snorm,               2, 1, 1, FormatKind.Snorm),
        new(PixelFormat.Rg8Uint,                2, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rg8Sint,                2, 1, 1, FormatKind.Sint),
        // Packed 16-Bit Pixel Formats
        new(PixelFormat.Bgra4Unorm,             2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.B5G6R5Unorm,            2, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Bgr5A1Unorm,            2, 1, 1, FormatKind.Unorm),
        // 32-bit pixel formats
        new(PixelFormat.R32Uint,                4, 1, 1, FormatKind.Uint),
        new(PixelFormat.R32Sint,                4, 1, 1, FormatKind.Sint),
        new(PixelFormat.R32Float,               4, 1, 1, FormatKind.Float),
        new(PixelFormat.Rg16Unorm,              4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Rg16Snorm,              4, 1, 1, FormatKind.Snorm),
        new(PixelFormat.Rg16Uint,               4, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rg16Sint,               4, 1, 1, FormatKind.Sint),
        new(PixelFormat.Rg16Float,              4, 1, 1, FormatKind.Float),
        new(PixelFormat.Rgba8Unorm,             4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Rgba8UnormSrgb,         4, 1, 1, FormatKind.UnormSrgb),
        new(PixelFormat.Rgba8Snorm,             4, 1, 1, FormatKind.Snorm),
        new(PixelFormat.Rgba8Uint,              4, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rgba8Sint,              4, 1, 1, FormatKind.Uint),
        new(PixelFormat.Bgra8Unorm,             4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Bgra8UnormSrgb,         4, 1, 1, FormatKind.UnormSrgb),
        // Packed 32-Bit Pixel formats
        new(PixelFormat.Rgb9e5Ufloat,           4, 1, 1, FormatKind.Float),
        new(PixelFormat.Rgb10a2Unorm,           4, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Rgb10a2Uint,            4, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rg11b10Float,           4, 1, 1, FormatKind.Float),
        // 64-Bit Pixel Formats
        new(PixelFormat.Rg32Uint,               8, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rg32Sint,               8, 1, 1, FormatKind.Sint),
        new(PixelFormat.Rg32Float,              8, 1, 1, FormatKind.Float),
        new(PixelFormat.Rgba16Unorm,            8, 1, 1, FormatKind.Unorm),
        new(PixelFormat.Rgba16Snorm,            8, 1, 1, FormatKind.Snorm),
        new(PixelFormat.Rgba16Uint,             8, 1, 1, FormatKind.Uint),
        new(PixelFormat.Rgba16Sint,             8, 1, 1, FormatKind.Sint),
        new(PixelFormat.Rgba16Float,            8, 1, 1, FormatKind.Float),
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
        new(PixelFormat.Bc6hRgbSfloat,         16, 4, 4, FormatKind.Float),
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
    };

    public static ref readonly PixelFormatInfo GetFormatInfo(this PixelFormat format)
    {
        if (format >= PixelFormat.Count)
        {
            return ref s_formatInfos[0]; // UNKNOWN
        }

        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);
        return ref s_formatInfos[(int)format];
    }

    /// <summary>
    /// Get the number of bytes per format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatBytesPerBlock(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(uint)format].BytesPerBlock;
    }

    public static uint GetFormatPixelsPerBlock(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(uint)format].BlockWidth * s_formatInfos[(uint)format].BlockHeight;
    }

    public static FormatKind GetKind(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);
        return s_formatInfos[(uint)format].Kind;
    }

    public static bool IsInteger(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);
        return s_formatInfos[(int)format].Kind == FormatKind.Uint || s_formatInfos[(int)format].Kind == FormatKind.Sint;
    }

    public static bool IsSigned(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(int)format].Kind == FormatKind.Sint;
    }

    public static bool IsSrgb(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(int)format].Kind == FormatKind.UnormSrgb;
    }

    /// <summary>
    /// Check if the format is a compressed format.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static bool IsCompressedFormat(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(int)format].BlockWidth > 1;
    }

    /// <summary>
    /// Get the format compression ration along the x-axis.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatWidthCompressionRatio(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

        return s_formatInfos[(int)format].BlockWidth;
    }

    /// <summary>
    /// Get the format compression ration along the y-axis.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    public static uint GetFormatHeightCompressionRatio(this PixelFormat format)
    {
        Guard.IsTrue(s_formatInfos[(int)format].DxgiFormat == format);

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
            case PixelFormat.Rgba8UnormSrgb:
                return PixelFormat.Rgba8Unorm;
            case PixelFormat.Bgra8UnormSrgb:
                return PixelFormat.Bgra8Unorm;
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
            case PixelFormat.Rgba8Unorm:
                return PixelFormat.Rgba8UnormSrgb;
            case PixelFormat.Bgra8Unorm:
                return PixelFormat.Bgra8UnormSrgb;

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
    public static uint GetBitsPerPixel(this PixelFormat format)
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

            case PixelFormat.Rgba16Unorm:
            case PixelFormat.Rgba16Snorm:
            case PixelFormat.Rgba16Uint:
            case PixelFormat.Rgba16Sint:
            case PixelFormat.Rgba16Float:
            case PixelFormat.Rg32Uint:
            case PixelFormat.Rg32Sint:
            case PixelFormat.Rg32Float:
            case PixelFormat.Depth32FloatStencil8:
                return 64;

            case PixelFormat.Rgb9e5Ufloat:
            case PixelFormat.Rgb10a2Unorm:
            case PixelFormat.Rgb10a2Uint:
            case PixelFormat.Rg11b10Float:
            case PixelFormat.Rgba8Unorm:
            case PixelFormat.Rgba8UnormSrgb:
            case PixelFormat.Rgba8Snorm:
            case PixelFormat.Rgba8Uint:
            case PixelFormat.Rgba8Sint:
            case PixelFormat.Rg16Unorm:
            case PixelFormat.Rg16Snorm:
            case PixelFormat.Rg16Uint:
            case PixelFormat.Rg16Sint:
            case PixelFormat.Rg16Float:
            case PixelFormat.Depth32Float:
            case PixelFormat.R32Uint:
            case PixelFormat.R32Sint:
            case PixelFormat.R32Float:
            case PixelFormat.Depth24UnormStencil8:
            case PixelFormat.Bgra8Unorm:
            case PixelFormat.Bgra8UnormSrgb:
                return 32;

            case PixelFormat.Rg8Unorm:
            case PixelFormat.Rg8Snorm:
            case PixelFormat.Rg8Uint:
            case PixelFormat.Rg8Sint:
            case PixelFormat.R16Unorm:
            case PixelFormat.R16Snorm:
            case PixelFormat.R16Uint:
            case PixelFormat.R16Sint:
            case PixelFormat.R16Float:
            case PixelFormat.Depth16Unorm:
            case PixelFormat.B5G6R5Unorm:
            case PixelFormat.Bgr5A1Unorm:
            case PixelFormat.Bgra4Unorm:
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
            case PixelFormat.Bc6hRgbSfloat:
            case PixelFormat.Bc7RgbaUnorm:
            case PixelFormat.Bc7RgbaUnormSrgb:
                return 8;

            case PixelFormat.Bc1RgbaUnorm:
            case PixelFormat.Bc1RgbaUnormSrgb:
            case PixelFormat.Bc4RUnorm:
            case PixelFormat.Bc4RSnorm:
                return 4;

            default:
                return 0;
        }
    }

    public static void GetSurfaceInfo(
        PixelFormat format,
        uint width,
        uint height,
        out uint rowPitch,
        out uint slicePitch,
        out uint rowCount)
    {
        bool bc = false;
        bool packed = false;
        bool planar = false;
        uint bpe = 0;

        switch (format)
        {
            case PixelFormat.Bc1RgbaUnorm:
            case PixelFormat.Bc1RgbaUnormSrgb:
            case PixelFormat.Bc4RUnorm:
            case PixelFormat.Bc4RSnorm:
                bc = true;
                bpe = 8;
                break;

            case PixelFormat.Bc2RgbaUnorm:
            case PixelFormat.Bc2RgbaUnormSrgb:
            case PixelFormat.Bc3RgbaUnorm:
            case PixelFormat.Bc3RgbaUnormSrgb:
            case PixelFormat.Bc5RgUnorm:
            case PixelFormat.Bc5RgSnorm:
            case PixelFormat.Bc6hRgbUfloat:
            case PixelFormat.Bc7RgbaUnorm:
            case PixelFormat.Bc7RgbaUnormSrgb:
                bc = true;
                bpe = 16;
                break;

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
                break;
        }

        if (bc)
        {
            uint numBlocksWide = 0;
            if (width > 0)
            {
                numBlocksWide = Math.Max(1, (width + 3) / 4);
            }
            uint numBlocksHigh = 0;
            if (height > 0)
            {
                numBlocksHigh = Math.Max(1, (height + 3) / 4);
            }
            rowPitch = numBlocksWide * bpe;
            rowCount = numBlocksHigh;
            slicePitch = rowPitch * numBlocksHigh;
        }
        else if (packed)
        {
            rowPitch = ((width + 1) >> 1) * bpe;
            rowCount = height;
            slicePitch = rowPitch * height;
        }
        //else if (format == Format.NV11)
        //{
        //    rowPitch = ((width + 3) >> 2) * 4;
        //    rowCount = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        //    slicePitch = rowPitch * rowCount;
        //}
        else if (planar)
        {
            rowPitch = ((width + 1) >> 1) * bpe;
            slicePitch = (rowPitch * height) + ((rowPitch * height + 1) >> 1);
            rowCount = height + ((height + 1u) >> 1);
        }
        else
        {
            uint bpp = GetBitsPerPixel(format);
            rowPitch = (width * bpp + 7) / 8; // round up to nearest byte
            rowCount = height;
            slicePitch = rowPitch * height;
        }
    }

    public static void GetSurfaceInfo(PixelFormat format, uint width, uint height, out uint rowPitch, out uint slicePitch)
    {
        GetSurfaceInfo(format, width, height, out rowPitch, out slicePitch, out _);
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
            case PixelFormat.Rg8Unorm: return DxgiFormat.R8G8Unorm;
            case PixelFormat.Rg8Snorm: return DxgiFormat.R8G8Snorm;
            case PixelFormat.Rg8Uint: return DxgiFormat.R8G8Uint;
            case PixelFormat.Rg8Sint: return DxgiFormat.R8G8Sint;
            // Packed 16-Bit Pixel Formats
            case PixelFormat.Bgra4Unorm: return DxgiFormat.B4G4R4A4Unorm;
            case PixelFormat.B5G6R5Unorm: return DxgiFormat.B5G6R5Unorm;
            case PixelFormat.Bgr5A1Unorm: return DxgiFormat.B5G5R5A1Unorm;
            // 32-bit formats
            case PixelFormat.R32Uint: return DxgiFormat.R32Uint;
            case PixelFormat.R32Sint: return DxgiFormat.R32Sint;
            case PixelFormat.R32Float: return DxgiFormat.R32Float;
            case PixelFormat.Rg16Unorm: return DxgiFormat.R16G16Unorm;
            case PixelFormat.Rg16Snorm: return DxgiFormat.R16G16Snorm;
            case PixelFormat.Rg16Uint: return DxgiFormat.R16G16Uint;
            case PixelFormat.Rg16Sint: return DxgiFormat.R16G16Sint;
            case PixelFormat.Rg16Float: return DxgiFormat.R16G16Float;
            case PixelFormat.Rgba8Unorm: return DxgiFormat.R8G8B8A8Unorm;
            case PixelFormat.Rgba8UnormSrgb: return DxgiFormat.R8G8B8A8UnormSrgb;
            case PixelFormat.Rgba8Snorm: return DxgiFormat.R8G8B8A8Snorm;
            case PixelFormat.Rgba8Uint: return DxgiFormat.R8G8B8A8Uint;
            case PixelFormat.Rgba8Sint: return DxgiFormat.R8G8B8A8Sint;
            case PixelFormat.Bgra8Unorm: return DxgiFormat.B8G8R8A8Unorm;
            case PixelFormat.Bgra8UnormSrgb: return DxgiFormat.B8G8R8A8UnormSrgb;
            // Packed 32-Bit formats
            case PixelFormat.Rgb9e5Ufloat: return DxgiFormat.R9G9B9E5SharedExp;
            case PixelFormat.Rgb10a2Unorm: return DxgiFormat.R10G10B10A2Unorm;
            case PixelFormat.Rgb10a2Uint: return DxgiFormat.R10G10B10A2Uint;
            case PixelFormat.Rg11b10Float: return DxgiFormat.R11G11B10Float;
            // 64-Bit formats
            case PixelFormat.Rg32Uint: return DxgiFormat.R32G32Uint;
            case PixelFormat.Rg32Sint: return DxgiFormat.R32G32Sint;
            case PixelFormat.Rg32Float: return DxgiFormat.R32G32Float;
            case PixelFormat.Rgba16Unorm: return DxgiFormat.R16G16B16A16Unorm;
            case PixelFormat.Rgba16Snorm: return DxgiFormat.R16G16B16A16Snorm;
            case PixelFormat.Rgba16Uint: return DxgiFormat.R16G16B16A16Uint;
            case PixelFormat.Rgba16Sint: return DxgiFormat.R16G16B16A16Sint;
            case PixelFormat.Rgba16Float: return DxgiFormat.R16G16B16A16Float;
            // 128-Bit formats
            case PixelFormat.Rgba32Uint: return DxgiFormat.R32G32B32A32Uint;
            case PixelFormat.Rgba32Sint: return DxgiFormat.R32G32B32A32Sint;
            case PixelFormat.Rgba32Float: return DxgiFormat.R32G32B32A32Float;
            // Depth-stencil formats
            //case PixelFormat.Stencil8:              return DxgiFormat.D24UnormS8Uint;
            case PixelFormat.Depth16Unorm:          return DxgiFormat.D16Unorm;
            case PixelFormat.Depth32Float:          return DxgiFormat.D32Float;
            case PixelFormat.Depth24UnormStencil8:  return DxgiFormat.D24UnormS8Uint;
            case PixelFormat.Depth32FloatStencil8:  return DxgiFormat.D32FloatS8X24Uint;
            // Compressed BC formats
            case PixelFormat.Bc1RgbaUnorm: return DxgiFormat.BC1Unorm;
            case PixelFormat.Bc1RgbaUnormSrgb: return DxgiFormat.BC1UnormSrgb;
            case PixelFormat.Bc2RgbaUnorm: return DxgiFormat.BC2Unorm;
            case PixelFormat.Bc2RgbaUnormSrgb: return DxgiFormat.BC2UnormSrgb;
            case PixelFormat.Bc3RgbaUnorm: return DxgiFormat.BC3Unorm;
            case PixelFormat.Bc3RgbaUnormSrgb: return DxgiFormat.BC3UnormSrgb;
            case PixelFormat.Bc4RSnorm: return DxgiFormat.BC4Unorm;
            case PixelFormat.Bc4RUnorm: return DxgiFormat.BC4Snorm;
            case PixelFormat.Bc5RgUnorm: return DxgiFormat.BC5Unorm;
            case PixelFormat.Bc5RgSnorm: return DxgiFormat.BC5Snorm;
            case PixelFormat.Bc6hRgbSfloat: return DxgiFormat.BC6HSF16;
            case PixelFormat.Bc6hRgbUfloat: return DxgiFormat.BC6HUF16;
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
            case DxgiFormat.R8G8Unorm: return PixelFormat.Rg8Unorm;
            case DxgiFormat.R8G8Snorm: return PixelFormat.Rg8Snorm;
            case DxgiFormat.R8G8Uint: return PixelFormat.Rg8Uint;
            case DxgiFormat.R8G8Sint: return PixelFormat.Rg8Sint;
            // Packed 16-Bit Pixel Formats
            case DxgiFormat.B4G4R4A4Unorm: return PixelFormat.Bgra4Unorm;
            case DxgiFormat.B5G6R5Unorm: return PixelFormat.B5G6R5Unorm;
            case DxgiFormat.B5G5R5A1Unorm: return PixelFormat.Bgr5A1Unorm;
            // 32-bit formats
            case DxgiFormat.R32Uint: return PixelFormat.R32Uint;
            case DxgiFormat.R32Sint: return PixelFormat.R32Sint;
            case DxgiFormat.R32Float: return PixelFormat.R32Float;
            case DxgiFormat.R16G16Unorm: return PixelFormat.Rg16Unorm;
            case DxgiFormat.R16G16Snorm: return PixelFormat.Rg16Snorm;
            case DxgiFormat.R16G16Uint: return PixelFormat.Rg16Uint;
            case DxgiFormat.R16G16Sint: return PixelFormat.Rg16Sint;
            case DxgiFormat.R16G16Float: return PixelFormat.Rg16Float;
            case DxgiFormat.R8G8B8A8Unorm: return PixelFormat.Rgba8Unorm;
            case DxgiFormat.R8G8B8A8UnormSrgb: return PixelFormat.Rgba8UnormSrgb;
            case DxgiFormat.R8G8B8A8Snorm: return PixelFormat.Rgba8Snorm;
            case DxgiFormat.R8G8B8A8Uint: return PixelFormat.Rgba8Uint;
            case DxgiFormat.R8G8B8A8Sint: return PixelFormat.Rgba8Sint;
            case DxgiFormat.B8G8R8A8Unorm: return PixelFormat.Bgra8Unorm;
            case DxgiFormat.B8G8R8A8UnormSrgb: return PixelFormat.Bgra8UnormSrgb;
            // Packed 32-Bit formats
            case DxgiFormat.R9G9B9E5SharedExp: return PixelFormat.Rgb9e5Ufloat;
            case DxgiFormat.R10G10B10A2Unorm: return PixelFormat.Rgb10a2Unorm;
            case DxgiFormat.R10G10B10A2Uint: return PixelFormat.Rgb10a2Uint;
            case DxgiFormat.R11G11B10Float: return PixelFormat.Rg11b10Float;
            // 64-Bit formats
            case DxgiFormat.R32G32Uint: return PixelFormat.Rg32Uint;
            case DxgiFormat.R32G32Sint: return PixelFormat.Rg32Sint;
            case DxgiFormat.R32G32Float: return PixelFormat.Rg32Float;
            case DxgiFormat.R16G16B16A16Unorm: return PixelFormat.Rgba16Unorm;
            case DxgiFormat.R16G16B16A16Snorm: return PixelFormat.Rgba16Snorm;
            case DxgiFormat.R16G16B16A16Uint: return PixelFormat.Rgba16Uint;
            case DxgiFormat.R16G16B16A16Sint: return PixelFormat.Rgba16Sint;
            case DxgiFormat.R16G16B16A16Float: return PixelFormat.Rgba16Float;
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
            case DxgiFormat.BC4Unorm: return PixelFormat.Bc4RSnorm;
            case DxgiFormat.BC4Snorm: return PixelFormat.Bc4RUnorm;
            case DxgiFormat.BC5Unorm: return PixelFormat.Bc5RgUnorm;
            case DxgiFormat.BC5Snorm: return PixelFormat.Bc5RgSnorm;
            case DxgiFormat.BC6HSF16: return PixelFormat.Bc6hRgbSfloat;
            case DxgiFormat.BC6HUF16: return PixelFormat.Bc6hRgbUfloat;
            case DxgiFormat.BC7Unorm: return PixelFormat.Bc7RgbaUnorm;
            case DxgiFormat.BC7UnormSrgb: return PixelFormat.Bc7RgbaUnormSrgb;

            default:
                return PixelFormat.Undefined;
        }
    }
}
