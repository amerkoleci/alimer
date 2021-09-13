// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"
#include <vector>

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

    struct ShaderReflection
    {
        struct InputElement
        {
            VertexFormat format = VertexFormat::Undefined;
            std::string semanticName;
            size_t semanticIndex;
        };

        enum class ResourceBindType : u32
        {
            Unknown,
            ConstantBuffer,
            ShaderResource,
            UnorderedAccess,
            Sampler
        };

        struct Resource
        {
            std::string name;
            ResourceBindType type;
            uint32_t set;
            uint32_t binding;
            uint32_t arraySize;
            uint32_t size;
        };

        std::vector<InputElement> inputElements;
        std::vector<Resource> resources;
    };

    class ALIMER_API Shader : public RefCounted
    {
    public:
        /// Create new shader.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");

        /// Create new shader.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const void* bytecode, size_t bytecodeLength);

        const ShaderReflection& GetReflection() const { return reflection; }
        size_t GetReflectionHash() const { return reflectionHash; }

    protected:
        /// Constructor.
        Shader(ShaderStages stage);

        ShaderStages stage;
        ShaderReflection reflection;
        size_t reflectionHash = 0;
    };
}

