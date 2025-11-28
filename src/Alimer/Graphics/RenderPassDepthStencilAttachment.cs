// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public record struct RenderPassDepthStencilAttachment
{
    /// <summary>
    /// The <see cref="Graphics.Texture"/> associated with this attachment.
    /// </summary>
    public Texture? Texture;

    /// <summary>
    /// The mipmap level of the texture used for rendering to the attachment.
    /// </summary>
    public uint MipLevel;

    /// <summary>
    /// The slice of the texture used for rendering to the attachment.
    /// </summary>
    public uint Slice;

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

    public RenderPassDepthStencilAttachment(Texture texture)
    {
        Guard.IsNotNull(texture, nameof(texture));

        Texture = texture;
    }
}
