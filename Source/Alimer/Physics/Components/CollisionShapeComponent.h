// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Physics/Types.h"
#include "Alimer/Math/Vector3.h"
#include "Alimer/Scene/Component.h"

namespace Alimer
{
    static constexpr CollisionShapeType kDefaultCollisionShapeType = CollisionShapeType::Box;

    class ALIMER_API CollisionShapeComponent : public Component
    {
        ALIMER_OBJECT(CollisionShapeComponent, Component);

    public:
        /// Register object factory and properties.
        static void Register();

        CollisionShapeComponent(CollisionShapeType type = kDefaultCollisionShapeType);
        ~CollisionShapeComponent() override;

        /// Return shape type.
        [[nodiscard]] CollisionShapeType GetShapeType() const { return _shapeType; }

        /// Set shape type.
        void SetShapeType(CollisionShapeType type);

    private:
        CollisionShapeType _shapeType = kDefaultCollisionShapeType;
    };
}
