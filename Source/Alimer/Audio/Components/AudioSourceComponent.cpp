// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.


#include "Alimer/Audio/Components/AudioSourceComponent.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"

using namespace Alimer;

void AudioSourceComponent::Register()
{
    auto reflection = GetTypeInfoReflection(AudioSourceComponent::GetTypeInfoStatic());
    reflection->SetFactory<AudioSourceComponent>();
    reflection->SetCategory("Audio");
    reflection->SetDisplayName("Audio Source Component");
}

AudioSourceComponent::AudioSourceComponent()
    : _source(new AudioSource())
{
}

AudioSourceComponent::~AudioSourceComponent()
{
    delete _source;
}
