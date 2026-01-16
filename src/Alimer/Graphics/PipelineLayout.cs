// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.RHI;

namespace Alimer.Graphics;

public abstract class PipelineLayout : RHIObject
{
    protected PipelineLayout(in PipelineLayoutDescription description)
        : base(description.Label)
    {
        BindGroupLayoutCount = description.BindGroupLayouts.Length;
    }

    public int BindGroupLayoutCount { get; }
}
