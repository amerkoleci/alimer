// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics;

/// <summary>
/// Describes a single resource entry for <see cref="BindGroup"/>.
/// </summary>
public struct BindGroupEntry
{
    /// <summary>
    /// Register index to bind to (supplied in shader).
    /// </summary>
    public uint Binding;

    /// <summary>
    /// The <see cref="GraphicsBuffer"/> bound.
    /// </summary>
    public GraphicsBuffer? Buffer;

    public ulong Offset = 0;
    public ulong Size = WholeSize;

    /// <summary>
    /// The <see cref="Graphics.TextureView"/> bound.
    /// </summary>
    public TextureView? TextureView;

    /// <summary>
    /// The <see cref="Sampler"/> bound.
    /// </summary>
    public Sampler? Sampler;

    public BindGroupEntry(uint binding, GraphicsBuffer buffer, ulong offset = 0, ulong size = WholeSize)
    {
        Guard.IsNotNull(buffer, nameof(buffer));

        Binding = binding;
        Buffer = buffer;
        Offset = offset;
        Size = size;
    }

    public BindGroupEntry(uint binding, TextureView textureView)
    {
        Guard.IsNotNull(textureView, nameof(textureView));

        Binding = binding;
        TextureView = textureView;
    }

    public BindGroupEntry(uint binding, Sampler sampler)
    {
        Guard.IsNotNull(sampler, nameof(sampler));

        Binding = binding;
        Sampler = sampler;
    }
}

/// <summary>
/// Structure that describes a compute <see cref="BindGroup"/>.
/// </summary>
public readonly record struct BindGroupDescriptor
{
    public BindGroupDescriptor(params BindGroupEntry[] entries)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        Entries = entries;
    }

    // TODO: Separate per type (buffers/textures/samplers etc)
    public BindGroupEntry[] Entries { get; init; }

    /// <summary>
    /// The label of <see cref="BindGroup"/>.
    /// </summary>
    public string? Label { get; init; }
}
