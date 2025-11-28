// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Describes the configuration of a render pass, including its color and depth-stencil attachments, for use in graphics
/// rendering operations.
/// </summary>
/// <remarks>
/// A render pass defines the set of attachments and parameters used when rendering a frame or a portion
/// of a frame. Use this descriptor to specify the color attachments and, optionally, a depth-stencil attachment for a
/// rendering operation. The configuration provided by this descriptor is typically consumed by graphics APIs or engines
/// to set up rendering pipelines. Modifying the attachments or parameters affects how rendering output is generated and
/// stored.
/// </remarks>
public ref struct RenderPassDescriptor
{
    public Span<RenderPassColorAttachment> ColorAttachments;
    public RenderPassDepthStencilAttachment DepthStencilAttachment;

    /// <summary>
    /// Gets or sets the label of the render pass.
    /// </summary>
    public Utf8ReadOnlyString Label;

    public RenderPassDescriptor()
    {
        ColorAttachments = [];
        DepthStencilAttachment = default;
    }

    public RenderPassDescriptor(params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
        DepthStencilAttachment = default;
    }

    public RenderPassDescriptor(RenderPassDepthStencilAttachment depthStencilAttachment, params RenderPassColorAttachment[] colorAttachments)
    {
        ColorAttachments = colorAttachments;
        DepthStencilAttachment = depthStencilAttachment;
    }
}
