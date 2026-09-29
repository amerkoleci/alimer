// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Physics;

public class SphereColliderShape : ColliderShape
{
    public SphereColliderShape(float radius)
    {
        Radius = radius;
    }

    public float Radius { get; }

    public override void CreateShape(RigidBody body) => throw new NotImplementedException();
}
