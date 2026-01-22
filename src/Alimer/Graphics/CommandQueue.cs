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
}
