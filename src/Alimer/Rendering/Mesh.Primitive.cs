// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

partial class Mesh 
{
    public static Mesh CreateCube(GraphicsDevice device, float size)
    {
        return CreateBox(device, new Vector3(size));
    }

    public static Mesh CreateBox(GraphicsDevice device, in Vector3 size)
    {
        // TODO
        Mesh mesh = new(device, 24,
            new VertexAttribute(VertexFormat.Float3, 0, 0),     // Position
            new VertexAttribute(VertexFormat.Float3, 12, 1),    // Normal
            new VertexAttribute(VertexFormat.Float2, 24, 2)     // TexCoord
            );
        return mesh;
    }
}
