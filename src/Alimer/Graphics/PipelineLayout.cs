// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class PipelineLayout : GraphicsObject
{
    protected PipelineLayout(in PipelineLayoutDescriptor description)
        : base(description.Label)
    {
        BindGroupLayoutCount = description.BindGroupLayouts.Length;
    }

    public int BindGroupLayoutCount { get; }
}
