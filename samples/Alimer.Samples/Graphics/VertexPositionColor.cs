// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Samples;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly record struct VertexPositionColor
{
    public static readonly unsafe int SizeInBytes = sizeof(VertexPositionColor);

    public static readonly VertexAttribute[] VertexAttributes =
    [
        new VertexAttribute(VertexAttributeSemantic.Position, VertexAttributeFormat.Float32x3, 0),
        new VertexAttribute(VertexAttributeSemantic.Color, VertexAttributeFormat.Float32x4, 12)
    ];

    public VertexPositionColor(in Vector3 position, in Color color)
    {
        Position = position;
        Color = color;
    }

    public readonly Vector3 Position;
    public readonly Color Color;
}
