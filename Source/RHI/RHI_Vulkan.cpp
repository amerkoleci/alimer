// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_VULKAN) 
#include "RHI_Vulkan.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <set>

namespace RHI
{
    namespace
    {
        const char* kDeviceTypes[] = {
            "Other",
            "IntegratedGPU",
            "DiscreteGPU",
            "VirtualGPU",
            "CPU"
        };

        [[nodiscard]] constexpr const char* ToString(VkResult result)
        {
            switch (result)
            {
#define STR(r)   \
	case VK_##r: \
		return #r
                STR(SUCCESS);
                STR(NOT_READY);
                STR(TIMEOUT);
                STR(EVENT_SET);
                STR(EVENT_RESET);
                STR(INCOMPLETE);
                STR(ERROR_OUT_OF_HOST_MEMORY);
                STR(ERROR_OUT_OF_DEVICE_MEMORY);
                STR(ERROR_INITIALIZATION_FAILED);
                STR(ERROR_DEVICE_LOST);
                STR(ERROR_MEMORY_MAP_FAILED);
                STR(ERROR_LAYER_NOT_PRESENT);
                STR(ERROR_EXTENSION_NOT_PRESENT);
                STR(ERROR_FEATURE_NOT_PRESENT);
                STR(ERROR_INCOMPATIBLE_DRIVER);
                STR(ERROR_TOO_MANY_OBJECTS);
                STR(ERROR_FORMAT_NOT_SUPPORTED);
                STR(ERROR_SURFACE_LOST_KHR);
                STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
                STR(SUBOPTIMAL_KHR);
                STR(ERROR_OUT_OF_DATE_KHR);
                STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
                STR(ERROR_VALIDATION_FAILED_EXT);
                STR(ERROR_INVALID_SHADER_NV);
#undef STR
                default:
                    return "UNKNOWN_ERROR";
            }
        }

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
                LogWarn("Vulkan - %s: %s", messageTypeStr.c_str(), pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LogError("Vulkan - %s: %s", messageTypeStr.c_str(), pCallbackData->pMessage);
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
                    LogWarn("Validation Layer '%s' not found", layer);
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

                LogWarn("Couldn't enable validation layers (see log for error) - falling back");
            }

            // Else return nothing
            return {};
        }

