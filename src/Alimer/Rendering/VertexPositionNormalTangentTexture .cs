// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Rendering;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public record struct VertexPositionNormalTangentTexture : IMeshVertex
{
    public static unsafe int SizeInBytes => sizeof(VertexPositionNormalTangentTexture);

    public Vector3 Position;
    public Vector3 Normal;
    public Vector3 Tangent;
    public Vector2 TextureCoordinate;

    public static VertexAttribute[] VertexAttributes { get; } = [
        new(VertexAttributeSemantic.Position, VertexAttributeFormat.Float32x3),
        new(VertexAttributeSemantic.Normal, VertexAttributeFormat.Float32x3),
        new(VertexAttributeSemantic.Tangent, VertexAttributeFormat.Float32x3),
        new(VertexAttributeSemantic.TexCoord, VertexAttributeFormat.Float32x2)
    ];

    public VertexPositionNormalTangentTexture(in Vector3 position, in Vector3 normal, in Vector3 tangent, in Vector2 textureCoordinate)
    {
        Position = position;
        Normal = normal;
        Tangent = tangent;
        TextureCoordinate = textureCoordinate;
    }
}
