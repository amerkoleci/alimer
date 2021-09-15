// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipelineLayout.h"
#include "VulkanSwapChain.h"
#include "VulkanSampler.h"
#include "VulkanGraphics.h"

#include "Math/MathHelper.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Alimer
{
    namespace
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
        {
            std::string messageTypeStr = "General";

            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                messageTypeStr = "Validation";
            else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                messageTypeStr = "Performance";

            // Log debug messge
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                LOGW("Vulkan - {}: {}", messageTypeStr.c_str(), pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LOGE("Vulkan - {}: {}", messageTypeStr.c_str(), pCallbackData->pMessage);
            }

            return VK_FALSE;
        }

        bool ValidateLayers(const std::vector<const char*>& required,
            const std::vector<VkLayerProperties>& available)
        {
            for (auto layer : required)
            {
                bool found = false;
                for (auto& available_layer : available)
                {
                    if (strcmp(available_layer.layerName, layer) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    LOGW("Validation Layer '{}' not found", layer);
                    return false;
                }
            }

            return true;
        }

        std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& supported_instance_layers)
        {
            std::vector<std::vector<const char*>> validation_layer_priority_list =
            {
                // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
                {"VK_LAYER_KHRONOS_validation"},

                // Otherwise we fallback to using the LunarG meta layer
                {"VK_LAYER_LUNARG_standard_validation"},

                // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
                {
                    "VK_LAYER_GOOGLE_threading",
                    "VK_LAYER_LUNARG_parameter_validation",
                    "VK_LAYER_LUNARG_object_tracker",
                    "VK_LAYER_LUNARG_core_validation",
                    "VK_LAYER_GOOGLE_unique_objects",
                },

                // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
                {"VK_LAYER_LUNARG_core_validation"}
            };

            for (auto& validation_layers : validation_layer_priority_list)
            {
                if (ValidateLayers(validation_layers, supported_instance_layers))
                {
                    return validation_layers;
                }

                LOGW("Couldn't enable validation layers (see log for error) - falling back");
            }

            // Else return nothing
            return {};
        }

        const char* kDeviceTypes[] = {
            "Other",
            "IntegratedGPU",
            "DiscreteGPU",
            "VirtualGPU",
            "CPU"
        };

        struct PhysicalDeviceExtensions
        {
            bool swapchain;
            bool depth_clip_enable;
            bool memory_budget;
            bool performance_query;
            bool host_query_reset;
            bool sampler_mirror_clamp_to_edge;
            bool spirv_1_4;
            bool buffer_device_address;
            bool deferred_host_operations;
            bool descriptor_indexing;
            bool accelerationStructure;
            bool raytracingPipeline;
            bool rayQuery;
            bool create_renderpass2;
            bool fragment_shading_rate;
            bool NV_mesh_shader;
            bool win32_full_screen_exclusive;
        };

        PhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
        {
            uint32_t count = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));
            std::vector<VkExtensionProperties> vk_extensions(count);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data()));

            PhysicalDeviceExtensions extensions{};

            for (uint32_t i = 0; i < count; ++i)
            {
                //LOG_INFO("Extension: {}", vk_extensions[i].extensionName);

                if (strcmp(vk_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    extensions.swapchain = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME) == 0) {
                    extensions.depth_clip_enable = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
                    extensions.memory_budget = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0) {
                    extensions.performance_query = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0) {
                    extensions.host_query_reset = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) == 0) {
                    extensions.sampler_mirror_clamp_to_edge = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0) {
                    extensions.spirv_1_4 = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0) {
                    extensions.buffer_device_address = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
                    extensions.deferred_host_operations = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0) {
                    extensions.descriptor_indexing = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
                    extensions.accelerationStructure = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                    extensions.raytracingPipeline = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
                    extensions.rayQuery = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0) {
                    extensions.create_renderpass2 = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0) {
                    extensions.fragment_shading_rate = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0) {
                    extensions.NV_mesh_shader = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
                    extensions.win32_full_screen_exclusive = true;
                }
            }

            // We run on vulkan 1.1 or higher.
            VkPhysicalDeviceProperties gpuProps;
            vkGetPhysicalDeviceProperties(physicalDevice, &gpuProps);

            // Promoted in 1.2
            if (gpuProps.apiVersion >= VK_API_VERSION_1_2)
            {
                extensions.host_query_reset = true;
                extensions.sampler_mirror_clamp_to_edge = true;
                extensions.spirv_1_4 = true;
                //extensions.buffer_device_address = true;
                extensions.descriptor_indexing = true;
                extensions.create_renderpass2 = true;
            }

            return extensions;
        }

        inline VkBool32 GetPhysicalDevicePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
        {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
#else
            return true;
#endif
        }
    }

    VulkanGraphics::VulkanGraphics(ValidationMode validationMode)
    {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            return;
        }

        const uint32_t instanceVersion = volkGetInstanceVersion();
        if (instanceVersion < VK_API_VERSION_1_2)
        {
            //ErrorDialog("Error", "Vulkan 1.2 is required");
            return;
        }

        // Create instance and debug utils first.
        {
            uint32_t instanceExtensionCount;
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
            std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data()));

            uint32_t instanceLayerCount;
            VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
            std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
            VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

            std::vector<const char*> instanceLayers;
            std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

            // Enable surface extensions depending on os
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
            instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
            instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
            instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
            instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

            if (validationMode != ValidationMode::Disabled)
            {
                // Determine the optimal validation layers to enable that are necessary for useful debugging
                std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
                instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
            }

            // Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
            for (auto& available_extension : availableInstanceExtensions)
            {
                if (strcmp(available_extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                {
                    debugUtils = true;
                    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }
                else if (strcmp(available_extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
                {
                    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                }
            }


#if defined(_DEBUG)
            bool validationFeatures = false;
            if (validationMode == ValidationMode::GPU)
            {
                uint32_t layerInstanceExtensionCount;
                VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, nullptr));
                std::vector<VkExtensionProperties> availableLayerInstanceExtensions(layerInstanceExtensionCount);
                VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, availableLayerInstanceExtensions.data()));

                for (auto& availableExtension : availableLayerInstanceExtensions)
                {
                    if (strcmp(availableExtension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
                    {
                        validationFeatures = true;
                        instanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                    }
                }
            }
#endif 

            VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
            appInfo.pApplicationName = "Alimer";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Alimer";
            appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
            appInfo.apiVersion = instanceVersion;

            VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
            createInfo.ppEnabledLayerNames = instanceLayers.data();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
            createInfo.ppEnabledExtensionNames = instanceExtensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

            if (validationMode != ValidationMode::Disabled && debugUtils)
            {
                debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
                createInfo.pNext = &debugUtilsCreateInfo;
            }

#if defined(_DEBUG)
            VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
            if (validationFeatures)
            {
                static const VkValidationFeatureEnableEXT enable_features[2] = {
                    VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                    VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                };
                validationFeaturesInfo.enabledValidationFeatureCount = 2;
                validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
                validationFeaturesInfo.pNext = createInfo.pNext;
                createInfo.pNext = &validationFeaturesInfo;
            }
#endif

            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
                return;
            }

            volkLoadInstanceOnly(instance);

            if (validationMode != ValidationMode::Disabled && debugUtils)
            {
                result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
                if (result != VK_SUCCESS)
                {
                    VK_LOG_ERROR(result, "Could not create debug utils messenger");
                }
            }

            LOGI("Created VkInstance with version: {}.{}.{}",
                VK_VERSION_MAJOR(appInfo.apiVersion),
                VK_VERSION_MINOR(appInfo.apiVersion),
                VK_VERSION_PATCH(appInfo.apiVersion)
            );

            if (createInfo.enabledLayerCount)
            {
                LOGI("Enabled {} Validation Layers:", createInfo.enabledLayerCount);

                for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
                {
                    LOGI("	\t{}", createInfo.ppEnabledLayerNames[i]);
                }
            }

            LOGI("Enabled {} Instance Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
            }
        }

        // Enumerate physical devices and create logical device.
        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDeviceVulkan11Properties properties_1_1 = {};
        VkPhysicalDeviceVulkan12Properties properties_1_2 = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_properties = {};
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = {};

        VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceVulkan11Features features_1_1 = {};
        VkPhysicalDeviceVulkan12Features features_1_2 = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {};
        VkPhysicalDeviceRayQueryFeaturesKHR raytracing_query_features = {};
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {};
        VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = {};
        VkPhysicalDevicePerformanceQueryFeaturesKHR perf_counter_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR };
        VkPhysicalDeviceHostQueryResetFeatures host_query_reset_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES };

        {
            uint32_t deviceCount = 0;
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

            if (deviceCount == 0)
            {
                LOGF("Vulkan: Failed to find GPUs with Vulkan support");
                //ErrorDialog("Error", "Vulkan: Failed to find GPUs with Vulkan support");
                return;
            }

            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));

            std::vector<const char*> enabledExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
            };

            for (const VkPhysicalDevice& physicalDevice : physicalDevices)
            {
                bool suitable = true;

                uint32_t extensionCount;
                VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data()));

                PhysicalDeviceExtensions physicalDeviceExt = QueryPhysicalDeviceExtensions(physicalDevice);
                suitable = physicalDeviceExt.swapchain && physicalDeviceExt.depth_clip_enable;

                if (!suitable)
                {
                    continue;
                }

                features_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                features2.pNext = &features_1_1;
                features_1_1.pNext = &features_1_2;
                void** features_chain = &features_1_2.pNext;
                acceleration_structure_features = {};
                raytracing_features = {};
                raytracing_query_features = {};
                fragment_shading_rate_features = {};
                mesh_shader_features = {};

                properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                properties_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
                properties_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
                properties2.pNext = &properties_1_1;
                properties_1_1.pNext = &properties_1_2;
                void** properties_chain = &properties_1_2.pNext;
                acceleration_structure_properties = {};
                raytracing_properties = {};
                fragment_shading_rate_properties = {};
                mesh_shader_properties = {};

                if (physicalDeviceExt.memory_budget)
                {
                    enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
                }

                // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
                if (physicalDeviceExt.performance_query &&
                    physicalDeviceExt.host_query_reset)
                {
                    VkPhysicalDeviceFeatures2 physical_device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
                    physical_device_features.pNext = &perf_counter_features;
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);

                    physical_device_features.pNext = &host_query_reset_features;
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);

                    if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset)
                    {
                        *features_chain = &perf_counter_features;
                        features_chain = &perf_counter_features.pNext;
                        *features_chain = &host_query_reset_features;
                        features_chain = &host_query_reset_features.pNext;

                        enabledExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
                        enabledExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
                    }
                }

                if (physicalDeviceExt.spirv_1_4)
                {
                    // Required for VK_KHR_ray_tracing_pipeline
                    enabledExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

                    // Required by VK_KHR_spirv_1_4
                    enabledExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
                }

                if (physicalDeviceExt.buffer_device_address)
                {
                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
                }

                if (physicalDeviceExt.descriptor_indexing)
                {
                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
                }

                if (physicalDeviceExt.accelerationStructure)
                {
                    ALIMER_ASSERT(physicalDeviceExt.buffer_device_address);
                    ALIMER_ASSERT(physicalDeviceExt.descriptor_indexing);
                    ALIMER_ASSERT(physicalDeviceExt.deferred_host_operations);

                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

                    enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                    acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                    *features_chain = &acceleration_structure_features;
                    features_chain = &acceleration_structure_features.pNext;
                    acceleration_structure_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
                    *properties_chain = &acceleration_structure_properties;
                    properties_chain = &acceleration_structure_properties.pNext;

                    if (physicalDeviceExt.raytracingPipeline)
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                        raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                        *features_chain = &raytracing_features;
                        features_chain = &raytracing_features.pNext;
                        raytracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                        *properties_chain = &raytracing_properties;
                        properties_chain = &raytracing_properties.pNext;
                    }

                    if (physicalDeviceExt.rayQuery)
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
                        raytracing_query_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                        *features_chain = &raytracing_query_features;
                        features_chain = &raytracing_query_features.pNext;
                    }
                }

                if (physicalDeviceExt.create_renderpass2)
                {
                    enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
                }

                if (physicalDeviceExt.fragment_shading_rate)
                {
                    ALIMER_ASSERT(physicalDeviceExt.create_renderpass2);

                    enabledExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
                    fragment_shading_rate_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
                    *features_chain = &fragment_shading_rate_features;
                    features_chain = &fragment_shading_rate_features.pNext;
                    fragment_shading_rate_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
                    *properties_chain = &fragment_shading_rate_properties;
                    properties_chain = &fragment_shading_rate_properties.pNext;
                }

                if (physicalDeviceExt.NV_mesh_shader)
                {
                    enabledExtensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
                    mesh_shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
                    *features_chain = &mesh_shader_features;
                    features_chain = &mesh_shader_features.pNext;
                    mesh_shader_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
                    *properties_chain = &mesh_shader_properties;
                    properties_chain = &mesh_shader_properties.pNext;
                }

                vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

                bool discrete = properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                if (discrete || this->physicalDevice == VK_NULL_HANDLE)
                {
                    this->physicalDevice = physicalDevice;
                    if (discrete)
                    {
                        break; // if this is discrete GPU, look no further (prioritize discrete GPU)
                    }
                }
            }

            if (physicalDevice == VK_NULL_HANDLE)
            {
                LOGE("Vulkan: Failed to find a suitable GPU");
                return;
            }