        bool CheckExtensionSupport(const char* checkExtension, const std::vector<VkExtensionProperties>& availableExtensions)
        {
            for (const auto& availableExtension : availableExtensions)
            {
                if (strcmp(availableExtension.extensionName, checkExtension) == 0)
                {
                    return true;
                }
            }

            return false;
        }
    }

    /// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			LogError("Detected Vulkan error: %s", ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) LogError("Vulkan: %s, error: %s", message, ToString(result));


    VulkanDevice::VulkanDevice(ValidationMode validationMode)
    {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            LogError("Failed to initialize volk");
            return;
        }

        const uint32_t instanceVersion = volkGetInstanceVersion();
        if (instanceVersion < VK_API_VERSION_1_2)
        {
            LogError("Vulkan version 1.2 is required");
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
            //appInfo.pApplicationName = "Alimer";
            //appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            //appInfo.pEngineName = "Alimer";
            //appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
            appInfo.apiVersion = volkGetInstanceVersion();

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

            LogDebug("Created VkInstance with version: %d.%d.%d",
                VK_VERSION_MAJOR(appInfo.apiVersion),
                VK_VERSION_MINOR(appInfo.apiVersion),
                VK_VERSION_PATCH(appInfo.apiVersion)
            );

            if (createInfo.enabledLayerCount)
            {
                LogDebug("Enabled %d Validation Layers:", createInfo.enabledLayerCount);

                for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
                {
                    LogDebug("	\t%s", createInfo.ppEnabledLayerNames[i]);
                }
            }

            LogDebug("Enabled %d Instance Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LogDebug("	\t%s", createInfo.ppEnabledExtensionNames[i]);
            }
        }

        // Enumerating and creating logical device.
        {
            uint32_t deviceCount = 0;
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

            if (deviceCount == 0)
            {
                LogError("Failed to find GPUs with Vulkan support!");
                return;
            }

            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));

            const std::vector<const char*> requiredDeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
            };

            std::vector<const char*> enabledExtensions;

            for (VkPhysicalDevice candidatePhysicalDevice : physicalDevices)
            {
                bool suitable = true;

                uint32_t extensionCount;
                VK_CHECK(vkEnumerateDeviceExtensionProperties(candidatePhysicalDevice, nullptr, &extensionCount, nullptr));
                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(candidatePhysicalDevice, nullptr, &extensionCount, availableExtensions.data()));

                for (auto& x : requiredDeviceExtensions)
                {
                    if (!CheckExtensionSupport(x, availableExtensions))
                    {
                        suitable = false;
                    }
                }
                if (!suitable)
                    continue;

                features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
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

                enabledExtensions = requiredDeviceExtensions;

                if (CheckExtensionSupport(VK_KHR_SPIRV_1_4_EXTENSION_NAME, availableExtensions))
                {
                    enabledExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
                }

                if (CheckExtensionSupport(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, availableExtensions))
                {
                    enabledExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
                }

                if (CheckExtensionSupport(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, availableExtensions))
                {
                    enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                    assert(CheckExtensionSupport(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, availableExtensions));
                    enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
                    acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                    *features_chain = &acceleration_structure_features;
                    features_chain = &acceleration_structure_features.pNext;
                    acceleration_structure_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
                    *properties_chain = &acceleration_structure_properties;
                    properties_chain = &acceleration_structure_properties.pNext;

                    if (CheckExtensionSupport(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, availableExtensions))
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                        enabledExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
                        raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                        *features_chain = &raytracing_features;
                        features_chain = &raytracing_features.pNext;
                        raytracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                        *properties_chain = &raytracing_properties;
                        properties_chain = &raytracing_properties.pNext;
                    }

                    if (CheckExtensionSupport(VK_KHR_RAY_QUERY_EXTENSION_NAME, availableExtensions))
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
                        raytracing_query_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                        *features_chain = &raytracing_query_features;
                        features_chain = &raytracing_query_features.pNext;
                    }
                }

                if (CheckExtensionSupport(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, availableExtensions))
                {
                    enabledExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
                    fragment_shading_rate_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
                    *features_chain = &fragment_shading_rate_features;
                    features_chain = &fragment_shading_rate_features.pNext;
                    fragment_shading_rate_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
                    *properties_chain = &fragment_shading_rate_properties;
                    properties_chain = &fragment_shading_rate_properties.pNext;
                }

                if (CheckExtensionSupport(VK_NV_MESH_SHADER_EXTENSION_NAME, availableExtensions))
                {
                    enabledExtensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
                    mesh_shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
                    *features_chain = &mesh_shader_features;
                    features_chain = &mesh_shader_features.pNext;
                    mesh_shader_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
                    *properties_chain = &mesh_shader_properties;
                    properties_chain = &mesh_shader_properties.pNext;
                }

                vkGetPhysicalDeviceProperties2(candidatePhysicalDevice, &properties2);

                bool discrete = properties2.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                if (discrete || physicalDevice == VK_NULL_HANDLE)
                {
                    physicalDevice = candidatePhysicalDevice;
                    if (discrete)
                    {
                        break; // if this is discrete GPU, look no further (prioritize discrete GPU)
                    }
                }
            }

            if (physicalDevice == VK_NULL_HANDLE)
            {
                LogError("Failed to find a suitable GPU!");
                return;
            }

            assert(properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE);

            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

            assert(features2.features.imageCubeArray == VK_TRUE);
            assert(features2.features.independentBlend == VK_TRUE);
            assert(features2.features.geometryShader == VK_TRUE);
            assert(features2.features.samplerAnisotropy == VK_TRUE);
            assert(features2.features.shaderClipDistance == VK_TRUE);
            assert(features2.features.textureCompressionBC == VK_TRUE);
            assert(features2.features.occlusionQueryPrecise == VK_TRUE);
            if (features2.features.tessellationShader == VK_TRUE)
            {
                features.tessellation = true;
            }
            if (features2.features.shaderStorageImageExtendedFormats == VK_TRUE)
            {
                //capabilities |= GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_COMMON;
            }
            if (features2.features.multiViewport == VK_TRUE)
            {
                //capabilities |= GRAPHICSDEVICE_CAPABILITY_RENDERTARGET_AND_VIEWPORT_ARRAYINDEX_WITHOUT_GS;
            }

            if (raytracing_features.rayTracingPipeline == VK_TRUE &&
                raytracing_query_features.rayQuery == VK_TRUE &&
                acceleration_structure_features.accelerationStructure == VK_TRUE &&
                features_1_2.bufferDeviceAddress == VK_TRUE
                )
            {
                features.rayTracing = true;
                //SHADER_IDENTIFIER_SIZE = raytracing_properties.shaderGroupHandleSize;
            }

            if (mesh_shader_features.meshShader == VK_TRUE && mesh_shader_features.taskShader == VK_TRUE)
            {
                features.meshShader = true;
            }

            if (fragment_shading_rate_features.pipelineFragmentShadingRate == VK_TRUE)
            {
                features.variableRateShading = true;
            }

            if (fragment_shading_rate_features.attachmentFragmentShadingRate == VK_TRUE)
            {
                features.variableRateShadingTier2 = true;
                //VARIABLE_RATE_SHADING_TILE_SIZE = std::min(fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.width, fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.height);
            }

            assert(features_1_2.descriptorIndexing == VK_TRUE);

            //VkFormatProperties formatProperties = {};
            //vkGetPhysicalDeviceFormatProperties(physicalDevice, _ConvertFormat(FORMAT_R11G11B10_FLOAT), &formatProperties);
            //if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            //{
            //    capabilities |= GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT;
            //}

            // Limits
            limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
            limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
            limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
            limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
            limits.maxTextureArraySize = properties2.properties.limits.maxImageArrayLayers;
            limits.minConstantBufferOffsetAlignment = properties2.properties.limits.minUniformBufferOffsetAlignment;
            limits.minStorageBufferOffsetAlignment = properties2.properties.limits.minStorageBufferOffsetAlignment;
            limits.maxDrawIndirectCount = properties2.properties.limits.maxDrawIndirectCount;

            shaderFormat = ShaderFormat::SPIRV;

            // Find queue families
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            physicalDeviceQueueFamilies.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, physicalDeviceQueueFamilies.data());

            // Query base queue families:
            int familyIndex = 0;
            for (const auto& queueFamily : physicalDeviceQueueFamilies)
            {
                if (graphicsFamily == VK_QUEUE_FAMILY_IGNORED &&
                    queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsFamily = familyIndex;
                }

                if (copyFamily == VK_QUEUE_FAMILY_IGNORED &&
                    queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    copyFamily = familyIndex;
                }

                if (computeFamily == VK_QUEUE_FAMILY_IGNORED &&
                    queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    computeFamily = familyIndex;
                }

                familyIndex++;
            }

            // Now try to find dedicated compute and transfer queues:
            familyIndex = 0;
            for (const auto& queueFamily : physicalDeviceQueueFamilies)
            {
                if (queueFamily.queueCount > 0 &&
                    queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    ) {
                    copyFamily = familyIndex;
                }

                if (queueFamily.queueCount > 0 &&
                    queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    ) {
                    computeFamily = familyIndex;
                }

                familyIndex++;
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, copyFamily, computeFamily };

            float queuePriority = 1.0f;
            for (int queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
                queueFamilies.push_back(queueFamily);
            }

            VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
            createInfo.pNext = &features2;
            createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();
            createInfo.pEnabledFeatures = nullptr;

            VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

            volkLoadDevice(device);

            vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
            vkGetDeviceQueue(device, computeFamily, 0, &computeQueue);
            vkGetDeviceQueue(device, copyFamily, 0, &copyQueue);

            LogDebug("Vendor : %s", GetVendorName(properties2.properties.vendorID));
            LogDebug("Name   : %s", properties2.properties.deviceName);
            LogDebug("Type   : %s", kDeviceTypes[properties2.properties.deviceType]);
            LogDebug("API    : %d.%d.%d",
                VK_VERSION_MAJOR(properties2.properties.apiVersion),
                VK_VERSION_MINOR(properties2.properties.apiVersion),
                VK_VERSION_PATCH(properties2.properties.apiVersion)
            );
            LogDebug("Driver : %d.%d.%d",
                VK_VERSION_MAJOR(properties2.properties.driverVersion),
                VK_VERSION_MINOR(properties2.properties.driverVersion),
                VK_VERSION_PATCH(properties2.properties.driverVersion)
            );
            LogDebug("Enabled %d Device Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LogDebug("	\t%s", createInfo.ppEnabledExtensionNames[i]);
            }
        }

        // Create memory allocator (VMA)
        {
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = physicalDevice;
            allocatorInfo.device = device;
            allocatorInfo.instance = instance;
            if (features_1_2.bufferDeviceAddress == VK_TRUE)
            {
                allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            VK_CHECK(vmaCreateAllocator(&allocatorInfo, &allocator));
        }

        // Queues
        {
            queues[(uint8_t)CommandQueue::Graphics].queue = graphicsQueue;
            queues[(uint8_t)CommandQueue::Compute].queue = computeQueue;

            VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
            timelineCreateInfo.pNext = nullptr;
            timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineCreateInfo.initialValue = 0;

            VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            createInfo.pNext = &timelineCreateInfo;
            createInfo.flags = 0;

            VK_CHECK(vkCreateSemaphore(device, &createInfo, nullptr, &queues[(uint8_t)CommandQueue::Graphics].semaphore));
            VK_CHECK(vkCreateSemaphore(device, &createInfo, nullptr, &queues[(uint8_t)CommandQueue::Compute].semaphore));
        }

        copyAllocator.Init(this);

        // Create frame resources:
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
            {
                VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
                VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frames[i].fence[queue]));
            }

            // Create resources for transition command buffer:
            {
                VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
                poolInfo.queueFamilyIndex = graphicsFamily;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

                VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].initCommandPool));

                VkCommandBufferAllocateInfo commandBufferInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
                commandBufferInfo.commandBufferCount = 1;
                commandBufferInfo.commandPool = frames[i].initCommandPool;
                commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &frames[i].initCommandBuffer));

                VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = nullptr; // Optional

                VK_CHECK(vkBeginCommandBuffer(frames[i].initCommandBuffer, &beginInfo));
            }
        }

        // Create default null descriptors:
        {
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = 4;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.flags = 0;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &nullBuffer, &nullBufferAllocation, nullptr));

            VkBufferViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            viewInfo.range = VK_WHOLE_SIZE;
            viewInfo.buffer = nullBuffer;
            VK_CHECK(vkCreateBufferView(device, &viewInfo, nullptr, &nullBufferView));
        }
        {
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

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage1D, &nullImageAllocation1D, nullptr));

            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            imageInfo.arrayLayers = 6;
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage2D, &nullImageAllocation2D, nullptr));

            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.flags = 0;
            imageInfo.arrayLayers = 1;
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage3D, &nullImageAllocation3D, nullptr));

            // Transitions:
            initLocker.lock();
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = imageInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = nullImage1D;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(
                    GetFrameResources().initCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
                barrier.image = nullImage2D;
                barrier.subresourceRange.layerCount = 6;
                vkCmdPipelineBarrier(
                    GetFrameResources().initCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
                barrier.image = nullImage3D;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(
                    GetFrameResources().initCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
            submit_inits = true;
            initLocker.unlock();

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

            VkSamplerCreateInfo samplerInfo { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
            VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &nullSampler));
        }

        //TIMESTAMP_FREQUENCY = uint64_t(1.0 / double(properties2.properties.limits.timestampPeriod) * 1000 * 1000 * 1000);

        // Dynamic PSO states:
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        if (features.variableRateShading)
        {
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
        }

        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = (uint32_t)psoDynamicStates.size();
        dynamicStateInfo.pDynamicStates = psoDynamicStates.data();
    }

    VulkanDevice::~VulkanDevice()
    {
        VK_CHECK(vkDeviceWaitIdle(device));

        for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
        {
            vkDestroySemaphore(device, queues[queue].semaphore, nullptr);
            for (uint32_t cmd = 0; cmd < kMaxCommandLists; ++cmd)
            {
                commandLists[cmd][queue].reset();
            }
        }

        // Frame resources
        for (auto& frame : frames)
        {
            for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
            {
                vkDestroyFence(device, frame.fence[queue], nullptr);
                
            }
            vkDestroyCommandPool(device, frame.initCommandPool, nullptr);
        }

        copyAllocator.Shutdown();

        // Null resources
        {
            vmaDestroyBuffer(allocator, nullBuffer, nullBufferAllocation);
            vkDestroyBufferView(device, nullBufferView, nullptr);
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

        if (allocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(allocator, &stats);

            if (stats.total.usedBytes > 0)
            {
                LogInfo("Total device memory leaked: {} bytes.", stats.total.usedBytes);
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

    void VulkanDevice::WaitIdle()
    {
        VK_CHECK(vkDeviceWaitIdle(device));
    }

    bool VulkanDevice::BeginFrame()
    {
        commandListCount.store(0);
        return true;
    }

    void VulkanDevice::EndFrame()
    {
        SubmitCommandLists();
    }

    ICommandList* VulkanDevice::BeginCommandList(CommandQueue queue)
    {
        uint8_t cmd = commandListCount.fetch_add(1);
        assert(cmd < kMaxCommandLists);
        commandListMeta[cmd].queue = queue;
        commandListMeta[cmd].waits.clear();

        if (GetCommandList(cmd) == nullptr)
        {
            commandLists[cmd][(uint8_t)queue] = std::make_unique<VulkanCommandList>(this, queue, cmd);
        }

        GetCommandList(cmd)->Reset(frameIndex);

        return GetCommandList(cmd);
    }

    void VulkanDevice::SubmitCommandLists()
    {
        initLocker.lock();

        // Submit current frame.
        {
            auto& frame = GetFrameResources();

            CommandQueue submitQueue = CommandQueue::Count;

            // Transitions:
            if (submit_inits)
            {
                VK_CHECK(vkEndCommandBuffer(frame.initCommandBuffer));
            }

            // Sync with copy queue
            uint64_t copySyncFence = copyAllocator.Flush();

            uint8_t cmd_last = commandListCount.load();
            commandListCount.store(0);
            for (uint8_t cmd = 0; cmd < cmd_last; ++cmd)
            {
                VK_CHECK(vkEndCommandBuffer(GetCommandList(cmd)->GetHandle()));

                const CommandListMetadata& meta = commandListMeta[cmd];
                if (submitQueue == CommandQueue::Count) // start first batch
                {
                    submitQueue = meta.queue;
                }

                if (copySyncFence > 0) // sync up with copyallocator before first submit
                {
                    queues[(uint8_t)submitQueue].submit_waitStages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
                    queues[(uint8_t)submitQueue].submit_waitSemaphores.push_back(copyAllocator.semaphore);
                    queues[(uint8_t)submitQueue].submit_waitValues.push_back(copySyncFence);
                    copySyncFence = 0;
                }

                if (submitQueue != meta.queue || !meta.waits.empty()) // new queue type or wait breaks submit batch
                {
                    // New batch signals its last cmd:
                    queues[(uint8_t)submitQueue].submit_signalSemaphores.push_back(queues[(uint8_t)submitQueue].semaphore);
                    queues[(uint8_t)submitQueue].submit_signalValues.push_back(kMaxFramesInFlight * kMaxCommandLists + (uint64_t)cmd);
                    queues[(uint8_t)submitQueue].Submit(VK_NULL_HANDLE);
                    submitQueue = meta.queue;

                    for (auto& wait : meta.waits)
                    {
                        // record wait for signal on a previous submit:
                        const CommandListMetadata& wait_meta = commandListMeta[wait];
                        queues[(uint8_t)submitQueue].submit_waitStages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
                        queues[(uint8_t)submitQueue].submit_waitSemaphores.push_back(queues[(uint8_t)wait_meta.queue].semaphore);
                        queues[(uint8_t)submitQueue].submit_waitValues.push_back(kMaxFramesInFlight * kMaxCommandLists + (uint64_t)wait);
                    }
                }

                if (submit_inits)
                {
                    queues[(uint8_t)submitQueue].submit_cmds.push_back(frame.initCommandBuffer);
                    submit_inits = false;
                }

#if TODO
                for (auto& swapchain : prev_swapchains[cmd])
                {
                    auto internal_state = to_internal(swapchain);

                    queues[submit_queue].submit_swapchains.push_back(internal_state->swapChain);
                    queues[submit_queue].submit_swapChainImageIndices.push_back(internal_state->swapChainImageIndex);
                    queues[submit_queue].submit_waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    queues[submit_queue].submit_waitSemaphores.push_back(internal_state->swapchainAcquireSemaphore);
                    queues[submit_queue].submit_waitValues.push_back(0); // not a timeline semaphore
                    queues[submit_queue].submit_signalSemaphores.push_back(internal_state->swapchainReleaseSemaphore);
                    queues[submit_queue].submit_signalValues.push_back(0); // not a timeline semaphore
                }

#endif // TODO
                queues[(uint8_t)submitQueue].submit_cmds.push_back(GetCommandList(cmd)->GetHandle());
            }

            // Final submits with fences
            for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
            {
                queues[queue].Submit(frame.fence[queue]);
            }
        }

        frameCount++;
        frameIndex = frameCount % kMaxFramesInFlight;

        // Begin next frame
        {
            auto& frame = GetFrameResources();

            // Initiate stalling CPU when GPU is not yet finished with next frame:
            if (frameCount >= kMaxFramesInFlight)
            {
                for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
                {
                    VK_CHECK(vkWaitForFences(device, 1, &frame.fence[queue], VK_TRUE, UINT64_MAX));
                    VK_CHECK(vkResetFences(device, 1, &frame.fence[queue]));
                }
            }

            ProcessDeletionQueue();

            // Restart transition command buffers:
            {
                VK_CHECK(vkResetCommandPool(device, frame.initCommandPool, 0));

                VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                VK_CHECK(vkBeginCommandBuffer(frame.initCommandBuffer, &beginInfo));
            }
        }

        submit_inits = false;
        initLocker.unlock();
    }

    void VulkanDevice::ProcessDeletionQueue()
    {
        destroyMutex.lock();
        while (!destroyedImages.empty())
        {
            if (destroyedImages.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = destroyedImages.front();
                destroyedImages.pop_front();
                vmaDestroyImage(allocator, item.first.first, item.first.second);
            }
            else
            {
                break;
            }
        }
        destroyMutex.unlock();
    }

    /* CopyAllocator */
    void VulkanDevice::CopyAllocator::Init(VulkanDevice* device_)
    {
        device = device_;

        VkSemaphoreTypeCreateInfo timelineSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
        timelineSemaphoreInfo.pNext = nullptr;
        timelineSemaphoreInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineSemaphoreInfo.initialValue = 0;

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &timelineSemaphoreInfo;
        semaphoreInfo.flags = 0;

        VK_CHECK(vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &semaphore));
    }

    void VulkanDevice::CopyAllocator::Shutdown()
    {
        VK_CHECK(vkQueueWaitIdle(device->copyQueue));
        for (auto& x : freeList)
        {
            vkDestroyCommandPool(device->device, x.commandPool, nullptr);
        }

        vkDestroySemaphore(device->device, semaphore, nullptr);
    }

    VulkanCopyContext VulkanDevice::CopyAllocator::Allocate(uint64_t size)
    {
        locker.lock();

        // create a new command list if there are no free ones:
        if (freeList.empty())
        {
            VulkanCopyContext context;

            VkCommandPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.queueFamilyIndex = device->copyFamily;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VK_CHECK(vkCreateCommandPool(device->device, &poolInfo, nullptr, &context.commandPool));

            VkCommandBufferAllocateInfo allocateInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocateInfo.commandPool = context.commandPool;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(device->device, &allocateInfo, &context.commandBuffer));

            freeList.push_back(context);
        }

        VulkanCopyContext context = freeList.back();
        if (context.uploadBuffer == nullptr ||
            context.uploadBuffer->GetDesc().size < size)
        {
            // Try to search for a staging buffer that can fit the request:
            for (size_t i = 0; i < freeList.size(); ++i)
            {
                if (freeList[i].uploadBuffer->GetDesc().size >= size)
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
        if (context.uploadBuffer == nullptr ||
            context.uploadBuffer->GetDesc().size < size)
        {
            //BufferDesc uploadBufferDesc;
            //uploadBufferDesc.size = wiMath::GetNextPowerOfTwo((uint32_t)staging_size);
            //uploadBufferDesc.usage = USAGE_UPLOAD;
            //bool upload_success = device->CreateBuffer(&uploaddesc, nullptr, &cmd.uploadbuffer);
            //assert(upload_success);
        }

        // begin command list in valid state:
        VK_CHECK(vkResetCommandPool(device->device, context.commandPool, 0));

        VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VK_CHECK(vkBeginCommandBuffer(context.commandBuffer, &beginInfo));

        return context;
    }

    void VulkanDevice::CopyAllocator::Submit(VulkanCopyContext context)
    {
        VK_CHECK(vkEndCommandBuffer(context.commandBuffer));

        // It was very slow in Vulkan to submit the copies immediately
        //	In Vulkan, the submit is not thread safe, so it had to be locked
        //	Instead, the submits are batched and performed in flush() function
        locker.lock();
        context.target = ++fenceValue;
        workList.push_back(context);
        submit_cmds.push_back(context.commandBuffer);
        submit_wait = std::max(submit_wait, context.target);
        locker.unlock();
    }

    uint64_t VulkanDevice::CopyAllocator::Flush()
    {
        locker.lock();
        if (!submit_cmds.empty())
        {
            VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
            timelineInfo.pNext = nullptr;
            timelineInfo.waitSemaphoreValueCount = 0;
            timelineInfo.pWaitSemaphoreValues = nullptr;
            timelineInfo.signalSemaphoreValueCount = 1;
            timelineInfo.pSignalSemaphoreValues = &submit_wait;

            VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.pNext = &timelineInfo;
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.commandBufferCount = (uint32_t)submit_cmds.size();
            submitInfo.pCommandBuffers = submit_cmds.data();
            submitInfo.pSignalSemaphores = &semaphore;
            submitInfo.signalSemaphoreCount = 1;

            VK_CHECK(vkQueueSubmit(device->copyQueue, 1, &submitInfo, VK_NULL_HANDLE));

            submit_cmds.clear();
        }

        // free up the finished command lists:
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

        uint64_t value = submit_wait;
        submit_wait = 0;
        locker.unlock();

        return value;
    }

    /* VulkanCommandList */
    VulkanCommandList::VulkanCommandList(VulkanDevice* device_, CommandQueue queue_, uint8_t index_)
        : device(device_)
        , queue(queue_)
        , index(index_)
    {
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            switch (queue)
            {
                case CommandQueue::Graphics:
                    poolInfo.queueFamilyIndex = device->graphicsFamily;
                    break;
                case CommandQueue::Compute:
                    poolInfo.queueFamilyIndex = device->computeFamily;
                    break;
                default:
                    assert(0); // queue type not handled
                    break;
            }
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VK_CHECK(vkCreateCommandPool(device->device, &poolInfo, nullptr, &commandPools[i]));

            VkCommandBufferAllocateInfo allocateInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocateInfo.commandPool = commandPools[i];
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(device->device, &allocateInfo, &commandBuffers[i]));
        }

        commandBuffer = commandBuffers[0];
    }

    VulkanCommandList::~VulkanCommandList()
    {
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            vkFreeCommandBuffers(device->device, commandPools[i], 1, &commandBuffers[i]);
            vkDestroyCommandPool(device->device, commandPools[i], nullptr);
        }
    }

    VkCommandBuffer VulkanCommandList::GetHandle() const
    {
        return commandBuffers[device->GetFrameIndex()];
    }

    void VulkanCommandList::Reset(uint32_t frameIndex)
    {
        VK_CHECK(vkResetCommandPool(device->device, commandPools[frameIndex], 0));

        VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(commandBuffers[frameIndex], &beginInfo));

        commandBuffer = commandBuffers[frameIndex];

        // Reset descriptor allocators
        //binders[frameIndex].Reset();

        if (queue == CommandQueue::Graphics)
        {
            VkRect2D scissors[kMaxViewportsAndScissors];
            for (uint32_t i = 0; i < kMaxViewportsAndScissors; ++i)
            {
                scissors[i].offset.x = 0;
                scissors[i].offset.y = 0;
                scissors[i].extent.width = 65535;
                scissors[i].extent.height = 65535;
            }
            vkCmdSetScissor(commandBuffer, 0, kMaxViewportsAndScissors, scissors);

            float blendConstants[] = { 0.0f, 0.0f, 0.0f, 0.0f, };
            vkCmdSetBlendConstants(commandBuffer, blendConstants);
        }
    }

    void VulkanCommandList::PushDebugGroup(const char* name)
    {
        if (!device->debugUtils)
            return;

        VkDebugUtilsLabelEXT utilsLabel;
        utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        utilsLabel.pNext = nullptr;
        utilsLabel.pLabelName = name;
        utilsLabel.color[0] = 0.0;
        utilsLabel.color[1] = 0.0;
        utilsLabel.color[2] = 0.0;
        utilsLabel.color[3] = 1.0;

        vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &utilsLabel);
    }

    void VulkanCommandList::PopDebugGroup()
    {
        if (!device->debugUtils)
            return;

        vkCmdEndDebugUtilsLabelEXT(commandBuffer);
    }

    void VulkanCommandList::InsertDebugMarker(const char* name)
    {
        if (!device->debugUtils)
            return;

        VkDebugUtilsLabelEXT utilsLabel;
        utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        utilsLabel.pNext = nullptr;
        utilsLabel.pLabelName = name;
        utilsLabel.color[0] = 0.0;
        utilsLabel.color[1] = 0.0;
        utilsLabel.color[2] = 0.0;
        utilsLabel.color[3] = 1.0;
        vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &utilsLabel);
    }
}

#endif /* defined(ALIMER_RHI_VULKAN) */
