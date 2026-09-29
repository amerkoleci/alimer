// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Physics;

public class CapsuleColliderShape : ColliderShape
{
    public CapsuleColliderShape(float height, float radius)
    {
    }

    public override void CreateShape(RigidBody body) => throw new NotImplementedException();
}
