// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.RHI;

namespace Alimer.Graphics;

public abstract class RenderPipeline : RHIObject
{
    protected RenderPipeline(string? label)
        : base(label)
    {
    }

    /// <summary>
    /// Get the <see cref="PipelineLayout"/>.
    /// </summary>
    public abstract PipelineLayout Layout { get; }
}
