// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Numerics;

/// <summary>
/// Defines extension methods for <see cref="Plane"/>
/// </summary>
public static class PlaneExtensions
{
    /// <summary>
    /// Returns the distance from this plane to given point.
    /// </summary>
    /// <param name="point"></param>
    /// <returns></returns>
    public static float GetDistance(this in Plane plane, in Vector3 point)
    {
        return plane.Normal.X * point.X + plane.Normal.Y * point.Y + plane.Normal.Z * point.Z + plane.D;
    }

    public static PlaneIntersectionType Intersects(this in Plane plane, in BoundingSphere sphere) => sphere.Intersects(plane);
    public static PlaneIntersectionType Intersects(this in Plane plane, in BoundingBox box) => box.Intersects(plane);
}
