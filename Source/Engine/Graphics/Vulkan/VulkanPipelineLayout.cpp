// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanPipelineLayout.h"
#include "VulkanGraphics.h"

namespace Alimer
{
    /* VulkanPipelineLayout */
    VulkanPipelineLayout::VulkanPipelineLayout(VulkanGraphics& device, const std::vector<VulkanShader*>& shaders)
        : device{ device }
        , shaders{ shaders }
    {
        // Collect and combine all the shader resources from each of the shader modules
        // Collate them all into a map that is indexed by the name of the resource
        for (auto* shader : shaders)
        {
            for (const auto& resource : shader->GetResources())
            {
                std::string key = resource.name;

                // Since 'Input' and 'Output' resources can have the same name, we modify the key string
                if (resource.type == ShaderResourceType::Input ||
                    resource.type == ShaderResourceType::Output)
                {
                    //key = std::to_string(resource.stages) + "_" + key;
                }

                if (resource.arraySize > 1 || resource.set > 0)
                {
                    bindless = true;
                    continue;
                }

                auto it = shaderResources.find(key);

                if (it != shaderResources.end())
                {
                    // Append stage flags if resource already exists
                    it->second.stages |= resource.stages;
                }
                else
                {
                    // Create a new entry in the map
                    shaderResources.emplace(key, resource);
                }
            }
        }

        // Sift through the map of name indexed shader resources
        // Separate them into their respective sets
        for (auto& it : shaderResources)
        {
            auto& shaderResource = it.second;

            // Find binding by set index in the map.
            auto it2 = shaderSets.find(shaderResource.set);

            if (it2 != shaderSets.end())
            {
                // Add resource to the found set index
                it2->second.push_back(shaderResource);
            }
            else
            {
                // Create a new set index and with the first resource
                shaderSets.emplace(shaderResource.set, std::vector<ShaderResource>{shaderResource});
            }
        }

        // Create a descriptor set layout for each shader set in the shader modules
        for (auto& it : shaderSets)
        {
            descriptorSetLayouts.emplace_back(&device.RequestDescriptorSetLayout(it.first, it.second));
        }

        // Collect all the descriptor set layout handles, maintaining set order
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
        for (uint32_t i = 0; i < descriptorSetLayouts.size(); ++i)
        {
            if (descriptorSetLayouts[i])
            {
                vkDescriptorSetLayouts.push_back(descriptorSetLayouts[i]->handle);
            }
            else
            {
                vkDescriptorSetLayouts.push_back(VK_NULL_HANDLE);
            }
        }

        // TODO: Actually detect which bindless sets are used
        if (bindless)
        {
            vkDescriptorSetLayouts.push_back(device.GetBindlessSampledImageDescriptorSetLayout());
        }

        // Collect all the push constant shader resources
        std::vector<VkPushConstantRange> pushConstantRanges;
        for (auto& pushConstantResource : GetResources(ShaderResourceType::PushConstant))
        {
            VkShaderStageFlags stage = ToVulkan(pushConstantResource.stages);
            pushConstantStage |= stage;
            pushConstantRanges.push_back({ stage, pushConstantResource.offset, pushConstantResource.size });
        }

        VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
        createInfo.pSetLayouts = vkDescriptorSetLayouts.data();
        createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
        createInfo.pPushConstantRanges = pushConstantRanges.data();

        // Create the Vulkan pipeline layout handle
        VkResult result = vkCreatePipelineLayout(device.GetHandle(), &createInfo, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Cannot create PipelineLayout");
        }
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device.GetHandle(), handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }

    const std::vector<ShaderResource> VulkanPipelineLayout::GetResources(const ShaderResourceType& type, ShaderStages stage) const
    {
        std::vector<ShaderResource> result;

        for (auto& it : shaderResources)
        {
            auto& shader_resource = it.second;

            if (shader_resource.type == type || type == ShaderResourceType::All)
            {
                if (shader_resource.stages == stage || stage == ShaderStages::All)
                {
                    result.push_back(shader_resource);
                }
            }
        }

        return result;
    }
}
