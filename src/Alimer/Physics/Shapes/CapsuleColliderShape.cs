// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Physics;

public class CapsuleColliderShape : ColliderShape
{
    public CapsuleColliderShape(float height, float radius)
    {
        Handle = alimerPhysicsShapeCreateCapsule(height, radius, PhysicsMaterial.Null);
    }

    internal override PhysicsShape Handle { get; }
}
