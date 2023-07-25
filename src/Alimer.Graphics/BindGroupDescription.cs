// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="BindGroup"/>.
/// </summary>
public readonly record struct BindGroupDescription
{
    public BindGroupDescription(params BindGroupEntry[] entries)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        Entries = entries;
    }

    public BindGroupEntry[] Entries { get; init; }

    /// <summary>
    /// The label of <see cref="BindGroup"/>.
    /// </summary>
    public string? Label { get; init; }
}

/// <summary>
/// Single entry for <see cref="BindGroup"/>.
/// </summary>
public readonly record struct BindGroupEntry
{
    public BindGroupEntry(uint binding, GraphicsBuffer buffer, ulong offset = 0, ulong size = 0)
    {
        Binding = binding;
        Buffer = buffer;
        Offset = offset;
        Size = (size == 0) ? buffer.Size : size;
    }

    public BindGroupEntry(uint binding, Texture texture)
    {
        Binding = binding;
        Texture = texture;
    }

    public BindGroupEntry(uint binding, Sampler sampler)
    {
        Binding = binding;
        Sampler = sampler;
    }

    /// <summary>
    /// Register index to bind to (supplied in shader).
    /// </summary>
    public uint Binding { get; init; }

    /// <summary>
    /// The <see cref="GraphicsBuffer"/> bound.
    /// </summary>
    public GraphicsBuffer? Buffer { get; init; }

    public ulong Offset { get; init; }
    public ulong Size { get; init; }

    /// <summary>
    /// The <see cref="Texture"/> bound.
    /// </summary>
    public Texture? Texture { get; init; }

    /// <summary>
    /// The <see cref="Sampler"/> bound.
    /// </summary>
    public Sampler? Sampler { get; init; }
}
