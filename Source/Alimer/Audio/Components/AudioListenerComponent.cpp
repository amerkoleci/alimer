// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Audio/Components/AudioListenerComponent.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"

using namespace Alimer;

void AudioListenerComponent::Register()
{
    auto reflection = GetTypeInfoReflection(AudioListenerComponent::GetTypeInfoStatic());
    reflection->SetFactory<AudioListenerComponent>();
    reflection->SetCategory("Audio");
    reflection->SetDisplayName("Audio Listener Component");

    CopyBaseProperties<AudioListenerComponent, Component>();
}

AudioListenerComponent::~AudioListenerComponent()
{
}
