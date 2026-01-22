// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public  record struct RasterizerState
{
    public FillMode FillMode = FillMode.Solid;
    public CullMode CullMode = CullMode.Back;
    public FrontFace FrontFace = FrontFace.CounterClockwise;
    public float DepthBias = 0.0f;
    public float DepthBiasClamp = 0.0f;
    public float DepthBiasSlopeScale = 0.0f;
    public DepthClipMode DepthClipMode = DepthClipMode.Clip;
    public bool ConservativeRasterEnable = false;

    public RasterizerState()
    {
    }

    public static RasterizerState CullNone => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.None,
    };

    public static RasterizerState CullFront => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Front,
    };

    public static RasterizerState CullBack => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Back,
    };

    public static RasterizerState Default => CullBack;
}
