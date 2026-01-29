// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Numerics;

/// <summary>
/// Defines extension methods for <see cref="Plane"/>
/// </summary>
public static class PlaneExtensions
{
    public static float Distance(this in Plane plane, in Vector3 point)
    {
        return plane.Normal.X * point.X + plane.Normal.Y * point.Y + plane.Normal.Z * point.Z + plane.D;
    }
}
