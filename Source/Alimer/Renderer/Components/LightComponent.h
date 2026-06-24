// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Renderer/Types.h"
#include "Alimer/Math/Color.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    static constexpr LightType kDefaultLightType = LightType::Point;
    static constexpr Color kDefaultLightColor = { 1.0f, 1.0f, 1.0f };
    static constexpr float kDefaultLightIntensity = 1.0f;
    static constexpr float kDefaultLightRange = 10.0f;
    static constexpr float kDefaultLightInnerAngle = 45.0f;
    static constexpr float kDefaultLightOuterAngle = 60.0f;

    /// Light component that can be attached to an entity.
    class ALIMER_API LightComponent final : public Component
    {
        ALIMER_OBJECT(LightComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        LightComponent(LightType type = kDefaultLightType);

        LightType GetLightType() const noexcept { return _lightType; }
        void SetLightType(LightType value);

        const Color& GetColor() const noexcept { return _color; }
        void SetColor(const Color& value);

        float GetIntensity() const noexcept { return _intensity; }
        void SetIntensity(float value);

        float GetRange() const noexcept { return _range; }
        void SetRange(float value);

    private:
        LightType _lightType = kDefaultLightType;
        Color _color = kDefaultLightColor;
        float _intensity = kDefaultLightIntensity;
        float _range = kDefaultLightRange;
        float _innerAngleInDegrees = kDefaultLightInnerAngle;  // Inner cone half-angle in degrees
        float _outerAngleInDegrees = kDefaultLightOuterAngle;  // Outer cone half-angle in degrees
    };

}
