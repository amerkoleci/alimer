// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    class ALIMER_API RigidBodyComponent final : public Component
    {
        ALIMER_OBJECT(RigidBodyComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        RigidBodyComponent();
        ~RigidBodyComponent() override;

    private:
    };
}
