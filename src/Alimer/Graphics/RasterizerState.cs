// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum FillMode
{
    Solid,
    Wireframe,
}

public enum CullMode
{
    Back,
    Front,
    None,
}

public enum FrontFace
{
    Clockwise,
    CounterClockwise,
}


public enum DepthClipMode
{
    Clip,
    Clamp
}

public readonly record struct RasterizerState
{
    public FillMode FillMode { get; init; }
    public CullMode CullMode { get; init; }
    public FrontFace FrontFace { get; init; }
    public DepthClipMode DepthClipMode { get; init; }
    public bool ConservativeRaster { get; init; }

    public RasterizerState()
    {
        FillMode = FillMode.Solid;
        CullMode = CullMode.Back;
        FrontFace = FrontFace.Clockwise;
        DepthClipMode = DepthClipMode.Clip;
        ConservativeRaster = false;
    }

    public static RasterizerState CullNone => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.None,
        FrontFace = FrontFace.Clockwise,
    };

    public static RasterizerState CullFront => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Front,
        FrontFace = FrontFace.Clockwise,
    };

    public static RasterizerState CullBack => new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Back,
        FrontFace = FrontFace.Clockwise,
    };

    public static RasterizerState Default => CullBack;
}
