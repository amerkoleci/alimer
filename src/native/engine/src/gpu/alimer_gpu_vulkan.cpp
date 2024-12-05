// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_GPU_VULKAN)
#include "alimer_gpu_internal.h"
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#if defined(_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#elif defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#else
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_visualid_t;

//#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan_xcb.h>

struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>
#endif

ALIMER_DISABLE_WARNINGS()
#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
//#include <spirv_reflect.h>
ALIMER_ENABLE_WARNINGS()

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

#include <inttypes.h>
#include <vector>
#include <deque>

#if defined(_DEBUG)
/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err < 0) \
		{ \
			alimerLogError(LogCategory_GPU,"Detected Vulkan error: %s", VkResultToString(err)); \
		} \
	} while (0)
#else
#define VK_CHECK(x) (void)(x)
#endif
#define VK_LOG_ERROR(result, message) alimerLogError(LogCategory_GPU,"Vulkan: %s, error: %s", message, VkResultToString(result));

    // Declare function pointers
    static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

#define VULKAN_GLOBAL_FUNCTION(name) static PFN_##name name = nullptr;
#define VULKAN_INSTANCE_FUNCTION(name) static PFN_##name name = nullptr;
#include "alimer_gpu_vulkan_funcs.h"

namespace
{
    inline const char* VkResultToString(VkResult result)
    {
        switch (result)
        {
#define STR(r)   \
	case VK_##r: \
		return #r
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

    template<typename MainT, typename NewT>
    inline void PnextChainPushFront(MainT* mainStruct, NewT* newStruct)
    {
        newStruct->pNext = mainStruct->pNext;
        mainStruct->pNext = newStruct;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        ALIMER_UNUSED(pUserData);

        const char* messageTypeStr = "General";

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            messageTypeStr = "Validation";
        else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            messageTypeStr = "Performance";

        // Log debug messge
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            alimerLogWarn(LogCategory_GPU, "Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            alimerLogError(LogCategory_GPU, "Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
#if defined(_DEBUG)
            ALIMER_DEBUG_BREAK();
#endif
        }

        return VK_FALSE;
    }

    bool ValidateLayers(const std::vector<const char*>& required, const std::vector<VkLayerProperties>& available)
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
                alimerLogWarn(LogCategory_GPU, "Validation Layer '%s' not found", layer);
                return false;
            }
        }

        return true;
    }

    static bool GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
    {
#if defined(_WIN32)
        //PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
        if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
        {
            return false;
        }

        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex) == VK_TRUE;
#elif defined(__ANDROID__)
        return true;
#elif defined(__APPLE__)
        return true;
#else
        return true;
#endif
    }
}

struct VK_State {
#if defined(_WIN32)
    HMODULE vk_module;
#else
    void* vk_module;
#endif

    ~VK_State()
    {
        if (vk_module)
        {
#if defined(_WIN32)
            FreeLibrary(vk_module);
#else
            dlclose(vk_module);
#endif
            vk_module = nullptr;
        }
    }

} vk_state;

struct VulkanPhysicalDeviceExtensions final
{
    // Core 1.3
    bool maintenance4;
    bool dynamicRendering;
    bool synchronization2;
    bool extendedDynamicState;
    bool extendedDynamicState2;
    bool pipelineCreationCacheControl;
    bool formatFeatureFlags2;

    // Core 1.4
    bool pushDescriptor;

    // Extensions
    bool swapchain;
    bool memoryBudget;
    bool AMD_device_coherent_memory;
    bool EXT_memory_priority;
    bool performanceQuery;
    bool hostQueryReset;
    bool deferredHostOperations;
    bool multiview;
    bool portabilitySubset;
    bool depthClipEnable;
    bool textureCompressionAstcHdr;
    bool shaderViewportIndexLayer;
    bool conservativeRasterization;

    bool externalMemory;
    bool externalSemaphore;
    bool externalFence;

    bool maintenance5;
    bool maintenance6;
    bool accelerationStructure;
    bool raytracingPipeline;
    bool rayQuery;
    bool fragmentShadingRate;
    bool meshShader;
    bool conditionalRendering;
    struct
    {
        bool queue;
        bool decode_queue;
        bool decode_h264;
        bool decode_h265;
        bool encode_queue;
        bool encode_h264;
        bool encode_h265;
    } video;
    bool win32_full_screen_exclusive;
};

struct VulkanQueueFamilyIndices final
{
    uint32_t queueFamilyCount = 0;

    uint32_t familyIndices[GPUQueueType_Count] = {};
    uint32_t queueIndices[GPUQueueType_Count] = {};
    uint32_t counts[GPUQueueType_Count] = {};

    uint32_t timestampValidBits = 0;

    std::vector<uint32_t> queueOffsets;
    std::vector<std::vector<float>> queuePriorities;

    VulkanQueueFamilyIndices()
    {
        for (auto& index : familyIndices)
        {
            index = VK_QUEUE_FAMILY_IGNORED;
        }
    }

    bool IsComplete()
    {
        return familyIndices[GPUQueueType_Graphics] != VK_QUEUE_FAMILY_IGNORED;
    }
};

static inline VulkanPhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    if (result != VK_SUCCESS)
        return {};

    std::vector<VkExtensionProperties> vk_extensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data());

    VulkanPhysicalDeviceExtensions extensions{};

    for (uint32_t i = 0; i < count; ++i)
    {
        // Core in 1.3
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_4_EXTENSION_NAME) == 0)
        {
            extensions.maintenance4 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
        {
            extensions.dynamicRendering = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
        {
            extensions.synchronization2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
        {
            extensions.extendedDynamicState = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0)
        {
            extensions.extendedDynamicState2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0)
        {
            extensions.pipelineCreationCacheControl = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) == 0)
        {
            extensions.formatFeatureFlags2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0)
        {
            extensions.pushDescriptor = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            extensions.swapchain = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
        {
            extensions.memoryBudget = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) == 0)
        {
            extensions.AMD_device_coherent_memory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
        {
            extensions.EXT_memory_priority = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0)
        {
            extensions.performanceQuery = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0)
        {
            extensions.hostQueryReset = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
        {
            extensions.deferredHostOperations = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0)
        {
            extensions.multiview = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, "VK_KHR_portability_subset") == 0) // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        {
            extensions.portabilitySubset = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME) == 0)
        {
            extensions.depthClipEnable = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME) == 0)
        {
            extensions.textureCompressionAstcHdr = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
        {
            extensions.shaderViewportIndexLayer = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) == 0)
        {
            extensions.conservativeRasterization = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_5_EXTENSION_NAME) == 0)
        {
            extensions.maintenance5 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_6_EXTENSION_NAME) == 0)
        {
            extensions.maintenance6 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
        {
            extensions.accelerationStructure = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
        {
            extensions.raytracingPipeline = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
        {
            extensions.rayQuery = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0)
        {
            extensions.fragmentShadingRate = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
        {
            extensions.meshShader = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME) == 0)
        {
            extensions.conditionalRendering = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_h264 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_h265 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_h264 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_h265 = true;
        }

#if defined(_WIN32)
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalMemory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalSemaphore = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalFence = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0)
        {
            extensions.win32_full_screen_exclusive = true;
        }
#else
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalMemory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalSemaphore = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalFence = true;
        }
#endif
    }

    VkPhysicalDeviceProperties gpuProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &gpuProps);

    // Core 1.4
    if (gpuProps.apiVersion >= VK_API_VERSION_1_4)
    {
        extensions.maintenance6 = true;
        extensions.pushDescriptor = true;
    }

    // Core 1.3
    if (gpuProps.apiVersion >= VK_API_VERSION_1_3)
    {
        extensions.maintenance4 = true;
        extensions.dynamicRendering = true;
        extensions.synchronization2 = true;
        extensions.extendedDynamicState = true;
        extensions.extendedDynamicState2 = true;
        extensions.pipelineCreationCacheControl = true;
        extensions.formatFeatureFlags2 = true;
    }

    return extensions;
}

static VulkanQueueFamilyIndices QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
    std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;

        if (supportsVideoQueue)
        {
            queueFamilies[i].pNext = &queueFamiliesVideo[i];
            queueFamiliesVideo[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
        }
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilies.data());

    VulkanQueueFamilyIndices indices;
    indices.queueFamilyCount = queueFamilyCount;
    indices.queueOffsets.resize(queueFamilyCount);
    indices.queuePriorities.resize(queueFamilyCount);

    const auto FindVacantQueue = [&](uint32_t& family, uint32_t& index,
        VkQueueFlags required, VkQueueFlags ignore_flags,
        float priority) -> bool
        {
            for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
            {
                if ((queueFamilies[familyIndex].queueFamilyProperties.queueFlags & ignore_flags) != 0)
                    continue;

                // A graphics queue candidate must support present for us to select it.
                if ((required & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    bool supported = GetPresentationSupport(physicalDevice, familyIndex);
                    if (!supported)
                        continue;
                }

                // A video decode queue candidate must support VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR or VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                if ((required & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) == 0 &&
                        (videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR) == 0)
                    {
                        continue;
                    }
                }

#if defined(RHI_VIDEO_ENCODE)
                // A video decode queue candidate must support VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR or VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                if ((required & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT) == 0 &&
                        (videoCodecOperations & VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_EXT) == 0)
                    {
                        continue;
                    }
                }
