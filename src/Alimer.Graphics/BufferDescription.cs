// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GraphicsBuffer"/>.
/// </summary>
public readonly record struct BufferDescription
{
    [SetsRequiredMembers]
    public BufferDescription(
        ulong size,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode access = CpuAccessMode.None,
        string? label = default)
    {
        Usage = usage;
        Size = size;
        CpuAccess = access;
        Label = label;
    }

    /// <summary>
    /// Size in bytes of <see cref="Buffer"/>
    /// </summary>
    public required ulong Size { get; init; }

    /// <summary>
    /// Gets or Sets the <see cref="BufferUsage"/> of <see cref="Buffer"/>.
    /// </summary>
    public BufferUsage Usage { get; init; } = BufferUsage.ShaderReadWrite;

    /// <summary>
    /// Gets or Sets the CPU access of the <see cref="Buffer"/>.
    /// </summary>
    public CpuAccessMode CpuAccess { get; init; } = CpuAccessMode.None;

    /// <summary>
    /// Gets or sets the label of <see cref="Buffer"/>.
    /// </summary>
    public string? Label { get; init; }
}
