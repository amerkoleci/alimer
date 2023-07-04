// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Pipeline : GraphicsObject
{
    protected Pipeline(PipelineType pipelineType, string? label)
        : base(label)
    {
        PipelineType = pipelineType;
    }

    public PipelineType PipelineType { get; }
}
