// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Vortice.Mathematics;

namespace Alimer.Rendering;

public sealed class MeshData 
{
    public MeshData()
    {
    }

    /// <summary>
    /// Gets or sets number of vertices in the vertex buffers.
    /// </summary>
    public int VertexCount { get; set; }

    public Vector3[] Positions { get; set; } = [];
    public Vector3[]? Normals { get; set; }
    public Vector3[]? Tangents { get; set; }
    public Vector2[]? Texcoords { get; set; }
    public Vector2[]? Texcoords2 { get; set; }
    public Color4[]? Colors { get; set; }
    public uint[]? Indices { get; set; }
}
