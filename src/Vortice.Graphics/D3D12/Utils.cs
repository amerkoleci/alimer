// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop;
using static TerraFX.Interop.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.DXGI_FORMAT;

namespace Vortice.Graphics.D3D12
{
    internal static unsafe class Utils
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGISwapChainFormat(TextureFormat format)
        {
            // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
            switch (format)
            {
                case TextureFormat.RGBA16Float:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case TextureFormat.BGRA8UNorm:
                case TextureFormat.BGRA8UNormSrgb:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;

                case TextureFormat.RGBA8UNorm:
                case TextureFormat.RGBA8UNormSrgb:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;

                case TextureFormat.RGB10A2UNorm:
                    return DXGI_FORMAT_R10G10B10A2_UNORM;

                default:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGIFormat(this TextureFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case TextureFormat.R8UNorm: return DXGI_FORMAT_R8_UNORM;
                case TextureFormat.R8SNorm: return DXGI_FORMAT_R8_SNORM;
                case TextureFormat.R8UInt: return DXGI_FORMAT_R8_UINT;
                case TextureFormat.R8SInt: return DXGI_FORMAT_R8_SINT;
                // 16-bit formats
                case TextureFormat.R16UNorm:
                    return DXGI_FORMAT_R16_UNORM;
                case TextureFormat.R16SNorm:
                    return DXGI_FORMAT_R16_SNORM;
                case TextureFormat.R16UInt:
                    return DXGI_FORMAT_R16_UINT;
                case TextureFormat.R16SInt:
                    return DXGI_FORMAT_R16_SINT;
                case TextureFormat.R16Float:
                    return DXGI_FORMAT_R16_FLOAT;
                case TextureFormat.RG8UNorm:
                    return DXGI_FORMAT_R8G8_UNORM;
                case TextureFormat.RG8SNorm:
                    return DXGI_FORMAT_R8G8_SNORM;
                case TextureFormat.RG8UInt:
                    return DXGI_FORMAT_R8G8_UINT;
                case TextureFormat.RG8SInt:
                    return DXGI_FORMAT_R8G8_SINT;
                // 32-bit formats
                case TextureFormat.R32UInt:
                    return DXGI_FORMAT_R32_UINT;
                case TextureFormat.R32SInt:
                    return DXGI_FORMAT_R32_SINT;
                case TextureFormat.R32Float:
                    return DXGI_FORMAT_R32_FLOAT;
                case TextureFormat.RG16UNorm:
                    return DXGI_FORMAT_R16G16_UNORM;
                case TextureFormat.RG16SNorm:
                    return DXGI_FORMAT_R16G16_SNORM;
                case TextureFormat.RG16UInt:
                    return DXGI_FORMAT_R16G16_UINT;
                case TextureFormat.RG16SInt:
                    return DXGI_FORMAT_R16G16_SINT;
                case TextureFormat.RG16Float:
                    return DXGI_FORMAT_R16G16_FLOAT;
                case TextureFormat.RGBA8UNorm:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
                case TextureFormat.RGBA8UNormSrgb:
                    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case TextureFormat.RGBA8SNorm:
                    return DXGI_FORMAT_R8G8B8A8_SNORM;
                case TextureFormat.RGBA8UInt:
                    return DXGI_FORMAT_R8G8B8A8_UINT;
                case TextureFormat.RGBA8SInt:
                    return DXGI_FORMAT_R8G8B8A8_SINT;
                case TextureFormat.BGRA8UNorm:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                case TextureFormat.BGRA8UNormSrgb:
                    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                // Packed 32-Bit formats
                case TextureFormat.RGB10A2UNorm: return DXGI_FORMAT_R10G10B10A2_UNORM;
                case TextureFormat.RG11B10Float: return DXGI_FORMAT_R11G11B10_FLOAT;
                case TextureFormat.RGB9E5Float: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                // 64-Bit formats
                case TextureFormat.RG32UInt: return DXGI_FORMAT_R32G32_UINT;
                case TextureFormat.RG32SInt: return DXGI_FORMAT_R32G32_SINT;
                case TextureFormat.RG32Float: return DXGI_FORMAT_R32G32_FLOAT;
                case TextureFormat.RGBA16UNorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
                case TextureFormat.RGBA16SNorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
                case TextureFormat.RGBA16UInt: return DXGI_FORMAT_R16G16B16A16_UINT;
                case TextureFormat.RGBA16SInt: return DXGI_FORMAT_R16G16B16A16_SINT;
                case TextureFormat.RGBA16Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
                // 128-Bit formats
                case TextureFormat.RGBA32UInt: return DXGI_FORMAT_R32G32B32A32_UINT;
                case TextureFormat.RGBA32SInt: return DXGI_FORMAT_R32G32B32A32_SINT;
                case TextureFormat.RGBA32Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                // Depth-stencil formats
                case TextureFormat.Depth16UNorm: return DXGI_FORMAT_D16_UNORM;
                case TextureFormat.Depth32Float: return DXGI_FORMAT_D32_FLOAT;
                case TextureFormat.Depth24UNormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case TextureFormat.Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                // Compressed BC formats
                case TextureFormat.BC1RGBAUNorm: return DXGI_FORMAT_BC1_UNORM;
                case TextureFormat.BC1RGBAUNormSrgb: return DXGI_FORMAT_BC1_UNORM_SRGB;
                case TextureFormat.BC2RGBAUNorm: return DXGI_FORMAT_BC2_UNORM;
                case TextureFormat.BC2RGBAUNormSrgb: return DXGI_FORMAT_BC2_UNORM_SRGB;
                case TextureFormat.BC3RGBAUNorm: return DXGI_FORMAT_BC3_UNORM;
                case TextureFormat.BC3RGBAUNormSrgb: return DXGI_FORMAT_BC3_UNORM_SRGB;
                case TextureFormat.BC4RSNorm: return DXGI_FORMAT_BC4_SNORM;
                case TextureFormat.BC4RUNorm: return DXGI_FORMAT_BC4_UNORM;
                case TextureFormat.BC5RGSNorm: return DXGI_FORMAT_BC5_SNORM;
                case TextureFormat.BC5RGUNorm: return DXGI_FORMAT_BC5_UNORM;
                case TextureFormat.BC6HRGBUFloat: return DXGI_FORMAT_BC6H_UF16;
                case TextureFormat.BC6HRGBFloat: return DXGI_FORMAT_BC6H_SF16;
                case TextureFormat.BC7RGBAUNorm: return DXGI_FORMAT_BC7_UNORM;
                case TextureFormat.BC7RGBAUNormSrgb: return DXGI_FORMAT_BC7_UNORM_SRGB;

                default:
                    return ThrowHelper.ThrowArgumentException<DXGI_FORMAT>("Invalid texture format");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static D3D12_RESOURCE_DIMENSION ToD3D12(this TextureDimension dimension)
        {
            switch (dimension)
            {
                case TextureDimension.Texture1D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                case TextureDimension.Texture2D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                case TextureDimension.Texture3D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE3D;

                default:
                    return ThrowHelper.ThrowArgumentException<D3D12_RESOURCE_DIMENSION>("Invalid texture dimension");
            }
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
            switch (sampleCount)
            {
                case TextureSampleCount.Count1:
                    return 1;
                case TextureSampleCount.Count2:
                    return 2;
                case TextureSampleCount.Count4:
                    return 4;
                case TextureSampleCount.Count8:
                    return 8;
                case TextureSampleCount.Count16:
                    return 16;
                case TextureSampleCount.Count32:
                    return 32;

                default:
                    return 1;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint PresentModeToBufferCount(PresentMode mode)
        {
            switch (mode)
            {
                case PresentMode.Immediate:
                case PresentMode.Fifo:
                    return 2;
                case PresentMode.Mailbox:
                    return 3;
                default:
                    return ThrowHelper.ThrowArgumentException<uint>("Invalid present mode");
            }
        }

        [Conditional("DEBUG")]
        public static void SetName(this ref ID3D12Resource resource, string name)
        {
            fixed (char* p = name)
            {
                resource.SetName((ushort*)p).Assert();
            }
        }

        [Conditional("DEBUG")]
        public static void SetName(this ref ID3D12CommandQueue resource, string name)
        {
            fixed (char* p = name)
            {
                resource.SetName((ushort*)p).Assert();
            }
        }
    }
}
