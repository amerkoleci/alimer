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
        uint32_t arraySize;

        uint32_t offset;
        uint32_t size;
    };

	class ALIMER_API Shader : public GPUObject, public RefCounted
	{
	public:
		static ShaderRef Create(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint = "main");

        ShaderStages GetStage() const noexcept { return stage; }
		const std::string& GetEntryPoint() const { return entryPoint; }
		const std::vector<uint8_t>& GetByteCode() const { return byteCode; }
		const std::vector<ShaderResource>& GetResources() const { return resources; }
		size_t GetHash() const { return hash; }

	protected:
		/// Constructor.
		Shader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint);

        ShaderStages stage;
		std::vector<uint8_t> byteCode;
		std::string entryPoint;
		std::vector<ShaderResource> resources;
		size_t hash = 0;
	};
}
