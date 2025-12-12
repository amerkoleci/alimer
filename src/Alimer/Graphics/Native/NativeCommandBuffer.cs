// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Drawing;
using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeCommandBuffer : CommandBuffer
{
    private readonly NativeCommandQueue _queue;
    
    public NativeCommandBuffer(NativeCommandQueue queue, GPUCommandBuffer handle)
    {
        _queue = queue;
        Handle = handle;
    }

    public override GraphicsDevice Device => _queue.Device;

    public GPUCommandBuffer Handle { get; }

    public override void InsertDebugMarker(Utf8ReadOnlyString debugLabel) => throw new NotImplementedException();
    public override void PopDebugGroup() => throw new NotImplementedException();
    public override void Present(SwapChain swapChain) => throw new NotImplementedException();
    public override void PushDebugGroup(Utf8ReadOnlyString groupLabel) => throw new NotImplementedException();
    protected override ComputePassEncoder BeginComputePassCore(in ComputePassDescriptor descriptor) => throw new NotImplementedException();
    protected override RenderPassEncoder BeginRenderPassCore(in RenderPassDescriptor descriptor) => throw new NotImplementedException();
}
