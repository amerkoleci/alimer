// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Metal;

internal unsafe sealed class MetalQueryHeap : QueryHeap
{
    private readonly MetalGraphicsDevice _device;

    public MetalQueryHeap(MetalGraphicsDevice device, in QueryHeapDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
    }

    public override GraphicsDevice Device => _device;

    public override uint QueryResultSize => Type == QueryType.PipelineStatistics ? (uint)sizeof(QueryDataPipelineStatistics) : sizeof(ulong);

    protected internal override void Destroy()
    {
    }
}
