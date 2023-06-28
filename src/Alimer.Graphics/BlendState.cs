// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct BlendState
{
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

    public static readonly BlendState Opaque = new(RenderTargetBlendState.Opaque);
    public static readonly BlendState AlphaBlend = new(RenderTargetBlendState.AlphaBlend);
    public static readonly BlendState Additive = new(RenderTargetBlendState.Additive);
    public static readonly BlendState NonPremultiplied = new(RenderTargetBlendState.NonPremultiplied);
}