#endif

                if (queueFamilies[familyIndex].queueFamilyProperties.queueCount &&
                    (queueFamilies[familyIndex].queueFamilyProperties.queueFlags & required) == required)
                {
                    family = familyIndex;
                    queueFamilies[familyIndex].queueFamilyProperties.queueCount--;
                    index = indices.queueOffsets[familyIndex]++;
                    indices.queuePriorities[familyIndex].push_back(priority);
                    return true;
                }
            }

            return false;
        };

    if (!FindVacantQueue(
        indices.familyIndices[GPUQueueType_Graphics],
        indices.queueIndices[GPUQueueType_Graphics], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
    {
        alimerLogError(LogCategory_GPU, "Vulkan: Could not find suitable graphics queue.");
        return indices;
    }

    // XXX: This assumes timestamp valid bits is the same for all queue types.
    indices.timestampValidBits = queueFamilies[indices.familyIndices[GPUQueueType_Graphics]].queueFamilyProperties.timestampValidBits;

    // Prefer another graphics queue since we can do async graphics that way.
    // The compute queue is to be treated as high priority since we also do async graphics on it.
    if (!FindVacantQueue(indices.familyIndices[GPUQueueType_Compute], indices.queueIndices[GPUQueueType_Compute], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
        !FindVacantQueue(indices.familyIndices[GPUQueueType_Compute], indices.queueIndices[GPUQueueType_Compute], VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
    {
        // Fallback to the graphics queue if we must.
        indices.familyIndices[GPUQueueType_Compute] = indices.familyIndices[GPUQueueType_Graphics];
        indices.queueIndices[GPUQueueType_Compute] = indices.queueIndices[GPUQueueType_Graphics];
    }

    // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
    // If not, fallback to a dedicated compute queue.
    // Finally, fallback to same queue as compute.
    if (!FindVacantQueue(indices.familyIndices[GPUQueueType_Copy], indices.queueIndices[GPUQueueType_Copy], VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
        !FindVacantQueue(indices.familyIndices[GPUQueueType_Copy], indices.queueIndices[GPUQueueType_Copy], VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
    {
        indices.familyIndices[GPUQueueType_Copy] = indices.familyIndices[GPUQueueType_Compute];
        indices.queueIndices[GPUQueueType_Copy] = indices.queueIndices[GPUQueueType_Compute];
    }

    if (supportsVideoQueue)
    {
        if (!FindVacantQueue(indices.familyIndices[GPUQueueType_VideoDecode],
            indices.queueIndices[GPUQueueType_VideoDecode],
            VK_QUEUE_VIDEO_DECODE_BIT_KHR, 0, 0.5f))
        {
            indices.familyIndices[GPUQueueType_VideoDecode] = VK_QUEUE_FAMILY_IGNORED;
            indices.queueIndices[GPUQueueType_VideoDecode] = UINT32_MAX;
        }

#ifdef VK_ENABLE_BETA_EXTENSIONS
        //if ((flags & CONTEXT_CREATION_ENABLE_VIDEO_ENCODE_BIT) != 0)
        //{
        //    if (!find_vacant_queue(queue_info.family_indices[QUEUE_INDEX_VIDEO_ENCODE],
        //        queue_indices[QUEUE_INDEX_VIDEO_ENCODE],
        //        VK_QUEUE_VIDEO_ENCODE_BIT_KHR, 0, 0.5f))
        //    {
        //        queue_info.family_indices[QUEUE_INDEX_VIDEO_ENCODE] = VK_QUEUE_FAMILY_IGNORED;
        //        queue_indices[QUEUE_INDEX_VIDEO_ENCODE] = UINT32_MAX;
        //    }
        //}
#endif
    }

    return indices;
}

struct VulkanGPUInstance;
struct VulkanGPUAdapter;
struct VulkanQueue;
struct VulkanGPUDevice;

struct VulkanBuffer final : public GPUBuffer
{
    GPUBufferDesc desc;
    VulkanGPUDevice* device = nullptr;
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    uint64_t allocatedSize = 0;
    VkDeviceAddress deviceAddress = 0;
    void* pMappedData = nullptr;
    void* sharedHandle = nullptr;

    ~VulkanBuffer() override;
    void SetLabel(const char* label) override;
    uint64_t GetSize() const override { return desc.size; }
    GPUDeviceAddress GetDeviceAddress() const override { return deviceAddress; }
};

struct VulkanGPUTexture final : public GPUTexture
{
    VulkanGPUDevice* device = nullptr;
    VkImage handle = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;

    ~VulkanGPUTexture() override;
};

struct VulkanCommandBuffer final : public GPUCommandBuffer
{
    VulkanQueue* queue = nullptr;
    uint32_t index = 0;
    VkCommandPool commandPools[GPU_MAX_INFLIGHT_FRAMES] = {};
    VkCommandBuffer commandBuffers[GPU_MAX_INFLIGHT_FRAMES] = {};
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    uint32_t numBarriersToCommit = 0;
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;

    ~VulkanCommandBuffer() override;
    void Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc);
    VkCommandBuffer End() const;
};

struct VulkanQueue final : public GPUQueue
{
    VulkanGPUDevice* device = nullptr;
    GPUQueueType type = GPUQueueType_Count;
    VkQueue handle = VK_NULL_HANDLE;
    VkFence frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};
    std::mutex mutex;

    std::vector<VulkanCommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUQueueType GetType() const override { return type; }
    GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) override;
    void Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers) override;
    void Submit(VkFence fence);
};

struct VulkanUploadContext final
{
    VkCommandPool transferCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer transferCommandBuffer = VK_NULL_HANDLE;
    VkCommandPool transitionCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer transitionCommandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore semaphores[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE }; // graphics, compute, video
    VulkanBuffer* uploadBuffer = nullptr;
    void* uploadBufferData = nullptr;
    uint64_t uploadBufferSize = 0;

    inline bool IsValid() const { return transferCommandBuffer != VK_NULL_HANDLE; }
};

struct VulkanCopyAllocator final
{
    VulkanGPUDevice* device = nullptr;
    std::mutex locker;
    std::vector<VulkanUploadContext> freeList;

    void Init(VulkanGPUDevice* device);
    void Shutdown();
    VulkanUploadContext Allocate(uint64_t size);
    void Submit(VulkanUploadContext context);
};

struct VulkanGPUDevice final : public GPUDevice
{
    VulkanGPUAdapter* adapter = nullptr;
    VkDevice handle = VK_NULL_HANDLE;
    VulkanQueue queues[GPUQueueType_Count];
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VmaAllocator allocator = nullptr;
    VmaAllocator externalAllocator = nullptr;
    VulkanCopyAllocator copyAllocator;

    std::vector<VkDynamicState> psoDynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;

    // Deletion queue objects
    std::mutex destroyMutex;
    std::deque<std::pair<VmaAllocation, uint64_t>> destroyedAllocations;
    std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyedImages;
    std::deque<std::pair<VkImageView, uint64_t>> destroyedImageViews;
    std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> destroyedBuffers;
    //std::deque<std::pair<VkBufferView, uint64_t>> destroyedBufferViews;
    //std::deque<std::pair<VkDescriptorSetLayout, uint64_t>> destroyedDescriptorSetLayouts;
    //std::deque<std::pair<VkPipelineLayout, uint64_t>> destroyedPipelineLayouts;
    //std::deque<std::pair<VkPipeline, uint64_t>> destroyedPipelines;
    //std::deque<std::pair<VkQueryPool, uint64_t>> destroyedQueryPools;
    //std::deque<std::pair<VkSemaphore, uint64_t>> destroyedSemaphores;
    //std::deque<std::pair<VkSwapchainKHR, uint64_t>> destroyedSwapchains;
    //std::deque<std::pair<VkSurfaceKHR, uint64_t>> destroyedSurfaces;
    //std::deque<std::pair<std::pair<VkDescriptorPool, VkDescriptorSet>, uint64_t>> destroyedDescriptorSets;

#define VULKAN_DEVICE_FUNCTION(func) PFN_##func func;
#include "alimer_gpu_vulkan_funcs.h"

    ~VulkanGPUDevice() override;
    GPUQueue* GetQueue(GPUQueueType type) override;
    bool WaitIdle() override;
    uint64_t CommitFrame() override;
    void ProcessDeletionQueue(bool force);

    /* Resource creation */
    GPUBuffer* CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData) override;
    GPUTexture* CreateTexture(const GPUTextureDesc* desc, const void* pInitialData) override;

    void SetObjectName(VkObjectType type, uint64_t handle_, const char* label) const;
    void FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices) const;
    void FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices) const;
};

struct VulkanSurface final : public GPUSurface
{
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR handle = VK_NULL_HANDLE;

    ~VulkanSurface() override;
    void Configure(const GPUSurfaceConfiguration* config) override;
    void Unconfigure() override;
};

struct VulkanGPUAdapter final : public GPUAdapter
{
    VulkanGPUInstance* instance = nullptr;
    VkPhysicalDevice handle = nullptr;
    VulkanPhysicalDeviceExtensions extensions;
    VulkanQueueFamilyIndices queueFamilyIndices;
    VkPhysicalDeviceProperties properties;
    bool synchronization2;
    bool dynamicRendering;

    // Features
    VkPhysicalDeviceFeatures2 features2 = {};
    VkPhysicalDeviceVulkan11Features features11 = {};
    VkPhysicalDeviceVulkan12Features features12 = {};
    VkPhysicalDeviceVulkan13Features features13 = {};
    VkPhysicalDeviceVulkan14Features features14 = {};

