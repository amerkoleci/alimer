// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Physics/Components/RigidBodyComponent.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"
#include "Alimer/Physics/Physics.h"

using namespace Alimer;

void RigidBodyComponent::Register()
{
    auto reflection = GetTypeInfoReflection(RigidBodyComponent::GetTypeInfoStatic());
    reflection->SetFactory<RigidBodyComponent>();
    reflection->SetCategory("Physics");
    reflection->SetDisplayName("Rigid Body Component");
}

RigidBodyComponent::RigidBodyComponent()
{
}

RigidBodyComponent::~RigidBodyComponent()
{
}
