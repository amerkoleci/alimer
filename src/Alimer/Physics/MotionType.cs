// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Physics;

/// <summary>
/// Motion type of a physics body
/// </summary>
public enum MotionType
{
    /// <summary>
    /// Non movable
    /// </summary>
    Static = 0,
    /// <summary>
    /// Movable using velocities only, does not respond to forces
    /// </summary>
    Kinematic = 1,
    /// <summary>
    /// Responds to forces as a normal physics object
    /// </summary>
    Dynamic = 2
}

