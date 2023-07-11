// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="PipelineLayout"/>.
/// </summary>
public readonly record struct PipelineLayoutDescription
{
    public PipelineLayoutDescription()
    {
        BindGroupLayouts = Array.Empty<BindGroupLayout>();
    }

    public PipelineLayoutDescription(BindGroupLayout[] bindGroupLayouts, string? label = default)
    {
        Guard.IsNotNull(bindGroupLayouts, nameof(bindGroupLayouts));

        BindGroupLayouts = bindGroupLayouts;
        Label = label;
    }

    public BindGroupLayout[] BindGroupLayouts { get; init; }

    /// <summary>
    /// The label of <see cref="PipelineLayout"/>.
    /// </summary>
    public string? Label { get; init; }
}
