// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.RHI;

namespace Alimer.Graphics;

public abstract class BindGroup : RHIObject
{
    protected BindGroup(in BindGroupDescription description)
        : base(description.Label)
    {
    }

    /// <summary>
    /// Get the <see cref="BindGroupLayout"/>.
    /// </summary>
    public abstract BindGroupLayout Layout { get; }
}
