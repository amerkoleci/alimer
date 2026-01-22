// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

partial class Mesh
{
    public static Mesh CreateCube(float size)
    {
        return CreateBox(new Vector3(size));
    }

    public static Mesh CreateBox(in Vector3 size)
    {
        Span<VertexPositionNormalTexture> vertices = stackalloc VertexPositionNormalTexture[24];
        Span<ushort> indices = stackalloc ushort[36];

        const int CubeFaceCount = 6;

        Vector3[] faceNormals =
        [
            Vector3.UnitZ,
            new Vector3(0.0f, 0.0f, -1.0f),
            Vector3.UnitX,
            new Vector3(-1.0f, 0.0f, 0.0f),
            Vector3.UnitY,
            new Vector3(0.0f, -1.0f, 0.0f),
        ];

        Vector2[] textureCoordinates =
        [
            Vector2.UnitX,
            Vector2.One,
            Vector2.UnitY,
            Vector2.Zero,
        ];

        Vector3 tsize = size / 2.0f;

        // Create each face in turn.
        int vbase = 0;
        int vertexIndex = 0;
        int index = 0;
        for (int i = 0; i < CubeFaceCount; i++)
        {
            Vector3 normal = faceNormals[i];

            // Get two vectors perpendicular both to the face normal and to each other.
            Vector3 basis = (i >= 4) ? Vector3.UnitZ : Vector3.UnitY;

            Vector3 side1 = Vector3.Cross(normal, basis);
            Vector3 side2 = Vector3.Cross(normal, side1);

            // Six indices (two triangles) per face.
            indices[index++] = (ushort)(vbase + 0);
            indices[index++] = (ushort)(vbase + 1);
            indices[index++] = (ushort)(vbase + 2);

            indices[index++] = (ushort)(vbase + 0);
            indices[index++] = (ushort)(vbase + 2);
            indices[index++] = (ushort)(vbase + 3);

            // Four vertices per face.
            // (normal - side1 - side2) * tsize // normal // t0
            vertices[vertexIndex++] = new VertexPositionNormalTexture(
                Vector3.Multiply(Vector3.Subtract(Vector3.Subtract(normal, side1), side2), tsize),
                normal,
                textureCoordinates[0]
                );

            // (normal - side1 + side2) * tsize // normal // t1
            vertices[vertexIndex++] = new VertexPositionNormalTexture(
                Vector3.Multiply(Vector3.Add(Vector3.Subtract(normal, side1), side2), tsize),
                normal,
                textureCoordinates[1]
                );

            // (normal + side1 + side2) * tsize // normal // t2
            vertices[vertexIndex++] = new VertexPositionNormalTexture(
                    Vector3.Multiply(Vector3.Add(normal, Vector3.Add(side1, side2)), tsize),
                    normal,
                    textureCoordinates[2]
                    );

            // (normal + side1 - side2) * tsize // normal // t3
            vertices[vertexIndex++] = new VertexPositionNormalTexture(
                Vector3.Multiply(Vector3.Subtract(Vector3.Add(normal, side1), side2), tsize),
                normal,
                textureCoordinates[3]
                );

            vbase += 4;
        }

        Mesh mesh = new();
        mesh.SetVertices(vertices);
        mesh.SetIndices(indices);
        return mesh;

    }
}
