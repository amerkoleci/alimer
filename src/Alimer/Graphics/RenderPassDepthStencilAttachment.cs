// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public record struct RenderPassDepthStencilAttachment
{
    /// <summary>
    /// The <see cref="TextureView"/> associated with this attachment.
    /// </summary>
    public TextureView? View;

    /// <summary>
    /// The action performed by this attachment at the start of a rendering pass.
    /// </summary>
    public LoadAction DepthLoadAction = LoadAction.Clear;

    /// <summary>
    /// The action performed by this attachment at the end of a rendering pass.
    /// </summary>
    public StoreAction DepthStoreAction = StoreAction.Discard;

    /// <summary>
    /// The depth to use when clearing the depth attachment.
    /// </summary>
    public float DepthClearValue = 1.0f;

    /// <summary>
    /// Is depth read-only during the rendering pass.
    /// </summary>
    public bool DepthReadOnly = false;

    /// <summary>
    /// The action performed by this attachment at the start of a rendering pass.
    /// </summary>
    public LoadAction StencilLoadAction  = LoadAction.Clear;

    /// <summary>
    /// The action performed by this attachment at the end of a rendering pass.
    /// </summary>
    public StoreAction StencilStoreAction = StoreAction.Discard;

    /// <summary>
    /// The value to use when clearing the stencil attachment.
    /// </summary>
    public byte StencilClearValue = 0;

    /// <summary>
    /// Gets a value indicating whether the stencil buffer is read-only.
    /// </summary>
    public bool StencilReadOnly  = false;

    public RenderPassDepthStencilAttachment(TextureView view)
    {
        Guard.IsNotNull(view, nameof(view));

        View = view;
    }

    public RenderPassDepthStencilAttachment(Texture texture)
    {
        Guard.IsNotNull(texture, nameof(texture));
        Guard.IsNotNull(texture.DefaultView, nameof(texture.DefaultView));

        View = texture.DefaultView;
    }
}
