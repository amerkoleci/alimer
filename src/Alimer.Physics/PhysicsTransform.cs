// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Physics;

public readonly record struct PhysicsTransform(Vector3 Position, Quaternion Quaternion)
{
    public Matrix4x4 ToMatrix4x4()
    {
        return
            Matrix4x4.CreateFromQuaternion(Quaternion) *
            Matrix4x4.CreateTranslation(Position);
    }
}
