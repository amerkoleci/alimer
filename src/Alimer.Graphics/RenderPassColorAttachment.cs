// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct RenderPassColorAttachment
{
    public RenderPassColorAttachment(Texture texture)
    {
        Guard.IsNotNull(texture, nameof(texture));

        Texture = texture;
    }

    public RenderPassColorAttachment(Texture texture, in Color clearColor)
        : this(texture)
    {
        LoadAction = LoadAction.Clear;
        StoreAction = StoreAction.Store;
        ClearColor = clearColor;
    }

    /// <summary>
    /// The <see cref="Graphics.Texture"/> associated with this attachment.
    /// </summary>
    public Texture Texture { get; init; }

    /// <summary>
    /// The mipmap level of the texture used for rendering to the attachment.
    /// </summary>
    public int MipLevel { get; init; }

    /// <summary>
    /// The slice of the texture used for rendering to the attachment.
    /// </summary>
    public int Slice { get; init; }

    /// <summary>
    /// The action performed by this attachment at the start of a rendering pass.
    /// </summary>
    public LoadAction LoadAction { get; init; }

    /// <summary>
    /// The action performed by this attachment at the end of a rendering pass
    /// </summary>
    public StoreAction StoreAction { get; init; }

    /// <summary>
    /// The color to use when clearing the color attachment.
    /// </summary>
    public Color ClearColor { get; init; } = new(0.0f, 0.0f, 0.0f, 1.0f);

    /// <summary>
    /// The destination <see cref="Graphics.Texture"/> used when resolving multisampled texture data into single sample values.
    /// </summary>
    public Texture? ResolveTexture { get; init; }

    /// <summary>
    /// The mipmap level of the texture used for the multisample resolve action.
    /// </summary>
    public int ResolveMipLevel { get; init; }

    /// <summary>
    /// The slice of the texture used for the multisample resolve action.
    /// </summary>
    public int ResolveSlice { get; init; }
}
