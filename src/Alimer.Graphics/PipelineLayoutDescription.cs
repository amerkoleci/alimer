// Copyright © Amer Koleci and Contributors.
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
        BindGroupLayouts = Array.Empty<BindGroupLayout>();
        PushConstantRanges = Array.Empty<PushConstantRange>();
    }

    public PipelineLayoutDescription(BindGroupLayout bindGroupLayout, string? label = default)
    {
        Guard.IsNotNull(bindGroupLayout, nameof(bindGroupLayout));

        BindGroupLayouts = new[] { bindGroupLayout };
        PushConstantRanges = Array.Empty<PushConstantRange>();
        Label = label;
    }

    public PipelineLayoutDescription(BindGroupLayout[] bindGroupLayouts, string? label = default)
    {
        Guard.IsNotNull(bindGroupLayouts, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts;
        PushConstantRanges = Array.Empty<PushConstantRange>();
        Label = label;
    }

    public PipelineLayoutDescription(BindGroupLayout[] bindGroupLayouts, PushConstantRange[] pushConstantRanges, string? label = default)
    {
        Guard.IsNotNull(bindGroupLayouts, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts;
        PushConstantRanges = pushConstantRanges;
        Label = label;
    }

    public BindGroupLayout[] BindGroupLayouts { get; init; }

    public PushConstantRange[] PushConstantRanges { get; init; }

    /// <summary>
    /// The label of <see cref="PipelineLayout"/>.
    /// </summary>
    public string? Label { get; init; }
}
