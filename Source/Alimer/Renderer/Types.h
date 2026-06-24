// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"
#include "Alimer/Math/Vector3.h"

namespace Alimer
{
    /// Light types.
    enum class LightType
    {
        /// Omnidirectional point light. Position is taken from the entity transform.
        Point,
        /// Infinite directional light (e.g. sun). Direction is derived from the entity forward vector.
        Directional,
        /// Spot light with inner/outer cone angles. Position and direction from the entity transform.
        Spot,
    };

    struct ALIMER_API TextureCoordinateTransform
    {
        Vector2 offset = Vector2::Zero;
        Vector2 scale = Vector2::One;
        float rotation = 0.0f;
    };

    class Mesh;
    using MeshRef = SharedPtr<Mesh>;
}
