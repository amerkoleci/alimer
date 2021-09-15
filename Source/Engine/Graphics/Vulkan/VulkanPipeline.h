// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Pipeline.h"
#include "VulkanUtils.h"
#include <array>

namespace Alimer
{
	class VulkanPipeline final : public Pipeline
	{
	public:
		VulkanPipeline(VulkanGraphics& device, const RenderPipelineStateCreateInfo* info);
		VulkanPipeline(VulkanGraphics& device, const ComputePipelineCreateInfo* info);
		~VulkanPipeline() override;
		void Destroy() override;

		VulkanPipelineLayout* GetPipelineLayout() const noexcept { return pipelineLayout; }
		VkPipeline GetHandle() const { return handle; }
		VkPipelineBindPoint GetBindPoint() const { return bindPoint; }

	private:
		VulkanGraphics& device;

		VulkanPipelineLayout* pipelineLayout = nullptr;
		VkPipeline handle = VK_NULL_HANDLE;
		VkPipelineBindPoint bindPoint;

		// The shader resources that this pipeline layout uses, indexed by their name
		std::unordered_map<std::string, ShaderResource> shaderResources;

		// A map of each set and the resources it owns used by the pipeline layout
		std::unordered_map<uint32_t, std::vector<ShaderResource>> shaderSets;

		// The different descriptor set layouts for this pipeline layout
		std::vector<VulkanDescriptorSetLayout*> descriptorSetLayouts;
	};

	constexpr VulkanPipeline* ToVulkan(Pipeline* resource)
	{
		return static_cast<VulkanPipeline*>(resource);
	}

	constexpr const VulkanPipeline* ToVulkan(const Pipeline* resource)
	{
		return static_cast<const VulkanPipeline*>(resource);
	}
}