#if TODO
            caps.backendType = GPUBackendType::Vulkan;
            caps.vendorId = properties2.properties.vendorID;
            caps.adapterId = properties2.properties.deviceID;

            switch (properties2.properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                caps.adapterType = GPUAdapterType::IntegratedGPU;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                caps.adapterType = GPUAdapterType::DiscreteGPU;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                caps.adapterType = GPUAdapterType::CPU;
                break;

            default:
                caps.adapterType = GPUAdapterType::Unknown;
                break;
            }

            caps.adapterName = properties2.properties.deviceName;
            caps.blobType = ShaderBlobType::SPIRV;

            caps.features.independentBlend = features2.features.independentBlend;
            caps.features.computeShader = true;
            caps.features.multiViewport = features2.features.multiViewport;
            caps.features.indexUInt32 = features2.features.fullDrawIndexUint32;
            caps.features.multiDrawIndirect = features2.features.multiDrawIndirect;
            caps.features.fillModeNonSolid = features2.features.fillModeNonSolid;
            caps.features.samplerAnisotropy = features2.features.samplerAnisotropy;
            caps.features.textureCompressionETC2 = features2.features.textureCompressionETC2;
            caps.features.textureCompressionASTC_LDR = features2.features.textureCompressionASTC_LDR;
            caps.features.textureCompressionBC = features2.features.textureCompressionBC;
            caps.features.textureCubeArray = features2.features.imageCubeArray;
            caps.features.raytracing = false;

            // Search for depth stencil format
            const std::vector<VkFormat> depthStencilFormats = {
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT
            };

            for (VkFormat format : depthStencilFormats)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    caps.defaultDepthStencilFormat = FromVulkanFormat(format);
                    break;
                }
            }

            const std::vector<VkFormat> depthFormats = {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D16_UNORM
            };

            for (VkFormat format : depthFormats)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    caps.defaultDepthFormat = FromVulkanFormat(format);
                    break;
                }
            }

            ALIMER_ASSERT(properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE);

            caps.limits.minUniformBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minUniformBufferOffsetAlignment;
            caps.limits.minStorageBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minStorageBufferOffsetAlignment;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

            ALIMER_ASSERT(features2.features.imageCubeArray == VK_TRUE);
            ALIMER_ASSERT(features2.features.independentBlend == VK_TRUE);
            ALIMER_ASSERT(features2.features.geometryShader == VK_TRUE);
            ALIMER_ASSERT(features2.features.samplerAnisotropy == VK_TRUE);
            ALIMER_ASSERT(features2.features.shaderClipDistance == VK_TRUE);
            ALIMER_ASSERT(features2.features.textureCompressionBC == VK_TRUE);
            ALIMER_ASSERT(features2.features.occlusionQueryPrecise == VK_TRUE);

            bufferDeviceAddress = features_1_2.bufferDeviceAddress;

            if (features2.features.tessellationShader == VK_TRUE)
            {
            }

            if (features2.features.shaderStorageImageExtendedFormats == VK_TRUE)
            {
            }

            if (raytracing_features.rayTracingPipeline == VK_TRUE)
            {
                ALIMER_ASSERT(acceleration_structure_features.accelerationStructure == VK_TRUE);
                ALIMER_ASSERT(features_1_2.bufferDeviceAddress == VK_TRUE);
                caps.features.raytracing = true;
                //SHADER_IDENTIFIER_SIZE = raytracing_properties.shaderGroupHandleSize;
            }
            if (raytracing_query_features.rayQuery == VK_TRUE)
            {
                ALIMER_ASSERT(acceleration_structure_features.accelerationStructure == VK_TRUE);
                ALIMER_ASSERT(features_1_2.bufferDeviceAddress == VK_TRUE);
                caps.features.raytracingInline = true;
            }
            if (mesh_shader_features.meshShader == VK_TRUE && mesh_shader_features.taskShader == VK_TRUE)
            {
                caps.features.meshShader = true;
            }
            if (fragment_shading_rate_features.pipelineFragmentShadingRate == VK_TRUE)
            {
                caps.features.variableRateShading = true;
            }
            if (fragment_shading_rate_features.attachmentFragmentShadingRate == VK_TRUE)
            {
                caps.features.variableRateShadingExtended = true;
                //VARIABLE_RATE_SHADING_TILE_SIZE = std::min(fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.width, fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.height);
            }

            ALIMER_ASSERT(features_1_2.hostQueryReset == VK_TRUE);

            if (features_1_2.descriptorIndexing)
            {
                caps.features.bindlessDescriptors = true;
            }

            VkFormatProperties formatProperties = {};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, ToVulkanFormat(PixelFormat::RG11B10Float), &formatProperties);
            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            {
                //capabilities |= GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT;
            }


            // Limits
            caps.limits.maxVertexAttributes = properties2.properties.limits.maxVertexInputAttributes;
            caps.limits.maxVertexBindings = properties2.properties.limits.maxVertexInputBindings;
            caps.limits.maxVertexAttributeOffset = properties2.properties.limits.maxVertexInputAttributeOffset;
            caps.limits.maxVertexBindingStride = properties2.properties.limits.maxVertexInputBindingStride;

            caps.limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
            caps.limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
            caps.limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
            caps.limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
            caps.limits.maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
            caps.limits.maxColorAttachments = properties2.properties.limits.maxColorAttachments;
            caps.limits.minUniformBufferOffsetAlignment = properties2.properties.limits.minUniformBufferOffsetAlignment;
            caps.limits.minStorageBufferOffsetAlignment = properties2.properties.limits.minStorageBufferOffsetAlignment;
            caps.limits.maxSamplerAnisotropy = (uint16_t)properties2.properties.limits.maxSamplerAnisotropy;
            caps.limits.maxViewports = properties2.properties.limits.maxViewports;
            caps.limits.maxViewportWidth = properties2.properties.limits.maxViewportDimensions[0];
            caps.limits.maxViewportHeight = properties2.properties.limits.maxViewportDimensions[1];
            caps.limits.maxTessellationPatchSize = properties2.properties.limits.maxTessellationPatchSize;
            caps.limits.maxComputeSharedMemorySize = properties2.properties.limits.maxComputeSharedMemorySize;
            caps.limits.maxComputeWorkGroupCountX = properties2.properties.limits.maxComputeWorkGroupCount[0];
            caps.limits.maxComputeWorkGroupCountY = properties2.properties.limits.maxComputeWorkGroupCount[1];
            caps.limits.maxComputeWorkGroupCountZ = properties2.properties.limits.maxComputeWorkGroupCount[2];
            caps.limits.maxComputeWorkGroupInvocations = properties2.properties.limits.maxComputeWorkGroupInvocations;
            caps.limits.maxComputeWorkGroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0];
            caps.limits.maxComputeWorkGroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1];
            caps.limits.maxComputeWorkGroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2];

            for (uint32_t i = ecast(PixelFormat::Undefined) + 1; i < ecast(PixelFormat::Count); i++)
            {
                const VkFormat vkFormat = ToVulkanFormat((PixelFormat)i);
                if (vkFormat == VK_FORMAT_UNDEFINED)
                    continue;

                vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &formatProperties);
                const VkFormatFeatureFlags vkFeatures = formatProperties.optimalTilingFeatures;

                PixelFormatFeatures features = PixelFormatFeatures::None;
                if (vkFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
                    features |= PixelFormatFeatures::Sampled;
                if (vkFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
                    features |= PixelFormatFeatures::RenderTarget;
                if (vkFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
                    features |= PixelFormatFeatures::RenderTargetBlend;
                if (vkFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    features |= PixelFormatFeatures::DepthStencil;
                if (vkFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
                    features |= PixelFormatFeatures::Filter;
                if (vkFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
                    features |= PixelFormatFeatures::Storage;
                if (vkFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)
                    features |= PixelFormatFeatures::StorageAtomic;

                if (vkFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)
                    features |= PixelFormatFeatures::Blit;
                if (vkFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)
                    features |= PixelFormatFeatures::Blit;

                caps.formatProperties[i].features = features;
            }
#endif // TODO


            // Find queue families:
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            std::vector<uint32_t> queueOffsets(queueFamilyCount);
            std::vector<std::vector<float>> queuePriorities(queueFamilyCount);

            uint32_t graphicsQueueIndex = 0;
            uint32_t computeQueueIndex = 0;
            uint32_t copyQueueIndex = 0;

            const auto FindVacantQueue = [&](uint32_t& family, uint32_t& index,
                VkQueueFlags required, VkQueueFlags ignore_flags,
                float priority) -> bool
            {
                for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
                {
                    if ((queueFamilies[familyIndex].queueFlags & ignore_flags) != 0)
                        continue;

                    // A graphics queue candidate must support present for us to select it.
                    if ((required & VK_QUEUE_GRAPHICS_BIT) != 0)
                    {
                        VkBool32 supported = GetPhysicalDevicePresentationSupport(physicalDevice, familyIndex);
                        if (!supported)
                            continue;
                    }

                    if (queueFamilies[familyIndex].queueCount &&
                        (queueFamilies[familyIndex].queueFlags & required) == required)
                    {
                        family = familyIndex;
                        queueFamilies[familyIndex].queueCount--;
                        index = queueOffsets[familyIndex]++;
                        queuePriorities[familyIndex].push_back(priority);
                        return true;
                    }
                }

                return false;
            };

            if (!FindVacantQueue(graphicsQueueFamily, graphicsQueueIndex,
                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
            {
                LOGE("Vulkan: Could not find suitable graphics queue.");
                return;
            }

            // Prefer another graphics queue since we can do async graphics that way.
            // The compute queue is to be treated as high priority since we also do async graphics on it.
            if (!FindVacantQueue(computeQueueFamily, computeQueueIndex, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
                !FindVacantQueue(computeQueueFamily, computeQueueIndex, VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
            {
                // Fallback to the graphics queue if we must.
                computeQueueFamily = graphicsQueueFamily;
                computeQueueIndex = graphicsQueueIndex;
            }

            // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
            // If not, fallback to a dedicated compute queue.
            // Finally, fallback to same queue as compute.
            if (!FindVacantQueue(copyQueueFamily, copyQueueIndex, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
                !FindVacantQueue(copyQueueFamily, copyQueueIndex, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
            {
                copyQueueFamily = computeQueueFamily;
                copyQueueIndex = computeQueueIndex;
            }

            VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

            std::vector<VkDeviceQueueCreateInfo> queueInfos;
            for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
            {
                if (queueOffsets[familyIndex] == 0)
                    continue;

                VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                info.queueFamilyIndex = familyIndex;
                info.queueCount = queueOffsets[familyIndex];
                info.pQueuePriorities = queuePriorities[familyIndex].data();
                queueInfos.push_back(info);
            }

            createInfo.pNext = &features2;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
            createInfo.pQueueCreateInfos = queueInfos.data();
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pEnabledFeatures = nullptr;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();

            VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create device");
                return;
            }

            volkLoadDevice(device);

            graphicsQueue = new VulkanCommandQueue(*this, CommandQueueType::Graphics, graphicsQueueFamily, graphicsQueueIndex);
            computeQueue = new VulkanCommandQueue(*this, CommandQueueType::Compute, computeQueueFamily, computeQueueIndex);
            vkGetDeviceQueue(device, copyQueueFamily, copyQueueIndex, &copyQueue);

            LOGI("Vendor : {}", GetVendorName(properties2.properties.vendorID));
            LOGI("Name   : {}", properties2.properties.deviceName);
            LOGI("Type   : {}", kDeviceTypes[properties2.properties.deviceType]);
            LOGI("Driver : {}", properties2.properties.driverVersion);

            LOGI("Enabled {} Device Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
            }

#ifdef _DEBUG
            LOGD("Graphics queue: family {}, index {}.", graphicsQueueFamily, graphicsQueueIndex);
            LOGD("Compute queue: family {}, index {}.", computeQueueFamily, computeQueueIndex);
            LOGD("Transfer queue: family {}, index {}.", copyQueueFamily, copyQueueIndex);
#endif
        }

        // Create memory allocator
        {
            VmaVulkanFunctions vmaVulkanFunc{};
            vmaVulkanFunc.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
            vmaVulkanFunc.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
            vmaVulkanFunc.vkAllocateMemory = vkAllocateMemory;
            vmaVulkanFunc.vkFreeMemory = vkFreeMemory;
            vmaVulkanFunc.vkMapMemory = vkMapMemory;
            vmaVulkanFunc.vkUnmapMemory = vkUnmapMemory;
            vmaVulkanFunc.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
            vmaVulkanFunc.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
            vmaVulkanFunc.vkBindBufferMemory = vkBindBufferMemory;
            vmaVulkanFunc.vkBindImageMemory = vkBindImageMemory;
            vmaVulkanFunc.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
            vmaVulkanFunc.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
            vmaVulkanFunc.vkCreateBuffer = vkCreateBuffer;
            vmaVulkanFunc.vkDestroyBuffer = vkDestroyBuffer;
            vmaVulkanFunc.vkCreateImage = vkCreateImage;
            vmaVulkanFunc.vkDestroyImage = vkDestroyImage;
            vmaVulkanFunc.vkCmdCopyBuffer = vkCmdCopyBuffer;

            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = physicalDevice;
            allocatorInfo.device = device;
            allocatorInfo.instance = instance;

            // Core in 1.1
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
            vmaVulkanFunc.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
            vmaVulkanFunc.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
            vmaVulkanFunc.vkBindBufferMemory2KHR = vkBindBufferMemory2;
            vmaVulkanFunc.vkBindImageMemory2KHR = vkBindImageMemory2;

            if (features_1_2.bufferDeviceAddress == VK_TRUE)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            allocatorInfo.pVulkanFunctions = &vmaVulkanFunc;

            VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create allocator");
            }
        }

        // Init copy allocator.
        copyAllocator.Init(this);

        /* Create pipeline cache */
        {
            VkPipelineCacheCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
            //createInfo.initialDataSize = pipelineData.size();
            //createInfo.pInitialData = pipelineData.data();

            VK_CHECK(vkCreatePipelineCache(device, &createInfo, nullptr, &pipelineCache));
        }

        // Create frame sync primitives
        {
            const VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

            for (uint32_t i = 0; i < kMaxFramesInFlight; i++)
            {
                VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frameFences[i]));

                if (debugUtils)
                {
                    SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)frameFences[i], fmt::format("Frame Fence {}", i));
                }
            }
        }

        // Create bindless
        const bool enableBindless = true;
        if (enableBindless)
        {
            if (features_1_2.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE)
            {
                bindlessSampledImages.Init(device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, properties_1_2.maxDescriptorSetUpdateAfterBindSampledImages / 4);
            }

            if (features_1_2.descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE)
            {
                bindlessStorageBuffers.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, properties_1_2.maxDescriptorSetUpdateAfterBindStorageBuffers / 4);
            }
        }

        // Null objects
        {
            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.extent.width = 1;
            imageInfo.extent.height = 1;
            imageInfo.extent.depth = 1;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.arrayLayers = 1;
            imageInfo.mipLevels = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
            imageInfo.flags = 0;
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            VK_CHECK(
                vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage1D, &nullImageAllocation1D, nullptr)
            );

            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            imageInfo.arrayLayers = 6;
            VK_CHECK(
                vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage2D, &nullImageAllocation2D, nullptr)
            );

            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.flags = 0;
            imageInfo.arrayLayers = 1;
            VK_CHECK(
                vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage3D, &nullImageAllocation3D, nullptr)
            );

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewInfo.image = nullImage1D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageView1D));

            viewInfo.image = nullImage1D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageView1DArray));

            viewInfo.image = nullImage2D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageView2D));

            viewInfo.image = nullImage2D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageView2DArray));

            viewInfo.image = nullImage2D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            viewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageViewCube));

            viewInfo.image = nullImage2D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            viewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageViewCubeArray));

            viewInfo.image = nullImage3D;
            viewInfo.subresourceRange.layerCount = 1;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &nullImageView3D));

            VkSamplerCreateInfo samplerCreateInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
            VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &nullSampler));

            // Transition images
            VkCommandBuffer transitionCommandBuffer = CreateCommandBuffer();
            std::array<VkImageMemoryBarrier, 3> barriers = {};

            barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[0].oldLayout = imageInfo.initialLayout;
            barriers[0].newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barriers[0].srcAccessMask = 0;
            barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barriers[0].subresourceRange.baseArrayLayer = 0;
            barriers[0].subresourceRange.baseMipLevel = 0;
            barriers[0].subresourceRange.levelCount = 1;
            barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[0].image = nullImage1D;
            barriers[0].subresourceRange.layerCount = 1;
            barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[1].oldLayout = imageInfo.initialLayout;
            barriers[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barriers[1].srcAccessMask = 0;
            barriers[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barriers[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barriers[1].subresourceRange.baseArrayLayer = 0;
            barriers[1].subresourceRange.baseMipLevel = 0;
            barriers[1].subresourceRange.levelCount = 1;
            barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1].image = nullImage2D;
            barriers[1].subresourceRange.layerCount = 6;
            barriers[2].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[2].oldLayout = imageInfo.initialLayout;
            barriers[2].newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barriers[2].srcAccessMask = 0;
            barriers[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barriers[2].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barriers[2].subresourceRange.baseArrayLayer = 0;
            barriers[2].subresourceRange.baseMipLevel = 0;
            barriers[2].subresourceRange.levelCount = 1;
            barriers[2].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[2].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[2].image = nullImage3D;
            barriers[2].subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                transitionCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                (uint32_t)barriers.size(), barriers.data()
            );

            FlushCommandBuffer(transitionCommandBuffer);
        }

        processCommandsThread = std::thread(&VulkanGraphics::ProccessCommands, this);

        LOGI("Vulkan graphics backend initialized with success");
    }

    VulkanGraphics::~VulkanGraphics()
    {
        VK_CHECK(vkDeviceWaitIdle(device));

        processCommands = false;
        processCommandsThread.join();

        // Framebuffer cache
        {
            std::lock_guard<std::mutex> guard(framebufferCacheMutex);
            for (auto it : framebufferCache)
            {
                vkDestroyFramebuffer(device, it.second, nullptr);
            }
            framebufferCache.clear();
        }

        // RenderPass cache
        {
            std::lock_guard<std::mutex> guard(renderPassCacheMutex);

            for (auto it : renderPassCache)
            {
                vkDestroyRenderPass(device, it.second, nullptr);
            }
            renderPassCache.clear();
        }

        // DescriptorSetLayout cache
        {
            std::lock_guard<std::mutex> guard(descriptorSetLayoutCacheMutex);
            descriptorSetLayoutCache.clear();
        }

        // PipelineLayout cache
        {
            std::lock_guard<std::mutex> guard(pipelineLayoutCacheMutex);
            pipelineLayoutCache.clear();
        }

        // Shutdown copy allocator.
        copyAllocator.Shutdown();

        // Pipeline cache
        vkDestroyPipelineCache(device, pipelineCache, nullptr);

        for (uint32_t i = 0; i < kMaxFramesInFlight; i++)
        {
            vkDestroyFence(device, frameFences[i], nullptr);
        }

        {
            // Destroy all created fences.
            std::lock_guard<std::mutex> guard(fenceMutex);
            for (auto fence : allFences)
            {
                vkDestroyFence(device, fence, nullptr);
            }
        }

        // Bindless data
        {
            bindlessSampledImages.Destroy(device);
            bindlessStorageBuffers.Destroy(device);
        }

        delete graphicsQueue; graphicsQueue = nullptr;
        delete computeQueue; computeQueue = nullptr;

        frameCount = UINT64_MAX;
        ProcessDeletionQueue();
        frameCount = 0;

        // Null resources
        {
            //vmaDestroyBuffer(allocator, nullBuffer, nullBufferAllocation);
            //vkDestroyBufferView(device, nullBufferView, nullptr);
            vmaDestroyImage(allocator, nullImage1D, nullImageAllocation1D);
            vmaDestroyImage(allocator, nullImage2D, nullImageAllocation2D);
            vmaDestroyImage(allocator, nullImage3D, nullImageAllocation3D);
            vkDestroyImageView(device, nullImageView1D, nullptr);
            vkDestroyImageView(device, nullImageView1DArray, nullptr);
            vkDestroyImageView(device, nullImageView2D, nullptr);
            vkDestroyImageView(device, nullImageView2DArray, nullptr);
            vkDestroyImageView(device, nullImageViewCube, nullptr);
            vkDestroyImageView(device, nullImageViewCubeArray, nullptr);
            vkDestroyImageView(device, nullImageView3D, nullptr);
            vkDestroySampler(device, nullSampler, nullptr);
        }

        // Destroy pending resources that still exist.
        Destroy();

        if (allocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(allocator, &stats);

            if (stats.total.usedBytes > 0)
            {
                LOGI("Total device memory leaked: {} bytes.", stats.total.usedBytes);
            }

            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }

        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

        if (debugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
        }

        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
        }
    }

    uint32_t VulkanGraphics::AllocateSRV()
    {
        return bindlessSampledImages.Allocate();
    }

    VkDescriptorSetLayout VulkanGraphics::GetBindlessSampledImageDescriptorSetLayout() const
    {
        return bindlessSampledImages.descriptorSetLayout;
    }

    VkDescriptorSet VulkanGraphics::GetBindlessSampledImageDescriptorSet() const
    {
        return bindlessSampledImages.descriptorSet;
    }

    uint32_t VulkanGraphics::AllocateUAV()
    {
        return bindlessStorageBuffers.Allocate();
    }

    void VulkanGraphics::SetObjectName(VkObjectType type, uint64_t handle, const std::string_view& name)
    {
        if (!debugUtils)
        {
            return;
        }

        VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        info.objectType = type;
        info.objectHandle = handle;
        info.pObjectName = name.data();
        VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &info));
    }

    void VulkanGraphics::WaitIdle()
    {
        VK_CHECK(vkDeviceWaitIdle(device));

        ProcessDeletionQueue();
    }

    void VulkanGraphics::FinishFrame()
    {
        // Update and Render additional Platform Windows
        /*{
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }*/

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        VK_CHECK(
            vkQueueSubmit(ToVulkan(graphicsQueue)->GetHandle(), 1, &submitInfo, frameFences[frameIndex])
        );

        frameCount++;
        frameIndex = frameCount % kMaxFramesInFlight;
        if (frameCount >= kMaxFramesInFlight)
        {
            VK_CHECK(vkWaitForFences(device, 1, &frameFences[frameIndex], VK_TRUE, UINT64_MAX));
            VK_CHECK(vkResetFences(device, 1, &frameFences[frameIndex]));
        }

        ProcessDeletionQueue();

        // Reset frame command pools
        ToVulkan(graphicsQueue)->Reset(frameIndex);
        ToVulkan(computeQueue)->Reset(frameIndex);
    }

    VkCommandBuffer VulkanGraphics::CreateCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = ToVulkan(graphicsQueue)->GetCommandPool();
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to allocate command buffer");
        }

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        return commandBuffer;
    }

    void VulkanGraphics::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        VK_CHECK(vkEndCommandBuffer(commandBuffer));

        VkFence fence = AcquireFence();

        // Submit to the queue.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1u;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(ToVulkan(graphicsQueue)->GetHandle(), 1, &submitInfo, fence);

        // Wait for the fence to signal that command buffer has finished executing
        vkWaitForFences(device, 1, &fence, VK_TRUE, VK_DEFAULT_FENCE_TIMEOUT);

        ReleaseFence(fence);
    }

    VulkanUploadContext VulkanGraphics::UploadBegin(uint64_t size)
    {
        return copyAllocator.Allocate(size);
    }

    void VulkanGraphics::UploadEnd(VulkanUploadContext context)
    {
        copyAllocator.Submit(context);
    }

    uint64_t VulkanGraphics::FlushCopy()
    {
        return copyAllocator.Flush();
    }

    VkSemaphore VulkanGraphics::GetCopySemaphore() const
    {
        return copyAllocator.semaphore;
    }


    TextureRef VulkanGraphics::CreateTextureCore(const TextureCreateInfo& info, const void* initialData)
    {
        auto result = new VulkanTexture(*this, info, nullptr, initialData);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    BufferRef VulkanGraphics::CreateBuffer(const BufferCreateInfo& info, const void* initialData)
    {
        auto result = new VulkanBuffer(*this, info, initialData);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    ShaderRef VulkanGraphics::CreateShader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint)
    {
        auto result = new VulkanShader(*this, stage, byteCode, entryPoint);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return ShaderRef::Create(result);
        }

        delete result;
        return nullptr;
    }

    SamplerRef VulkanGraphics::CreateSampler(const SamplerCreateInfo* info)
    {
        auto result = new VulkanSampler(*this, info);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return SamplerRef::Create(result);
        }

        delete result;
        return nullptr;
    }

    PipelineRef VulkanGraphics::CreateRenderPipeline(const RenderPipelineStateCreateInfo* info)
    {
        auto result = new VulkanPipeline(*this, info);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    PipelineRef VulkanGraphics::CreateComputePipeline(const ComputePipelineCreateInfo* info)
    {
        auto result = new VulkanPipeline(*this, info);

        if (result->GetHandle() != nullptr)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    SwapChainRef VulkanGraphics::CreateSwapChain(void* windowHandle, const SwapChainCreateInfo& info)
    {
        auto result = new VulkanSwapChain(*this, windowHandle, info);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return SwapChainRef::Create(result);
        }

        delete result;
        return nullptr;
    }

    void VulkanGraphics::DeferDestroy(VkImage texture, VmaAllocation allocation)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionImagesQueue.push_back(std::make_pair(std::make_pair(texture, allocation), frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkBuffer buffer, VmaAllocation allocation)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionBuffersQueue.push_back(std::make_pair(std::make_pair(buffer, allocation), frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkImageView resource)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionImageViews.push_back(std::make_pair(resource, frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkSampler resource)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionSamplers.push_back(std::make_pair(resource, frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkShaderModule resource)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionShaderModulesQueue.push_back(std::make_pair(resource, frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkPipeline resource)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionPipelinesQueue.push_back(std::make_pair(resource, frameCount));
    }

    void VulkanGraphics::DeferDestroy(VkDescriptorPool resource)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        deletionDescriptorPoolQueue.push_back(std::make_pair(resource, frameCount));
    }

    void VulkanGraphics::ProccessCommands()
    {
        //std::unique_lock<std::mutex> lock(processCommandsThreadMutex, std::defer_lock);
        //
        //while (processCommands)
        //{
        //    VkFence fence;
        //
        //    lock.lock();
        //    while (submittedFences.try_pop(fence))
        //    {
        //        if (vkGetFenceStatus(device, fence) == VK_SUCCESS)
        //        {
        //            // Reuse fences
        //            ReleaseFence(fence);
        //        }
        //    }
        //
        //    lock.unlock();
        //}
    }

    void VulkanGraphics::ProcessDeletionQueue()
    {
        std::lock_guard<std::mutex> guard(destroyMutex);
        while (!deletionImagesQueue.empty())
        {
            if (deletionImagesQueue.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionImagesQueue.front();
                deletionImagesQueue.pop_front();
                vmaDestroyImage(allocator, item.first.first, item.first.second);
            }
            else
            {
                break;
            }
        }

        while (!deletionImageViews.empty())
        {
            if (deletionImageViews.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionImageViews.front();
                deletionImageViews.pop_front();
                vkDestroyImageView(device, item.first, nullptr);
            }
            else
            {
                break;
            }
        }

        while (!deletionSamplers.empty())
        {
            if (deletionSamplers.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionSamplers.front();
                deletionSamplers.pop_front();
                vkDestroySampler(device, item.first, nullptr);
            }
            else
            {
                break;
            }
        }

        while (!deletionBuffersQueue.empty())
        {
            if (deletionBuffersQueue.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionBuffersQueue.front();
                deletionBuffersQueue.pop_front();
                vmaDestroyBuffer(allocator, item.first.first, item.first.second);
            }
            else
            {
                break;
            }
        }

        while (!deletionShaderModulesQueue.empty())
        {
            if (deletionShaderModulesQueue.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionShaderModulesQueue.front();
                deletionShaderModulesQueue.pop_front();
                vkDestroyShaderModule(device, item.first, nullptr);
            }
            else
            {
                break;
            }
        }

        while (!deletionPipelinesQueue.empty())
        {
            if (deletionPipelinesQueue.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionPipelinesQueue.front();
                deletionPipelinesQueue.pop_front();
                vkDestroyPipeline(device, item.first, nullptr);
            }
            else
            {
                break;
            }
        }

        while (!deletionDescriptorPoolQueue.empty())
        {
            if (deletionDescriptorPoolQueue.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deletionDescriptorPoolQueue.front();
                deletionDescriptorPoolQueue.pop_front();
                vkDestroyDescriptorPool(device, item.first, nullptr);
            }
            else
            {
                break;
            }
        }
    }

    VkFence VulkanGraphics::AcquireFence()
    {
        // See if there's a free fence available.
        std::lock_guard<std::mutex> guard(fenceMutex);
        VkFence fence;
        if (availableFences.size() > 0)
        {
            fence = availableFences.front();
            availableFences.pop();
        }
        // Else create a new one.
        else
        {
            VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            VK_CHECK(vkCreateFence(device, &createInfo, nullptr, &fence));
            allFences.emplace(fence);

#ifdef _DEBUG
            if (debugUtils) {
                static uint32_t fenceCount = 0;
                fenceCount++;
                SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)fence, fmt::format("Submit Fence {}", fenceCount));
            }
#endif
        }

        return fence;
    }

    void VulkanGraphics::ReleaseFence(VkFence fence)
    {
        std::lock_guard<std::mutex> guard(fenceMutex);
        if (allFences.find(fence) != allFences.end())
        {
            VK_CHECK(vkResetFences(device, 1, &fence));
            availableFences.push(fence);
        }
    }

    void VulkanGraphics::SubmitFence(VkFence fence)
    {
        //submittedFences.push(fence);
    }

    /* Cache */
    VkRenderPass VulkanGraphics::GetVkRenderPass(const VulkanRenderPassKey& key)
    {
        std::lock_guard<std::mutex> guard(renderPassCacheMutex);

        const size_t hash = key.GetHash();
        auto it = renderPassCache.find(hash);
        if (it == renderPassCache.end())
        {
            std::array<VkAttachmentReference, kMaxColorAttachments> colorAttachmentRefs;
            //std::array<VkAttachmentReference, kMaxColorAttachments> resolveAttachmentRefs;
            VkAttachmentReference depthStencilAttachmentRef;

            constexpr uint8_t kMaxAttachmentCount = kMaxColorAttachments * 2 + 1;
            std::array<VkAttachmentDescription, kMaxColorAttachments> attachmentDescs = {};

            const VkSampleCountFlagBits sampleCount = VulkanSampleCount(key.sampleCount);

            uint32_t colorAttachmentIndex = 0;
            for (uint32_t i = 0; i < key.colorAttachmentCount; ++i)
            {
                auto& attachmentRef = colorAttachmentRefs[colorAttachmentIndex];
                auto& attachmentDesc = attachmentDescs[colorAttachmentIndex];

                attachmentRef.attachment = colorAttachmentIndex;
                attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachmentDesc.flags = 0;
                attachmentDesc.format = ToVulkanFormat(key.colorAttachments[i].format);
                attachmentDesc.samples = sampleCount;
                attachmentDesc.loadOp = ToVulkan(key.colorAttachments[i].loadAction);
                attachmentDesc.storeOp = ToVulkan(key.colorAttachments[i].storeAction);
                attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                ++colorAttachmentIndex;
            }

            uint32_t attachmentCount = colorAttachmentIndex;
            VkAttachmentReference* depthStencilAttachment = nullptr;
            if (key.depthStencilAttachment.format != PixelFormat::Undefined)
            {
                auto& attachmentDesc = attachmentDescs[attachmentCount];

                depthStencilAttachment = &depthStencilAttachmentRef;
                depthStencilAttachmentRef.attachment = attachmentCount;
                depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                attachmentDesc.flags = 0;
                attachmentDesc.format = ToVulkanFormat(key.depthStencilAttachment.format);
                attachmentDesc.samples = sampleCount;
                attachmentDesc.loadOp = ToVulkan(key.depthStencilAttachment.loadAction);
                attachmentDesc.storeOp = ToVulkan(key.depthStencilAttachment.storeAction);
                attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                ++attachmentCount;
            }

            VkSubpassDescription subpassDesc;
            subpassDesc.flags = 0;
            subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDesc.inputAttachmentCount = 0;
            subpassDesc.pInputAttachments = nullptr;
            subpassDesc.colorAttachmentCount = colorAttachmentIndex;
            subpassDesc.pColorAttachments = colorAttachmentRefs.data();
            subpassDesc.pResolveAttachments = nullptr;
            subpassDesc.pDepthStencilAttachment = depthStencilAttachment;
            subpassDesc.preserveAttachmentCount = 0u;
            subpassDesc.pPreserveAttachments = nullptr;

            // Use subpass dependencies for layout transitions
            /*std::array<VkSubpassDependency, 2> dependencies;
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;*/

            // Finally, create the renderpass.
            VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
            createInfo.attachmentCount = attachmentCount;
            createInfo.pAttachments = attachmentDescs.data();
            createInfo.subpassCount = 1;
            createInfo.pSubpasses = &subpassDesc;
            createInfo.dependencyCount = 0;
            createInfo.pDependencies = nullptr;
            //createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            //createInfo.pDependencies = dependencies.data();

            VkRenderPass renderPass;
            VK_CHECK(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));

            renderPassCache[hash] = renderPass;

#if defined(_DEBUG)
            LOGD("RenderPass created with hash {}", hash);
#endif

            return renderPass;
        }

        return it->second;
    }

    VkFramebuffer VulkanGraphics::GetVkFramebuffer(uint64_t hash, const VulkanFboKey& key)
    {
        std::lock_guard<std::mutex> guard(framebufferCacheMutex);

        auto it = framebufferCache.find(hash);
        if (it == framebufferCache.end())
        {
            VkFramebufferCreateInfo info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            info.pNext = nullptr;
            info.flags = 0;
            info.renderPass = key.renderPass;
            info.attachmentCount = key.attachmentCount;
            info.pAttachments = key.attachments;
            info.width = key.width;
            info.height = key.height;
            info.layers = key.layers;

            VkFramebuffer framebuffer;
            VkResult result = vkCreateFramebuffer(device, &info, nullptr, &framebuffer);
            framebufferCache[hash] = framebuffer;

#if defined(_DEBUG)
            LOGD("Framebuffer created with hash {}", hash);
#endif

            return framebuffer;
        }

        return it->second;
    }

    VulkanDescriptorSetLayout& VulkanGraphics::RequestDescriptorSetLayout(const uint32_t setIndex, const std::vector<ShaderResource>& resources)
    {
        std::lock_guard<std::mutex> guard(descriptorSetLayoutCacheMutex);

        size_t hash = 0;
        HashCombine(hash, setIndex);

        for (auto& resource : resources)
        {
            if (resource.type == ShaderResourceType::Input ||
                resource.type == ShaderResourceType::Output ||
                resource.type == ShaderResourceType::PushConstant)
            {
                continue;
            }

            HashCombine(hash, static_cast<uint32_t>(resource.stages));
            HashCombine(hash, static_cast<uint32_t>(resource.type));
            HashCombine(hash, resource.set);
            HashCombine(hash, resource.binding);
            //HashCombine(hash, shaderResource.mode);
        }

        auto it = descriptorSetLayoutCache.find(hash);
        if (it == descriptorSetLayoutCache.end())
        {
            auto setLayout = std::make_unique<VulkanDescriptorSetLayout>(*this, setIndex, resources);
            descriptorSetLayoutCache[hash] = std::move(setLayout);

#if defined(_DEBUG)
            LOGD("VkDescriptorSetLayout created with hash {}", hash);
#endif

            it = descriptorSetLayoutCache.find(hash);
        }

        return *it->second;
    }

    VulkanPipelineLayout& VulkanGraphics::RequestPipelineLayout(const std::vector<VulkanShader*>& shaders)
    {
        std::lock_guard<std::mutex> guard(pipelineLayoutCacheMutex);

        size_t hash = 0;

        for (auto& shader : shaders)
        {
            HashCombine(hash, shader->GetHash());
        }

        auto it = pipelineLayoutCache.find(hash);
        if (it == pipelineLayoutCache.end())
        {
            auto pipelineLayout = std::make_unique<VulkanPipelineLayout>(*this, shaders);
            pipelineLayoutCache[hash] = std::move(pipelineLayout);

#if defined(_DEBUG)
            LOGD("PipelineLayout created with hash {}", hash);
#endif
            it = pipelineLayoutCache.find(hash);
        }

        return *it->second;
    }

    /* Copy Allocator */
    void VulkanGraphics::CopyAllocator::Init(VulkanGraphics* device_)
    {
        device = device_;

        VkSemaphoreTypeCreateInfo timelineCreateInfo = {};
        timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineCreateInfo.pNext = nullptr;
        timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreateInfo.initialValue = 0;

        VkSemaphoreCreateInfo createInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        createInfo.pNext = &timelineCreateInfo;
        createInfo.flags = 0;
        VK_CHECK(vkCreateSemaphore(device->device, &createInfo, nullptr, &semaphore));
    }

    void VulkanGraphics::CopyAllocator::Shutdown()
    {
        vkQueueWaitIdle(device->copyQueue);
        for (auto& x : freeList)
        {
            x.uploadBuffer.Reset();
            vkDestroyCommandPool(device->device, x.commandPool, nullptr);
        }

        vkDestroySemaphore(device->device, semaphore, nullptr);
    }

    VulkanUploadContext VulkanGraphics::CopyAllocator::Allocate(uint64_t size)
    {
        locker.lock();

        // create a new command list if there are no free ones:
        if (freeList.empty())
        {
            VulkanUploadContext context;

            VkCommandPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device->copyQueueFamily;

            VK_CHECK(vkCreateCommandPool(device->device, &poolInfo, nullptr, &context.commandPool));

            VkCommandBufferAllocateInfo commandBufferInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            commandBufferInfo.commandBufferCount = 1;
            commandBufferInfo.commandPool = context.commandPool;
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VK_CHECK(
                vkAllocateCommandBuffers(device->device, &commandBufferInfo, &context.commandBuffer)
            );

            freeList.push_back(context);
        }

        VulkanUploadContext context = freeList.back();
        if (context.uploadBuffer != nullptr
            && context.uploadBuffer->GetSize() < size)
        {
            // Try to search for a staging buffer that can fit the request:
            for (size_t i = 0; i < freeList.size(); ++i)
            {
                if (freeList[i].uploadBuffer->GetSize() >= size)
                {
                    context = freeList[i];
                    std::swap(freeList[i], freeList.back());
                    break;
                }
            }
        }

        freeList.pop_back();
        locker.unlock();

        // If no buffer was found that fits the data, create one:
        if (context.uploadBuffer == nullptr
            || context.uploadBuffer->GetSize() < size)
        {
            BufferCreateInfo uploadBufferDesc{};
            uploadBufferDesc.size = NextPowerOfTwo((uint32_t)size);
            uploadBufferDesc.memoryUsage = MemoryUsage::CpuOnly;
            context.uploadBuffer = new VulkanBuffer(*device, uploadBufferDesc, nullptr);
            context.data = context.uploadBuffer->Map();
            ALIMER_ASSERT(context.data != nullptr);
        }

        // Begin command list in valid state.
        VK_CHECK(vkResetCommandPool(device->device, context.commandPool, 0));

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VK_CHECK(vkBeginCommandBuffer(context.commandBuffer, &beginInfo));

        return context;
    }

    void VulkanGraphics::CopyAllocator::Submit(VulkanUploadContext context)
    {
        VK_CHECK(vkEndCommandBuffer(context.commandBuffer));

        // It was very slow in Vulkan to submit the copies immediately
        //	In Vulkan, the submit is not thread safe, so it had to be locked
        //	Instead, the submits are batched and performed in flush() function
        locker.lock();
        context.target = ++fenceValue;
        workList.push_back(context);
        submitCommandBuffers.push_back(context.commandBuffer);
        submitWait = Max(submitWait, context.target);
        locker.unlock();
    }

    uint64_t VulkanGraphics::CopyAllocator::Flush()
    {
        locker.lock();
        if (!submitCommandBuffers.empty())
        {
            VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
            submitInfo.pCommandBuffers = submitCommandBuffers.data();
            submitInfo.pSignalSemaphores = &semaphore;
            submitInfo.signalSemaphoreCount = 1;

            VkTimelineSemaphoreSubmitInfo timelineInfo { VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
            timelineInfo.waitSemaphoreValueCount = 0;
            timelineInfo.pWaitSemaphoreValues = nullptr;
            timelineInfo.signalSemaphoreValueCount = 1;
            timelineInfo.pSignalSemaphoreValues = &submitWait;

            submitInfo.pNext = &timelineInfo;

            VK_CHECK(vkQueueSubmit(device->copyQueue, 1, &submitInfo, VK_NULL_HANDLE));

            submitCommandBuffers.clear();
        }

        // Free up the finished command lists:
        uint64_t completedFenceValue;
        VK_CHECK(vkGetSemaphoreCounterValue(device->device, semaphore, &completedFenceValue));
        for (size_t i = 0; i < workList.size(); ++i)
        {
            if (workList[i].target <= completedFenceValue)
            {
                freeList.push_back(workList[i]);
                workList[i] = workList.back();
                workList.pop_back();
                i--;
            }
        }

        uint64_t value = submitWait;
        submitWait = 0;
        locker.unlock();
        return value;
    }
}
