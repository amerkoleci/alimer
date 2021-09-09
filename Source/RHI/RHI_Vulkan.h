// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#define NOMINMAX
#include "RHI.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

#include <unordered_map>

namespace RHI
{
    class VulkanDevice final : public IDevice
    {
    public:
        VulkanDevice(ValidationMode validationMode);
        ~VulkanDevice() override;

    private:
        bool debugUtils = false;
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDeviceVulkan11Properties properties_1_1 = {};
        VkPhysicalDeviceVulkan12Properties properties_1_2 = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_properties = {};
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = {};

        VkPhysicalDeviceFeatures2 features2 = {};
        VkPhysicalDeviceVulkan11Features features_1_1 = {};
        VkPhysicalDeviceVulkan12Features features_1_2 = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {};
        VkPhysicalDeviceRayQueryFeaturesKHR raytracing_query_features = {};
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {};
        VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = {};

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilies;

        uint32_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t computeFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t copyFamily = VK_QUEUE_FAMILY_IGNORED;
        std::vector<uint32_t> queueFamilies; // Unique queue families

        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue copyQueue = VK_NULL_HANDLE;

        VmaAllocator allocator = VK_NULL_HANDLE;
    };
}
