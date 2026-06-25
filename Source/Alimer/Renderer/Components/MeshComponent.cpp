// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Components/MeshComponent.h"

using namespace Alimer;

void MeshComponent::Register()
{
    auto reflection = GetTypeInfoReflection(MeshComponent::GetTypeInfoStatic());
    reflection->SetFactory<MeshComponent>();
    reflection->SetCategory("Rendering");
    reflection->SetDisplayName("Mesh Component");

    //RegisterMixedRefProperty("mesh", &MeshComponent::GetMeshAttr, &MeshComponent::SetMeshAttr, AssetRef(Mesh::GetTypeStatic()));
}

MeshComponent::MeshComponent(Mesh* mesh)
{
    _requireTransformChangeListener = true;
    SetMesh(mesh);
}

void MeshComponent::SetMesh(Mesh* value)
{
    if (_mesh == value)
        return;

    _mesh = value;
    if (value)
    {
        _materials.resize(value->GetSubmeshCount());
        //for (size_t i = 0; i < materials.size(); ++i)
        //    materials[i] = Material::DefaultMaterial();

        _localBoundingBox = value->GetBoundingBox();
    }
    else
    {
        _materials.clear();
        _localBoundingBox = {};
    }

    _worldBoundingBoxDirty = true;
}
