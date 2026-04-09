// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GPUBuffer"/>.
/// </summary>
public record struct GPUBufferDescriptor
{
    /// <summary>
    /// Gets or sets the size in bytes of the buffer.
    /// </summary>
    public required ulong Size;

    /// <summary>
    /// Gets or sets the <see cref="GPUBufferUsage"/> of the buffer.
    /// </summary>
    public GPUBufferUsage Usage = GPUBufferUsage.ShaderReadWrite;

    /// <summary>
    /// Gets or sets the memory type of the buffer.
    /// </summary>
    public MemoryType MemoryType = MemoryType.Private;

    /// <summary>
    /// Gets or sets the label of  the buffer.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public GPUBufferDescriptor(
        ulong size,
        GPUBufferUsage usage = GPUBufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
    {
        Usage = usage;
        Size = size;
        MemoryType = memoryType;
        Label = label;
    }
}
