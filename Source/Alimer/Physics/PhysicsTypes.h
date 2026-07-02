// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Vector.h"
#include "Alimer/Core/RefCounted.h"
#include "Alimer/Math/Quaternion.h"

namespace Alimer
{
    /* Foward declaration */
    class CollisionShape;
    class RigidBody;
    class PhysicsWorld;

    static constexpr uint32_t kInvalidBodyID = 0xFFFFFFFF;

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
        Cylinder,
        Plane,
        ConvexHull,
        Mesh,
        Compound,
        Terrain,

        Count
    };

    struct RigidBodyTransform final
    {
        Vector3 position = Vector3::Zero;
        Quaternion rotation = Quaternion::Identity;
    };

    struct RigidBodyDesc
    {
        RigidBodyType type = RigidBodyType::Dynamic;
        RigidBodyTransform initialTransform = {};
        float mass = 1.0f;
        float linearDamping = 0.05f;
        float angularDamping = 0.05f;
        float gravityScale = 1.0f;
        bool isTrigger = false;
        bool allowSleeping = true;
        bool continuous = false;
        Vector<CollisionShape*> shapes;
    };

    using RigidBodyRef = SharedPtr<RigidBody>;
    using PhysicsWorldRef = SharedPtr<PhysicsWorld>;
}
