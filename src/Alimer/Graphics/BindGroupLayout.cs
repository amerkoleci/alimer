// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.RHI;

namespace Alimer.Graphics;

public abstract class BindGroupLayout : RHIObject
{
    protected BindGroupLayout(in BindGroupLayoutDescription description)
        : base(description.Label)
    {
    }
}
