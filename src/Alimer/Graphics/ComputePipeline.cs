// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class ComputePipeline : GraphicsObject
{
    protected ComputePipeline(string? label)
        : base(label)
    {
    }

    /// <summary>
    /// Get the <see cref="PipelineLayout"/>.
    /// </summary>
    public abstract PipelineLayout Layout { get; }
}
