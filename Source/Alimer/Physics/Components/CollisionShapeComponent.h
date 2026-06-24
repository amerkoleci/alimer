// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Component.h"
#include "Alimer/Math/Vector3.h"

namespace Alimer
{
    class ALIMER_API CollisionShapeComponent : public Component
    {
        ALIMER_OBJECT(CollisionShapeComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        CollisionShapeComponent() = default;
        ~CollisionShapeComponent() override;

    private:
    };
}
