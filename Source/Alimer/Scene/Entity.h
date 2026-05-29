// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/UnorderedSet.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    // TODO: Layers
    using Tags = UnorderedSet<String>;

    class Entity;
    using EntityRef = SharedPtr<Entity>;

    class ALIMER_API Entity final : public Serializable
    {
        ALIMER_OBJECT(Entity, Serializable);

        friend class Component;
        friend class Scene;

    private:
        /// Register factory and attributes (called by Scene::Register).
        static void Register();

    public:
        Entity(StringView name = kEmptyStringView);
        ~Entity() override;

        [[nodiscard]] const String& GetName() const { return _name; }
        void SetName(const String& name);
        void SetName(StringView name);

    private:
        String _name;
    };
}
