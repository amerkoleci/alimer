// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct RasterizerState
{
    public FillMode FillMode { get; init; }
    public CullMode CullMode { get; init; }
    public bool FrontFaceCounterClockwise { get; init; }
    public DepthClipMode DepthClipMode { get; init; }

    public RasterizerState()
    {
        FillMode = FillMode.Solid;
        CullMode = CullMode.Back;
        FrontFaceCounterClockwise = false;
    }

    public static readonly RasterizerState CullNone = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.None,
        FrontFaceCounterClockwise = false,
    };

    public static readonly RasterizerState CullFront = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Front,
        FrontFaceCounterClockwise = false,
    };

    public static readonly RasterizerState CullBack = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Back,
        FrontFaceCounterClockwise = false,
    };

    public static readonly RasterizerState Default = CullBack;
}
