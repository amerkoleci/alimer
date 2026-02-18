// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
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
    /// Specifies the index in a binding array.
    /// </summary>
    public uint ArrayElement = 0;

    /// <summary>
    /// The <see cref="IGraphicsBindableResource"/> bound.
    /// </summary>
    public IGraphicsBindableResource Resource;

    /// <summary>
    /// If the binding is a buffer, this is the byte offset of the binding range, ignored otherwise.
    /// </summary>
    public ulong Offset = 0;

    /// <summary>
    /// If the binding is a buffer, this is the byte size of the binding range
    /// (<see cref="WholeSize"/> means the binding ends at the end of the buffer), ignored otherwise.
    /// </summary>
    public ulong Size = WholeSize;

    /// <summary>
    /// Gets the buffer stride (when using StructuredBuffer or RWStructuredBuffer in shader)
    /// </summary>
    public uint Stride;

    public BindGroupEntry(uint binding, IGraphicsBindableResource resource, ulong offset = 0, ulong size = WholeSize, uint stride = 0)
    {
        Guard.IsNotNull(resource, nameof(resource));

        Binding = binding;
        Resource = resource;
        Offset = offset;
        Size = size;
        Stride = stride;

        // Structured buffer offset must be aligned to structure stride
        if (stride > 0)
        {
            if (!MathUtilities.IsAligned(offset, stride))
            {
                throw new ArgumentException("Offset must be aligned to stride for structured buffers.", nameof(offset));
            }
        }
    }
}

/// <summary>
/// Structure that describes a compute <see cref="BindGroup"/>.
/// </summary>
public ref struct BindGroupDescriptor
{
    // TODO: Separate per type (buffers/textures/samplers etc)
    public Span<BindGroupEntry> Entries;

    /// <summary>
    /// The label of <see cref="BindGroup"/>.
    /// </summary>
    public string? Label;

    public BindGroupDescriptor(Span<BindGroupEntry> entries, string? label = default)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        //ConstantBufferEntries = constantBufferEntries;
        Entries = entries;
        Label = label;
    }
}
