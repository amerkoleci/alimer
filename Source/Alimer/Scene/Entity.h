// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    // TODO: Layers
    using Tags = UnorderedSet<String>;

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

        /// Gets parent entity or null if root entity.
        [[nodiscard]] Entity* GetParent() const { return _parent; }

        /// Gets the scene this entity belongs to.
        [[nodiscard]] Scene* GetScene() const { return _scene; }

        /// Gets entity ID.
        [[nodiscard]] UUID GetID() const { return _id; }

        /// Gets entity name.
        [[nodiscard]] const String& GetName() const { return _name; }

        /// Sets entity name.
        void SetName(const String& name);

        /// Sets entity name.
        void SetName(StringView name);

        /// Gets whether the entity is enabled.
        [[nodiscard]] bool IsEnabled() const { return _enabled; }

        /// Gets entity tags.
        [[nodiscard]] const Tags& GetTags() const { return _tags; }

    private:
        /// Parent entity.
        Entity* _parent = nullptr;
        /// Scene owner.
        Scene* _scene = nullptr;
        /// Entity ID.
        UUID _id = {};
        /// Entity name.
        String _name;
        /// Enabled state.
        bool _enabled = true;
        /// Tags.
        Tags _tags;
    };
}
