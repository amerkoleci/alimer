// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanPipelineLayout.h"
#include "VulkanShader.h"
#include "VulkanGraphics.h"

namespace Alimer
{
    namespace
    {
        inline VkDescriptorType ToVulkan(ShaderResourceType type, bool dynamic)
        {
            switch (type)
            {
                //case ShaderResourceType::InputAttachment:
                //	return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                case ShaderResourceType::SampledTexture:
                    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case ShaderResourceType::StorageTexture:
                    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                case ShaderResourceType::Sampler:
                    return VK_DESCRIPTOR_TYPE_SAMPLER;
                case ShaderResourceType::UniformBuffer:
                    return dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case ShaderResourceType::StorageBuffer:
                    return dynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                default:
                    LOGE("No conversion possible for the shader resource type.");
                    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            }
        }
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanGraphics& device_, const uint32_t setIndex_, const std::vector<ShaderResource>& resources)
        : device(device_)
        , setIndex(setIndex_)
    {
        for (auto& resource : resources)
        {
            // Skip shader resources whitout a binding point
            if (resource.type == ShaderResourceType::Input ||
                resource.type == ShaderResourceType::Output ||
                resource.type == ShaderResourceType::PushConstant)
            {
                continue;
            }

            // Convert ShaderResource to VkDescriptorSetLayoutBinding
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = resource.binding;
            layoutBinding.descriptorType = ToVulkan(resource.type, false);
            layoutBinding.descriptorCount = resource.arraySize;
            layoutBinding.stageFlags = ToVulkan(resource.stages);

            bindings.push_back(layoutBinding);

            // Store mapping between binding and the binding point
            bindingsLookup.emplace(resource.binding, layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        createInfo.flags = 0;
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        // Create the Vulkan descriptor set layout handle
        VkResult result = vkCreateDescriptorSetLayout(device.GetHandle(), &createInfo, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Cannot create DescriptorSetLayout");
        }
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device.GetHandle(), handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }

    const std::vector<VkDescriptorSetLayoutBinding>& VulkanDescriptorSetLayout::GetLayoutBindings() const
    {
        return bindings;
    }

    std::unique_ptr<VkDescriptorSetLayoutBinding> VulkanDescriptorSetLayout::GetLayoutBinding(const uint32_t bindingIndex) const
    {
        auto it = bindingsLookup.find(bindingIndex);

        if (it == bindingsLookup.end())
        {
            return nullptr;
        }

        return std::make_unique<VkDescriptorSetLayoutBinding>(it->second);
    }

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
                vkDescriptorSetLayouts.push_back(descriptorSetLayouts[i]->GetHandle());
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

    VulkanDescriptorSetLayout* VulkanPipelineLayout::GetDescriptorSetLayout(const uint32_t setIndex) const
    {
        for (auto& descriptorSetLayout : descriptorSetLayouts)
        {
            if (descriptorSetLayout->GetIndex() == setIndex)
            {
                return descriptorSetLayout;
            }
        }

        LOGE("Couldn't find descriptor set layout at set index {}", setIndex);
        return nullptr;
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
