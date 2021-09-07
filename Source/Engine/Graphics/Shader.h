// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"

namespace Alimer
{
    enum class ShaderStages : uint32_t
    {
        None = 0x0000,

        Compute = 0x0020,

        Vertex = 0x0001,
        Hull = 0x0002,
        Domain = 0x0004,
        Geometry = 0x0008,
        Pixel = 0x0010,
        Amplification = 0x0040,
        Mesh = 0x0080,
        AllGraphics = 0x00FE,

        RayGeneration = 0x0100,
        AnyHit = 0x0200,
        ClosestHit = 0x0400,
        Miss = 0x0800,
        Intersection = 0x1000,
        Callable = 0x2000,
        AllRayTracing = 0x3F00,

        All = 0x3FFF,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ShaderStages);

    class ALIMER_API Shader : public RefCounted
    {
    public:
        /// Create new shader.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");

    protected:
        /// Constructor.
        Shader(ShaderStages stage);

        ShaderStages stage;
    };
}