    // Core 1.3
    VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
    VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};

    // Extensions
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures{};
    VkPhysicalDevicePerformanceQueryFeaturesKHR performanceQueryFeatures{};
    VkPhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures = {};
    VkPhysicalDeviceTextureCompressionASTCHDRFeatures astcHdrFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures{};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures{};

    // Properties
    VkPhysicalDeviceProperties2 properties2 = {};
    VkPhysicalDeviceVulkan11Properties properties11 = {};
    VkPhysicalDeviceVulkan12Properties properties12 = {};
    VkPhysicalDeviceVulkan13Properties properties13 = {};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = {};
    VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = {};
    VkPhysicalDeviceMultiviewProperties multiviewProperties = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
    VkPhysicalDeviceMemoryProperties2 memoryProperties2;

    GPUResult GetLimits(GPULimits* limits) const override;
    GPUDevice* CreateDevice() override;
};

struct VulkanGPUInstance final : public GPUInstance
{
    bool debugUtils = false;
    bool headless = false;
    bool xcbSurface = false;
    bool xlibSurface = false;
    bool waylandSurface = false;

    VkInstance handle = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

    ~VulkanGPUInstance() override;
    GPUSurface* CreateSurface(Window* window) override;
    GPUAdapter* RequestAdapter(const GPURequestAdapterOptions* options) override;
};

/* VulkanBuffer */
VulkanBuffer::~VulkanBuffer()
{
    const uint64_t frameCount = device->frameCount;

    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedBuffers.push_back(std::make_pair(std::make_pair(handle, allocation), frameCount));
        handle = nullptr;
    }
    else if (allocation != nullptr)
    {
        device->destroyedAllocations.push_back(std::make_pair(allocation, frameCount));
    }
    device->destroyMutex.unlock();

    handle = VK_NULL_HANDLE;
    allocation = nullptr;
}

void VulkanBuffer::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanCommandBuffer */
VulkanCommandBuffer::~VulkanCommandBuffer()
{
    for (uint32_t i = 0; i < GPU_MAX_INFLIGHT_FRAMES; ++i)
    {
        queue->device->vkDestroyCommandPool(queue->device->handle, commandPools[i], nullptr);
    }

    //vkDestroySemaphore(device->device, semaphore, nullptr);
}

void VulkanCommandBuffer::Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc)
{
    //GraphicsContext::Reset(frameIndex);
    //waits.clear();
    //hasPendingWaits.store(false);
    //currentPipeline.Reset();
    //currentPipelineLayout.Reset();
    //presentSwapChains.clear();
    memoryBarriers.clear();
    imageBarriers.clear();
    bufferBarriers.clear();

    VK_CHECK(queue->device->vkResetCommandPool(queue->device->handle, commandPools[frameIndex], 0));
    commandBuffer = commandBuffers[frameIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK(queue->device->vkBeginCommandBuffer(commandBuffer, &beginInfo));

#if TODO
    if (queue->type != GPUQueueType_Copy)
    {
        bindGroupsDirty = false;
        numBoundBindGroups = 0;
        for (uint32_t i = 0; i < kMaxBindGroups; ++i)
        {
            boundBindGroups[i].Reset();
            descriptorSets[i] = VK_NULL_HANDLE;
        }
    }
#endif
    if (queue->type == GPUQueueType_Graphics)
    {
        VkRect2D scissors[16];
        for (uint32_t i = 0; i < 16; ++i)
        {
            scissors[i].offset.x = 0;
            scissors[i].offset.y = 0;
            scissors[i].extent.width = 65535;
            scissors[i].extent.height = 65535;
        }
        queue->device->vkCmdSetScissor(commandBuffer, 0, 16, scissors);

        const float blendConstants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        queue->device->vkCmdSetBlendConstants(commandBuffer, blendConstants);
        queue->device->vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FRONT_AND_BACK, ~0u);

#if TODO
        if (device->physicalDeviceFeatures2.features.depthBounds == VK_TRUE)
        {
            vkCmdSetDepthBounds(commandBuffer, 0.0f, 1.0f);
        }

        if (device->QueryFeatureSupport(RHIFeature::VariableRateShading))
        {
            VkExtent2D fragmentSize = {};
            fragmentSize.width = 1;
            fragmentSize.height = 1;

            VkFragmentShadingRateCombinerOpKHR combiner[] = {
                VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
            };

            vkCmdSetFragmentShadingRateKHR(commandBuffer, &fragmentSize, combiner);
        }
#endif // TODO

    }

#if TODO
    hasLabel = !label.empty();
    if (hasLabel)
    {
        PushDebugGroup(label);
        hasLabel = true;
    }
#endif // TODO

}

VkCommandBuffer VulkanCommandBuffer::End() const
{
    //for (auto& swapChain : presentSwapChains)
    //{
    //    VulkanTexture* swapChainTexture = swapChain->backbufferTextures[swapChain->imageIndex].Get();
    //    TextureBarrier(swapChainTexture, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, 1, 0, 1);
    //}
    //CommitBarriers();
    //
    //if (hasLabel)
    //{
    //    PopDebugGroup();
    //}

    VK_CHECK(queue->device->vkEndCommandBuffer(commandBuffer));
    return commandBuffer;
}

/* VulkanQueue */
GPUCommandBuffer* VulkanQueue::AcquireCommandBuffer(const GPUCommandBufferDesc* desc)
{
    cmdBuffersLocker.lock();
    uint32_t index = cmdBuffersCount++;
    if (index >= commandBuffers.size())
    {
        VulkanCommandBuffer* commandBuffer = new VulkanCommandBuffer();
        commandBuffer->queue = this;
        commandBuffer->index = index;

        for (uint32_t i = 0; i < GPU_MAX_INFLIGHT_FRAMES; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device->adapter->queueFamilyIndices.familyIndices[type];
            VK_CHECK(device->vkCreateCommandPool(device->handle, &poolInfo, nullptr, &commandBuffer->commandPools[i]));

            VkCommandBufferAllocateInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferInfo.commandPool = commandBuffer->commandPools[i];
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferInfo.commandBufferCount = 1;
            VK_CHECK(device->vkAllocateCommandBuffers(device->handle, &commandBufferInfo, &commandBuffer->commandBuffers[i]));
        }

        commandBuffers.push_back(commandBuffer);
    }
    cmdBuffersLocker.unlock();
    commandBuffers[index]->Begin(device->frameIndex, desc);

    return commandBuffers[index];
}

void VulkanQueue::Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers)
{
    std::vector<VkSemaphoreSubmitInfo> submitWaitSemaphoreInfos;
    std::vector<VkSemaphoreSubmitInfo> submitSignalSemaphoreInfos;
    std::vector<VkCommandBufferSubmitInfo> submitCommandBufferInfos;

    for (uint32_t i = 0; i < numCommandBuffers; i++)
    {
        const VulkanCommandBuffer* commandBuffer = static_cast<const VulkanCommandBuffer*>(commandBuffers[i]);

        VkCommandBufferSubmitInfo& commandBufferSubmitInfo = submitCommandBufferInfos.emplace_back();
        commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.commandBuffer = commandBuffer->End();
    }

    VkFence fence = VK_NULL_HANDLE;
    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = (uint32_t)submitWaitSemaphoreInfos.size();
    submitInfo.pWaitSemaphoreInfos = submitWaitSemaphoreInfos.data();
    submitInfo.commandBufferInfoCount = (uint32_t)submitCommandBufferInfos.size();
    submitInfo.pCommandBufferInfos = submitCommandBufferInfos.data();
    submitInfo.signalSemaphoreInfoCount = (uint32_t)submitSignalSemaphoreInfos.size();
    submitInfo.pSignalSemaphoreInfos = submitSignalSemaphoreInfos.data();
    VK_CHECK(device->vkQueueSubmit2(handle, 1, &submitInfo, fence));
}

void VulkanQueue::Submit(VkFence fence)
{
    if (handle == VK_NULL_HANDLE)
        return;

    std::scoped_lock lock(mutex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VK_CHECK(device->vkQueueSubmit(handle, 1, &submitInfo, fence));

#if TODO
    if (device->adapter->synchronization2)
    {
        ALIMER_ASSERT(submitSignalSemaphores.size() == submitSignalSemaphoreInfos.size());
        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = (uint32_t)submitWaitSemaphoreInfos.size();
        submitInfo.pWaitSemaphoreInfos = submitWaitSemaphoreInfos.data();
        submitInfo.commandBufferInfoCount = (uint32_t)submitCommandBufferInfos.size();
        submitInfo.pCommandBufferInfos = submitCommandBufferInfos.data();
        submitInfo.signalSemaphoreInfoCount = (uint32_t)submitSignalSemaphoreInfos.size();
        submitInfo.pSignalSemaphoreInfos = submitSignalSemaphoreInfos.data();
        VK_CHECK(device->vkQueueSubmit2(queue, 1, &submitInfo, fence));

        if (!submitSwapchains.empty())
        {
            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = (uint32_t)submitSignalSemaphores.size();
            presentInfo.pWaitSemaphores = submitSignalSemaphores.data();
            presentInfo.swapchainCount = (uint32_t)submitSwapchains.size();
            presentInfo.pSwapchains = submitSwapchains.data();
            presentInfo.pImageIndices = submitSwapchainImageIndices.data();

            const VkResult result = vkQueuePresentKHR(queue, &presentInfo);
            if (result != VK_SUCCESS)
            {
                // Handle outdated error in present
                if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    for (auto& swapchain : swapchainUpdates)
                    {
                        device->UpdateSwapChain(swapchain.Get());
                    }
                }
                else
                {
                    ALIMER_UNREACHABLE();
                }
            }
        }
    }

    swapchainUpdates.clear();
    submitSwapchains.clear();
    submitSwapchainImageIndices.clear();
    submitSignalSemaphores.clear();
    // KHR_synchronization2
    submitWaitSemaphoreInfos.clear();
    submitSignalSemaphoreInfos.clear();
    submitCommandBufferInfos.clear();
#endif // TODO

}

