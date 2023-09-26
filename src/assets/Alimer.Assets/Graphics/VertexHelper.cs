// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Assets.Graphics;

public static unsafe class VertexHelper
{
    public static Span<Vector3> GenerateTangents(
        Span<Vector3> positions,
        Span<Vector2> texcoords,
        Span<uint> indices)
    {
        Span<Vector3> tangentBuffer = new Vector3[positions.Length];

        int indexCount = indices.IsEmpty
            ? positions.Length / sizeof(Vector3)
            : indices.Length;

        for (int i = 0; i < indexCount; i += 3)
        {
            int index1 = i + 0;
            int index2 = i + 1;
            int index3 = i + 2;

            if (!indices.IsEmpty)
            {
                index1 = (int)indices[index1];
                index2 = (int)indices[index2];
                index3 = (int)indices[index3];
            }

            Vector3 position1 = positions[index1];
            Vector3 position2 = positions[index2];
            Vector3 position3 = positions[index3];

            Vector2 uv1 = texcoords[index1];
            Vector2 uv2 = texcoords[index2];
            Vector2 uv3 = texcoords[index3];

            Vector3 edge1 = position2 - position1;
            Vector3 edge2 = position3 - position1;

            Vector2 uvEdge1 = uv2 - uv1;
            Vector2 uvEdge2 = uv3 - uv1;

            float dR = uvEdge1.X * uvEdge2.Y - uvEdge2.X * uvEdge1.Y;

            if (Math.Abs(dR) < 1e-6f)
            {
                dR = 1.0f;
            }

            float r = 1.0f / dR;
            Vector3 t = (uvEdge2.Y * edge1 - uvEdge1.Y * edge2) * r;

            tangentBuffer[index1] += t;
            tangentBuffer[index2] += t;
            tangentBuffer[index3] += t;
        }

        return tangentBuffer;
    }
}

