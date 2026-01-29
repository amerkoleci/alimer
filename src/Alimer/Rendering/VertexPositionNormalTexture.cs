// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Rendering;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public record struct VertexPositionNormalTexture : IMeshVertex
{
    public static unsafe uint SizeInBytes => (uint)sizeof(VertexPositionNormalTexture);

    public Vector3 Position;
    public Vector3 Normal;
    public Vector2 TextureCoordinate;

    public static Mesh.VertexAttribute[] VertexAttributes { get; } = [
        new(Mesh.VertexAttributeSemantic.Position, VertexFormat.Float3, 0),
        new(Mesh.VertexAttributeSemantic.Normal, VertexFormat.Float3, 12),
        new(Mesh.VertexAttributeSemantic.TexCoord0, VertexFormat.Float2, 24)
    ];

    public static VertexAttribute[] RHIVertexAttributes { get; } = [
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
}
