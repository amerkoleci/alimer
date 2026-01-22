// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

/// <summary>
/// Defines a portion of a <see cref="Mesh"/> with its own material
/// </summary>
public sealed class SubMesh
{
    internal SubMesh(Mesh mesh, int indexStart, int indexCount, int materialIndex = 0)
    {
        Guard.IsNotNull(mesh, nameof(mesh));

        Mesh = mesh;
        IndexStart = indexStart;
        IndexCount = indexCount;
        MaterialIndex = materialIndex;
    }

    public Mesh Mesh { get; }

    public int IndexStart { get; set; }
    public int IndexCount { get; set; }
    public int MaterialIndex { get; set; }
}
