// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using JoltPhysicsSharp;

namespace Alimer.Physics;

public class CapsuleColliderShape : ColliderShape
{
    public CapsuleColliderShape(float height, float radius)
    {
        Handle = new CapsuleShape(height/2, radius);
    }

    internal override Shape Handle { get; }
}