/* VulkanCopyAllocator */
void VulkanCopyAllocator::Init(VulkanGPUDevice* device_)
{
    device = device_;
}

void VulkanCopyAllocator::Shutdown()
{
    device->vkQueueWaitIdle(device->queues[GPUQueueType_Copy].handle);
    for (auto& context : freeList)
    {
        device->vkDestroyCommandPool(device->handle, context.transferCommandPool, nullptr);
        device->vkDestroyCommandPool(device->handle, context.transitionCommandPool, nullptr);
        device->vkDestroySemaphore(device->handle, context.semaphores[0], nullptr);
        device->vkDestroySemaphore(device->handle, context.semaphores[1], nullptr);
        device->vkDestroySemaphore(device->handle, context.semaphores[2], nullptr);
        device->vkDestroyFence(device->handle, context.fence, nullptr);

        context.uploadBuffer->Release();
        context.uploadBuffer = nullptr;
        context.uploadBufferData = nullptr;
    }
}

VulkanUploadContext VulkanCopyAllocator::Allocate(uint64_t size)
{
    VulkanUploadContext context;

    locker.lock();
    // Try to search for a staging buffer that can fit the request:
    for (size_t i = 0; i < freeList.size(); ++i)
    {
        if (freeList[i].uploadBufferSize >= size)
        {
            if (device->vkGetFenceStatus(device->handle, freeList[i].fence) == VK_SUCCESS)
            {
                context = std::move(freeList[i]);
                std::swap(freeList[i], freeList.back());
                freeList.pop_back();
                break;
            }
        }
    }
    locker.unlock();

    // If no buffer was found that fits the data then create new one.
    if (!context.IsValid())
    {
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolCreateInfo.queueFamilyIndex = device->adapter->queueFamilyIndices.familyIndices[GPUQueueType_Copy];
        VK_CHECK(device->vkCreateCommandPool(device->handle, &poolCreateInfo, nullptr, &context.transferCommandPool));

        poolCreateInfo.queueFamilyIndex = device->adapter->queueFamilyIndices.familyIndices[GPUQueueType_Graphics];
        VK_CHECK(device->vkCreateCommandPool(device->handle, &poolCreateInfo, nullptr, &context.transitionCommandPool));

        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = context.transferCommandPool;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1u;
        VK_CHECK(device->vkAllocateCommandBuffers(device->handle, &commandBufferInfo, &context.transferCommandBuffer));

        commandBufferInfo.commandPool = context.transitionCommandPool;
        VK_CHECK(device->vkAllocateCommandBuffers(device->handle, &commandBufferInfo, &context.transitionCommandBuffer));

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CHECK(device->vkCreateFence(device->handle, &fenceInfo, nullptr, &context.fence));

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &context.semaphores[0]));
        VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &context.semaphores[1]));
        VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &context.semaphores[2]));

        context.uploadBufferSize = VmaNextPow2(size);
        context.uploadBufferSize = VMA_MAX(context.uploadBufferSize, uint64_t(65536));

        GPUBufferDesc uploadBufferDesc;
        uploadBufferDesc.label = "CopyAllocator::UploadBuffer";
        uploadBufferDesc.size = context.uploadBufferSize;
        uploadBufferDesc.memoryType = GPUMemoryType_Upload;

        if (context.uploadBuffer)
        {
            context.uploadBuffer->Release();
        }
        context.uploadBuffer = static_cast<VulkanBuffer*>(device->CreateBuffer(&uploadBufferDesc, nullptr));
        ALIMER_ASSERT(context.uploadBuffer != nullptr);
        context.uploadBufferData = context.uploadBuffer->pMappedData;
    }

    // Begin command list in valid state.
    VK_CHECK(device->vkResetCommandPool(device->handle, context.transferCommandPool, 0));
    VK_CHECK(device->vkResetCommandPool(device->handle, context.transitionCommandPool, 0));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    VK_CHECK(device->vkBeginCommandBuffer(context.transferCommandBuffer, &beginInfo));
    VK_CHECK(device->vkBeginCommandBuffer(context.transitionCommandBuffer, &beginInfo));
    VK_CHECK(device->vkResetFences(device->handle, 1, &context.fence));

    return context;
}

