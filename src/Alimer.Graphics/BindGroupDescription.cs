// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="BindGroup"/>.
/// </summary>
public readonly record struct BindGroupDescription
{
    /// <summary>
    /// The label of <see cref="BindGroup"/>.
    /// </summary>
    public string? Label { get; init; }
}
