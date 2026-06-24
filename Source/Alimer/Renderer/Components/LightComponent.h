// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Renderer/Types.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    /// Light component that can be attached to an entity.
    class ALIMER_API LightComponent final : public Component
    {
        ALIMER_OBJECT(LightComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        LightComponent() = default;
        ~LightComponent() override;
    };

}
