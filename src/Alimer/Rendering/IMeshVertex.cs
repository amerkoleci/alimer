// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// A Vertex struct to be used in a Mesh
/// </summary>
public interface IMeshVertex
{
    [NotNull]
    static abstract Mesh.VertexAttribute[] VertexAttributes { get; }
}
