// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Physics/Components/CollisionShapeComponent.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"
#include "Alimer/Physics/Physics.h"

using namespace Alimer;

void CollisionShapeComponent::Register()
{
    auto reflection = GetTypeInfoReflection(CollisionShapeComponent::GetTypeInfoStatic());
    reflection->SetFactory<CollisionShapeComponent>();
    reflection->SetCategory("Physics");
    reflection->SetDisplayName("Collision Shape Component");

    //CopyBaseProperties<CollisionShapeComponent, Component>();
}

CollisionShapeComponent::CollisionShapeComponent(CollisionShapeType type)
    : _shapeType(type)
{
}

CollisionShapeComponent::~CollisionShapeComponent()
{
}
