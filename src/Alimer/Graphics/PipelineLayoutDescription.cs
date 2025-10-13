// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a <see cref="PipelineLayout"/>.
/// </summary>
public readonly record struct PipelineLayoutDescription
{
    public PipelineLayoutDescription()
    {
        BindGroupLayouts = [];
        PushConstantRanges = [];
    }

    public PipelineLayoutDescription(params Span<BindGroupLayout> bindGroupLayouts)
    {
        Guard.IsGreaterThan(bindGroupLayouts.Length, 0, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts.ToArray();
        PushConstantRanges = [];
    }

    public PipelineLayoutDescription(string label, params Span<BindGroupLayout> bindGroupLayouts)
        : this(bindGroupLayouts)
    {
        Label = label;
    }

    public PipelineLayoutDescription(Span<BindGroupLayout> bindGroupLayouts, Span<PushConstantRange> pushConstantRanges, string? label = default)
    {
        Guard.IsFalse(bindGroupLayouts.IsEmpty, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts.ToArray();
        PushConstantRanges = pushConstantRanges.ToArray();
        Label = label;
    }

    public BindGroupLayout[] BindGroupLayouts { get; init; }

    public PushConstantRange[] PushConstantRanges { get; init; }

    /// <summary>
    /// The label of <see cref="PipelineLayout"/>.
    /// </summary>
    public string? Label { get; init; }
}
