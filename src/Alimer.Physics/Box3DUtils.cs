// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Physics.Box3DApi;
using static Alimer.Physics.Box3DApi.b3BodyType;

namespace Alimer.Physics;

internal static class Box3DUtils
{
    public static b3BodyType ToBox3D(this RigidBodyType type)
    {
        return type switch
        {
            RigidBodyType.Static => b3_staticBody,
            RigidBodyType.Kinematic => b3_kinematicBody,
            RigidBodyType.Dynamic => b3_dynamicBody,
            _ => throw new ArgumentOutOfRangeException(nameof(type), type, null)
        };
    }

    public static RigidBodyType ToRigidBodyType(this b3BodyType type)
    {
        return type switch
        {
            b3_staticBody => RigidBodyType.Static,
            b3_kinematicBody => RigidBodyType.Kinematic,
            b3_dynamicBody => RigidBodyType.Dynamic,
            _ => throw new ArgumentOutOfRangeException(nameof(type), type, null)
        };
    }
}

