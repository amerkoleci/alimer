// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"

namespace Alimer
{
    enum class RigidBodyType 
    {
        Static = 0,
        Kinematic,
        Dynamic
    };

    enum class CollisionShapeType
    {
        Box,
        Sphere,
        Capsule,
        Cylinder,
        ConvexHull,
        Mesh,
        Compound,
        Terrain,
    };

    class CollisionShape;
    class RigidBody;
    class PhysicsWorld;

    using RigidBodyRef = SharedPtr<RigidBody>;
    using PhysicsWorldRef = SharedPtr<PhysicsWorld>;
}
