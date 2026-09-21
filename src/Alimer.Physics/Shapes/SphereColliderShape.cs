// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;


namespace Alimer.Physics;

public class SphereColliderShape : ColliderShape
{
    public SphereColliderShape(float radius)
    {
        Radius = radius;

        Handle = alimerPhysicsShapeCreateSphere(radius, PhysicsMaterial.Null);
    }

    public float Radius { get; }

    internal override PhysicsShape Handle { get; }
}
