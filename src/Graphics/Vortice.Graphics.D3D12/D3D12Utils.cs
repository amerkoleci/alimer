// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.DXGI_FORMAT;
using static TerraFX.Interop.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.D3D12_COMMAND_LIST_TYPE;
using Microsoft.Toolkit.Diagnostics;
using System.Runtime.CompilerServices;

namespace Vortice.Graphics
{
    internal static unsafe class D3D12Utils
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGISwapChainFormat(TextureFormat format)
        {
            // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
            return format switch
            {
                TextureFormat.RGBA16Float => DXGI_FORMAT_R16G16B16A16_FLOAT,
                TextureFormat.BGRA8UNorm or TextureFormat.BGRA8UNormSrgb => DXGI_FORMAT_B8G8R8A8_UNORM,
                TextureFormat.RGBA8UNorm or TextureFormat.RGBA8UNormSrgb => DXGI_FORMAT_R8G8B8A8_UNORM,
                TextureFormat.RGB10A2UNorm => DXGI_FORMAT_R10G10B10A2_UNORM,
                _ => DXGI_FORMAT_B8G8R8A8_UNORM,
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGIFormat(this TextureFormat format)
        {
            return format switch
            {
                // 8-bit formats
                TextureFormat.R8UNorm => DXGI_FORMAT_R8_UNORM,
                TextureFormat.R8SNorm => DXGI_FORMAT_R8_SNORM,
                TextureFormat.R8UInt => DXGI_FORMAT_R8_UINT,
                TextureFormat.R8SInt => DXGI_FORMAT_R8_SINT,
                // 16-bit formats
                TextureFormat.R16UNorm => DXGI_FORMAT_R16_UNORM,
                TextureFormat.R16SNorm => DXGI_FORMAT_R16_SNORM,
                TextureFormat.R16UInt => DXGI_FORMAT_R16_UINT,
                TextureFormat.R16SInt => DXGI_FORMAT_R16_SINT,
                TextureFormat.R16Float => DXGI_FORMAT_R16_FLOAT,
                TextureFormat.RG8UNorm => DXGI_FORMAT_R8G8_UNORM,
                TextureFormat.RG8SNorm => DXGI_FORMAT_R8G8_SNORM,
                TextureFormat.RG8UInt => DXGI_FORMAT_R8G8_UINT,
                TextureFormat.RG8SInt => DXGI_FORMAT_R8G8_SINT,
                // 32-bit formats
                TextureFormat.R32UInt => DXGI_FORMAT_R32_UINT,
                TextureFormat.R32SInt => DXGI_FORMAT_R32_SINT,
                TextureFormat.R32Float => DXGI_FORMAT_R32_FLOAT,
                TextureFormat.RG16UNorm => DXGI_FORMAT_R16G16_UNORM,
                TextureFormat.RG16SNorm => DXGI_FORMAT_R16G16_SNORM,
                TextureFormat.RG16UInt => DXGI_FORMAT_R16G16_UINT,
                TextureFormat.RG16SInt => DXGI_FORMAT_R16G16_SINT,
                TextureFormat.RG16Float => DXGI_FORMAT_R16G16_FLOAT,
                TextureFormat.RGBA8UNorm => DXGI_FORMAT_R8G8B8A8_UNORM,
                TextureFormat.RGBA8UNormSrgb => DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                TextureFormat.RGBA8SNorm => DXGI_FORMAT_R8G8B8A8_SNORM,
                TextureFormat.RGBA8UInt => DXGI_FORMAT_R8G8B8A8_UINT,
                TextureFormat.RGBA8SInt => DXGI_FORMAT_R8G8B8A8_SINT,
                TextureFormat.BGRA8UNorm => DXGI_FORMAT_B8G8R8A8_UNORM,
                TextureFormat.BGRA8UNormSrgb => DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                // Packed 32-Bit formats
                TextureFormat.RGB10A2UNorm => DXGI_FORMAT_R10G10B10A2_UNORM,
                TextureFormat.RG11B10Float => DXGI_FORMAT_R11G11B10_FLOAT,
                TextureFormat.RGB9E5Float => DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
                // 64-Bit formats
                TextureFormat.RG32UInt => DXGI_FORMAT_R32G32_UINT,
                TextureFormat.RG32SInt => DXGI_FORMAT_R32G32_SINT,
                TextureFormat.RG32Float => DXGI_FORMAT_R32G32_FLOAT,
                TextureFormat.RGBA16UNorm => DXGI_FORMAT_R16G16B16A16_UNORM,
                TextureFormat.RGBA16SNorm => DXGI_FORMAT_R16G16B16A16_SNORM,
                TextureFormat.RGBA16UInt => DXGI_FORMAT_R16G16B16A16_UINT,
                TextureFormat.RGBA16SInt => DXGI_FORMAT_R16G16B16A16_SINT,
                TextureFormat.RGBA16Float => DXGI_FORMAT_R16G16B16A16_FLOAT,
                // 128-Bit formats
                TextureFormat.RGBA32UInt => DXGI_FORMAT_R32G32B32A32_UINT,
                TextureFormat.RGBA32SInt => DXGI_FORMAT_R32G32B32A32_SINT,
                TextureFormat.RGBA32Float => DXGI_FORMAT_R32G32B32A32_FLOAT,
                // Depth-stencil formats
                TextureFormat.Depth16UNorm => DXGI_FORMAT_D16_UNORM,
                TextureFormat.Depth32Float => DXGI_FORMAT_D32_FLOAT,
                TextureFormat.Depth24UNormStencil8 => DXGI_FORMAT_D24_UNORM_S8_UINT,
                TextureFormat.Depth32FloatStencil8 => DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                // Compressed BC formats
                TextureFormat.BC1RGBAUNorm => DXGI_FORMAT_BC1_UNORM,
                TextureFormat.BC1RGBAUNormSrgb => DXGI_FORMAT_BC1_UNORM_SRGB,
                TextureFormat.BC2RGBAUNorm => DXGI_FORMAT_BC2_UNORM,
                TextureFormat.BC2RGBAUNormSrgb => DXGI_FORMAT_BC2_UNORM_SRGB,
                TextureFormat.BC3RGBAUNorm => DXGI_FORMAT_BC3_UNORM,
                TextureFormat.BC3RGBAUNormSrgb => DXGI_FORMAT_BC3_UNORM_SRGB,
                TextureFormat.BC4RSNorm => DXGI_FORMAT_BC4_SNORM,
                TextureFormat.BC4RUNorm => DXGI_FORMAT_BC4_UNORM,
                TextureFormat.BC5RGSNorm => DXGI_FORMAT_BC5_SNORM,
                TextureFormat.BC5RGUNorm => DXGI_FORMAT_BC5_UNORM,
                TextureFormat.BC6HRGBUFloat => DXGI_FORMAT_BC6H_UF16,
                TextureFormat.BC6HRGBFloat => DXGI_FORMAT_BC6H_SF16,
                TextureFormat.BC7RGBAUNorm => DXGI_FORMAT_BC7_UNORM,
                TextureFormat.BC7RGBAUNormSrgb => DXGI_FORMAT_BC7_UNORM_SRGB,
                _ => ThrowHelper.ThrowArgumentException<DXGI_FORMAT>("Invalid texture format"),
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT GetTypelessFormatFromDepthFormat(this TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat.Depth16UNorm:
                    return DXGI_FORMAT_R16_TYPELESS;
                case TextureFormat.Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;
                case TextureFormat.Depth24UNormStencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case TextureFormat.Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    Guard.IsFalse(format.IsDepthFormat(), nameof(format));
                    return ToDXGIFormat(format);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint ToD3D12(this TextureSampleCount sampleCount)
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
        public static uint PresentModeToBufferCount(PresentMode mode)
        {
            return mode switch
            {
                PresentMode.Immediate or PresentMode.Fifo => 2,
                PresentMode.Mailbox => 3,
                _ => ThrowHelper.ThrowArgumentException<uint>("Invalid present mode"),
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint PresentModeToSwapInterval(PresentMode mode)
        {
            return mode switch
            {
                PresentMode.Immediate or PresentMode.Mailbox => 0,
                PresentMode.Fifo => 1,
                _ => ThrowHelper.ThrowArgumentException<uint>("Invalid present mode"),
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static D3D12_RESOURCE_DIMENSION ToD3D12(this TextureDimension dimension)
        {
            return dimension switch
            {
                TextureDimension.Texture1D => D3D12_RESOURCE_DIMENSION_TEXTURE1D,
                TextureDimension.Texture2D => D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                TextureDimension.Texture3D => D3D12_RESOURCE_DIMENSION_TEXTURE3D,
                _ => ThrowHelper.ThrowArgumentException<D3D12_RESOURCE_DIMENSION>("Invalid texture dimension"),
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static D3D12_COMMAND_LIST_TYPE ToD3D12(this CommandQueueType type)
        {
            return type switch
            {
                CommandQueueType.Compute => D3D12_COMMAND_LIST_TYPE_COMPUTE,
                CommandQueueType.Copy => D3D12_COMMAND_LIST_TYPE_COPY,
                _ => D3D12_COMMAND_LIST_TYPE_DIRECT,
            };
        }
    }
}
