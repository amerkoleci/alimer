// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D_PRIMITIVE_TOPOLOGY;

namespace Alimer.Graphics.D3D;

internal static unsafe class D3DUtils
{
    public static DXGI_FORMAT ToDxgiFormat(this VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat.UByte2: return DXGI_FORMAT_R8G8_UINT;
            case VertexFormat.UByte4: return DXGI_FORMAT_R8G8B8A8_UINT;
            case VertexFormat.Byte2: return DXGI_FORMAT_R8G8_SINT;
            case VertexFormat.Byte4: return DXGI_FORMAT_R8G8B8A8_SINT;
            case VertexFormat.UByte2Normalized: return DXGI_FORMAT_R8G8_UNORM;
            case VertexFormat.UByte4Normalized: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case VertexFormat.Byte2Normalized: return DXGI_FORMAT_R8G8_SNORM;
            case VertexFormat.Byte4Normalized: return DXGI_FORMAT_R8G8B8A8_SNORM;

            case VertexFormat.UShort2: return DXGI_FORMAT_R16G16_UINT;
            case VertexFormat.UShort4: return DXGI_FORMAT_R16G16B16A16_UINT;
            case VertexFormat.Short2: return DXGI_FORMAT_R16G16_SINT;
            case VertexFormat.Short4: return DXGI_FORMAT_R16G16B16A16_SINT;
            case VertexFormat.UShort2Normalized: return DXGI_FORMAT_R16G16_UNORM;
            case VertexFormat.UShort4Normalized: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case VertexFormat.Short2Normalized: return DXGI_FORMAT_R16G16_SNORM;
            case VertexFormat.Short4Normalized: return DXGI_FORMAT_R16G16B16A16_SNORM;
            case VertexFormat.Half2: return DXGI_FORMAT_R16G16_FLOAT;
            case VertexFormat.Half4: return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case VertexFormat.Float: return DXGI_FORMAT_R32_FLOAT;
            case VertexFormat.Float2: return DXGI_FORMAT_R32G32_FLOAT;
            case VertexFormat.Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case VertexFormat.Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case VertexFormat.UInt: return DXGI_FORMAT_R32_UINT;
            case VertexFormat.UInt2: return DXGI_FORMAT_R32G32_UINT;
            case VertexFormat.UInt3: return DXGI_FORMAT_R32G32B32_UINT;
            case VertexFormat.UInt4: return DXGI_FORMAT_R32G32B32A32_UINT;

            case VertexFormat.Int: return DXGI_FORMAT_R32_SINT;
            case VertexFormat.Int2: return DXGI_FORMAT_R32G32_SINT;
            case VertexFormat.Int3: return DXGI_FORMAT_R32G32B32_SINT;
            case VertexFormat.Int4: return DXGI_FORMAT_R32G32B32A32_SINT;

            case VertexFormat.UInt1010102Normalized: return DXGI_FORMAT_R10G10B10A2_UNORM;
            case VertexFormat.RG11B10Float: return DXGI_FORMAT_R11G11B10_FLOAT;
            case VertexFormat.RGB9E5Float: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }



    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DXGI_FORMAT GetTypelessFormatFromDepthFormat(this PixelFormat format)
    {
        switch (format)
        {
            //case PixelFormat.Stencil8:
            //    return DxgiFormat.R24G8Typeless;
            case PixelFormat.Depth16Unorm:
                return DXGI_FORMAT_R16_TYPELESS;

            case PixelFormat.Depth32Float:
                return DXGI_FORMAT_R32_TYPELESS;

            case PixelFormat.Depth24UnormStencil8:
                return DXGI_FORMAT_R24G8_TYPELESS;
            case PixelFormat.Depth32FloatStencil8:
                return DXGI_FORMAT_R32G8X24_TYPELESS;

            default:
                Guard.IsFalse(format.IsDepthFormat(), nameof(format));
                return (DXGI_FORMAT)format.ToDxgiFormat();
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DXGI_FORMAT ToDxgiSwapChainFormat(this PixelFormat format)
    {
        // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
        switch (format)
        {
            case PixelFormat.RGBA16Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case PixelFormat.BGRA8Unorm:
            case PixelFormat.BGRA8UnormSrgb:
                return DXGI_FORMAT_B8G8R8A8_UNORM;

            case PixelFormat.RGBA8Unorm:
            case PixelFormat.RGBA8UnormSrgb:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case PixelFormat.RGB10A2Unorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;

            default:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
        }
    }

    public static DXGI_FORMAT ToDxgiFormat(this IndexType indexType)
    {
        switch (indexType)
        {
            case IndexType.Uint16: return DXGI_FORMAT_R16_UINT;
            case IndexType.Uint32: return DXGI_FORMAT_R32_UINT;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(this PrimitiveTopology value, uint patchControlPoints = 1u)
    {
        if (value == PrimitiveTopology.PatchList)
        {
            if (patchControlPoints == 0 || patchControlPoints > 32)
            {
                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }

            return (D3D_PRIMITIVE_TOPOLOGY)((uint)D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (patchControlPoints - 1));
        }

        return value switch
        {
            PrimitiveTopology.PointList => D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
            PrimitiveTopology.LineList => D3D_PRIMITIVE_TOPOLOGY_LINELIST,
            PrimitiveTopology.LineStrip => D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
            PrimitiveTopology.TriangleList => D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            PrimitiveTopology.TriangleStrip => D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            _ => D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint ToSampleCount(this TextureSampleCount sampleCount)
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
                return 2;
        }
    }

    public static uint PresentModeToSyncInterval(PresentMode mode)
    {
        switch (mode)
        {
            case PresentMode.Immediate:
            case PresentMode.Mailbox:
                return 0u;

            case PresentMode.Fifo:
            default:
                return 1u;
        }
    }
}
