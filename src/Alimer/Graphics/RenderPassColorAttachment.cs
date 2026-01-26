// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public record struct RenderPassColorAttachment
{
    /// <summary>
    /// The <see cref="TextureView"/> associated with this attachment.
    /// </summary>
    public required TextureView View;

    /// <summary>
    /// The action performed by this attachment at the start of a rendering pass.
    /// </summary>
    public LoadAction LoadAction;

    /// <summary>
    /// The action performed by this attachment at the end of a rendering pass
    /// </summary>
    public StoreAction StoreAction;

    /// <summary>
    /// The color to use when clearing the color attachment.
    /// </summary>
    public Color ClearColor = new(0.0f, 0.0f, 0.0f, 1.0f);

    /// <summary>
    /// The optional destination <see cref="TextureView"/> used when resolving multisampled texture data into single sample values.
    /// </summary>
    public TextureView? ResolveView;

    [SetsRequiredMembers]
    public RenderPassColorAttachment([NotNull] TextureView view)
    {
        Guard.IsNotNull(view, nameof(view));

        View = view;
    }

    [SetsRequiredMembers]
    public RenderPassColorAttachment([NotNull] TextureView view, in Color clearColor)
    {
        Guard.IsNotNull(view, nameof(view));

        View = view;
        LoadAction = LoadAction.Clear;
        StoreAction = StoreAction.Store;
        ClearColor = clearColor;
    }
}
