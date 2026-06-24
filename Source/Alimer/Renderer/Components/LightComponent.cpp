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

    RegisterEnumProperty("type", &LightComponent::GetLightType, &LightComponent::SetLightType, kDefaultLightType);
    RegisterRefProperty("color", &LightComponent::GetColor, &LightComponent::SetColor, kDefaultLightColor);
    RegisterProperty("intensity", &LightComponent::GetIntensity, &LightComponent::SetIntensity, kDefaultLightIntensity);
    RegisterProperty("range", &LightComponent::GetRange, &LightComponent::SetRange, kDefaultLightRange);
}

LightComponent::LightComponent(LightType type)
    : _lightType(type)
{
    _requireTransformChangeListener = true;
}

void LightComponent::SetLightType(LightType value)
{
    if (_lightType == value)
        return;

    _lightType = value;
}

void LightComponent::SetColor(const Color& value)
{
    if (_color == value)
        return;

    _color = value;
}

void LightComponent::SetIntensity(float value)
{
    if (_intensity == value)
        return;

    _intensity = value;
}

void LightComponent::SetRange(float value)
{
    if (_range == value)
        return;

    _range = value;
}
