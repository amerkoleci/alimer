// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Physics/Physics.h"
#include "Alimer/Physics/Components/CollisionShapeComponent.h"
#include "Alimer/Physics/Components/RigidBodyComponent.h"
#include "Alimer/Physics/Jolt/JoltPhysicsBackend.h" // In case we support multiple backends in the future, we can use a factory pattern to create the appropriate backend based on configuration or platform.
#include "Alimer/Core/Log.h"

using namespace Alimer;

PhysicsBackend* Physics::s_backend = nullptr;

bool Physics::Initialize()
{
    if (s_backend != nullptr)
        return true;

    s_backend = CreateBackend();
    if (!s_backend)
        return false;

    Register();

    LOGI("Physics: {} Initialized", s_backend->GetName());
    return true;
}

PhysicsBackend* Physics::CreateBackend()
{
    // For now, we only have the Jolt backend. In the future, we can add more backends and select based on configuration or platform.
    return new JoltPhysicsBackend();
}

void Physics::Shutdown()
{
    if (!s_backend)
        return;

    delete s_backend;
    s_backend = nullptr;
}

void Physics::Register()
{
    static bool registered = false;
    if (registered)
        return;

    // Register components
    CollisionShapeComponent::Register();
    RigidBodyComponent::Register();

    registered = true;
}
