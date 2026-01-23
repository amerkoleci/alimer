// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class BindGroupLayout : GraphicsObject
{
    protected BindGroupLayout(in BindGroupLayoutDescriptor description)
        : base(description.Label)
    {
    }
}
