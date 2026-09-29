// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Physics;

public record struct PhysicsWorldSettings
{
    public const float EarthGravity = -9.81f;

    /// <summary>
    /// Multiplier applied to global gravity strength.
    /// </summary>
    public float GravityScale = 1.0f;

    /// <summary>
    /// Normalized direction of gravity in world space.
    /// </summary>
    public Vector3 GravityDirection = new(0, -1, 0);

    public PhysicsWorldSettings()
    {

    }
}
