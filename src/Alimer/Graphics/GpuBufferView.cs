// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

// TODO: Understand how to handle AppendStructuredBuffer, ConsumeStructuredBuffer

public abstract class GpuBufferView : GraphicsObject
{
    protected GpuBufferView(GpuBuffer buffer, in GpuBufferViewDescriptor descriptor)
    {
        ArgumentNullException.ThrowIfNull(buffer, nameof(buffer));

        Buffer = buffer;
        ElementOffset = descriptor.ElementOffset;
        ElementCount = descriptor.ElementCount;
        ElementSize = descriptor.ElementSize;
        ElementFormat = descriptor.ElementFormat;
    }

    /// <summary>
    /// Gets the owner <see cref="GpuBuffer"/>
    /// </summary>
    public GpuBuffer Buffer { get; }

    /// <inheritdoc />
    public override GraphicsDevice Device => Buffer.Device;

    public uint ElementOffset { get; }
    public uint ElementCount { get; }
    public uint ElementSize { get; }
    public PixelFormat ElementFormat { get; }

    /// <summary>
    /// Gets the bindless read index of the view.
    /// </summary>
    public abstract int BindlessReadIndex { get; }

    /// <summary>
    /// Gets the bindless read-write index of the view.
    /// </summary>
    public abstract int BindlessReadWriteIndex { get; }
}
