// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/PhysicsTypes.h"
#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Math/BoundingBox.h"

namespace Alimer
{
    class ALIMER_API CollisionShape 
    {
    public:
        [[nodiscard]] virtual CollisionShapeType GetType() const = 0;
        [[nodiscard]] virtual float GetVolume() const = 0;
        [[nodiscard]] virtual float GetDensity() const = 0;
        [[nodiscard]] virtual float GetMass() const = 0;
        [[nodiscard]] virtual BoundingBox GetLocalBounds() const = 0;
        [[nodiscard]] virtual Vector3 GetCenterOfMass() const = 0;
    };

    class ALIMER_API RigidBody : public RefCounted
    {
    public:
        [[nodiscard]] virtual uint32_t GetID() const = 0;

        [[nodiscard]] virtual RigidBodyType GetType() const = 0;
        virtual void SetType(RigidBodyType type) = 0;

        [[nodiscard]] virtual bool IsActive() const = 0;
        virtual void Activate() = 0;
        virtual void Deactivate() = 0;

        [[nodiscard]] virtual Vector3 GetPosition() const = 0;
        [[nodiscard]] virtual Quaternion GetRotation() const = 0;

        [[nodiscard]] virtual RigidBodyTransform GetTransform() const = 0;
        virtual void SetTransform(const RigidBodyTransform& transform) = 0;
        [[nodiscard]] virtual Matrix4x4 GetWorldTransform() const = 0;

        [[nodiscard]] virtual Vector3 GetCenterOfMass() const = 0;

        [[nodiscard]] virtual Vector3 GetLinearVelocity() const = 0;
        virtual void SetLinearVelocity(const Vector3& velocity) = 0;

        [[nodiscard]] virtual Vector3 GetAngularVelocity() const = 0;
        virtual void SetAngularVelocity(const Vector3& velocity) = 0;

        /// Add force at center of mass.
        virtual void AddForce(const Vector3& force) = 0;
        /// Add force at world space position.
        virtual void AddForce(const Vector3& force, const Vector3& position) = 0;
        /// Add torque.
        virtual void AddTorque(const Vector3& torque) = 0;
        /// Add force and torque.
        virtual void AddForceAndTorque(const Vector3& force, const Vector3& torque) = 0;

        /// Add impulse to center of mass (unit: kg m/s).
        virtual void AddImpulse(const Vector3& impulse) = 0;
        /// Add impulse at world space position.
        virtual void AddImpulse(const Vector3& impulse, const Vector3& position) = 0;
        /// Add angular impulse in world space
        virtual void AddAngularImpulse(const Vector3& angularImpulse) = 0;

        virtual bool ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3& fluidVelocity, float deltaTime) = 0;
    };

    class ALIMER_API PhysicsWorld : public RefCounted
    {
    public:
        [[nodiscard]] virtual Vector3 GetGravity() const = 0;
        virtual void SetGravity(const Vector3& gravity) = 0;

        [[nodiscard]] virtual uint32_t GetBodyCount() const = 0;
        [[nodiscard]] virtual uint32_t GetActiveBodyCount() const = 0;

        [[nodiscard]] virtual RigidBodyRef CreateRigidBody(const RigidBodyDesc& desc) = 0;

        virtual void OptimizeBroadPhase() = 0;
    };

    class PhysicsBackend
    {
    protected:
        PhysicsBackend() = default;

    public:
        virtual ~PhysicsBackend() = default;

        // Non-copyable and non-movable
        ALIMER_DISABLE_COPY_MOVE(PhysicsBackend);

        /// Get the name of the physics backend.
        [[nodiscard]] virtual const char* GetName() const = 0;

        /// Create a new physics world.
        [[nodiscard]] virtual PhysicsWorldRef CreatePhysicsWorld() = 0;

        /* CollisionShape */
        [[nodiscard]] virtual CollisionShape* CreateBoxShape(const Vector3& size) const = 0;
        [[nodiscard]] virtual CollisionShape* CreateSphereShape(float radius) const = 0;
        [[nodiscard]] virtual CollisionShape* CreateCapsuleShape(float height, float radius) const = 0;
        [[nodiscard]] virtual CollisionShape* CreateCylinderShape(float height, float radius) const = 0;
        [[nodiscard]] virtual CollisionShape* CreatePlaneShape(float halfExtent = 1000.0f) const = 0;
        [[nodiscard]] virtual CollisionShape* CreateConvexHullShape(const Vector3* points, uint32_t pointsCount) const = 0;
        virtual void DestroyShape(CollisionShape* shape) = 0;
    };

    class Physics final
    {
    public:
        static bool Initialize();
        static void Shutdown();
        static PhysicsBackend* GetBackend() { return s_backend; }

    private:
        static PhysicsBackend* CreateBackend();
        static void Register();

        static PhysicsBackend* s_backend;
    };
}
