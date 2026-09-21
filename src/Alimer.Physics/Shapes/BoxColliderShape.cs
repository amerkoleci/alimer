// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using static Alimer.AlimerApi;

namespace Alimer.Physics;

public class BoxColliderShape : ColliderShape
{
    public BoxColliderShape(in Vector3 size)
    {
        Size = size;

        Handle = alimerPhysicsShapeCreateBox(size, PhysicsMaterial.Null);
    }

    public Vector3 Size { get; }

    internal override PhysicsShape Handle { get; }
}
