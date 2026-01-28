// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// Base material class.
/// </summary>
public abstract class Material
{
    private static int s_nextMaterialId = 1;

    public Material()
    {
        Id = s_nextMaterialId++;
    }

    /// <summary>
    /// Gets the unique material id.
    /// </summary>
    public int Id { get; }
}
