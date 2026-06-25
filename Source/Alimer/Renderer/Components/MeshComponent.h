// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Component.h"
#include "Alimer/Renderer/Mesh.h"
#include "Alimer/Renderer/Material.h"

namespace Alimer
{
    /// Mesh component that can be attached to an entity.
    class ALIMER_API MeshComponent final : public Component
    {
        ALIMER_OBJECT(MeshComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        MeshComponent(Mesh* mesh = nullptr);

        Mesh* GetMesh() const { return _mesh.Get(); }
        void SetMesh(Mesh* value);

    private:
        MeshRef _mesh;
        Vector<MaterialRef> _materials;
        /// Local-space bounding box.
        BoundingBox _localBoundingBox;
        /// World-space bounding box.
        mutable BoundingBox _worldBoundingBox;
        /// World-space bounding box dirty flag.
        mutable bool _worldBoundingBoxDirty{ true };
    };
}
