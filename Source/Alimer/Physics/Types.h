// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"
#include "Alimer/Math/Quaternion.h"

namespace Alimer
{
    enum class RigidBodyType 
    {
        Static = 0,
        Kinematic,
        Dynamic,

        Count
    };

    enum class CollisionShapeType
    {
        Box,
        Sphere,
        Capsule,
        ConvexHull,
        Mesh,
        Compound,
        Terrain,

        Count
    };

    struct ALIMER_API RigidBodyTransform final
    {
        Vector3 position = Vector3::Zero;
        Quaternion rotation = Quaternion::Identity;
    };

    class CollisionShape;
    class RigidBody;
    class PhysicsWorld;

    using RigidBodyRef = SharedPtr<RigidBody>;
    using PhysicsWorldRef = SharedPtr<PhysicsWorld>;
}
