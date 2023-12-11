// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Rendering;
using Alimer.Numerics;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Mesh"/> asset.
/// </summary>
public class MeshAsset : AssetWithSource
{
    public int VertexCount { get; set; }

    public Vector3[]? Positions { get; set; }
    public Vector3[]? Normals { get; set; }
    public Vector3[]? Tangents { get; set; }
    public Vector2[]? Texcoords { get; set; }
    public Vector2[]? Texcoords2 { get; set; }
    public Color[]? Colors { get; set; }
    public uint[]? Indices { get; set; }
}
