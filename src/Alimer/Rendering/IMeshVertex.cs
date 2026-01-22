// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// A Vertex struct to be used in a Mesh
/// </summary>
public interface IMeshVertex
{
    MeshVertexAttribute[] VertexAttributes { get; }
}
