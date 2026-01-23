// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class BindGroup : GraphicsObject
{
    protected BindGroup(in BindGroupDescriptor description)
        : base(description.Label)
    {
    }

    /// <summary>
    /// Get the <see cref="BindGroupLayout"/>.
    /// </summary>
    public abstract BindGroupLayout Layout { get; }
}
