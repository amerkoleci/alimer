// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/SceneTypes.h"
#include "Alimer/Scene/Entity.h"

namespace Alimer
{
    class PhysicsWorld;

    class ALIMER_API Scene final : public Serializable
    {
        ALIMER_OBJECT(Scene, Serializable);

        friend class Entity;
        friend class SceneSystem;

    private:
        /// Register factory and attributes.
        static void Register();

    public:
        Scene(StringView name = kEmptyStringView);
        ~Scene() override;

        [[nodiscard]] const String& GetName() const { return _name; }
        void SetName(const String& name);
        void SetName(StringView name);

        void Load(const SerializeValue& source, ObjectResolver& resolver) override;
        void Save(SerializeValue& dest) override;

        /// Load from binary stream. Store node references to be resolved later.
        void Load(Stream& source, ObjectResolver& resolver) override;
        void Save(Stream& dest) override;

        [[nodiscard]] bool LoadGLTF(const String& path);

        [[nodiscard]] PhysicsWorld* GetPhysicsWorld() const { return _physicsWorld.Get(); }

    private:
        String _name;
        SharedPtr<PhysicsWorld> _physicsWorld;
    };
}
