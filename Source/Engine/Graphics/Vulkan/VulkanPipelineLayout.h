// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "VulkanShader.h"

namespace Alimer
{
	class VulkanDescriptorSetLayout final
	{
	public:
		VulkanDescriptorSetLayout(VulkanGraphics& device, const uint32_t setIndex, const std::vector<ShaderResource>& resources);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout GetHandle() const { return handle; }

		const uint32_t GetIndex() const { return setIndex; }

		const std::vector<VkDescriptorSetLayoutBinding>& GetLayoutBindings() const;
		std::unique_ptr<VkDescriptorSetLayoutBinding> GetLayoutBinding(const uint32_t bindingIndex) const;

	private:
		VulkanGraphics& device;
		VkDescriptorSetLayout handle{ VK_NULL_HANDLE };
		const uint32_t setIndex;

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsLookup;
	};

	class VulkanPipelineLayout final
	{
	public:
		VulkanPipelineLayout(VulkanGraphics& device, const std::vector<VulkanShader*>& shaders);
		~VulkanPipelineLayout();

		VkPipelineLayout GetHandle() const { return handle; }


		const std::unordered_map<uint32_t, std::vector<ShaderResource>>& GetShaderSets() const
		{
			return shaderSets;
		}

		bool HasDescriptorSetLayout(const uint32_t setIndex) const
		{
			return setIndex < descriptorSetLayouts.size();
		}

		const std::vector<VulkanDescriptorSetLayout*>& GetDescriptorSetLayouts() const { return descriptorSetLayouts; }
		VulkanDescriptorSetLayout* GetDescriptorSetLayout(const uint32_t setIndex) const;
		const std::vector<ShaderResource> GetResources(const ShaderResourceType& type = ShaderResourceType::All, ShaderStages stage = ShaderStages::All) const;

        bool GetBindless() const noexcept { return bindless; }
		VkShaderStageFlags GetPushConstantStage() const noexcept { return pushConstantStage; }

	private:
		VulkanGraphics& device;
		VkPipelineLayout handle{ VK_NULL_HANDLE };

        bool bindless{ false };

		// The shader that this pipeline layout uses.
		std::vector<VulkanShader*> shaders;

		// The shader resources that this pipeline layout uses, indexed by their name
		std::unordered_map<std::string, ShaderResource> shaderResources;

		// A map of each set and the resources it owns used by the pipeline layout
		std::unordered_map<uint32_t, std::vector<ShaderResource>> shaderSets;

		// The different descriptor set layouts for this pipeline layout
		std::vector<VulkanDescriptorSetLayout*> descriptorSetLayouts;

		VkShaderStageFlags pushConstantStage{ 0u };
	};
}

