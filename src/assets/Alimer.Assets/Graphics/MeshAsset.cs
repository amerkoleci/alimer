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
    public MeshData? Data { get; set; }
}
