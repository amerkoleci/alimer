// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"
#include "Alimer/Math/Vector3.h"
#include "Alimer/RHI/RHI.h"

namespace Alimer
{
    enum class MaterialAlphaMode : uint8_t
    {
        Opaque,
        Mask,
        Blend,

        Count
    };

    enum class MaterialTextureUVChannel : uint8_t
    {
        UV0,
        UV1,
    };

    enum class MaterialTextureSlot : uint8_t
    {
        MetalnessRoughnessMap,
        NormalMap,
        EmissiveMap,
        DisplacementeMap,
        OcclusionMap,
        TransmissionMap,
        SheenColorMap,
        SheenRoughnessMap,
        ClearCoatMap,
        ClearCoatRoughnessMap,
        ClearCoatNormalMap,
        SpeculareMap,
        AnisotropyMap,
        TransparencyMap,

        Count
    };

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


    struct ALIMER_API MaterialTextureMap final
    {
        String name;
        RHITextureRef resource;
        MaterialTextureUVChannel channel = MaterialTextureUVChannel::UV0;
    };

    class Material;
    class Mesh;

    using MaterialRef = SharedPtr<Material>;
    using MeshRef = SharedPtr<Mesh>;
}
