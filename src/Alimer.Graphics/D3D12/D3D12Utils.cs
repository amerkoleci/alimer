// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Win32.Graphics.Direct3D12;

namespace Alimer.Graphics.D3D12;

internal static unsafe class D3D12Utils
{
    public static HeapProperties DefaultHeapProps => new(HeapType.Default);
    public static HeapProperties UploadHeapProps => new(HeapType.Upload);
    public static HeapProperties ReadbackHeapProps => new(HeapType.Readback);

    private static readonly ResourceDimension[] s_d3dImageTypeMap = new ResourceDimension[(int)TextureDimension.Count] {
        ResourceDimension.Texture1D,
        ResourceDimension.Texture2D,
        ResourceDimension.Texture3D,
    };

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static CommandListType ToD3D12(this QueueType queue)
    {
        switch (queue)
        {
            default:
            case QueueType.Graphics:
                return CommandListType.Direct;

            case QueueType.Compute:
                return CommandListType.Compute;

            case QueueType.Copy:
                return CommandListType.Copy;

            case QueueType.VideoDecode:
                return CommandListType.VideoDecode;

            case QueueType.VideoEncode:
                return CommandListType.VideoEncode;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ResourceDimension ToD3D12(this TextureDimension value) => s_d3dImageTypeMap[(uint)value];

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ComparisonFunction ToD3D12(this CompareFunction function)
    {
        switch (function)
        {
            case CompareFunction.Never: return ComparisonFunction.Never;
            case CompareFunction.Less: return ComparisonFunction.Less;
            case CompareFunction.Equal: return ComparisonFunction.Equal;
            case CompareFunction.LessEqual: return ComparisonFunction.LessEqual;
            case CompareFunction.Greater: return ComparisonFunction.Greater;
            case CompareFunction.NotEqual: return ComparisonFunction.NotEqual;
            case CompareFunction.GreaterEqual: return ComparisonFunction.GreaterEqual;
            case CompareFunction.Always: return ComparisonFunction.Always;

            default:
                return ComparisonFunction.Never;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static FilterType ToD3D12(this SamplerMinMagFilter filter)
    {
        switch (filter)
        {
            case SamplerMinMagFilter.Nearest: return FilterType.Point;
            case SamplerMinMagFilter.Linear: return FilterType.Linear;

            default:
                return FilterType.Point;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static FilterType ToD3D12(this SamplerMipFilter filter)
    {
        switch (filter)
        {
            case SamplerMipFilter.Nearest: return FilterType.Point;
            case SamplerMipFilter.Linear: return FilterType.Linear;

            default:
                return FilterType.Point;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static TextureAddressMode ToD3D12(this SamplerAddressMode filter)
    {
        switch (filter)
        {
            case SamplerAddressMode.Wrap: return TextureAddressMode.Wrap;
            case SamplerAddressMode.Mirror: return TextureAddressMode.Mirror;
            case SamplerAddressMode.Clamp: return TextureAddressMode.Clamp;
            case SamplerAddressMode.Border: return TextureAddressMode.Border;
            case SamplerAddressMode.MirrorOnce: return TextureAddressMode.MirrorOnce;

            default:
                return TextureAddressMode.Wrap;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static QueryHeapType ToD3D12(this QueryType value)
    {
        switch (value)
        {
            default:
            case QueryType.Timestamp:
                return QueryHeapType.Timestamp;

            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
                return QueryHeapType.Occlusion;

            case QueryType.PipelineStatistics:
                return QueryHeapType.PipelineStatistics;
        }
    }
}
