// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Samples;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly struct VertexPositionColor
{
    public static readonly unsafe uint SizeInBytes = (uint)sizeof(VertexPositionColor);

    public static readonly VertexAttribute[] VertexAttributes =
    [
        new VertexAttribute(VertexFormat.Float3, 0, 0),
        new VertexAttribute(VertexFormat.Float4, 12, 1)
    ];

    public VertexPositionColor(in Vector3 position, in Vector4 color)
    {
        Position = position;
        Color = color;
    }

    public readonly Vector3 Position;
    public readonly Vector4 Color;
}
