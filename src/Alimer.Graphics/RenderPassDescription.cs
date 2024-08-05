// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public record struct RenderPassDescription
{
    public RenderPassDescription()
    {
        ColorAttachments = [];
        DepthStencilAttachment = default;
    }

    public RenderPassDescription(params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
        DepthStencilAttachment = default;
    }

    public RenderPassDescription(RenderPassDepthStencilAttachment depthStencilAttachment, params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
        DepthStencilAttachment = depthStencilAttachment;
    }

    public RenderPassColorAttachment[] ColorAttachments { get; set; }
    public RenderPassDepthStencilAttachment DepthStencilAttachment { get; set; }

    /// <summary>
    /// Gets or sets the label of the render pass.
    /// </summary>
    public string? Label { get; set; }
}
