// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="Pipeline"/>.
/// </summary>
public readonly record struct ComputePipelineDescription
{
    public ReadOnlyMemory<byte> ComputeShader { get; init; }

    /// <summary>
    /// Gets or sets the label of <see cref="Pipeline"/>.
    /// </summary>
    public string? Label { get; init; }
}
