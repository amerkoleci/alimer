// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

partial class Mesh
{
    public static Mesh CreateCube(float size, float uScale = 1.0f, float vScale = 1.0f)
    {
        return CreateBox(new(size), uScale, vScale);
    }

    public static Mesh CreateBox(Vector3 size, float uScale = 1.0f, float vScale = 1.0f)
    {
        Span<VertexPositionNormalTexture> vertices = stackalloc VertexPositionNormalTexture[24];
        Span<ushort> indices = stackalloc ushort[36];

        const int CubeFaceCount = 6;

        Vector3[] faceNormals =
        [
            new Vector3(0, 0, 1),
            new Vector3(0, 0, -1),
            new Vector3(1, 0, 0),
            new Vector3(-1, 0, 0),
            new Vector3(0, 1, 0),
            new Vector3(0, -1, 0),
        ];

        Vector2 uvScale = new(uScale, vScale);
        Vector2[] texCoords =
        [
            new Vector2(1, 0) * uvScale,
            new Vector2(1, 1) * uvScale,
            new Vector2(0, 1) * uvScale,
            new Vector2(0, 0) * uvScale,
        ];

        size /= 2.0f;

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
            indices[index++] = (ushort)(vbase + 2);
            indices[index++] = (ushort)(vbase + 1);

            indices[index++] = (ushort)(vbase + 0);
            indices[index++] = (ushort)(vbase + 3);
            indices[index++] = (ushort)(vbase + 2);

            // Four vertices per face.
            // (normal - side1 - side2) * tsize // normal // t0
            vertices[vertexIndex++] = new VertexPositionNormalTexture((normal - side1 - side2) * size, normal, texCoords[0]);

            // (normal - side1 + side2) * tsize // normal // t1
            vertices[vertexIndex++] = new VertexPositionNormalTexture((normal - side1 + side2) * size, normal, texCoords[1]);

            // (normal + side1 + side2) * tsize // normal // t2
            vertices[vertexIndex++] = new VertexPositionNormalTexture((normal + side1 + side2) * size, normal, texCoords[2]);

            // (normal + side1 - side2) * tsize // normal // t3
            vertices[vertexIndex++] = new VertexPositionNormalTexture((normal + side1 - side2) * size, normal, texCoords[3]);

            vbase += 4;
        }

        Mesh mesh = new();
        mesh.SetVertices(vertices);
        mesh.SetIndices(indices);
        mesh.RecalculateBounds();
        return mesh;
    }

    public static Mesh CreateSphere(float radius = 0.5f, int tessellation = 16, float uScale = 1.0f, float vScale = 1.0f)
    {
        if (tessellation < 3)
            tessellation = 3;

        int verticalSegments = tessellation;
        int horizontalSegments = tessellation * 2;

        int vertexCount = (verticalSegments + 1) * (horizontalSegments + 1);
        int indexCount = (verticalSegments) * (horizontalSegments + 1) * 6;

        Span<VertexPositionNormalTexture> vertices = stackalloc VertexPositionNormalTexture[vertexCount];
        Span<ushort> indices = stackalloc ushort[indexCount];

        vertexCount = 0;

        // Generate the first extremity points
        for (int j = 0; j <= horizontalSegments; j++)
        {
            Vector3 normal = new(0, -1, 0);
            Vector2 texcoord = new(uScale * j / horizontalSegments, vScale);

            vertices[vertexCount++] = new VertexPositionNormalTexture(
                normal * radius,
                normal,
                texcoord
                );
        }

        // Create rings of vertices at progressively higher latitudes.
        for (int i = 1; i < verticalSegments; i++)
        {
            float v = vScale * (1.0f - (float)i / verticalSegments);

            float latitude = (float)((i * MathF.PI / verticalSegments) - MathF.PI / 2.0f);
            (float dy, float dxz) = MathF.SinCos(latitude);

            // the first point
            Vector3 firstNormal = new(0, dy, dxz);
            VertexPositionNormalTexture firstHorizontalVertex = new(firstNormal * radius, firstNormal, new Vector2(0, v));

            vertices[vertexCount++] = firstHorizontalVertex;

            // Create a single ring of vertices at this latitude.
            for (int j = 1; j < horizontalSegments; j++)
            {
                float u = (uScale * j) / horizontalSegments;

                float longitude = j * 2.0f * MathF.PI / horizontalSegments;
                (float dx, float dz) = MathF.SinCos(longitude);

                dx *= dxz;
                dz *= dxz;

                Vector3 normal = new(dx, dy, dz);
                Vector2 textcoord = new(u, v);

                vertices[vertexCount++] = new VertexPositionNormalTexture(
                    normal * radius,
                    normal,
                    textcoord
                );
            }

            // the last point equal to the first point
            firstHorizontalVertex.TextureCoordinate = new Vector2(uScale, v);
            vertices[vertexCount++] = firstHorizontalVertex;
        }

        // Generate the end extremity points
        for (int j = 0; j <= horizontalSegments; j++)
        {
            Vector3 normal = Vector3.UnitY;
            Vector2 textcoord = new(uScale * j / horizontalSegments, 0f);

            vertices[vertexCount++] = new VertexPositionNormalTexture(normal * radius, normal, textcoord);
        }

        // Fill the index buffer with triangles joining each pair of latitude rings.
        int stride = horizontalSegments + 1;

        indexCount = 0;
        for (int i = 0; i < verticalSegments; i++)
        {
            for (int j = 0; j <= horizontalSegments; j++)
            {
                int nextI = i + 1;
                int nextJ = (j + 1) % stride;

                indices[indexCount++] = (ushort)(i * stride + j);
                indices[indexCount++] = (ushort)(nextI * stride + j);
                indices[indexCount++] = (ushort)(i * stride + nextJ);

                indices[indexCount++] = (ushort)(i * stride + nextJ);
                indices[indexCount++] = (ushort)(nextI * stride + j);
                indices[indexCount++] = (ushort)(nextI * stride + nextJ);
            }
        }

        Mesh mesh = new();
        mesh.SetVertices(vertices);
        mesh.SetIndices(indices);
        mesh.RecalculateBounds();
        return mesh;
    }
}
