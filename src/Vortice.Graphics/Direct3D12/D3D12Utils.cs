// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Runtime.CompilerServices;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    internal static unsafe class D3D12Utils
    {
        [Conditional("DEBUG")]
        public static void SetName(ID3D12Object @object, string name)
        {
            @object.Name = name;
        }

        [Conditional("DEBUG")]
        public static void SetNameIndexed(ID3D12Object pObject, string name, uint index)
        {
            string fullName = $"{name}[{index}]";
            SetName(pObject, fullName);
        }

        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Format ToDXGISwapChainFormat(TextureFormat format)
        {
            // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
            switch (format)
            {
                case TextureFormat.RGBA16Float:
                    return Format.R16G16B16A16_Float;

                case TextureFormat.BGRA8Unorm:
                case TextureFormat.BGRA8UnormSrgb:
                    return Format.B8G8R8A8_UNorm;

                case TextureFormat.RGBA8Unorm:
                case TextureFormat.RGBA8UnormSrgb:
                    return Format.R8G8B8A8_UNorm;

                case TextureFormat.RGB10A2Unorm:
                    return Format.R10G10B10A2_UNorm;

                default:
                    return Format.B8G8R8A8_UNorm;
            }
        }

        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Format ToDXGIFormat(TextureFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case TextureFormat.R8Unorm:
                    return Format.R8_UNorm;
                case TextureFormat.R8Snorm:
                    return Format.R8_SNorm;
                case TextureFormat.R8Uint:
                    return Format.R8_UInt;
                case TextureFormat.R8Sint:
                    return Format.R8_SInt;
                // 16-bit formats
                case TextureFormat.R16Unorm:
                    return Format.R16_UNorm;
                case TextureFormat.R16Snorm:
                    return Format.R16_SNorm;
                case TextureFormat.R16Uint:
                    return Format.R16_UInt;
                case TextureFormat.R16Sint:
                    return Format.R16_SInt;
                case TextureFormat.R16Float:
                    return Format.R16_Float;
                case TextureFormat.RG8Unorm:
                    return Format.R8G8_UNorm;
                case TextureFormat.RG8Snorm:
                    return Format.R8G8_SNorm;
                case TextureFormat.RG8Uint:
                    return Format.R8G8_UInt;
                case TextureFormat.RG8Sint:
                    return Format.R8G8_SInt;
                // 32-bit formats
                case TextureFormat.R32Uint:
                    return Format.R32_UInt;
                case TextureFormat.R32Sint:
                    return Format.R32_SInt;
                case TextureFormat.R32Float:
                    return Format.R32_Float;
                case TextureFormat.RG16Unorm:
                    return Format.R16G16_UNorm;
                case TextureFormat.RG16Snorm:
                    return Format.R16G16_SNorm;
                case TextureFormat.RG16Uint:
                    return Format.R16G16_UInt;
                case TextureFormat.RG16Sint:
                    return Format.R16G16_SInt;
                case TextureFormat.RG16Float:
                    return Format.R16G16_Float;
                case TextureFormat.RGBA8Unorm:
                    return Format.R8G8B8A8_UNorm;
                case TextureFormat.RGBA8UnormSrgb:
                    return Format.R8G8B8A8_UNorm_SRgb;
                case TextureFormat.RGBA8Snorm:
                    return Format.R8G8B8A8_SNorm;
                case TextureFormat.RGBA8Uint:
                    return Format.R8G8B8A8_UInt;
                case TextureFormat.RGBA8Sint:
                    return Format.R8G8B8A8_SInt;
                case TextureFormat.BGRA8Unorm:
                    return Format.B8G8R8A8_UNorm;
                case TextureFormat.BGRA8UnormSrgb:
                    return Format.B8G8R8A8_UNorm_SRgb;
                // Packed 32-Bit formats
                case TextureFormat.RGB10A2Unorm:
                    return Format.R10G10B10A2_UNorm;
                case TextureFormat.RG11B10Float:
                    return Format.R11G11B10_Float;
                case TextureFormat.RGB9E5Float:
                    return Format.R9G9B9E5_Sharedexp;
                // 64-Bit formats
                case TextureFormat.RG32Uint:
                    return Format.R32G32_UInt;
                case TextureFormat.RG32Sint:
                    return Format.R32G32_SInt;
                case TextureFormat.RG32Float:
                    return Format.R32G32_Float;
                case TextureFormat.RGBA16Unorm:
                    return Format.R16G16B16A16_UNorm;
                case TextureFormat.RGBA16Snorm:
                    return Format.R16G16B16A16_SNorm;
                case TextureFormat.RGBA16Uint:
                    return Format.R16G16B16A16_UInt;
                case TextureFormat.RGBA16Sint:
                    return Format.R16G16B16A16_SInt;
                case TextureFormat.RGBA16Float:
                    return Format.R16G16B16A16_Float;
                // 128-Bit formats
                case TextureFormat.RGBA32Uint:
                    return Format.R32G32B32A32_UInt;
                case TextureFormat.RGBA32Sint:
                    return Format.R32G32B32A32_SInt;
                case TextureFormat.RGBA32Float:
                    return Format.R32G32B32A32_Float;
                // Depth-stencil formats
                case TextureFormat.Depth16Unorm:
                    return Format.D16_UNorm;
                case TextureFormat.Depth32Float:
                    return Format.D32_Float;
                case TextureFormat.Depth24UnormStencil8:
                    return Format.D24_UNorm_S8_UInt;
                case TextureFormat.Depth32FloatStencil8:
                    return Format.D32_Float_S8X24_UInt;
                // Compressed BC formats
                case TextureFormat.BC1RGBAUnorm:
                    return Format.BC1_UNorm;
                case TextureFormat.BC1RGBAUnormSrgb:
                    return Format.BC1_UNorm_SRgb;
                case TextureFormat.BC2RGBAUnorm:
                    return Format.BC2_UNorm;
                case TextureFormat.BC2RGBAUnormSrgb:
                    return Format.BC2_UNorm_SRgb;
                case TextureFormat.BC3RGBAUnorm:
                    return Format.BC3_UNorm;
                case TextureFormat.BC3RGBAUnormSrgb:
                    return Format.BC3_UNorm_SRgb;
                case TextureFormat.BC4RSnorm:
                    return Format.BC4_SNorm;
                case TextureFormat.BC4RUnorm:
                    return Format.BC4_UNorm;
                case TextureFormat.BC5RGSnorm:
                    return Format.BC5_SNorm;
                case TextureFormat.BC5RGUnorm:
                    return Format.BC5_UNorm;
                case TextureFormat.BC6HRGBUfloat:
                    return Format.BC6H_Uf16;
                case TextureFormat.BC6HRGBSfloat:
                    return Format.BC6H_Sf16;
                case TextureFormat.BC7RGBAUnorm:
                    return Format.BC7_UNorm;
                case TextureFormat.BC7RGBAUnormSrgb:
                    return Format.BC7_UNorm_SRgb;

                default:
                    return ThrowHelper.ThrowArgumentException<Format>("Invalid texture type");
            }
        }
    }
}