void VulkanCopyAllocator::Submit(VulkanUploadContext context)
{
    VK_CHECK(device->vkEndCommandBuffer(context.transferCommandBuffer));
    VK_CHECK(device->vkEndCommandBuffer(context.transitionCommandBuffer));

    VkSemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

    // Copy queue first
    {
        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.commandBuffer = context.transferCommandBuffer;

        VkSemaphoreSubmitInfo signalSemaphoreInfo = {};
        signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphoreInfo.semaphore = context.semaphores[0]; // Signal for graphics queue
        signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

        std::scoped_lock lock(device->queues[GPUQueueType_Copy].mutex);
        VK_CHECK(device->vkQueueSubmit2(device->queues[GPUQueueType_Copy].handle, 1, &submitInfo, VK_NULL_HANDLE));
    }

    // Graphics queue
    {
        waitSemaphoreInfo.semaphore = context.semaphores[0]; // Wait for copy queue
        waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.commandBuffer = context.transitionCommandBuffer;

        VkSemaphoreSubmitInfo signalSemaphoreInfos[2] = {};
        signalSemaphoreInfos[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphoreInfos[0].semaphore = context.semaphores[1]; // Signal for compute queue
        signalSemaphoreInfos[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // Signal for compute queue

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;

        //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
        //{
        //    signalSemaphoreInfos[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        //    signalSemaphoreInfos[1].semaphore = cmd.semaphores[2]; // signal for video decode queue
        //    signalSemaphoreInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // signal for video decode queue
        //    submitInfo.signalSemaphoreInfoCount = 2;
        //}
        //else
        {
            submitInfo.signalSemaphoreInfoCount = 1;
        }
        submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos;

        std::scoped_lock lock(device->queues[GPUQueueType_Graphics].mutex);
        VK_CHECK(device->vkQueueSubmit2(device->queues[GPUQueueType_Graphics].handle, 1, &submitInfo, VK_NULL_HANDLE));
    }

    //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
    //{
    //    waitSemaphoreInfo.semaphore = cmd.semaphores[2]; // wait for graphics queue
    //    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    //
    //    submitInfo.waitSemaphoreInfoCount = 1;
    //    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    //    submitInfo.commandBufferInfoCount = 0;
    //    submitInfo.pCommandBufferInfos = nullptr;
    //    submitInfo.signalSemaphoreInfoCount = 0;
    //    submitInfo.pSignalSemaphoreInfos = nullptr;
    //
    //    std::scoped_lock lock(device->queues[QUEUE_VIDEO_DECODE].locker);
    //    res = vkQueueSubmit2(device->queues[QUEUE_VIDEO_DECODE].queue, 1, &submitInfo, VK_NULL_HANDLE);
    //    assert(res == VK_SUCCESS);
    //}

    // This must be final submit in this function because it will also signal a fence for state tracking by CPU!
    {
        waitSemaphoreInfo.semaphore = context.semaphores[1]; // wait for graphics queue
        waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
        submitInfo.commandBufferInfoCount = 0;
        submitInfo.pCommandBufferInfos = nullptr;
        submitInfo.signalSemaphoreInfoCount = 0;
        submitInfo.pSignalSemaphoreInfos = nullptr;

        // Final submit also signals fence!
        std::scoped_lock lock(device->queues[GPUQueueType_Compute].mutex);
        VK_CHECK(device->vkQueueSubmit2(device->queues[GPUQueueType_Compute].handle, 1, &submitInfo, context.fence));
    }

    std::scoped_lock lock(locker);
    freeList.push_back(context);
}

/* VulkanGPUDevice */
VulkanGPUDevice::~VulkanGPUDevice()
{
    VK_CHECK(vkDeviceWaitIdle(handle));

    for (uint32_t queueIndex = 0; queueIndex < GPUQueueType_Count; ++queueIndex)
    {
        if (queues[queueIndex].handle == VK_NULL_HANDLE)
            continue;

        for (uint32_t frameIndex = 0; frameIndex < GPU_MAX_INFLIGHT_FRAMES; ++frameIndex)
        {
            vkDestroyFence(handle, queues[queueIndex].frameFences[frameIndex], nullptr);
        }

        // Destroy command buffers and pools
        for (size_t cmdBufferIndex = 0, count = queues[queueIndex].commandBuffers.size(); cmdBufferIndex < count; ++cmdBufferIndex)
        {
            delete queues[queueIndex].commandBuffers[cmdBufferIndex];
        }
        queues[queueIndex].commandBuffers.clear();
    }

    copyAllocator.Shutdown();

    // Destory pending objects.
    ProcessDeletionQueue(true);
    frameCount = 0;

    if (allocator != nullptr)
    {
#if defined(_DEBUG)
        VmaTotalStatistics stats;
        vmaCalculateStatistics(allocator, &stats);

        if (stats.total.statistics.allocationBytes > 0)
        {
            alimerLogWarn(LogCategory_GPU, "Total device memory leaked: %" PRId64 " bytes.", stats.total.statistics.allocationBytes);
        }
#endif

        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

    if (externalAllocator != nullptr)
    {
#if defined(_DEBUG)
        VmaTotalStatistics stats;
        vmaCalculateStatistics(externalAllocator, &stats);

        if (stats.total.statistics.allocationBytes > 0)
        {
            alimerLogWarn(LogCategory_GPU, "Total device external memory leaked: %" PRId64 " bytes.", stats.total.statistics.allocationBytes);
        }
#endif

        vmaDestroyAllocator(externalAllocator);
        externalAllocator = VK_NULL_HANDLE;
    }

    if (pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(handle, pipelineCache, nullptr);
        pipelineCache = VK_NULL_HANDLE;
    }

    if (handle != VK_NULL_HANDLE)
    {
        vkDestroyDevice(handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

GPUQueue* VulkanGPUDevice::GetQueue(GPUQueueType type)
{
    return &queues[type];
}

bool VulkanGPUDevice::WaitIdle()
{
    VkResult result = vkDeviceWaitIdle(handle);
    if (result != VK_SUCCESS)
        return false;

    ProcessDeletionQueue(true);
    return true;
}

uint64_t VulkanGPUDevice::CommitFrame()
{
    // Final submits with fences.
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        queues[i].Submit(queues[i].frameFences[frameIndex]);
        queues[i].cmdBuffersCount = 0;
    }

    // Begin new frame
    frameCount++;
    frameIndex = frameCount % GPU_MAX_INFLIGHT_FRAMES;

    // Initiate stalling CPU when GPU is not yet finished with next frame
    if (frameCount >= GPU_MAX_INFLIGHT_FRAMES)
    {
        for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
        {
            if (queues[i].handle == VK_NULL_HANDLE)
                continue;

            VK_CHECK(vkWaitForFences(handle, 1, &queues[i].frameFences[frameIndex], true, 0xFFFFFFFFFFFFFFFF));
            VK_CHECK(vkResetFences(handle, 1, &queues[i].frameFences[frameIndex]));
        }
    }

    ProcessDeletionQueue(false);

    return frameCount;
}

void VulkanGPUDevice::ProcessDeletionQueue(bool force)
{
    const auto Destroy = [&](auto&& queue, auto&& handler) {
        while (!queue.empty()) {
            if (force || (queue.front().second + GPU_MAX_INFLIGHT_FRAMES < frameCount))
            {
                auto item = queue.front();
                queue.pop_front();
                handler(item.first);
            }
            else
            {
                break;
            }
        }
        };

    destroyMutex.lock();
    Destroy(destroyedAllocations, [&](auto& item) { vmaFreeMemory(allocator, item); });
    Destroy(destroyedImages, [&](auto& item) { vmaDestroyImage(allocator, item.first, item.second); });
    Destroy(destroyedImageViews, [&](auto& item) { vkDestroyImageView(handle, item, nullptr); });
    Destroy(destroyedBuffers, [&](auto& item) { vmaDestroyBuffer(allocator, item.first, item.second); });
    //Destroy(destroyedBufferViews, [&](auto& item) { vkDestroyBufferView(handle, item, nullptr); });
    //Destroy(destroyedDescriptorSetLayouts, [&](auto& item) { vkDestroyDescriptorSetLayout(handle, item, nullptr); });
    //Destroy(destroyedPipelineLayouts, [&](auto& item) { vkDestroyPipelineLayout(handle, item, nullptr); });
    //Destroy(destroyedPipelines, [&](auto& item) { vkDestroyPipeline(handle, item, nullptr); });
    //Destroy(destroyedQueryPools, [&](auto& item) { vkDestroyQueryPool(handle, item, nullptr); });
    //Destroy(destroyedSemaphores, [&](auto& item) {vkDestroySemaphore(handle, item, nullptr); });
    //Destroy(destroyedSwapchains, [&](auto& item) { vkDestroySwapchainKHR(handle, item, nullptr); });
    //Destroy(destroyedSurfaces, [&](auto& item) { vkDestroySurfaceKHR(instance, item, nullptr); });
    //Destroy(destroyedDescriptorSets, [&](auto& item) { vkFreeDescriptorSets(handle, item.first, 1u, &item.second); });
    destroyMutex.unlock();
}

GPUBuffer* VulkanGPUDevice::CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData)
{
    VulkanBuffer* buffer = new VulkanBuffer();
    buffer->device = this;
    buffer->desc = *desc;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = desc->size;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    bool needBufferDeviceAddress = false;
    if (desc->usage & GPUBufferUsage_Vertex)
    {
        createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if (desc->usage & GPUBufferUsage_Index)
    {
        createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if (desc->usage & GPUBufferUsage_Constant)
    {
        createInfo.size = VmaAlignUp(createInfo.size, adapter->properties2.properties.limits.minUniformBufferOffsetAlignment);
        createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (desc->usage & GPUBufferUsage_ShaderRead)
    {
        // Read only ByteAddressBuffer is also storage buffer
        createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }

    if (desc->usage & GPUBufferUsage_ShaderWrite)
    {
        createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    if (desc->usage & GPUBufferUsage_Indirect)
    {
        createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if ((desc->usage & GPUBufferUsage_Predication)
        /* && QueryFeatureSupport(RHIFeature::Predication)*/
        )
    {
        createInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }

    if ((desc->usage & GPUBufferUsage_RayTracing)
        /* && QueryFeatureSupport(RHIFeature::RayTracing) */
        )
    {
        createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        createInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        needBufferDeviceAddress = true;
    }

    // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
    if (adapter->features12.bufferDeviceAddress == VK_TRUE && needBufferDeviceAddress)
    {
        createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    uint32_t sharingIndices[3];
    FillBufferSharingIndices(createInfo, sharingIndices);

    VmaAllocationCreateInfo memoryInfo = {};
    memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (desc->memoryType == GPUMemoryType_Readback)
    {
        memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    else if (desc->memoryType == GPUMemoryType_Upload)
    {
        createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VkBufferUsageFlags2CreateInfoKHR bufUsageFlags2 = {};
    if (adapter->extensions.maintenance5)
    {
        bufUsageFlags2.sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR;
        bufUsageFlags2.usage = createInfo.usage;
        createInfo.pNext = &bufUsageFlags2;
    }

    VmaAllocationInfo allocationInfo{};
    VkResult result = vmaCreateBuffer(allocator,
        &createInfo,
        &memoryInfo,
        &buffer->handle,
        &buffer->allocation,
        &allocationInfo);

    if (result != VK_SUCCESS)
    {
        delete buffer;
        VK_LOG_ERROR(result, "Failed to create buffer.");
        return nullptr;
    }

    if (desc->label)
    {
        buffer->SetLabel(desc->label);
    }

    buffer->allocatedSize = allocationInfo.size;

    if (memoryInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
    {
        buffer->pMappedData = allocationInfo.pMappedData;
    }

    if (createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = buffer->handle;
        buffer->deviceAddress = vkGetBufferDeviceAddress(handle, &info);
    }

    // Issue data copy on request
    if (pInitialData != nullptr)
    {
        VulkanUploadContext context;
        void* pMappedData = nullptr;
        if (desc->memoryType == GPUMemoryType_Upload)
        {
            pMappedData = buffer->pMappedData;
        }
        else
        {
            context = copyAllocator.Allocate(createInfo.size);
            pMappedData = context.uploadBufferData;
        }

        std::memcpy(pMappedData, pInitialData, desc->size);

        if (context.IsValid())
        {
            VkBufferCopy copyRegion = {};
            copyRegion.size = desc->size;
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;

            vkCmdCopyBuffer(
                context.transferCommandBuffer,
                context.uploadBuffer->handle,
                buffer->handle,
                1,
                &copyRegion
            );

            VkBufferMemoryBarrier2 barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = buffer->handle;
            barrier.size = VK_WHOLE_SIZE;

            if (desc->usage & GPUBufferUsage_Vertex)
            {
                barrier.dstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            }

            if (desc->usage & GPUBufferUsage_Index)
            {
                barrier.dstStageMask |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
            }

            if (desc->usage & GPUBufferUsage_Constant)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_UNIFORM_READ_BIT;
            }

            if (desc->usage & GPUBufferUsage_ShaderRead)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
            }

            if (desc->usage & GPUBufferUsage_ShaderWrite)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_WRITE_BIT;
            }

            if (desc->usage & GPUBufferUsage_Indirect)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            }

            if (desc->usage & GPUBufferUsage_Predication)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
            }

            if (desc->usage & GPUBufferUsage_RayTracing)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            }

            //if (CheckBitsAny(desc.usage, BufferUsage::VideoDecode))
            //{
            //    barrier.dstAccessMask |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
            //}

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.bufferMemoryBarrierCount = 1;
            dependencyInfo.pBufferMemoryBarriers = &barrier;

            vkCmdPipelineBarrier2(context.transitionCommandBuffer, &dependencyInfo);

            copyAllocator.Submit(context);
        }
    }

    return buffer;
}

GPUTexture* VulkanGPUDevice::CreateTexture(const GPUTextureDesc* descriptor, const void* pInitialData)
{
    return nullptr;
}

static void AddUniqueFamily(uint32_t* sharing_indices, uint32_t& count, uint32_t family)
{
    if (family == VK_QUEUE_FAMILY_IGNORED)
        return;

    for (uint32_t i = 0; i < count; i++)
    {
        if (sharing_indices[i] == family)
            return;
    }

    sharing_indices[count++] = family;
}

void VulkanGPUDevice::SetObjectName(VkObjectType type, uint64_t handle_, const char* label) const
{
    if (!adapter->instance->debugUtils)
        return;

    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = type;
    nameInfo.objectHandle = handle_;
    nameInfo.pObjectName = label;
    vkSetDebugUtilsObjectNameEXT(handle, &nameInfo);
}

void VulkanGPUDevice::FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices) const
{
    for (auto& i : adapter->queueFamilyIndices.familyIndices)
    {
        AddUniqueFamily(sharingIndices, info.queueFamilyIndexCount, i);
    }

    if (info.queueFamilyIndexCount > 1)
    {
        // For buffers, always just use CONCURRENT access modes,
        // so we don't have to deal with acquire/release barriers in async compute.
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;

        info.pQueueFamilyIndices = sharingIndices;
    }
    else
    {
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
    }
}

void VulkanGPUDevice::FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices) const
{
    for (auto& i : adapter->queueFamilyIndices.familyIndices)
    {
        AddUniqueFamily(sharingIndices, info.queueFamilyIndexCount, i);
    }

    if (info.queueFamilyIndexCount > 1)
    {
        // For buffers, always just use CONCURRENT access modes,
        // so we don't have to deal with acquire/release barriers in async compute.
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        info.pQueueFamilyIndices = sharingIndices;
    }
    else
    {
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
    }
}

/* VulkanSurface */
VulkanSurface::~VulkanSurface()
{
    if (handle != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

void VulkanSurface::Configure(const GPUSurfaceConfiguration* config)
{

}

void VulkanSurface::Unconfigure()
{

}

/* VulkanGPUAdapter */
GPUResult VulkanGPUAdapter::GetLimits(GPULimits* limits) const
{
    limits->maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
    limits->maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
    limits->maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
    limits->maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
    limits->maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
    limits->maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
    limits->maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
    limits->minConstantBufferOffsetAlignment = properties2.properties.limits.minUniformBufferOffsetAlignment;
    limits->minStorageBufferOffsetAlignment = properties2.properties.limits.minStorageBufferOffsetAlignment;
    limits->maxBufferSize = properties13.maxBufferSize;
    limits->maxColorAttachments = properties2.properties.limits.maxColorAttachments;

    /* Compute */
    limits->maxComputeWorkgroupStorageSize = properties2.properties.limits.maxComputeSharedMemorySize;
    limits->maxComputeInvocationsPerWorkgroup = properties2.properties.limits.maxComputeWorkGroupInvocations;

    limits->maxComputeWorkgroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0];
    limits->maxComputeWorkgroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1];
    limits->maxComputeWorkgroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2];

    limits->maxComputeWorkgroupsPerDimension = std::min({
        properties2.properties.limits.maxComputeWorkGroupCount[0],
        properties2.properties.limits.maxComputeWorkGroupCount[1],
        properties2.properties.limits.maxComputeWorkGroupCount[2],
        });

    return GPUResult_Success;
}

GPUDevice* VulkanGPUAdapter::CreateDevice()
{
    VulkanGPUDevice* device = new VulkanGPUDevice();
    device->adapter = this;

    std::vector<const char*> enabledDeviceExtensions;
    enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Core in 1.3
    if (properties.apiVersion < VK_API_VERSION_1_3)
    {
        if (extensions.maintenance4)
        {
            enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        }

        if (extensions.dynamicRendering)
        {
            enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }

        if (extensions.synchronization2)
        {
            enabledDeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        }

        if (extensions.extendedDynamicState)
        {
            enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        }

        if (extensions.extendedDynamicState2)
        {
            enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        }
    }

    if (extensions.memoryBudget)
    {
        enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }

    if (extensions.AMD_device_coherent_memory)
    {
        enabledDeviceExtensions.push_back(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
    }

    if (extensions.EXT_memory_priority)
    {
        enabledDeviceExtensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    }

    if (extensions.deferredHostOperations)
    {
        enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    }

    if (extensions.multiview)
    {
        enabledDeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    }

    if (extensions.portabilitySubset)
    {
        enabledDeviceExtensions.push_back("VK_KHR_portability_subset");
    }

    if (extensions.depthClipEnable)
    {
        enabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    }

    // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
    if (extensions.performanceQuery && extensions.hostQueryReset)
    {
        enabledDeviceExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        enabledDeviceExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    }

    if (extensions.textureCompressionAstcHdr)
    {
        enabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);
    }

    if (extensions.shaderViewportIndexLayer)
    {
        enabledDeviceExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    }

    if (extensions.externalMemory)
    {
#if defined(_WIN32)
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
#else
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
#endif
    }

    if (extensions.externalSemaphore)
    {
#if defined(_WIN32)
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
#else
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
#endif
    }

    if (extensions.externalFence)
    {
#if defined(_WIN32)
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else
        enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
#endif
    }

    if (extensions.accelerationStructure)
    {
        ALIMER_ASSERT(extensions.deferredHostOperations);

        // Required by VK_KHR_acceleration_structure
        enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

        if (extensions.raytracingPipeline)
        {
            // Required by VK_KHR_pipeline_library
            enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
        }

        if (extensions.rayQuery)
        {
            enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        }
    }

    if (extensions.fragmentShadingRate)
    {
        enabledDeviceExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    }

    if (extensions.meshShader)
    {
        enabledDeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }

    if (extensions.conditionalRendering)
    {
        enabledDeviceExtensions.push_back(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    }

    if (extensions.video.queue)
    {
        enabledDeviceExtensions.push_back(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

        if (extensions.video.decode_queue)
        {
            enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

            if (extensions.video.decode_h264)
            {
                enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
            }

            if (extensions.video.decode_h265)
            {
                enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
            }
        }

#if defined(RHI_VIDEO_ENCODE)
        if (extensions.video.encode_queue)
        {
            enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

            if (physicalDeviceExtensions.video.encode_h264)
            {
                enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.video.encode_h265)
            {
                enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME);
            }
        }
#endif // RHI_VIDEO_ENCODE

    }

    if (!features2.features.textureCompressionBC &&
        !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
    {
        delete device;
        alimerLogError(LogCategory_GPU, "Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
        return nullptr;
    }

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (uint32_t familyIndex = 0; familyIndex < queueFamilyIndices.queueFamilyCount; familyIndex++)
    {
        if (queueFamilyIndices.queueOffsets[familyIndex] == 0)
            continue;

        VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        info.queueFamilyIndex = familyIndex;
        info.queueCount = queueFamilyIndices.queueOffsets[familyIndex];
        info.pQueuePriorities = queueFamilyIndices.queuePriorities[familyIndex].data();
        queueInfos.push_back(info);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &features2;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
    VkResult result = vkCreateDevice(handle, &createInfo, nullptr, &device->handle);

    if (result != VK_SUCCESS)
    {
        delete device;
        VK_LOG_ERROR(result, "Cannot create device");
        return nullptr;
    }

#define VULKAN_DEVICE_FUNCTION(func) device->func = (PFN_##func) vkGetDeviceProcAddr(device->handle, #func);
#include "alimer_gpu_vulkan_funcs.h"

    if (features13.synchronization2 == VK_FALSE &&
        synchronization2Features.synchronization2 == VK_TRUE)
    {
        device->vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(device->handle, "vkCmdPipelineBarrier2KHR");
        device->vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(device->handle, "vkCmdWriteTimestamp2KHR");
        device->vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device->handle, "vkQueueSubmit2KHR");
    }

    if (features13.synchronization2 == VK_FALSE &&
        synchronization2Features.synchronization2 == VK_TRUE)
    {
        device->vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(device->handle, "vkCmdPipelineBarrier2KHR");
        device->vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(device->handle, "vkCmdWriteTimestamp2KHR");
        device->vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device->handle, "vkQueueSubmit2KHR");
    }

    // Queues
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    for (uint32_t i = 0; i < GPUQueueType_Count; i++)
    {
        if (queueFamilyIndices.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
        {
            device->queues[i].device = device;
            device->queues[i].type = (GPUQueueType)i;

            device->vkGetDeviceQueue(device->handle, queueFamilyIndices.familyIndices[i], queueFamilyIndices.queueIndices[i], &device->queues[i].handle);
            queueFamilyIndices.counts[i] = queueFamilyIndices.queueOffsets[queueFamilyIndices.familyIndices[i]];

            for (uint32_t frameIndex = 0; frameIndex < GPU_MAX_INFLIGHT_FRAMES; ++frameIndex)
            {
                VK_CHECK(device->vkCreateFence(device->handle, &fenceInfo, nullptr, &device->queues[i].frameFences[frameIndex]));
            }
        }
        else
        {
            device->queues[i].handle = VK_NULL_HANDLE;
        }
    }

#ifdef _DEBUG
    alimerLogInfo(LogCategory_GPU, "Enabled %d Device Extensions:", createInfo.enabledExtensionCount);
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
    {
        alimerLogInfo(LogCategory_GPU, "	\t%s", createInfo.ppEnabledExtensionNames[i]);
    }
#endif

    // Create memory allocator
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = handle;
    allocatorInfo.device = device->handle;
    allocatorInfo.instance = instance->handle;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    // Core in 1.1
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;

    if (extensions.memoryBudget)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    }

    if (extensions.AMD_device_coherent_memory)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
    }

    if (features12.bufferDeviceAddress == VK_TRUE)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    if (extensions.EXT_memory_priority)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    }

    // Core in 1.3
    if (properties2.properties.apiVersion >= VK_API_VERSION_1_3
        || extensions.maintenance4)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
    }

    if (extensions.maintenance5)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
    }

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
    static VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

    result = vmaCreateAllocator(&allocatorInfo, &device->allocator);

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Cannot create allocator");
    }

    if (extensions.externalMemory)
    {
        std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
#if defined(_WIN32)
        externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
        externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif

        allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
        result = vmaCreateAllocator(&allocatorInfo, &device->externalAllocator);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
        }
    }

    // Init copy allocator
    device->copyAllocator.Init(device);

    // Dynamic PSO states
    device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    if (features2.features.depthBounds == VK_TRUE)
    {
        device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    }
    if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
    {
        device->psoDynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
    }
    //psoDynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);

    device->dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    device->dynamicStateInfo.dynamicStateCount = (uint32_t)device->psoDynamicStates.size();
    device->dynamicStateInfo.pDynamicStates = device->psoDynamicStates.data();

    return device;
}

