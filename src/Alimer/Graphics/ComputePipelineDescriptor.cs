// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="RenderPipeline"/>.
/// </summary>
public readonly record struct ComputePipelineDescriptor
{
    public ComputePipelineDescriptor(PipelineLayout layout, ShaderStageDescription computeShader)
    {
        Guard.IsNotNull(layout, nameof(layout));
        Guard.IsNotNull(computeShader.ByteCode, nameof(computeShader.ByteCode));
        Guard.IsTrue(computeShader.Stage == ShaderStages.Compute, nameof(computeShader.Stage));

        Layout = layout;
        ComputeShader = computeShader;
    }

    public PipelineLayout Layout { get; init; }

    public ShaderStageDescription ComputeShader { get; init; }

    /// <summary>
    /// Gets or sets the label of <see cref="RenderPipeline"/>.
    /// </summary>
    public string? Label { get; init; }
}
