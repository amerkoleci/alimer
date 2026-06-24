// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Component.h"
#include "Alimer/Math/Vector3.h"

namespace Alimer
{
    class ALIMER_API AudioListenerComponent : public Component
    {
        ALIMER_OBJECT(AudioListenerComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        AudioListenerComponent() = default;
        ~AudioListenerComponent() override;

        [[nodiscard]] Vector3 GetVelocity() const { return _velocity; }
        void SetVelocity(const Vector3& velocity) { _velocity = velocity; }

    private:
        Vector3 _velocity = Vector3(0.0f);
    };
}
