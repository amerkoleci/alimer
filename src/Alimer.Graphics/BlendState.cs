// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct BlendState
{
    public static BlendState Opaque => new(RenderTargetBlendState.Opaque);
    public static BlendState AlphaBlend => new(RenderTargetBlendState.AlphaBlend);
    public static BlendState Additive => new(RenderTargetBlendState.Additive);
    public static BlendState NonPremultiplied => new(RenderTargetBlendState.NonPremultiplied);

    public BlendState(params RenderTargetBlendState[] renderTargets)
    {
        AlphaToCoverageEnabled = false;
        RenderTargets = renderTargets;
    }

    public BlendState(bool alphaToCoverageEnabled, params RenderTargetBlendState[] renderTargets)
    {
        AlphaToCoverageEnabled = alphaToCoverageEnabled;
        RenderTargets = renderTargets;
    }

    public bool AlphaToCoverageEnabled { get; init; }

    public RenderTargetBlendState[] RenderTargets { get; init; }
}
