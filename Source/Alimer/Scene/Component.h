// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/SceneTypes.h"
#include "Alimer/Serialization/Serializable.h"

namespace Alimer
{
    /// Base class for Entity components.
    class ALIMER_API Component : public Serializable
    {
        ALIMER_OBJECT(Component, Serializable);

        friend class Entity;
        friend class Scene;

    public:
        Component() = default;

        /// Return entity owner.
        [[nodiscard]] Entity* GetEntity() const;

        /// Return id of the entity.
        [[nodiscard]] UUID GetID() const;

        /// Return the scene the owner entity belongs to.
        [[nodiscard]] Scene* GetScene() const;

        /// Return true if the component is enabled.
        [[nodiscard]] bool IsEnabled() const { return _enabled; }
        void SetEnabled(bool value);

    protected:
        /// Set the entity owner. Called by Entity when creating the component or when adding the component to an entity.
        void SetEntity(Entity* value);

        /// Set ID. 
        void SetID(UUID value);

        /// Owner entity.
        Entity* _entity = nullptr;
        /// Component ID.
        UUID _id = {};
        /// Enabled state.
        bool _enabled = true;

        // TODO: Add flags (static, transform changed, etc)
        bool _requireTransformChangeListener = false;
    };
}
