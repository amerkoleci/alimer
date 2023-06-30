// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct RenderPassDescription
{
    public RenderPassDescription()
    {
        ColorAttachments = Array.Empty<RenderPassColorAttachment>();
    }

    public RenderPassDescription(params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
    }

    public RenderPassDescription(RenderPassDepthStencilAttachment depthStencilAttachment, params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
        DepthStencilAttachment = depthStencilAttachment;
    }

    public RenderPassColorAttachment[] ColorAttachments { get; init; }
    public RenderPassDepthStencilAttachment? DepthStencilAttachment { get; init; }

    /// <summary>
    /// Gets or sets the label of the render pass.
    /// </summary>
    public string? Label { get; init; }
}
