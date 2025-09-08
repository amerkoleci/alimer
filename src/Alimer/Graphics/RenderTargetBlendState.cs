// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct RenderTargetBlendState
{
    public static RenderTargetBlendState Opaque => new()
    {
        SourceColorBlendFactor = BlendFactor.One,
        DestinationColorBlendFactor = BlendFactor.Zero,
        ColorBlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.One,
        DestinationAlphaBlendFactor = BlendFactor.Zero,
        AlphaBlendOperation = BlendOperation.Add,
        ColorWriteMask = ColorWriteMask.All
    };

    public static RenderTargetBlendState AlphaBlend => new()
    {
        SourceColorBlendFactor = BlendFactor.One,
        DestinationColorBlendFactor = BlendFactor.OneMinusSourceAlpha,
        ColorBlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.One,
        DestinationAlphaBlendFactor = BlendFactor.OneMinusSourceAlpha,
        AlphaBlendOperation = BlendOperation.Add,
        ColorWriteMask = ColorWriteMask.All
    };

    public static RenderTargetBlendState Additive => new()
    {
        SourceColorBlendFactor = BlendFactor.SourceAlpha,
        DestinationColorBlendFactor = BlendFactor.One,
        ColorBlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.SourceAlpha,
        DestinationAlphaBlendFactor = BlendFactor.One,
        AlphaBlendOperation = BlendOperation.Add,
        ColorWriteMask = ColorWriteMask.All
    };

    public static RenderTargetBlendState NonPremultiplied => new()
    {
        SourceColorBlendFactor = BlendFactor.SourceAlpha,
        DestinationColorBlendFactor = BlendFactor.OneMinusSourceAlpha,
        ColorBlendOperation = BlendOperation.Add,
        SourceAlphaBlendFactor = BlendFactor.SourceAlpha,
        DestinationAlphaBlendFactor = BlendFactor.OneMinusSourceAlpha,
        AlphaBlendOperation = BlendOperation.Add,
        ColorWriteMask = ColorWriteMask.All
    };

    /// <summary>
    /// Initializes a new instance of the <see cref="RenderTargetBlendState"/> struct with default values.
    /// </summary>
    public RenderTargetBlendState()
    {
        SourceColorBlendFactor = BlendFactor.One;
        DestinationColorBlendFactor = BlendFactor.Zero;
        ColorBlendOperation = BlendOperation.Add;
        SourceAlphaBlendFactor = BlendFactor.One;
        DestinationAlphaBlendFactor = BlendFactor.Zero;
        AlphaBlendOperation = BlendOperation.Add;
        ColorWriteMask = ColorWriteMask.All;
    }

    /// <summary>
    /// Constructs a new <see cref="RenderTargetBlendState"/>.
    /// </summary>
    /// <param name="sourceColorFactor">Controls the source color's influence on the blend result.</param>
    /// <param name="destinationColorFactor">Controls the destination color's influence on the blend result.</param>
    /// <param name="colorBlendOperation">Controls the function used to combine the source and destination color factors.</param>
    /// <param name="sourceAlphaFactor">Controls the source alpha's influence on the blend result.</param>
    /// <param name="destinationAlphaFactor">Controls the destination alpha's influence on the blend result.</param>
    /// <param name="alphaBlendOperation">Controls the function used to combine the source and destination alpha factors.</param>
    /// <param name="colorWriteMask"></param>
    public RenderTargetBlendState(
        BlendFactor sourceColorFactor,
        BlendFactor destinationColorFactor,
        BlendOperation colorBlendOperation,
        BlendFactor sourceAlphaFactor,
        BlendFactor destinationAlphaFactor,
        BlendOperation alphaBlendOperation,
        ColorWriteMask colorWriteMask = ColorWriteMask.All)
    {
        SourceColorBlendFactor = sourceColorFactor;
        DestinationColorBlendFactor = destinationColorFactor;
        ColorBlendOperation = colorBlendOperation;
        SourceAlphaBlendFactor = sourceAlphaFactor;
        DestinationAlphaBlendFactor = destinationAlphaFactor;
        AlphaBlendOperation = alphaBlendOperation;
        ColorWriteMask = colorWriteMask;
    }

    public BlendFactor SourceColorBlendFactor { get; init; }
    public BlendFactor DestinationColorBlendFactor { get; init; } 
    public BlendOperation ColorBlendOperation { get; init; } 
    public BlendFactor SourceAlphaBlendFactor { get; init; } 
    public BlendFactor DestinationAlphaBlendFactor { get; init; }
    public BlendOperation AlphaBlendOperation { get; init; } 
    public ColorWriteMask ColorWriteMask { get; init; } 

}
