// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Samples;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly record struct VertexPositionNormalTexture
{
    public static readonly unsafe uint SizeInBytes = (uint)sizeof(VertexPositionNormalTexture);

    public static readonly VertexAttribute[] VertexAttributes =
    [
        new VertexAttribute(VertexFormat.Float3, 0, 0),
        new VertexAttribute(VertexFormat.Float3, 12, 1),
        new VertexAttribute(VertexFormat.Float2, 24, 2)
    ];

    public VertexPositionNormalTexture(in Vector3 position, in Vector3 normal, in Vector2 textureCoordinate)
    {
        Position = position;
        Normal = normal;
        TextureCoordinate = textureCoordinate;
    }


    public readonly Vector3 Position;
    public readonly Vector3 Normal;
    public readonly Vector2 TextureCoordinate;

}
