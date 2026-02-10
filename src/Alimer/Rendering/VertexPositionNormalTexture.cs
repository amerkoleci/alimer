// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Rendering;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public record struct VertexPositionNormalTexture : IMeshVertex
{
    public static unsafe int SizeInBytes => sizeof(VertexPositionNormalTexture);

    public Vector3 Position;
    public Vector3 Normal;
    public Vector2 TextureCoordinate;

    public static VertexAttribute[] VertexAttributes { get; } = [
        new(VertexAttributeSemantic.Position, VertexFormat.Float3),
        new(VertexAttributeSemantic.Normal, VertexFormat.Float3),
        new(VertexAttributeSemantic.TexCoord, VertexFormat.Float2)
    ];

    public VertexPositionNormalTexture(in Vector3 position, in Vector3 normal, in Vector2 textureCoordinate)
    {
        Position = position;
        Normal = normal;
        TextureCoordinate = textureCoordinate;
    }
}
