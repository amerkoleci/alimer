// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.DXGI;
using Microsoft.Toolkit.Diagnostics;
using System.Runtime.CompilerServices;

namespace Vortice.Graphics;

internal static partial class D3DUtils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Format ToDXGISwapChainFormat(TextureFormat format)
    {
        // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
        return format switch
        {
            TextureFormat.RGBA16Float => Format.R16G16B16A16_Float,
            TextureFormat.BGRA8UNorm or TextureFormat.BGRA8UNormSrgb => Format.B8G8R8A8_UNorm,
            TextureFormat.RGBA8UNorm or TextureFormat.RGBA8UNormSrgb => Format.R8G8B8A8_UNorm,
            TextureFormat.RGB10A2UNorm => Format.R10G10B10A2_UNorm,
            _ => Format.B8G8R8A8_UNorm,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Format ToDXGIFormat(TextureFormat format)
    {
        return format switch
        {
            // 8-bit formats
            TextureFormat.R8UNorm => Format.R8_UNorm,
            TextureFormat.R8SNorm => Format.R8_SNorm,
            TextureFormat.R8UInt => Format.R8_UInt,
            TextureFormat.R8SInt => Format.R8_SInt,
            // 16-bit formats
            TextureFormat.R16UNorm => Format.R16_UNorm,
            TextureFormat.R16SNorm => Format.R16_SNorm,
            TextureFormat.R16UInt => Format.R16_UInt,
            TextureFormat.R16SInt => Format.R16_SInt,
            TextureFormat.R16Float => Format.R16_Float,
            TextureFormat.RG8UNorm => Format.R8G8_UNorm,
            TextureFormat.RG8SNorm => Format.R8G8_SNorm,
            TextureFormat.RG8UInt => Format.R8G8_UInt,
            TextureFormat.RG8SInt => Format.R8G8_SInt,
            // 32-bit formats
            TextureFormat.R32UInt => Format.R32_UInt,
            TextureFormat.R32SInt => Format.R32_SInt,
            TextureFormat.R32Float => Format.R32_Float,
            TextureFormat.RG16UNorm => Format.R16G16_UNorm,
            TextureFormat.RG16SNorm => Format.R16G16_SNorm,
            TextureFormat.RG16UInt => Format.R16G16_UInt,
            TextureFormat.RG16SInt => Format.R16G16_SInt,
            TextureFormat.RG16Float => Format.R16G16_Float,
            TextureFormat.RGBA8UNorm => Format.R8G8B8A8_UNorm,
            TextureFormat.RGBA8UNormSrgb => Format.R8G8B8A8_UNorm_SRgb,
            TextureFormat.RGBA8SNorm => Format.R8G8B8A8_SNorm,
            TextureFormat.RGBA8UInt => Format.R8G8B8A8_UInt,
            TextureFormat.RGBA8SInt => Format.R8G8B8A8_SInt,
            TextureFormat.BGRA8UNorm => Format.B8G8R8A8_UNorm,
            TextureFormat.BGRA8UNormSrgb => Format.B8G8R8A8_UNorm_SRgb,
            // Packed 32-Bit formats
            TextureFormat.RGB10A2UNorm => Format.R10G10B10A2_UNorm,
            TextureFormat.RG11B10Float => Format.R11G11B10_Float,
            TextureFormat.RGB9E5Float => Format.R9G9B9E5_SharedExp,
            // 64-Bit formats
            TextureFormat.RG32UInt => Format.R32G32_UInt,
            TextureFormat.RG32SInt => Format.R32G32_SInt,
            TextureFormat.RG32Float => Format.R32G32_Float,
            TextureFormat.RGBA16UNorm => Format.R16G16B16A16_UNorm,
            TextureFormat.RGBA16SNorm => Format.R16G16B16A16_SNorm,
            TextureFormat.RGBA16UInt => Format.R16G16B16A16_UInt,
            TextureFormat.RGBA16SInt => Format.R16G16B16A16_SInt,
            TextureFormat.RGBA16Float => Format.R16G16B16A16_Float,
            // 128-Bit formats
            TextureFormat.RGBA32UInt => Format.R32G32B32A32_UInt,
            TextureFormat.RGBA32SInt => Format.R32G32B32A32_SInt,
            TextureFormat.RGBA32Float => Format.R32G32B32A32_Float,
            // Depth-stencil formats
            TextureFormat.Depth16UNorm => Format.D16_UNorm,
            TextureFormat.Depth32Float => Format.D32_Float,
            TextureFormat.Depth24UNormStencil8 => Format.D24_UNorm_S8_UInt,
            TextureFormat.Depth32FloatStencil8 => Format.D32_Float_S8X24_UInt,
            // Compressed BC formats
            TextureFormat.BC1RGBAUNorm => Format.BC1_UNorm,
            TextureFormat.BC1RGBAUNormSrgb => Format.BC1_UNorm_SRgb,
            TextureFormat.BC2RGBAUNorm => Format.BC2_UNorm,
            TextureFormat.BC2RGBAUNormSrgb => Format.BC2_UNorm_SRgb,
            TextureFormat.BC3RGBAUNorm => Format.BC3_UNorm,
            TextureFormat.BC3RGBAUNormSrgb => Format.BC3_UNorm_SRgb,
            TextureFormat.BC4RSNorm => Format.BC4_SNorm,
            TextureFormat.BC4RUNorm => Format.BC4_UNorm,
            TextureFormat.BC5RGSNorm => Format.BC5_SNorm,
            TextureFormat.BC5RGUNorm => Format.BC5_UNorm,
            TextureFormat.BC6HRGBUFloat => Format.BC6H_Uf16,
            TextureFormat.BC6HRGBFloat => Format.BC6H_Sf16,
            TextureFormat.BC7RGBAUNorm => Format.BC7_UNorm,
            TextureFormat.BC7RGBAUNormSrgb => Format.BC7_UNorm_SRgb,
            _ => ThrowHelper.ThrowArgumentException<Format>("Invalid texture format"),
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Format GetTypelessFormatFromDepthFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat.Depth16UNorm:
                return Format.R16_Typeless;
            case TextureFormat.Depth32Float:
                return Format.R32_Typeless;
            case TextureFormat.Depth24UNormStencil8:
                return Format.R24G8_Typeless;
            case TextureFormat.Depth32FloatStencil8:
                return Format.R32G8X24_Typeless;

            default:
                Guard.IsFalse(format.IsDepthFormat(), nameof(format));
                return ToDXGIFormat(format);
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int ToD3D(TextureSampleCount sampleCount)
    {
        return sampleCount switch
        {
            TextureSampleCount.Count1 => 1,
            TextureSampleCount.Count2 => 2,
            TextureSampleCount.Count4 => 4,
            TextureSampleCount.Count8 => 8,
            TextureSampleCount.Count16 => 16,
            TextureSampleCount.Count32 => 32,
            _ => 1,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int PresentModeToBufferCount(PresentMode mode)
    {
        return mode switch
        {
            PresentMode.Immediate or PresentMode.Fifo => 2,
            PresentMode.Mailbox => 3,
            _ => ThrowHelper.ThrowArgumentException<int>("Invalid present mode"),
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int PresentModeToSwapInterval(PresentMode mode)
    {
        return mode switch
        {
            PresentMode.Immediate or PresentMode.Mailbox => 0,
            PresentMode.Fifo => 1,
            _ => ThrowHelper.ThrowArgumentException<int>("Invalid present mode"),
        };
    }
}
