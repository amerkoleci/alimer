// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Numerics;

/// <summary>
/// Defines the intersection between a <see cref="Plane"/> and a bounding volume.
/// </summary>
public enum PlaneIntersectionType
{
    /// <summary>
    /// There is no intersection, the bounding volume is in the negative half space of the plane.
    /// </summary>
    Front,
    /// <summary>
    /// There is no intersection, the bounding volume is in the positive half space of the plane.
    /// </summary>
    Back,
    /// <summary>
    /// The plane is intersected.
    /// </summary>
    Intersecting
}
