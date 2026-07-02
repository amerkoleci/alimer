// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Physics/Components/RigidBodyComponent.h"
#include "Alimer/Physics/Components/CollisionShapeComponent.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"
#include "Alimer/Physics/Physics.h"

using namespace Alimer;

void RigidBodyComponent::Register()
{
    auto reflection = GetTypeInfoReflection(RigidBodyComponent::GetTypeInfoStatic());
    reflection->SetFactory<RigidBodyComponent>();
    reflection->SetCategory("Physics");
    reflection->SetDisplayName("Rigid Body Component");
}

RigidBodyComponent::RigidBodyComponent(RigidBodyType type)
    : _bodyType(type)
{}

RigidBodyComponent::~RigidBodyComponent()
{}

Vector3 RigidBodyComponent::GetLinearVelocity() const
{
    return _rigidBody ? _rigidBody->GetLinearVelocity() : Vector3::Zero;
}

void RigidBodyComponent::SetLinearVelocity(const Vector3& velocity)
{
    if (_rigidBody)
    {
        _rigidBody->SetLinearVelocity(velocity);
    }
}

Vector3 RigidBodyComponent::GetAngularVelocity() const
{
    return _rigidBody ? _rigidBody->GetAngularVelocity() : Vector3::Zero;
}

void RigidBodyComponent::SetAngularVelocity(const Vector3& velocity)
{
    if (_rigidBody)
    {
        _rigidBody->SetAngularVelocity(velocity);
    }
}

void RigidBodyComponent::AddForce(const Vector3& force)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddForce(force);
}

void RigidBodyComponent::AddForce(const Vector3& force, const Vector3& position)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddForce(force, position);
}

void RigidBodyComponent::AddTorque(const Vector3& torque)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddTorque(torque);
}

void RigidBodyComponent::AddForceAndTorque(const Vector3& force, const Vector3& torque)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddForceAndTorque(force, torque);
}

void RigidBodyComponent::AddImpulse(const Vector3& impulse)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddImpulse(impulse);
}

void RigidBodyComponent::AddImpulse(const Vector3& impulse, const Vector3& position)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddImpulse(impulse, position);
}

void RigidBodyComponent::AddAngularImpulse(const Vector3& angularImpulse)
{
    if (IsBodyInvalid())
        return;

    _rigidBody->AddAngularImpulse(angularImpulse);
}

bool RigidBodyComponent::ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float deltaTime)
{
    if (IsBodyInvalid())
        return false;

    return _rigidBody->ApplyBuoyancyImpulse(surfacePosition, surfaceNormal, 1000.0f, 0.1f, 0.5f, Vector3::Zero, deltaTime);
}

bool RigidBodyComponent::ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3& fluidVelocity, float deltaTime)
{
    if (IsBodyInvalid())
        return false;

    return _rigidBody->ApplyBuoyancyImpulse(surfacePosition, surfaceNormal, buoyancy, linearDrag, angularDrag, fluidVelocity, deltaTime);
}
