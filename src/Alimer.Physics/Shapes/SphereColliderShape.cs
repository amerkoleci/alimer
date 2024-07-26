// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using JoltPhysicsSharp;

namespace Alimer.Physics;

public class SphereColliderShape : ColliderShape
{
    public SphereColliderShape(float radius)
    {
        Radius = radius;

        Handle = new SphereShape(radius);
    }

    public float Radius { get; }

    internal override Shape Handle { get; }
}
