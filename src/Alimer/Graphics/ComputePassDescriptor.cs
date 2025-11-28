// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a compute <see cref="ComputePassEncoder"/>.
/// </summary>
public ref struct ComputePassDescriptor
{
    /// <summary>
    /// Gets or sets the label of pass.
    /// </summary>
    public Utf8ReadOnlyString Label;
}
