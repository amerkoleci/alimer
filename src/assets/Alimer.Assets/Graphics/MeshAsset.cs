// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using Alimer.Rendering;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Mesh"/> asset.
/// </summary>
public class MeshAsset : AssetWithSource
{
    public MeshAsset()
    {

    }

    [SetsRequiredMembers]
    public MeshAsset(Mesh mesh, Material[] materials)
    {
        ArgumentNullException.ThrowIfNull(mesh, nameof(mesh));
        ArgumentNullException.ThrowIfNull(materials, nameof(materials));
        ArgumentOutOfRangeException.ThrowIfLessThanOrEqual(materials.Length, 0, nameof(materials));

        Mesh = mesh;
        Materials = materials;
    }


    public required Mesh Mesh { get; set; }
    public required Material[] Materials { get; set; }
    public Vector3 Translation { get; set; }
    public Quaternion Rotation { get; set; }
    public Vector3 Scale { get; set; }

    /// <inheritdoc/>
    protected override void Destroy()
    {
    }
}
