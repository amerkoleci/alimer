// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Pipeline : GraphicsObject
{
    protected Pipeline(GraphicsDevice device, PipelineType pipelineType, string? label)
        : base(device, label)
    {
        PipelineType = pipelineType;
    }

    public PipelineType PipelineType { get; }
}
