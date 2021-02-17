// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop;
using static TerraFX.Interop.DXGI_FORMAT;
using static TerraFX.Interop.Windows;

namespace Vortice.Graphics.D3D12
{
    internal static unsafe class D3D12Utils
    {
        public static void ThrowIfFailed(HRESULT hr)
        {
            if (FAILED(hr))
            {
                Marshal.ThrowExceptionForHR(hr);
            }
        }

        [Conditional("DEBUG")]
        public static void SetName(ID3D12Object* pObject, string name)
        {
            fixed (char* pName = name)
            {
                _ = pObject->SetName((ushort*)pName);
            }
        }

        [Conditional("DEBUG")]
        public static void SetNameIndexed(ID3D12Object* pObject, string name, uint index)
        {
            string fullName = $"{name}[{index}]";
            SetName(pObject, fullName);
        }

        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGISwapChainFormat(TextureFormat format)
        {
            // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
            switch (format)
            {
                case TextureFormat.RGBA16Float:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case TextureFormat.BGRA8Unorm:
                case TextureFormat.BGRA8UnormSrgb:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;

                case TextureFormat.RGBA8Unorm:
                case TextureFormat.RGBA8UnormSrgb:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;

                case TextureFormat.RGB10A2Unorm:
                    return DXGI_FORMAT_R10G10B10A2_UNORM;

                default:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
            }
        }

        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static DXGI_FORMAT ToDXGIFormat(TextureFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case TextureFormat.R8Unorm:
                    return DXGI_FORMAT_R8_UNORM;
                case TextureFormat.R8Snorm:
                    return DXGI_FORMAT_R8_SNORM;
                case TextureFormat.R8Uint:
                    return DXGI_FORMAT_R8_UINT;
                case TextureFormat.R8Sint:
                    return DXGI_FORMAT_R8_SINT;
                // 16-bit formats
                case TextureFormat.R16Unorm:
                    return DXGI_FORMAT_R16_UNORM;
                case TextureFormat.R16Snorm:
                    return DXGI_FORMAT_R16_SNORM;
                case TextureFormat.R16Uint:
                    return DXGI_FORMAT_R16_UINT;
                case TextureFormat.R16Sint:
                    return DXGI_FORMAT_R16_SINT;
                case TextureFormat.R16Float:
                    return DXGI_FORMAT_R16_FLOAT;
                case TextureFormat.RG8Unorm:
                    return DXGI_FORMAT_R8G8_UNORM;
                case TextureFormat.RG8Snorm:
                    return DXGI_FORMAT_R8G8_SNORM;
                case TextureFormat.RG8Uint:
                    return DXGI_FORMAT_R8G8_UINT;
                case TextureFormat.RG8Sint:
                    return DXGI_FORMAT_R8G8_SINT;
                // 32-bit formats
                case TextureFormat.R32Uint:
                    return DXGI_FORMAT_R32_UINT;
                case TextureFormat.R32Sint:
                    return DXGI_FORMAT_R32_SINT;
                case TextureFormat.R32Float:
                    return DXGI_FORMAT_R32_FLOAT;
                case TextureFormat.RG16Unorm:
                    return DXGI_FORMAT_R16G16_UNORM;
                case TextureFormat.RG16Snorm:
                    return DXGI_FORMAT_R16G16_SNORM;
                case TextureFormat.RG16Uint:
                    return DXGI_FORMAT_R16G16_UINT;
                case TextureFormat.RG16Sint:
                    return DXGI_FORMAT_R16G16_SINT;
                case TextureFormat.RG16Float:
                    return DXGI_FORMAT_R16G16_FLOAT;
                case TextureFormat.RGBA8Unorm:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
                case TextureFormat.RGBA8UnormSrgb:
                    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case TextureFormat.RGBA8Snorm:
                    return DXGI_FORMAT_R8G8B8A8_SNORM;
                case TextureFormat.RGBA8Uint:
                    return DXGI_FORMAT_R8G8B8A8_UINT;
                case TextureFormat.RGBA8Sint:
                    return DXGI_FORMAT_R8G8B8A8_SINT;
                case TextureFormat.BGRA8Unorm:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                case TextureFormat.BGRA8UnormSrgb:
                    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                // Packed 32-Bit formats
                case TextureFormat.RGB10A2Unorm:
                    return DXGI_FORMAT_R10G10B10A2_UNORM;
                case TextureFormat.RG11B10Float:
                    return DXGI_FORMAT_R11G11B10_FLOAT;
                case TextureFormat.RGB9E5Float:
                    return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                // 64-Bit formats
                case TextureFormat.RG32Uint:
                    return DXGI_FORMAT_R32G32_UINT;
                case TextureFormat.RG32Sint:
                    return DXGI_FORMAT_R32G32_SINT;
                case TextureFormat.RG32Float:
                    return DXGI_FORMAT_R32G32_FLOAT;
                case TextureFormat.RGBA16Unorm:
                    return DXGI_FORMAT_R16G16B16A16_UNORM;
                case TextureFormat.RGBA16Snorm:
                    return DXGI_FORMAT_R16G16B16A16_SNORM;
                case TextureFormat.RGBA16Uint:
                    return DXGI_FORMAT_R16G16B16A16_UINT;
                case TextureFormat.RGBA16Sint:
                    return DXGI_FORMAT_R16G16B16A16_SINT;
                case TextureFormat.RGBA16Float:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;
                // 128-Bit formats
                case TextureFormat.RGBA32Uint:
                    return DXGI_FORMAT_R32G32B32A32_UINT;
                case TextureFormat.RGBA32Sint:
                    return DXGI_FORMAT_R32G32B32A32_SINT;
                case TextureFormat.RGBA32Float:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
                // Depth-stencil formats
                case TextureFormat.Depth16Unorm:
                    return DXGI_FORMAT_D16_UNORM;
                case TextureFormat.Depth32Float:
                    return DXGI_FORMAT_D32_FLOAT;
                case TextureFormat.Depth24UnormStencil8:
                    return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case TextureFormat.Depth32FloatStencil8:
                    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                // Compressed BC formats
                case TextureFormat.BC1RGBAUnorm:
                    return DXGI_FORMAT_BC1_UNORM;
                case TextureFormat.BC1RGBAUnormSrgb:
                    return DXGI_FORMAT_BC1_UNORM_SRGB;
                case TextureFormat.BC2RGBAUnorm:
                    return DXGI_FORMAT_BC2_UNORM;
                case TextureFormat.BC2RGBAUnormSrgb:
                    return DXGI_FORMAT_BC2_UNORM_SRGB;
                case TextureFormat.BC3RGBAUnorm:
                    return DXGI_FORMAT_BC3_UNORM;
                case TextureFormat.BC3RGBAUnormSrgb:
                    return DXGI_FORMAT_BC3_UNORM_SRGB;
                case TextureFormat.BC4RSnorm:
                    return DXGI_FORMAT_BC4_SNORM;
                case TextureFormat.BC4RUnorm:
                    return DXGI_FORMAT_BC4_UNORM;
                case TextureFormat.BC5RGSnorm:
                    return DXGI_FORMAT_BC5_SNORM;
                case TextureFormat.BC5RGUnorm:
                    return DXGI_FORMAT_BC5_UNORM;
                case TextureFormat.BC6HRGBUfloat:
                    return DXGI_FORMAT_BC6H_UF16;
                case TextureFormat.BC6HRGBSfloat:
                    return DXGI_FORMAT_BC6H_SF16;
                case TextureFormat.BC7RGBAUnorm:
                    return DXGI_FORMAT_BC7_UNORM;
                case TextureFormat.BC7RGBAUnormSrgb:
                    return DXGI_FORMAT_BC7_UNORM_SRGB;

                default:
                    return ThrowHelper.ThrowArgumentException<DXGI_FORMAT>("Invalid texture type");
            }
        }
    }
}
