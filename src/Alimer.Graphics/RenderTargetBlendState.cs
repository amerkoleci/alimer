// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct RenderTargetBlendState
{
    public RenderTargetBlendState()
    {
    }

    public BlendFactor SourceColorBlendFactor { get; init; } = BlendFactor.One;
    public BlendFactor DestinationColorBlendFactor { get; init; } = BlendFactor.Zero;
    public BlendOperation BlendOperation { get; init; } = BlendOperation.Add;
    public BlendFactor SourceAlphaBlendFactor { get; init; } = BlendFactor.One;
    public BlendFactor DestinationAlphaBlendFactor { get; init; } = BlendFactor.Zero;
    public BlendOperation AlphaBlendOperation { get; init; } = BlendOperation.Add;
    public ColorWriteMask WriteMask { get; init; } = ColorWriteMask.All;

    public static readonly RenderTargetBlendState Opaque = new()
    {
        SourceColorBlendFactor = BlendFactor.One,
        DestinationColorBlendFactor = BlendFactor.Zero,
        BlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.One,
        DestinationAlphaBlendFactor = BlendFactor.Zero,
        AlphaBlendOperation = BlendOperation.Add,
        WriteMask = ColorWriteMask.All
    };

    public static readonly RenderTargetBlendState AlphaBlend = new()
    {
        SourceColorBlendFactor = BlendFactor.One,
        DestinationColorBlendFactor = BlendFactor.OneMinusSourceAlpha,
        BlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.One,
        DestinationAlphaBlendFactor = BlendFactor.OneMinusSourceAlpha,
        AlphaBlendOperation = BlendOperation.Add,
        WriteMask = ColorWriteMask.All
    };

    public static readonly RenderTargetBlendState Additive = new()
    {
        SourceColorBlendFactor = BlendFactor.SourceAlpha,
        DestinationColorBlendFactor = BlendFactor.One,
        BlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.SourceAlpha,
        DestinationAlphaBlendFactor = BlendFactor.One,
        AlphaBlendOperation = BlendOperation.Add,
        WriteMask = ColorWriteMask.All
    };

    public static readonly RenderTargetBlendState NonPremultiplied = new()
    {
        SourceColorBlendFactor = BlendFactor.SourceAlpha,
        DestinationColorBlendFactor = BlendFactor.OneMinusSourceAlpha,
        BlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.SourceAlpha,
        DestinationAlphaBlendFactor = BlendFactor.OneMinusSourceAlpha,
        AlphaBlendOperation = BlendOperation.Add,
        WriteMask = ColorWriteMask.All
    };
}
