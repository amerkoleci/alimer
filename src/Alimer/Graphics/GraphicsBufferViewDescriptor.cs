// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GraphicsBufferView"/>.
/// </summary>
public record struct GraphicsBufferViewDescriptor
{
    /// <summary>
    /// Gets or sets the number of elements from the start of the buffer to offset.
    /// </summary>
    public uint ElementOffset = 0;

    /// <summary>
    /// Gets or sets the number of elements in the view.
    /// </summary>
    public uint ElementCount = 0;

    /// <summary>
    /// Gets or sets the size in bytes of each element.
    /// </summary>
    public uint ElementSize = 0;

    /// <summary>
    /// Gets or sets the format of each element. Should be Unknown for structured buffers, or R32 for raw buffers.
    /// </summary>
    public PixelFormat ElementFormat = PixelFormat.Undefined;

    /// <summary>
    /// Initializes a new instance of the <see cref="GraphicsBufferViewDescriptor"/> struct with default values.
    /// </summary>
    public GraphicsBufferViewDescriptor()
    {
    }

    /// <summary>
    /// Creates a raw buffer view. 
    /// </summary>
    /// <param name="byteOffset">The offset in bytes</param>
    /// <param name="byteCount">The number of elements in the view.</param>
    /// <returns></returns>
    public static GraphicsBufferViewDescriptor CreateRaw(uint byteOffset, uint byteCount)
    {
        return new GraphicsBufferViewDescriptor()
        {
            ElementOffset = byteOffset >> 2,
            ElementCount = byteCount >> 2,
            ElementSize = 4,
            ElementFormat = PixelFormat.R32Uint
        };
    }

    /// <summary>
    /// Creates a structured buffer view. 
    /// </summary>
    /// <param name="elementOffset">The number of elements from the start of the buffer to offset</param>
    /// <param name="elementCount">The number of elements in the view.</param>
    /// <param name="elementSize">The size in bytes of each element.</param>
    /// <returns></returns>
    public static GraphicsBufferViewDescriptor CreateStructured(uint elementOffset, uint elementCount, uint elementSize)
    {
        return new GraphicsBufferViewDescriptor()
        {
            ElementOffset = elementOffset,
            ElementCount = elementCount,
            ElementSize = elementSize,
            ElementFormat = PixelFormat.Undefined
        };
    }

    /// <summary>
    /// Creates a typed buffer view. 
    /// </summary>
    /// <param name="elementOffset">The number of elements from the start of the buffer to offset</param>
    /// <param name="elementCount">The number of elements in the view</param>
    /// <param name="elementFormat">The <see cref="PixelFormat"/> of each element</param>
    /// <returns></returns>
    public static GraphicsBufferViewDescriptor CreateTyped(uint elementOffset, uint elementCount, PixelFormat elementFormat)
    {
        return new GraphicsBufferViewDescriptor()
        {
            ElementOffset = elementOffset,
            ElementCount = elementCount,
            ElementSize = elementFormat.GetSizeOrBytesPerBlock(),
            ElementFormat = elementFormat
        };
    }
}
