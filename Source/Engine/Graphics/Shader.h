// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GPUResource.h"
#include <vector>

namespace Alimer
{
    enum class ShaderResourceType : uint32_t
    {
        Invalid,
        Input,
        Output,
        SampledTexture,
        StorageTexture,
        Sampler,
        UniformBuffer,
        StorageBuffer,
        PushConstant,
        All
    };

    struct ShaderResource
    {
        ShaderStages stages = ShaderStages::None;
        ShaderResourceType type = ShaderResourceType::Invalid;
        std::string name;

        uint32_t set;
        uint32_t binding;
        uint32_t backend_binding;
        uint32_t arraySize;

        uint32_t offset;
        uint32_t size;
    };

	class ALIMER_API Shader : public GPUObjectOld, public RefCounted
	{
	public:
        /// Create new shader from source.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");

        /// Create new shader from bytecode.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint = "main");

        [[nodiscard]] ShaderStages GetStage() const noexcept { return stage; }
        [[nodiscard]] const std::string& GetEntryPoint() const { return entryPoint; }
        [[nodiscard]] virtual const std::vector<ShaderResource>& GetResources() const = 0;

	protected:
		/// Constructor.
		Shader(ShaderStages stage, const std::string& entryPoint);

        ShaderStages stage;
		std::string entryPoint;
	};
}
