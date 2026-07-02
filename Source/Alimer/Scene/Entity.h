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

        /// Return whether the entity is the root of the transform hierarchy. If true, local transform and world transform are the same.
        bool IsRoot() const;

        /// Get the local space position.
        const Vector3& GetLocalPosition() const { return _localPosition; }

        /// Get the local space rotation.
        const Quaternion& GetLocalRotation() const { return _localRotation; }
        /// Get the local space scale.
        const Vector3& GetLocalScale() const { return _localScale; }

        /// Get the local space rotation in euler angles.
        Vector3 GetLocalEulerAngles() const { return _localRotation.EulerAngles(); }

        /// Sets the local space position.
        void SetLocalPosition(float x, float y, float z);

        /// Sets the local space position.
        void SetLocalPosition(const Vector3& position);

    private:
        void OnTransformChanged();
        void MarkDirty(bool markLocalDirty = true);

        /// Scene owner.
        Scene* _scene = nullptr;
        /// Parent entity.
        Entity* _parent = nullptr;
        /// Child entities.
        Vector<EntityRef> _children;
        /// Entity ID.
        UUID _id = {};
        /// Entity name.
        String _name;
        /// Enabled state.
        bool _enabled = true;
        /// Tags.
        Tags _tags;

        // Entity local-space transform
        Vector3 _localPosition = Vector3::Zero;
        Quaternion _localRotation = Quaternion::Identity;
        Vector3 _localScale = Vector3::One;
        /// World-space rotation.
        mutable Quaternion _worldRotation = Quaternion::Identity;

        mutable Matrix4x4 _localMatrix = Matrix4x4::Identity;
        mutable Matrix4x4 _worldMatrix = Matrix4x4::Identity;
        mutable bool _localMatrixDirty = true;
        mutable bool _worldMatrixDirty = true;
    };
}
