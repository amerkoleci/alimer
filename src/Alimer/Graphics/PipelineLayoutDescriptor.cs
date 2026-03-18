// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a <see cref="PipelineLayout"/>.
/// </summary>
public ref struct PipelineLayoutDescriptor
{
    public Span<BindGroupLayout> BindGroupLayouts;

    /// <summary>
    /// The label of <see cref="PipelineLayout"/>.
    /// </summary>
    public string? Label;

    public PipelineLayoutDescriptor()
    {
        BindGroupLayouts = [];
    }

    public PipelineLayoutDescriptor(Span<BindGroupLayout> bindGroupLayouts)
    {
        ArgumentOutOfRangeException.ThrowIfZero(bindGroupLayouts.Length, nameof(bindGroupLayouts));
        BindGroupLayouts = bindGroupLayouts;
    }
}
