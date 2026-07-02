// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/Physics.h"

namespace JPH
{
    class JobSystem;
    class PhysicsSystem;
    class BodyInterface;
}

namespace Alimer
{
    namespace PhysicsLayers
    {
        static constexpr uint8_t NON_MOVING = 0;
        static constexpr uint8_t MOVING = 1;
        static constexpr uint8_t CHARACTER = 2;
        static constexpr uint8_t CHARACTER_GHOST = 3;
        static constexpr uint8_t TRIGGER = 4;
        static constexpr uint8_t NUM_LAYERS = 5;
    };

    class JoltPhysicsBackend final : public PhysicsBackend
    {
    public:
        JoltPhysicsBackend();
        ~JoltPhysicsBackend() override;


        [[nodiscard]] JPH::JobSystem& GetThreadPool();
        PhysicsWorldRef CreatePhysicsWorld() override;
        CollisionShape* CreateBoxShape(const Vector3& halfExtents) const override;
        CollisionShape* CreateSphereShape(float radius) const override;
        CollisionShape* CreateCapsuleShape(float height, float radius) const override;
        CollisionShape* CreateCylinderShape(float height, float radius) const override;
        CollisionShape* CreatePlaneShape(float halfExtent = 1000.0f) const override;
        CollisionShape* CreateConvexHullShape(const Vector3* points, uint32_t pointsCount) const override;

        void DestroyShape(CollisionShape* shape) override;

        const char* GetName() const override { return "Jolt"; }

    private:
        JPH::JobSystem* _threadPool = nullptr;
    };
}
