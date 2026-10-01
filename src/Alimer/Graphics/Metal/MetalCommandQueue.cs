// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal sealed class MetalCommandQueue : CommandQueue
{
    private readonly MetalGraphicsDevice _device;

    public MetalCommandQueue(MetalGraphicsDevice device, CommandQueueType queueType)
    {
        _device = device;
        Handle = device.Handle.newMTL4CommandQueue();
        QueueType = queueType;
    }

    public MTL4CommandQueue Handle { get;}
    public override GraphicsDevice Device => _device;

    public override CommandQueueType QueueType { get; }

    public override void Execute(Span<CommandBuffer> commandBuffers, bool waitForCompletion = false)
    {
        if (!commandBuffers.IsEmpty)
        {
            throw new NotSupportedException("Metal command submission is not implemented yet.");
        }
    }

    public override void WaitIdle()
    {
    }

    public override GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type) => GraphicsNativeHandle.InvalidHandle;
}
