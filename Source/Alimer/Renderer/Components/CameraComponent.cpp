// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Components/CameraComponent.h"

using namespace Alimer;

void CameraComponent::Register()
{
    auto reflection = GetTypeInfoReflection(CameraComponent::GetTypeInfoStatic());
    reflection->SetFactory<CameraComponent>();
    reflection->SetCategory("Rendering");
    reflection->SetDisplayName("Camera Component");
}

CameraComponent::~CameraComponent()
{}
