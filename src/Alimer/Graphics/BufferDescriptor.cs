// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GraphicsBuffer"/>.
/// </summary>
public record struct BufferDescriptor
{

    /// <summary>
    /// Gets or sets the size in bytes of the buffer.
    /// </summary>
    public required ulong Size;

    /// <summary>
    /// Gets or sets the <see cref="BufferUsage"/> of the buffer.
    /// </summary>
    public BufferUsage Usage = BufferUsage.ShaderReadWrite;

    /// <summary>
    /// Gets or sets the memory type of the buffer.
    /// </summary>
    public MemoryType MemoryType = MemoryType.Private;

    /// <summary>
    /// Gets or sets the label of  the buffer.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public BufferDescriptor(
        ulong size,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
    {
        Usage = usage;
        Size = size;
        MemoryType = memoryType;
        Label = label;
    }
}
