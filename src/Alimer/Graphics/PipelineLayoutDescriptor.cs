// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a <see cref="PipelineLayout"/>.
/// </summary>
public ref struct PipelineLayoutDescriptor
{
    public Span<BindGroupLayout> BindGroupLayouts;

    public Span<PushConstantRange> PushConstantRanges;

    /// <summary>
    /// The label of <see cref="PipelineLayout"/>.
    /// </summary>
    public string? Label;

    public PipelineLayoutDescriptor()
    {
        BindGroupLayouts = [];
        PushConstantRanges = [];
    }

    public PipelineLayoutDescriptor(Span<BindGroupLayout> bindGroupLayouts)
        : this(bindGroupLayouts, [])
    {
    }

    public PipelineLayoutDescriptor(Span<BindGroupLayout> bindGroupLayouts, Span<PushConstantRange> pushConstantRanges)
    {
        Guard.IsFalse(bindGroupLayouts.IsEmpty, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts;
        PushConstantRanges = pushConstantRanges;
    }
}
