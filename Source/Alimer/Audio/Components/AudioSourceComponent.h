// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    class ALIMER_API AudioSourceComponent final : public Component
    {
    public:
        /// Register object factory and properties.
        static void Register();

        AudioSourceComponent();
        ~AudioSourceComponent() override;

    private:
        AudioClipRef _audioClip;
        AudioSource* _source;
    };
}
