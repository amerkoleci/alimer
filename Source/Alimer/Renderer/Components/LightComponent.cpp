// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Components/LightComponent.h"

using namespace Alimer;

void LightComponent::Register()
{
    auto reflection = GetTypeInfoReflection(LightComponent::GetTypeInfoStatic());
    reflection->SetFactory<LightComponent>();
    reflection->SetCategory("Rendering");
    reflection->SetDisplayName("Light Component");
}

LightComponent::~LightComponent()
{}
