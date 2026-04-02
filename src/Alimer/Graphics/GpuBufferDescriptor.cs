// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GpuBuffer"/>.
/// </summary>
public record struct GpuBufferDescriptor
{
    /// <summary>
    /// Gets or sets the size in bytes of the buffer.
    /// </summary>
    public required ulong Size;

    /// <summary>
    /// Gets or sets the <see cref="GpuBufferUsage"/> of the buffer.
    /// </summary>
    public GpuBufferUsage Usage = GpuBufferUsage.ShaderReadWrite;

    /// <summary>
    /// Gets or sets the memory type of the buffer.
    /// </summary>
    public MemoryType MemoryType = MemoryType.Private;

    /// <summary>
    /// Gets or sets the label of  the buffer.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public GpuBufferDescriptor(
        ulong size,
        GpuBufferUsage usage = GpuBufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
    {
        Usage = usage;
        Size = size;
        MemoryType = memoryType;
        Label = label;
    }
}