/* VulkanGPUInstance */
VulkanGPUInstance::~VulkanGPUInstance()
{
    if (debugUtilsMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(handle, debugUtilsMessenger, nullptr);
        debugUtilsMessenger = VK_NULL_HANDLE;
    }

    if (handle != VK_NULL_HANDLE)
    {
        vkDestroyInstance(handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

GPUSurface* VulkanGPUInstance::CreateSurface(Window* window)
{
    VkResult result = VK_SUCCESS;
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

#if defined(_WIN32)
    //PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!vkCreateWin32SurfaceKHR)
    {
        alimerLogError(LogCategory_GPU, "%s extension is not enabled in the Vulkan instance.", VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        return nullptr;
    }

    HWND hwnd = static_cast<HWND>(alimerWindowGetNativeHandle(window));
    if (!IsWindow(hwnd))
    {
        alimerLogError(LogCategory_GPU, "Win32: Invalid vulkan hwnd handle");
        return nullptr;
    }

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
    surfaceCreateInfo.hwnd = hwnd;

    result = vkCreateWin32SurfaceKHR(handle, &surfaceCreateInfo, nullptr, &vk_surface);
#elif defined(__ANDROID__)
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.window = (ANativeWindow*)alimerWindowGetNativeHandle(window);

    result = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &vk_surface);
#elif defined(__APPLE__)
    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    //surfaceCreateInfo.pLayer = surface->GetMetalLayer();

    result = vkCreateMetalSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &vk_surface);
#else
#endif

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create surface");
        return nullptr;
    }

    if (vk_surface == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    VulkanSurface* surface = new VulkanSurface();
    surface->instance = handle;
    surface->handle = vk_surface;
    return surface;
}

GPUAdapter* VulkanGPUInstance::RequestAdapter(const GPURequestAdapterOptions* options)
{
    // Enumerate physical device and detect best one.
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr));
    if (physicalDeviceCount == 0)
    {
        //alimerLogDebug("Vulkan: Failed to find GPUs with Vulkan support");
        return nullptr;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, physicalDevices.data()));

    // The result adapter
    VulkanGPUAdapter* adapter = new VulkanGPUAdapter();
    adapter->instance = this;

    for (VkPhysicalDevice physicalDevice : physicalDevices)
    {
        // We require minimum 1.2
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        if (physicalDeviceProperties.apiVersion < VK_API_VERSION_1_2)
        {
            continue;
        }

        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

        if (physicalDeviceFeatures.robustBufferAccess != VK_TRUE
            || physicalDeviceFeatures.fullDrawIndexUint32 != VK_TRUE
            || physicalDeviceFeatures.depthClamp != VK_TRUE
            || physicalDeviceFeatures.depthBiasClamp != VK_TRUE
            || physicalDeviceFeatures.fragmentStoresAndAtomics != VK_TRUE
            || physicalDeviceFeatures.imageCubeArray != VK_TRUE
            || physicalDeviceFeatures.independentBlend != VK_TRUE
            || physicalDeviceFeatures.sampleRateShading != VK_TRUE
            || physicalDeviceFeatures.shaderClipDistance != VK_TRUE
            || physicalDeviceFeatures.occlusionQueryPrecise != VK_TRUE)
        {
            continue;
        }

        adapter->extensions = QueryPhysicalDeviceExtensions(physicalDevice);
        if (!adapter->extensions.swapchain)
        {
            continue;
        }

        adapter->queueFamilyIndices = QueryQueueFamilies(physicalDevice, adapter->extensions.video.queue);
        if (!adapter->queueFamilyIndices.IsComplete())
        {
            continue;
        }

        if (options && options->compatibleSurface != nullptr)
        {
            VulkanSurface* surface = static_cast<VulkanSurface*>(options->compatibleSurface);
            VkBool32 presentSupport = false;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                adapter->queueFamilyIndices.familyIndices[GPUQueueType_Graphics],
                surface->handle,
                &presentSupport
            );

            // Present family not found, we cannot create SwapChain
            if (result != VK_SUCCESS || presentSupport == VK_FALSE)
            {
                continue;
            }
        }

        // Features
        VkBaseOutStructure* featureChainCurrent{ nullptr };
        auto addToFeatureChain = [&featureChainCurrent](auto* next) {
            auto n = reinterpret_cast<VkBaseOutStructure*>(next);
            featureChainCurrent->pNext = n;
            featureChainCurrent = n;
            };

        adapter->features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        featureChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&adapter->features2);

        adapter->features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        addToFeatureChain(&adapter->features11);

        adapter->features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        addToFeatureChain(&adapter->features12);

        adapter->features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        addToFeatureChain(&adapter->features13);

        if (physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_4)
        {
            adapter->features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
            addToFeatureChain(&adapter->features14);
        }

        // Properties
        VkBaseOutStructure* propertiesChainCurrent{ nullptr };
        auto addToPropertiesChain = [&propertiesChainCurrent](auto* next) {
            auto n = reinterpret_cast<VkBaseOutStructure*>(next);
            propertiesChainCurrent->pNext = n;
            propertiesChainCurrent = n;
            };

        adapter->properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        propertiesChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&adapter->properties2);

        adapter->properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        addToPropertiesChain(&adapter->properties11);

        adapter->properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        addToPropertiesChain(&adapter->properties12);

        adapter->properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        addToPropertiesChain(&adapter->properties13);

        adapter->multiviewProperties = {};
        adapter->accelerationStructureProperties = {};
        adapter->rayTracingPipelineProperties = {};
        adapter->fragmentShadingRateProperties = {};
        adapter->meshShaderProperties = {};

        adapter->samplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
        addToPropertiesChain(&adapter->samplerFilterMinmaxProperties);

        adapter->depthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
        addToPropertiesChain(&adapter->depthStencilResolveProperties);

        // Core in 1.3
        if (physicalDeviceProperties.apiVersion < VK_API_VERSION_1_3)
        {
            if (adapter->extensions.maintenance4)
            {
                adapter->maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
                addToFeatureChain(&adapter->maintenance4Features);
            }

            if (adapter->extensions.dynamicRendering)
            {
                adapter->dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                addToFeatureChain(&adapter->dynamicRenderingFeatures);
            }

            if (adapter->extensions.synchronization2)
            {
                adapter->synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                addToFeatureChain(&adapter->synchronization2Features);
            }

            if (adapter->extensions.extendedDynamicState)
            {
                adapter->extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
                addToFeatureChain(&adapter->extendedDynamicStateFeatures);
            }

            if (adapter->extensions.extendedDynamicState2)
            {
                adapter->extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
                addToFeatureChain(&adapter->extendedDynamicState2Features);
            }
        }

        if (adapter->extensions.multiview)
        {
            adapter->multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
            addToPropertiesChain(&adapter->multiviewProperties);
        }

        if (adapter->extensions.depthClipEnable)
        {
            adapter->depthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
            addToFeatureChain(&adapter->depthClipEnableFeatures);
        }

        // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
        if (adapter->extensions.performanceQuery && adapter->extensions.hostQueryReset)
        {
            adapter->performanceQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR;
            addToFeatureChain(&adapter->performanceQueryFeatures);

            adapter->hostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
            addToFeatureChain(&adapter->hostQueryResetFeatures);
        }

        if (adapter->extensions.textureCompressionAstcHdr)
        {
            adapter->astcHdrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
            addToFeatureChain(&adapter->hostQueryResetFeatures);
        }

        if (adapter->extensions.accelerationStructure)
        {
            ALIMER_ASSERT(adapter->extensions.deferredHostOperations);

            adapter->accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            addToFeatureChain(&adapter->accelerationStructureFeatures);

            adapter->accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
            addToPropertiesChain(&adapter->accelerationStructureProperties);

            if (adapter->extensions.raytracingPipeline)
            {
                adapter->rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                addToFeatureChain(&adapter->rayTracingPipelineFeatures);

                adapter->rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                addToPropertiesChain(&adapter->rayTracingPipelineProperties);
            }

            if (adapter->extensions.rayQuery)
            {
                adapter->rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                addToFeatureChain(&adapter->rayQueryFeatures);
            }
        }

        if (adapter->extensions.fragmentShadingRate)
        {
            adapter->fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
            addToFeatureChain(&adapter->fragmentShadingRateFeatures);

            adapter->fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            addToPropertiesChain(&adapter->fragmentShadingRateProperties);
        }

        if (adapter->extensions.meshShader)
        {
            adapter->meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
            addToFeatureChain(&adapter->meshShaderFeatures);

            adapter->meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
            addToPropertiesChain(&adapter->meshShaderProperties);
        }

        if (adapter->extensions.conditionalRendering)
        {
            adapter->conditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
            addToFeatureChain(&adapter->conditionalRenderingFeatures);
        }

        vkGetPhysicalDeviceFeatures2(physicalDevice, &adapter->features2);
        vkGetPhysicalDeviceProperties2(physicalDevice, &adapter->properties2);

        bool priority = physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (options && options->powerPreference == GPUPowerPreference_LowPower)
        {
            priority = physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        }

        if (priority || adapter->handle == VK_NULL_HANDLE)
        {
            adapter->handle = physicalDevice;
            if (priority)
            {
                // If this is prioritized GPU type, look no further
                break;
            }
        }
    }

    if (adapter->handle == VK_NULL_HANDLE)
    {
        delete adapter;
        return nullptr;
    }

    adapter->synchronization2 = adapter->features13.synchronization2 == VK_TRUE || adapter->synchronization2Features.synchronization2 == VK_TRUE;
    adapter->dynamicRendering = adapter->features13.dynamicRendering == VK_TRUE || adapter->dynamicRenderingFeatures.dynamicRendering == VK_TRUE;

    ALIMER_ASSERT(adapter->synchronization2 == true);
    ALIMER_ASSERT(adapter->dynamicRendering == true);
    //ALIMER_ASSERT(adapter->properties2.properties.limits.maxPushConstantsSize >= kMaxPushConstantSize);

    vkGetPhysicalDeviceProperties(adapter->handle, &adapter->properties);
    adapter->memoryProperties2 = {};
    adapter->memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(adapter->handle, &adapter->memoryProperties2);
    return adapter;
}

