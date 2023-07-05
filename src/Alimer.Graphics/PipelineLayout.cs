// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class PipelineLayout : GraphicsObject
{
    protected PipelineLayout(in PipelineLayoutDescription description)
        : base(description.Label)
    {
    }
}
