// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMPARISON_FUNC;
using static TerraFX.Interop.DirectX.D3D12_FILTER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_ADDRESS_MODE;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_QUERY_HEAP_TYPE;
namespace Alimer.Graphics.D3D12;

internal static unsafe class D3D12Utils
{
    public static D3D12_HEAP_PROPERTIES DefaultHeapProps => new(D3D12_HEAP_TYPE_DEFAULT);
    public static D3D12_HEAP_PROPERTIES UploadHeapProps => new(D3D12_HEAP_TYPE_UPLOAD);
    public static D3D12_HEAP_PROPERTIES ReadbackHeapProps => new(D3D12_HEAP_TYPE_READBACK);

    private static readonly D3D12_RESOURCE_DIMENSION[] s_d3dImageTypeMap = new D3D12_RESOURCE_DIMENSION[(int)TextureDimension.Count] {
        D3D12_RESOURCE_DIMENSION_TEXTURE1D,
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        D3D12_RESOURCE_DIMENSION_TEXTURE3D,
    };

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMMAND_LIST_TYPE ToD3D12(this QueueType queue)
    {
        return queue switch
        {
            QueueType.Compute => D3D12_COMMAND_LIST_TYPE_COMPUTE,
            QueueType.Copy => D3D12_COMMAND_LIST_TYPE_COPY,
            QueueType.VideoDecode => D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
            QueueType.VideoEncode => D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
            _ => D3D12_COMMAND_LIST_TYPE_DIRECT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_RESOURCE_DIMENSION ToD3D12(this TextureDimension value) => s_d3dImageTypeMap[(uint)value];

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
        switch (filter)
        {
            case SamplerAddressMode.Wrap:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode.Mirror:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode.Clamp:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode.Border:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode.MirrorOnce:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

            default:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_QUERY_HEAP_TYPE ToD3D12(this QueryType value)
    {
        switch (value)
        {
            default:
            case QueryType.Timestamp:
                return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
                return D3D12_QUERY_HEAP_TYPE_OCCLUSION;

            case QueryType.PipelineStatistics:
                return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_SHADING_RATE ToD3D12(this ShadingRate value)
    {
        switch (value)
        {
            case ShadingRate.Rate1x2:
                return D3D12_SHADING_RATE_1X1;
            case ShadingRate.Rate2x1:
                return D3D12_SHADING_RATE_2X1;
            case ShadingRate.Rate2x2:
                return D3D12_SHADING_RATE_2X2;
            case ShadingRate.Rate2x4:
                return D3D12_SHADING_RATE_2X4;
            case ShadingRate.Rate4x2:
                return D3D12_SHADING_RATE_4X2;
            case ShadingRate.Rate4x4:
                return D3D12_SHADING_RATE_4X4;
            default:
                return D3D12_SHADING_RATE_1X1;
        }
    }

    public static D3D12_RESOURCE_STATES ToD3D12(this ResourceStates states)
    {
        if (states == ResourceStates.Common || states == ResourceStates.Present)
            return D3D12_RESOURCE_STATE_COMMON;

        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON; // also 0

        if ((states & ResourceStates.ConstantBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        if ((states & ResourceStates.VertexBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        if ((states & ResourceStates.IndexBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }

        if ((states & ResourceStates.IndirectArgument) != 0)
        {
            result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }

        if ((states & ResourceStates.ShaderResource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.UnorderedAccess) != 0)
        {
            result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }

        if ((states & ResourceStates.RenderTarget) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        }

        if ((states & ResourceStates.DepthWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }

        if ((states & ResourceStates.DepthRead) != 0)
        {
            result |= D3D12_RESOURCE_STATE_DEPTH_READ;
        }

        if ((states & ResourceStates.StreamOut) != 0)
        {
            result |= D3D12_RESOURCE_STATE_STREAM_OUT;
        }

        if ((states & ResourceStates.CopyDest) != 0)
        {
            result |= D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if ((states & ResourceStates.CopySource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        }

        if ((states & ResourceStates.ResolveDest) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        }

        if ((states & ResourceStates.ResolveSource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        }

        if ((states & ResourceStates.AccelStructRead) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.AccelStructWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.AccelStructBuildInput) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.AccelStructBuildBlas) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.ShadingRateSurface) != 0)
        {
            result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
        }

        if ((states & ResourceStates.OpacityMicromapBuildInput) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.OpacityMicromapWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        return result;
    }
}
