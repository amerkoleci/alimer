// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/Types.h"

namespace Alimer
{
    class ALIMER_API CollisionShape : public RefCounted
    {};

    class ALIMER_API RigidBody : public RefCounted
    {};

    class ALIMER_API PhysicsWorld : public RefCounted
    {
    public:
        //[[nodiscard]] virtual RigidBodyRef CreateRigidBody() = 0;
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