bool Vulkan_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;
#if defined(_WIN32)
    vk_state.vk_module = LoadLibraryW(L"vulkan-1.dll");
    if (!vk_state.vk_module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vk_state.vk_module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
    vk_state.vk_module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    // Add support for using Vulkan and MoltenVK in a Framework. App store rules for iOS
    // strictly enforce no .dylib's. If they aren't found it just falls through
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("vulkan.framework/vulkan", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("MoltenVK.framework/MoltenVK", RTLD_NOW | RTLD_LOCAL);
    // modern versions of macOS don't search /usr/local/lib automatically contrary to what man dlopen says
    // Vulkan SDK uses this as the system-wide installation location, so we're going to fallback to this if all else fails
    if (!vk_state.vk_module && getenv("DYLD_FALLBACK_LIBRARY_PATH") == NULL)
        vk_state.vk_module = dlopen("/usr/local/lib/libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk_state.vk_module, "vkGetInstanceProcAddr");
#else
    vk_state.vk_module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module) {
        vk_state.vk_module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (!vk_state.vk_module) {
        return false;
    }
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk_state.vk_module, "vkGetInstanceProcAddr");
#endif

#define VULKAN_GLOBAL_FUNCTION(name) \
    name = (PFN_##name)vkGetInstanceProcAddr(VK_NULL_HANDLE, #name); \
    if (name == NULL) { \
        alimerLogWarn(LogCategory_GPU,"vkGetInstanceProcAddr(VK_NULL_HANDLE, \"" #name "\") failed"); \
        return false; \
    } 
#include "alimer_gpu_vulkan_funcs.h"

    // We require vulkan 1.2
    uint32_t apiVersion;
    if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS)
        return false;

    // Check if the Vulkan API version is sufficient.
    static constexpr uint32_t kMinimumVulkanVersion = VK_API_VERSION_1_2;
    if (apiVersion < kMinimumVulkanVersion)
    {
        alimerLogWarn(LogCategory_GPU, "The Vulkan API version supported on the system (%d.%d.%d) is too low, at least %d.%d.%d is required.",
            VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion),
            VK_API_VERSION_MAJOR(kMinimumVulkanVersion), VK_API_VERSION_MINOR(kMinimumVulkanVersion), VK_API_VERSION_PATCH(kMinimumVulkanVersion)
        );
        return false;
    }

    // Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it.
    if (VK_API_VERSION_VARIANT(apiVersion) != 0)
    {
        alimerLogWarn(LogCategory_GPU, "The Vulkan API supported on the system uses an unexpected variant: %d.", VK_API_VERSION_VARIANT(apiVersion));
        return false;
    }

    available = true;
    return true;
}

GPUInstance* Vulkan_CreateInstance(const GPUConfig* config)
{
    VulkanGPUInstance* instance = new VulkanGPUInstance();

    uint32_t instanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

    uint32_t extensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));

    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

    for (auto& availableExtension : availableInstanceExtensions)
    {
        if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            instance->debugUtils = true;
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
        {
            instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0)
        {
            instanceExtensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME) == 0)
        {
            instance->headless = true;
            //instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_xcb_surface") == 0)
        {
            instance->xcbSurface = true;
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_xlib_surface") == 0)
        {
            instance->xlibSurface = true;
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_wayland_surface") == 0)
        {
            instance->waylandSurface = true;
        }
    }

    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    // Enable surface extensions depending on os
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    // https://vulkan.lunarg.com/doc/view/1.3.280.0/windows/synchronization2_layer.html
    // https://vulkan.lunarg.com/doc/view/latest/windows/shader_object_layer.html
    for (auto& availableLayer : availableInstanceLayers)
    {
        if (strcmp(availableLayer.layerName, "VK_LAYER_KHRONOS_synchronization2") == 0)
        {
            instanceLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
            break;
        }
    }
#else
    if (instance->xcbSurface)
    {
        instanceExtensions.push_back("VK_KHR_xcb_surface");
    }
    else
    {
        ALIMER_ASSERT(instance->xlibSurface);
        instanceExtensions.push_back("VK_KHR_xlib_surface");
    }

    if (instance->waylandSurface)
    {
        instanceExtensions.push_back("VK_KHR_wayland_surface");
    }
#endif

    if (config->validationMode != GPUValidationMode_Disabled)
    {
        // Determine the optimal validation layers to enable that are necessary for useful debugging
        std::vector<const char*> optimalValidationLyers = { "VK_LAYER_KHRONOS_validation" };
        if (ValidateLayers(optimalValidationLyers, availableInstanceLayers))
        {
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

    }

    bool validationFeatures = false;
    if (config->validationMode == GPUValidationMode_GPU)
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

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Alimer";
    appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#if defined(__APPLE__)
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    createInfo.ppEnabledLayerNames = instanceLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

    if (config->validationMode != GPUValidationMode_Disabled && instance->debugUtils)
    {
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsCreateInfo.messageType =
            //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        if (config->validationMode == GPUValidationMode_Verbose)
        {
            debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        }

        debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
        createInfo.pNext = &debugUtilsCreateInfo;
    }

    VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
    if (config->validationMode == GPUValidationMode_GPU && validationFeatures)
    {
        static const VkValidationFeatureEnableEXT enable_features[2] = {
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        };
        validationFeaturesInfo.enabledValidationFeatureCount = 2;
        validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
        PnextChainPushFront(&createInfo, &validationFeaturesInfo);
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance->handle);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
        delete instance;
        return nullptr;
    }

#define VULKAN_INSTANCE_FUNCTION(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(instance->handle, #fn);
#include "alimer_gpu_vulkan_funcs.h"

    if (config->validationMode != GPUValidationMode_Disabled && instance->debugUtils)
    {
        result = vkCreateDebugUtilsMessengerEXT(instance->handle, &debugUtilsCreateInfo, nullptr, &instance->debugUtilsMessenger);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Could not create debug utils messenger");
        }
    }

#ifdef _DEBUG
    alimerLogInfo(LogCategory_GPU, "Created VkInstance with version: %d.%d.%d",
        VK_VERSION_MAJOR(appInfo.apiVersion),
        VK_VERSION_MINOR(appInfo.apiVersion),
        VK_VERSION_PATCH(appInfo.apiVersion)
    );

    if (createInfo.enabledLayerCount)
    {
        alimerLogInfo(LogCategory_GPU, "Enabled %d Validation Layers:", createInfo.enabledLayerCount);

        for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
        {
            alimerLogInfo(LogCategory_GPU, "	\t%s", createInfo.ppEnabledLayerNames[i]);
        }
    }

    alimerLogInfo(LogCategory_GPU, "Enabled %d Instance Extensions:", createInfo.enabledExtensionCount);
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
    {
        alimerLogInfo(LogCategory_GPU, "	\t%s", createInfo.ppEnabledExtensionNames[i]);
    }
#endif

    return instance;
}

#endif /* defined(ALIMER_GPU_VULKAN) */
