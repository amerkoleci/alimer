// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
//using MeshOptimizer;

namespace Alimer.Rendering;

/// <summary>
/// Defiens type of light in scene.
/// </summary>
public enum LightType
{
    /// <summary>
    /// Directional light (sun, moon).
    /// </summary>
    Directional,
    /// <summary>
    /// Point light (omnidirectional).
    /// </summary>
    Point,
    /// <summary>
    /// Spot light (cone-shaped).
    /// </summary>
    Spot,
}

