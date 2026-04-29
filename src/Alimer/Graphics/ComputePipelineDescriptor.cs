// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="ComputePipeline"/>.
/// </summary>
public ref struct ComputePipelineDescriptor
{
    public required ShaderModule ComputeShader;

    /// <summary>
    /// Gets or sets the label of <see cref="ComputePipeline"/>.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public ComputePipelineDescriptor(ShaderModule computeShader)
    {
        ArgumentException.ThrowIfFalse(computeShader.Stage == ShaderStages.Compute, nameof(computeShader.Stage));

        ComputeShader = computeShader;
    }
}
