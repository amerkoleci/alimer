// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Pipeline : GraphicsObject
{
    protected Pipeline(GraphicsDevice device, in RenderPipelineDescriptor descriptor)
        : base(device, descriptor.Label)
    {
        PipelineType = PipelineType.Render;
    }

    protected Pipeline(GraphicsDevice device, in ComputePipelineDescriptor descriptor)
        : base(device, descriptor.Label)
    {
        PipelineType = PipelineType.Compute;
    }

    public PipelineType PipelineType { get; }
}
