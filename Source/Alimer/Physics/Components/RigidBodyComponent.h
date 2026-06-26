// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/Types.h"
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
        [[nodiscard]] bool IsStatic() const { return _bodyType == RigidBodyType::Static; }

        /// Check if this body is kinematic (keyframed), which means that it will move according to its current velocity, but forces don't affect it
        [[nodiscard]] bool IsKinematic() const { return _bodyType == RigidBodyType::Kinematic; }

        /// Check if this body is dynamic, which means that it moves and forces can act on it
        [[nodiscard]] bool IsDynamic() const { return _bodyType == RigidBodyType::Dynamic; }

    private:
        RigidBodyType _bodyType = RigidBodyType::Dynamic;
        RigidBodyRef _rigidBody;
    };
}
