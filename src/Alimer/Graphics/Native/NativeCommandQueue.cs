// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using static Alimer.Graphics.Native.AlimerGPUApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeCommandQueue : CommandQueue
{
    private readonly NativeGraphicsDevice _device;
    private readonly ConcurrentDictionary<GPUCommandBuffer, NativeCommandBuffer> _commandBuffers = [];

    public NativeCommandQueue(NativeGraphicsDevice device, CommandQueueType queueType, GPUCommandQueue handle)
    {
        _device = device;
        QueueType = queueType;
        Handle = handle;
    }

    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override CommandQueueType QueueType { get; }

    public GPUCommandQueue Handle { get; }

    public CommandBuffer AcquireCommandBuffer(Utf8ReadOnlyString label = default)
    {
        GPUCommandBuffer handle = agpuCommandQueueAcquireCommandBuffer(Handle, null);
        return _commandBuffers.GetOrAdd(handle, handle => new NativeCommandBuffer(this, handle));
    }

    public override void Execute(Span<CommandBuffer> commandBuffers, bool waitForCompletion = false) => throw new NotImplementedException();
    public override GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type) => throw new NotImplementedException();

    /// <inheritdoc />
    public override void WaitIdle()
    {
        agpuCommandQueueWaitIdle(Handle);
    }
}
