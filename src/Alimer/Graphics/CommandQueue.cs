// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class CommandQueue
{
    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    /// <summary>
    /// Gets the type of this queue.
    /// </summary>
    public abstract CommandQueueType QueueType { get; }

    public void Execute(params Span<CommandBuffer> commandBuffers)
    {
        Execute(commandBuffers, false);
    }

    public abstract void Execute(Span<CommandBuffer> commandBuffers, bool waitForCompletion = false);

    /// <summary>
    /// Wait for the queue to finish pending GPU operations.
    /// </summary>
    public abstract void WaitIdle();

    /// <summary>
    /// Get a native handle for this object.
    /// The type of the handle is determined by the <see cref="GraphicsNativeHandleType"/> parameter.
    /// The returned handle is platform and API specific, and may not be valid across different platforms or graphics APIs.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    public abstract GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type);
}
