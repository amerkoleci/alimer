// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/PhysicsTypes.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    class ALIMER_API RigidBodyComponent final : public Component
    {
        ALIMER_OBJECT(RigidBodyComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        RigidBodyComponent(RigidBodyType type = RigidBodyType::Dynamic);
        ~RigidBodyComponent() override;

        /// Check if this body is static (not movable)
        bool IsStatic() const { return _bodyType == RigidBodyType::Static; }

        /// Check if this body is kinematic (keyframed), which means that it will move according to its current velocity, but forces don't affect it
        bool IsKinematic() const { return _bodyType == RigidBodyType::Kinematic; }

        /// Check if this body is dynamic, which means that it moves and forces can act on it
        bool IsDynamic() const { return _bodyType == RigidBodyType::Dynamic; }

        /// Get the body ID
        uint32_t GetBodyID() const { return _rigidBodyID; }

        /// Check if the body handle is valid.
        bool IsBodyValid() const { return _rigidBodyID != kInvalidBodyID; }

        /// Check if the body handle is invalid.
        bool IsBodyInvalid() const { return _rigidBodyID == kInvalidBodyID; }

        Vector3 GetLinearVelocity() const;
        void SetLinearVelocity(const Vector3& velocity);

        Vector3 GetAngularVelocity() const;
        void SetAngularVelocity(const Vector3& velocity);

        /// Add force at center of mass.
        void AddForce(const Vector3& force);
        /// Add force at world space position.
        void AddForce(const Vector3& force, const Vector3& position);
        /// Add torque.
        void AddTorque(const Vector3& torque);
        /// Add force and torque.
        void AddForceAndTorque(const Vector3& force, const Vector3& torque);

        /// Add impulse to center of mass (unit: kg m/s).
        void AddImpulse(const Vector3& impulse);
        /// Add impulse at world space position.
        void AddImpulse(const Vector3& impulse, const Vector3& position);
        /// Add angular impulse in world space
        void AddAngularImpulse(const Vector3& angularImpulse);

        bool ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float deltaTime);
        bool ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3& fluidVelocity, float deltaTime);

    private:
        RigidBodyType _bodyType = RigidBodyType::Dynamic;

        /// Physics world.
        WeakPtr<PhysicsWorld> _physicsWorld;
        uint32_t _rigidBodyID = kInvalidBodyID;
        RigidBodyRef _rigidBody;
    };
}
