// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
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
    public MeshAsset(MeshData data, Material[] materials)
    {
        ArgumentNullException.ThrowIfNull(data.Positions, nameof(MeshData.Positions));
        ArgumentNullException.ThrowIfNull(materials, nameof(materials));
        ArgumentOutOfRangeException.ThrowIfLessThanOrEqual(materials.Length, 0, nameof(materials));

        Data = data;
        Materials = materials;
    }


    public required MeshData Data { get; set; }
    public required Material[] Materials { get; set; }
}
