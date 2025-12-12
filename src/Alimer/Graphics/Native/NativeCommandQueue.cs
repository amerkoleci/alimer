// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using Alimer.Utilities;
using static Alimer.AlimerApi;
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

    /// <inheritdoc />
    public override void Execute(IEnumerable<CommandBuffer> commandBuffers, bool waitForCompletion = false)
    {

    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        agpuCommandQueueWaitIdle(Handle);
    }
}
