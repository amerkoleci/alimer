// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

[StructLayout(LayoutKind.Sequential, Pack = 4)]
public record struct VertexPositionNormalTexture : IMeshVertex
{
    public static unsafe uint SizeInBytes => (uint)sizeof(VertexPositionNormalTexture);

    public Vector3 Position;
    public Vector3 Normal;
    public Vector2 TextureCoordinate;

    private static readonly MeshVertexAttribute[] s_vertexAttributes =
    [
        new MeshVertexAttribute(MeshVertexAttributeSemantic.Position, VertexFormat.Float3, 0),
        new MeshVertexAttribute(MeshVertexAttributeSemantic.Normal, VertexFormat.Float3, 12),
        new MeshVertexAttribute(MeshVertexAttributeSemantic.TexCoord0, VertexFormat.Float2, 24)
    ];

    public readonly MeshVertexAttribute[] VertexAttributes => s_vertexAttributes;

    public VertexPositionNormalTexture(in Vector3 position, in Vector3 normal, in Vector2 textureCoordinate)
    {
        Position = position;
        Normal = normal;
        TextureCoordinate = textureCoordinate;
    }
}
