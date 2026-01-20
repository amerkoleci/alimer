// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="ComputePipeline"/>.
/// </summary>
public record struct ComputePipelineDescriptor
{
    public required ShaderModule ComputeShader;
    public PipelineLayout Layout;

    /// <summary>
    /// Gets or sets the label of <see cref="ComputePipeline"/>.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public ComputePipelineDescriptor(ShaderModule computeShader, PipelineLayout layout)
    {
        Guard.IsTrue(computeShader.Stage == ShaderStages.Compute, nameof(computeShader.Stage));
        Guard.IsNotNull(layout, nameof(layout));

        ComputeShader = computeShader;
        Layout = layout;
    }
}
