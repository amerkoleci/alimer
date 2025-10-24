// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeCommandQueue : CommandQueue
{
    private readonly NativeGraphicsDevice _device;

    public NativeCommandQueue(NativeGraphicsDevice device, CommandQueueType queueType)
    {
        _device = device;
        QueueType = queueType;
    }

    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override CommandQueueType QueueType { get; }

    public override void WaitIdle() => throw new NotImplementedException();
}
