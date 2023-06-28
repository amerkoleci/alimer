// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct RasterizerState
{
    public FillMode FillMode { get; init; }
    public CullMode CullMode { get; init; }
    public FrontFaceWinding FrontFace { get; init; }

    public RasterizerState()
    {
        FillMode = FillMode.Solid;
        CullMode = CullMode.Back;
        FrontFace = FrontFaceWinding.Clockwise;
    }

    public static readonly RasterizerState CullNone = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.None,
        FrontFace = FrontFaceWinding.Clockwise,
    };

    public static readonly RasterizerState CullFront = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Front,
        FrontFace = FrontFaceWinding.Clockwise,
    };

    public static readonly RasterizerState CullBack = new()
    {
        FillMode = FillMode.Solid,
        CullMode = CullMode.Back,
        FrontFace = FrontFaceWinding.Clockwise,
    };

    public static readonly RasterizerState Default = CullBack;
}
