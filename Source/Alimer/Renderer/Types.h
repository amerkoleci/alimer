// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"

namespace Alimer
{
    /// Light types.
    enum class LightType : uint32_t
    {
        /// Omnidirectional point light. Position is taken from the entity transform.
        Point,
        /// Infinite directional light (e.g. sun). Direction is derived from the entity forward vector.
        Directional,
        /// Spot light with inner/outer cone angles. Position and direction from the entity transform.
        Spot,
    };

    class Mesh;
    using MeshRef = SharedPtr<Mesh>;
}
