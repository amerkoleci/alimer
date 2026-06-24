// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Component.h"

namespace Alimer
{
    class ALIMER_API CameraComponent final : public Component
    {
        ALIMER_OBJECT(CameraComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        CameraComponent() = default;
        ~CameraComponent() override;
    };
}
