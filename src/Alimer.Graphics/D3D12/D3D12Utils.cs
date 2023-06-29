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
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ResourceDimension ToD3D12(this TextureDimension value) => s_d3dImageTypeMap[(uint)value];

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
