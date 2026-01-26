// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D_PRIMITIVE_TOPOLOGY;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_ACCESS;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_LAYOUT;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_SYNC;
using static TerraFX.Interop.DirectX.D3D12_BLEND;
using static TerraFX.Interop.DirectX.D3D12_BLEND_OP;
using static TerraFX.Interop.DirectX.D3D12_COLOR_WRITE_ENABLE;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMPARISON_FUNC;
using static TerraFX.Interop.DirectX.D3D12_CULL_MODE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FILL_MODE;
using static TerraFX.Interop.DirectX.D3D12_FILTER_REDUCTION_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FILTER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_STATIC_BORDER_COLOR;
using static TerraFX.Interop.DirectX.D3D12_STENCIL_OP;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_ADDRESS_MODE;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_SAMPLER_FLAGS;

namespace Alimer.Graphics.D3D12;

internal static unsafe class D3D12Utils
{
    public static DXGI_FORMAT ToDxgiFormat(this VertexFormat format)
    {
        return format switch
        {
            VertexFormat.UByte2 => DXGI_FORMAT_R8G8_UINT,
            VertexFormat.UByte4 => DXGI_FORMAT_R8G8B8A8_UINT,
            VertexFormat.Byte2 => DXGI_FORMAT_R8G8_SINT,
            VertexFormat.Byte4 => DXGI_FORMAT_R8G8B8A8_SINT,
            VertexFormat.UByte2Normalized => DXGI_FORMAT_R8G8_UNORM,
            VertexFormat.UByte4Normalized => DXGI_FORMAT_R8G8B8A8_UNORM,
            VertexFormat.Byte2Normalized => DXGI_FORMAT_R8G8_SNORM,
            VertexFormat.Byte4Normalized => DXGI_FORMAT_R8G8B8A8_SNORM,
            VertexFormat.UShort2 => DXGI_FORMAT_R16G16_UINT,
            VertexFormat.UShort4 => DXGI_FORMAT_R16G16B16A16_UINT,
            VertexFormat.Short2 => DXGI_FORMAT_R16G16_SINT,
            VertexFormat.Short4 => DXGI_FORMAT_R16G16B16A16_SINT,
            VertexFormat.UShort2Normalized => DXGI_FORMAT_R16G16_UNORM,
            VertexFormat.UShort4Normalized => DXGI_FORMAT_R16G16B16A16_UNORM,
            VertexFormat.Short2Normalized => DXGI_FORMAT_R16G16_SNORM,
            VertexFormat.Short4Normalized => DXGI_FORMAT_R16G16B16A16_SNORM,
            VertexFormat.Half2 => DXGI_FORMAT_R16G16_FLOAT,
            VertexFormat.Half4 => DXGI_FORMAT_R16G16B16A16_FLOAT,
            VertexFormat.Float => DXGI_FORMAT_R32_FLOAT,
            VertexFormat.Float2 => DXGI_FORMAT_R32G32_FLOAT,
            VertexFormat.Float3 => DXGI_FORMAT_R32G32B32_FLOAT,
            VertexFormat.Float4 => DXGI_FORMAT_R32G32B32A32_FLOAT,
            VertexFormat.UInt => DXGI_FORMAT_R32_UINT,
            VertexFormat.UInt2 => DXGI_FORMAT_R32G32_UINT,
            VertexFormat.UInt3 => DXGI_FORMAT_R32G32B32_UINT,
            VertexFormat.UInt4 => DXGI_FORMAT_R32G32B32A32_UINT,
            VertexFormat.Int => DXGI_FORMAT_R32_SINT,
            VertexFormat.Int2 => DXGI_FORMAT_R32G32_SINT,
            VertexFormat.Int3 => DXGI_FORMAT_R32G32B32_SINT,
            VertexFormat.Int4 => DXGI_FORMAT_R32G32B32A32_SINT,
            VertexFormat.Unorm10_10_10_2 => DXGI_FORMAT_R10G10B10A2_UNORM,
            VertexFormat.Unorm8x4BGRA => DXGI_FORMAT_B8G8R8A8_UNORM,
            //case VertexFormat.RG11B10Float:     return DXGI_FORMAT_R11G11B10_FLOAT;
            //case VertexFormat.RGB9E5Float:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            _ => DXGI_FORMAT_UNKNOWN,
        };
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
        return format switch
        {
            PixelFormat.RGBA16Float => DXGI_FORMAT_R16G16B16A16_FLOAT,
            PixelFormat.BGRA8Unorm or PixelFormat.BGRA8UnormSrgb => DXGI_FORMAT_B8G8R8A8_UNORM,
            PixelFormat.RGBA8Unorm or PixelFormat.RGBA8UnormSrgb => DXGI_FORMAT_R8G8B8A8_UNORM,
            PixelFormat.RGB10A2Unorm => DXGI_FORMAT_R10G10B10A2_UNORM,
            _ => DXGI_FORMAT_B8G8R8A8_UNORM,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DXGI_FORMAT ToDxgiRTVFormat(this PixelFormat format)
    {
        return (DXGI_FORMAT)format.ToDxgiFormat();
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DXGI_FORMAT ToDxgiDSVFormat(this PixelFormat format)
    {
        return (DXGI_FORMAT)format.ToDxgiFormat();
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DXGI_FORMAT ToDxgiSRVFormat(this PixelFormat format)
    {
        // Try to resolve resource format:
        switch (format)
        {
            case PixelFormat.Depth16Unorm:
                return DXGI_FORMAT_R16_UNORM;

            case PixelFormat.Depth32Float:
                return DXGI_FORMAT_R32_FLOAT;

            //case PixelFormat.Stencil8:
            case PixelFormat.Depth24UnormStencil8:
                return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

            case PixelFormat.Depth32FloatStencil8:
                return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

            //case PixelFormat::NV12:
            //    srvDesc.Format = DXGI_FORMAT_R8_UNORM;
            //    break;
            default:
                return (DXGI_FORMAT)format.ToDxgiFormat();
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(this PrimitiveTopology value)
    {
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

    public static uint PresentModeToBufferCount(this PresentMode mode)
    {
        return mode switch
        {
            PresentMode.Immediate or PresentMode.Fifo => 2,
            PresentMode.Mailbox => 3,
            _ => 2,
        };
    }

    public static uint PresentModeToSyncInterval(PresentMode mode)
    {
        return mode switch
        {
            PresentMode.Immediate or PresentMode.Mailbox => 0u,
            _ => 1u,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMMAND_LIST_TYPE ToD3D12(this CommandQueueType queue)
    {
        return queue switch
        {
            CommandQueueType.Compute => D3D12_COMMAND_LIST_TYPE_COMPUTE,
            CommandQueueType.Copy => D3D12_COMMAND_LIST_TYPE_COPY,
            CommandQueueType.VideoDecode => D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
            //CommandQueueType.VideoEncode => D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
            _ => D3D12_COMMAND_LIST_TYPE_DIRECT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_RESOURCE_DIMENSION ToD3D12(this TextureDimension value)
    {
        return value switch
        {
            TextureDimension.Texture1D => D3D12_RESOURCE_DIMENSION_TEXTURE1D,
            TextureDimension.Texture2D => D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            TextureDimension.Texture3D => D3D12_RESOURCE_DIMENSION_TEXTURE3D,
            _ => D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMPARISON_FUNC ToD3D12(this CompareFunction function)
    {
        return function switch
        {
            CompareFunction.Never => D3D12_COMPARISON_FUNC_NEVER,
            CompareFunction.Less => D3D12_COMPARISON_FUNC_LESS,
            CompareFunction.Equal => D3D12_COMPARISON_FUNC_EQUAL,
            CompareFunction.LessEqual => D3D12_COMPARISON_FUNC_LESS_EQUAL,
            CompareFunction.Greater => D3D12_COMPARISON_FUNC_GREATER,
            CompareFunction.NotEqual => D3D12_COMPARISON_FUNC_NOT_EQUAL,
            CompareFunction.GreaterEqual => D3D12_COMPARISON_FUNC_GREATER_EQUAL,
            CompareFunction.Always => D3D12_COMPARISON_FUNC_ALWAYS,
            _ => D3D12_COMPARISON_FUNC_NEVER,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_REDUCTION_TYPE ToD3D12(this SamplerReductionType value)
    {
        return value switch
        {
            SamplerReductionType.Standard => D3D12_FILTER_REDUCTION_TYPE_STANDARD,
            SamplerReductionType.Comparison => D3D12_FILTER_REDUCTION_TYPE_COMPARISON,
            SamplerReductionType.Minimum => D3D12_FILTER_REDUCTION_TYPE_MINIMUM,
            SamplerReductionType.Maximum => D3D12_FILTER_REDUCTION_TYPE_MAXIMUM,
            _ => D3D12_FILTER_REDUCTION_TYPE_STANDARD,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_TYPE ToD3D12(this SamplerMinMagFilter filter)
    {
        return filter switch
        {
            SamplerMinMagFilter.Nearest => D3D12_FILTER_TYPE_POINT,
            SamplerMinMagFilter.Linear => D3D12_FILTER_TYPE_LINEAR,
            _ => D3D12_FILTER_TYPE_POINT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_TYPE ToD3D12(this SamplerMipFilter filter)
    {
        return filter switch
        {
            SamplerMipFilter.Nearest => D3D12_FILTER_TYPE_POINT,
            SamplerMipFilter.Linear => D3D12_FILTER_TYPE_LINEAR,
            _ => D3D12_FILTER_TYPE_POINT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_TEXTURE_ADDRESS_MODE ToD3D12(this SamplerAddressMode filter)
    {
        return filter switch
        {
            SamplerAddressMode.Repeat => D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            SamplerAddressMode.MirrorRepeat => D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
            SamplerAddressMode.ClampToEdge => D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            SamplerAddressMode.ClampToBorder => D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            SamplerAddressMode.MirrorClampToEdge => D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE,
            _ => D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILL_MODE ToD3D12(this FillMode value)
    {
        return value switch
        {
            FillMode.Wireframe => D3D12_FILL_MODE_WIREFRAME,
            _ => D3D12_FILL_MODE_SOLID,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_CULL_MODE ToD3D12(this CullMode value)
    {
        return value switch
        {
            CullMode.Front => D3D12_CULL_MODE_FRONT,
            CullMode.None => D3D12_CULL_MODE_NONE,
            _ => D3D12_CULL_MODE_BACK,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_BLEND ToD3D12(this BlendFactor value, bool alpha)
    {
        if (alpha)
        {
            switch (value)
            {
                case BlendFactor.SourceColor:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor.OneMinusSourceColor:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case BlendFactor.DestinationColor:
                    return D3D12_BLEND_DEST_ALPHA;
                case BlendFactor.OneMinusDestinationColor:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case BlendFactor.Source1Color:
                    return D3D12_BLEND_SRC1_ALPHA;
                case BlendFactor.OneMinusSource1Color:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                // Other blend factors translate to the same D3D12 enum as the color blend factors.
                default:
                    break;
            }
        }

        return value switch
        {
            BlendFactor.Zero => D3D12_BLEND_ZERO,
            BlendFactor.One => D3D12_BLEND_ONE,
            BlendFactor.SourceColor => D3D12_BLEND_SRC_COLOR,
            BlendFactor.OneMinusSourceColor => D3D12_BLEND_INV_SRC_COLOR,
            BlendFactor.SourceAlpha => D3D12_BLEND_SRC_ALPHA,
            BlendFactor.OneMinusSourceAlpha => D3D12_BLEND_INV_SRC_ALPHA,
            BlendFactor.DestinationColor => D3D12_BLEND_DEST_COLOR,
            BlendFactor.OneMinusDestinationColor => D3D12_BLEND_INV_DEST_COLOR,
            BlendFactor.DestinationAlpha => D3D12_BLEND_DEST_ALPHA,
            BlendFactor.OneMinusDestinationAlpha => D3D12_BLEND_INV_DEST_ALPHA,
            BlendFactor.SourceAlphaSaturate => D3D12_BLEND_SRC_ALPHA_SAT,
            BlendFactor.BlendColor => D3D12_BLEND_BLEND_FACTOR,
            BlendFactor.OneMinusBlendColor => D3D12_BLEND_INV_BLEND_FACTOR,
            BlendFactor.BlendAlpha => D3D12_BLEND_ALPHA_FACTOR,
            BlendFactor.OneMinusBlendAlpha => D3D12_BLEND_INV_ALPHA_FACTOR,
            BlendFactor.Source1Color => D3D12_BLEND_SRC1_COLOR,
            BlendFactor.OneMinusSource1Color => D3D12_BLEND_INV_SRC1_COLOR,
            BlendFactor.Source1Alpha => D3D12_BLEND_SRC1_ALPHA,
            BlendFactor.OneMinusSource1Alpha => D3D12_BLEND_INV_SRC1_ALPHA,
            _ => D3D12_BLEND_ZERO,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_BLEND_OP ToD3D12(this BlendOperation operation)
    {
        return operation switch
        {
            BlendOperation.Add => D3D12_BLEND_OP_ADD,
            BlendOperation.Subtract => D3D12_BLEND_OP_SUBTRACT,
            BlendOperation.ReverseSubtract => D3D12_BLEND_OP_REV_SUBTRACT,
            BlendOperation.Min => D3D12_BLEND_OP_MIN,
            BlendOperation.Max => D3D12_BLEND_OP_MAX,
            _ => D3D12_BLEND_OP_ADD,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COLOR_WRITE_ENABLE ToD3D12(this ColorWriteMask value)
    {
        D3D12_COLOR_WRITE_ENABLE result = 0;

        if ((value & ColorWriteMask.Red) != 0)
            result |= D3D12_COLOR_WRITE_ENABLE_RED;

        if ((value & ColorWriteMask.Green) != 0)
            result |= D3D12_COLOR_WRITE_ENABLE_GREEN;

        if ((value & ColorWriteMask.Blue) != 0)
            result |= D3D12_COLOR_WRITE_ENABLE_BLUE;

        if ((value & ColorWriteMask.Alpha) != 0)
            result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

        return result;
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_STENCIL_OP ToD3D12(this StencilOperation value)
    {
        return value switch
        {
            StencilOperation.Keep => D3D12_STENCIL_OP_KEEP,
            StencilOperation.Zero => D3D12_STENCIL_OP_ZERO,
            StencilOperation.Replace => D3D12_STENCIL_OP_REPLACE,
            StencilOperation.Invert => D3D12_STENCIL_OP_INVERT,
            StencilOperation.IncrementClamp => D3D12_STENCIL_OP_INCR_SAT,
            StencilOperation.DecrementClamp => D3D12_STENCIL_OP_DECR_SAT,
            StencilOperation.IncrementWrap => D3D12_STENCIL_OP_INCR,
            StencilOperation.DecrementWrap => D3D12_STENCIL_OP_DECR,
            _ => D3D12_STENCIL_OP_KEEP,
        };
    }

    public static D3D12_DEPTH_STENCILOP_DESC ToD3D12(this in StencilFaceState state)
    {
        return new D3D12_DEPTH_STENCILOP_DESC()
        {
            StencilFailOp = state.FailOperation.ToD3D12(),
            StencilDepthFailOp = state.DepthFailOperation.ToD3D12(),
            StencilPassOp = state.PassOperation.ToD3D12(),
            StencilFunc = state.CompareFunction.ToD3D12()
        };
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_SHADING_RATE ToD3D12(this ShadingRate value)
    {
        return value switch
        {
            ShadingRate.Rate1x2 => D3D12_SHADING_RATE_1X1,
            ShadingRate.Rate2x1 => D3D12_SHADING_RATE_2X1,
            ShadingRate.Rate2x2 => D3D12_SHADING_RATE_2X2,
            ShadingRate.Rate2x4 => D3D12_SHADING_RATE_2X4,
            ShadingRate.Rate4x2 => D3D12_SHADING_RATE_4X2,
            ShadingRate.Rate4x4 => D3D12_SHADING_RATE_4X4,
            _ => D3D12_SHADING_RATE_1X1,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_SHADER_VISIBILITY ToD3D12(this ShaderStages stage)
    {
        if (stage == ShaderStages.All)
            return D3D12_SHADER_VISIBILITY_ALL;

        return stage switch
        {
            ShaderStages.Vertex => D3D12_SHADER_VISIBILITY_VERTEX,
            ShaderStages.Fragment => D3D12_SHADER_VISIBILITY_PIXEL,
            ShaderStages.Amplification => D3D12_SHADER_VISIBILITY_AMPLIFICATION,
            ShaderStages.Mesh => D3D12_SHADER_VISIBILITY_MESH,
            _ => D3D12_SHADER_VISIBILITY_ALL,
        };
    }

    public static D3D12TextureLayoutMapping ConvertTextureLayout(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout.Undefined:
                return new(
                    D3D12_BARRIER_LAYOUT_COMMON,
                    D3D12_BARRIER_SYNC_NONE,
                    D3D12_BARRIER_ACCESS_COMMON
                    );

            case TextureLayout.CopySource:
                return new(
                    D3D12_BARRIER_LAYOUT_COPY_SOURCE,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_SOURCE
                    );

            case TextureLayout.CopyDest:
                return new(
                    D3D12_BARRIER_LAYOUT_COPY_DEST,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_DEST
                    );

            case TextureLayout.ResolveSource:
                return new(
                    D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_SOURCE
                    );

            case TextureLayout.ResolveDest:
                return new(
                    D3D12_BARRIER_LAYOUT_RESOLVE_DEST,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_DEST
                    );

            case TextureLayout.ShaderResource:
                return new(
                    D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_SHADER_RESOURCE
                    );

            case TextureLayout.UnorderedAccess:
                return new(
                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
                    );

            case TextureLayout.RenderTarget:
                return new(
                    D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                    D3D12_BARRIER_SYNC_RENDER_TARGET,
                    D3D12_BARRIER_ACCESS_RENDER_TARGET
                    );

            case TextureLayout.DepthWrite:
                return new(
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,
                    D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                    D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE
                    );

            case TextureLayout.DepthRead:
                return new(
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ,
                    D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                    D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ
                    );

            case TextureLayout.Present:
                return new(
                    D3D12_BARRIER_LAYOUT_PRESENT,
                    D3D12_BARRIER_SYNC_ALL,
                    D3D12_BARRIER_ACCESS_COMMON
                    );

            case TextureLayout.ShadingRateSurface:
                return new(
                    D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE,
                    D3D12_BARRIER_SYNC_PIXEL_SHADING,
                    D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE
                    );

            default:

                return ThrowHelper.ThrowArgumentException<D3D12TextureLayoutMapping>();
        }
    }

    public static D3D12_RESOURCE_STATES ConvertTextureLayoutLegacy(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout.Undefined:
                return D3D12_RESOURCE_STATE_COMMON;

            case TextureLayout.CopySource:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;

            case TextureLayout.CopyDest:
                return D3D12_RESOURCE_STATE_COPY_DEST;

            case TextureLayout.ShaderResource:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            case TextureLayout.UnorderedAccess:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            case TextureLayout.RenderTarget:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case TextureLayout.DepthWrite:
                return D3D12_RESOURCE_STATE_DEPTH_WRITE;

            case TextureLayout.DepthRead:
                return D3D12_RESOURCE_STATE_DEPTH_READ;

            case TextureLayout.Present:
                return D3D12_RESOURCE_STATE_PRESENT;

            case TextureLayout.ShadingRateSurface:
                return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

            default:
                return ThrowHelper.ThrowArgumentException<D3D12_RESOURCE_STATES>("Unsupported texture layout");
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_DESCRIPTOR_RANGE_TYPE ToD3D12(this BindingInfoType value)
    {
        switch (value)
        {
            case BindingInfoType.Buffer:
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

            case BindingInfoType.Sampler:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

            case BindingInfoType.Texture:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

            case BindingInfoType.StorageTexture:
                return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

            default:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

    public static D3D12_SAMPLER_DESC ToD3D12SamplerDesc(in SamplerDescriptor description)
    {
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        D3D12_SAMPLER_DESC desc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        desc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), D3D12_DEFAULT_MAX_ANISOTROPY);
        if (desc.MaxAnisotropy > 1)
        {
            desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
        }
        else
        {
            desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reductionType);
        }

        desc.AddressU = description.AddressModeU.ToD3D12();
        desc.AddressV = description.AddressModeV.ToD3D12();
        desc.AddressW = description.AddressModeW.ToD3D12();
        desc.MipLODBias = 0.0f;
        if (description.CompareFunction != CompareFunction.Never)
        {
            desc.ComparisonFunc = description.CompareFunction.ToD3D12();
        }
        else
        {
            desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }
        desc.MinLOD = description.LodMinClamp;
        desc.MaxLOD = description.LodMaxClamp;
        switch (description.BorderColor)
        {
            case SamplerBorderColor.FloatOpaqueBlack:
            case SamplerBorderColor.UIntOpaqueBlack:
                desc.BorderColor[0] = 0.0f;
                desc.BorderColor[1] = 0.0f;
                desc.BorderColor[2] = 0.0f;
                desc.BorderColor[3] = 1.0f;
                break;

            case SamplerBorderColor.FloatOpaqueWhite:
            case SamplerBorderColor.UIntOpaqueWhite:
                desc.BorderColor[0] = 1.0f;
                desc.BorderColor[1] = 1.0f;
                desc.BorderColor[2] = 1.0f;
                desc.BorderColor[3] = 1.0f;
                break;

            default:
                desc.BorderColor[0] = 0.0f;
                desc.BorderColor[1] = 0.0f;
                desc.BorderColor[2] = 0.0f;
                desc.BorderColor[3] = 0.0f;
                break;
        }

        return desc;
    }

    public static D3D12_SAMPLER_DESC2 ToD3D12SamplerDesc2(in SamplerDescriptor description)
    {
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        D3D12_SAMPLER_DESC2 desc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        desc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), D3D12_DEFAULT_MAX_ANISOTROPY);
        if (desc.MaxAnisotropy > 1)
        {
            desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
        }
        else
        {
            desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reductionType);
        }

        desc.AddressU = description.AddressModeU.ToD3D12();
        desc.AddressV = description.AddressModeV.ToD3D12();
        desc.AddressW = description.AddressModeW.ToD3D12();
        desc.MipLODBias = 0.0f;
        if (description.CompareFunction != CompareFunction.Never)
        {
            desc.ComparisonFunc = description.CompareFunction.ToD3D12();
        }
        else
        {
            desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }
        desc.MinLOD = description.LodMinClamp;
        desc.MaxLOD = description.LodMaxClamp;
        switch (description.BorderColor)
        {
            case SamplerBorderColor.FloatOpaqueBlack:
                desc.FloatBorderColor[0] = 0.0f;
                desc.FloatBorderColor[1] = 0.0f;
                desc.FloatBorderColor[2] = 0.0f;
                desc.FloatBorderColor[3] = 1.0f;
                break;

            case SamplerBorderColor.FloatOpaqueWhite:
                desc.FloatBorderColor[0] = 1.0f;
                desc.FloatBorderColor[1] = 1.0f;
                desc.FloatBorderColor[2] = 1.0f;
                desc.FloatBorderColor[3] = 1.0f;
                break;

            default:
                desc.FloatBorderColor[0] = 0.0f;
                desc.FloatBorderColor[1] = 0.0f;
                desc.FloatBorderColor[2] = 0.0f;
                desc.FloatBorderColor[3] = 0.0f;
                break;

            case SamplerBorderColor.UIntTransparentBlack:
                desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
                desc.UintBorderColor[0] = 0;
                desc.UintBorderColor[1] = 0;
                desc.UintBorderColor[2] = 0;
                desc.UintBorderColor[3] = 0;
                break;

            case SamplerBorderColor.UIntOpaqueBlack:
                desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
                desc.UintBorderColor[0] = 0;
                desc.UintBorderColor[1] = 0;
                desc.UintBorderColor[2] = 0;
                desc.UintBorderColor[3] = 1;
                break;

            case SamplerBorderColor.UIntOpaqueWhite:
                desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
                desc.UintBorderColor[0] = 1;
                desc.UintBorderColor[1] = 1;
                desc.UintBorderColor[2] = 1;
                desc.UintBorderColor[3] = 1;
                break;
        }

        return desc;
    }

    public static D3D12_STATIC_SAMPLER_DESC ToD3D12StaticSamplerDesc(
        uint shaderRegister,
        in SamplerDescriptor description,
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, uint registerSpace = 0u)
    {
        D3D12_SAMPLER_DESC samplerDesc = ToD3D12SamplerDesc(in description);

        D3D12_STATIC_SAMPLER_DESC staticDesc = new()
        {
            Filter = samplerDesc.Filter,
            AddressU = samplerDesc.AddressU,
            AddressV = samplerDesc.AddressV,
            AddressW = samplerDesc.AddressW,
            MipLODBias = samplerDesc.MipLODBias,
            MaxAnisotropy = samplerDesc.MaxAnisotropy,
            ComparisonFunc = samplerDesc.ComparisonFunc,
            MinLOD = samplerDesc.MinLOD,
            MaxLOD = samplerDesc.MaxLOD,
            ShaderRegister = shaderRegister,
            RegisterSpace = registerSpace,
            ShaderVisibility = shaderVisibility,
            BorderColor = description.BorderColor switch
            {
                SamplerBorderColor.FloatOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
                SamplerBorderColor.FloatOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
                SamplerBorderColor.UIntOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT,
                SamplerBorderColor.UIntOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT,
                _ => D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
            }
        };

        return staticDesc;
    }

    public static D3D12_RESOURCE_STATES ConvertBufferStateLegacy(this BufferStates states, CommandQueueType queueType)
    {
        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

        if ((states & BufferStates.CopyDest) != 0)
            result |= D3D12_RESOURCE_STATE_COPY_DEST;

        if ((states & BufferStates.CopySource) != 0)
            result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

        if ((states & BufferStates.ShaderResource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if (queueType == CommandQueueType.Graphics)
                result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }

        if ((states & BufferStates.UnorderedAccess) != 0)
            result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        if ((states & BufferStates.VertexBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if ((states & BufferStates.IndexBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if ((states & BufferStates.ConstantBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if ((states & BufferStates.Predication) != 0)
            result |= D3D12_RESOURCE_STATE_PREDICATION;
#if TODO
            if ((stateBits & ResourceStates::IndirectArgument) != 0) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            if ((stateBits & ResourceStates::StreamOut) != 0) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
            if ((stateBits & ResourceStates::AccelerationStructureRead) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::ShadingRateSurface) != 0) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
            if ((stateBits & ResourceStates::OpacityMicromapBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if ((stateBits & ResourceStates::OpacityMicromapWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif // TODO


        return result;
    }

    public readonly record struct D3D12TextureLayoutMapping(D3D12_BARRIER_LAYOUT Layout, D3D12_BARRIER_SYNC Sync, D3D12_BARRIER_ACCESS Access);
}
