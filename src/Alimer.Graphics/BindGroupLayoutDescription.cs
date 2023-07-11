// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="BindGroupLayout"/>.
/// </summary>
public readonly record struct BindGroupLayoutDescription
{
    public BindGroupLayoutDescription()
    {
        
    }

    /// <summary>
    /// The label of <see cref="BindGroupLayout"/>.
    /// </summary>
    public string? Label { get; init; }
}
