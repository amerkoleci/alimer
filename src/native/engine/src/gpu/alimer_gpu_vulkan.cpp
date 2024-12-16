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
    static_assert(sizeof(GPUViewport) == sizeof(VkViewport), "Viewport mismatch");
    static_assert(offsetof(GPUViewport, x) == offsetof(VkViewport, x), "Viewport layout mismatch");
    static_assert(offsetof(GPUViewport, y) == offsetof(VkViewport, y), "Viewport layout mismatch");
    static_assert(offsetof(GPUViewport, width) == offsetof(VkViewport, width), "Viewport layout mismatch");
    static_assert(offsetof(GPUViewport, height) == offsetof(VkViewport, height), "Viewport layout mismatch");
    static_assert(offsetof(GPUViewport, minDepth) == offsetof(VkViewport, minDepth), "Viewport layout mismatch");
    static_assert(offsetof(GPUViewport, maxDepth) == offsetof(VkViewport, maxDepth), "Viewport layout mismatch");

    static_assert(sizeof(GPUScissorRect) == sizeof(VkRect2D), "ScissorRect mismatch");
    static_assert(offsetof(GPUScissorRect, x) == offsetof(VkRect2D, offset.x), "GPUScissorRect layout mismatch");
    static_assert(offsetof(GPUScissorRect, y) == offsetof(VkRect2D, offset.y), "GPUScissorRect layout mismatch");
    static_assert(offsetof(GPUScissorRect, width) == offsetof(VkRect2D, extent.width), "GPUScissorRect layout mismatch");
    static_assert(offsetof(GPUScissorRect, height) == offsetof(VkRect2D, extent.height), "GPUScissorRect layout mismatch");

    static_assert(sizeof(GPUDispatchIndirectCommand) == sizeof(VkDispatchIndirectCommand), "DispatchIndirectCommand mismatch");
    static_assert(offsetof(GPUDispatchIndirectCommand, groupCountX) == offsetof(VkDispatchIndirectCommand, x), "DispatchIndirectCommand layout mismatch");
    static_assert(offsetof(GPUDispatchIndirectCommand, groupCountY) == offsetof(VkDispatchIndirectCommand, y), "DispatchIndirectCommand layout mismatch");
    static_assert(offsetof(GPUDispatchIndirectCommand, groupCountZ) == offsetof(VkDispatchIndirectCommand, z), "DispatchIndirectCommand layout mismatch");

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

    constexpr VkFormat ToVkVertexFormat(GPUVertexFormat format)
    {
        switch (format)
        {
            case GPUVertexFormat_UByte:               return VK_FORMAT_R8_UINT;
            case GPUVertexFormat_UByte2:              return VK_FORMAT_R8G8_UINT;
            case GPUVertexFormat_UByte4:              return VK_FORMAT_R8G8B8A8_UINT;
            case GPUVertexFormat_Byte:                return VK_FORMAT_R8_SINT;
            case GPUVertexFormat_Byte2:               return VK_FORMAT_R8G8_SINT;
            case GPUVertexFormat_Byte4:               return VK_FORMAT_R8G8B8A8_SINT;
            case GPUVertexFormat_UByteNormalized:     return VK_FORMAT_R8_UNORM;
            case GPUVertexFormat_UByte2Normalized:    return VK_FORMAT_R8G8_UNORM;
            case GPUVertexFormat_UByte4Normalized:    return VK_FORMAT_R8G8B8A8_UNORM;
            case GPUVertexFormat_ByteNormalized:      return VK_FORMAT_R8_SNORM;
            case GPUVertexFormat_Byte2Normalized:     return VK_FORMAT_R8G8_SNORM;
            case GPUVertexFormat_Byte4Normalized:     return VK_FORMAT_R8G8B8A8_SNORM;

            case GPUVertexFormat_UShort:              return VK_FORMAT_R16_UINT;
            case GPUVertexFormat_UShort2:             return VK_FORMAT_R16G16_UINT;
            case GPUVertexFormat_UShort4:             return VK_FORMAT_R16G16B16A16_UINT;
            case GPUVertexFormat_Short:               return VK_FORMAT_R16_SINT;
            case GPUVertexFormat_Short2:              return VK_FORMAT_R16G16_SINT;
            case GPUVertexFormat_Short4:              return VK_FORMAT_R16G16B16A16_SINT;
            case GPUVertexFormat_UShortNormalized:    return VK_FORMAT_R16_UNORM;
            case GPUVertexFormat_UShort2Normalized:   return VK_FORMAT_R16G16_UNORM;
            case GPUVertexFormat_UShort4Normalized:   return VK_FORMAT_R16G16B16A16_UNORM;
            case GPUVertexFormat_ShortNormalized:     return VK_FORMAT_R16_SNORM;
            case GPUVertexFormat_Short2Normalized:    return VK_FORMAT_R16G16_SNORM;
            case GPUVertexFormat_Short4Normalized:    return VK_FORMAT_R16G16B16A16_SNORM;

            case GPUVertexFormat_Half:                return VK_FORMAT_R16_SFLOAT;
            case GPUVertexFormat_Half2:               return VK_FORMAT_R16G16_SFLOAT;
            case GPUVertexFormat_Half4:               return VK_FORMAT_R16G16B16A16_SFLOAT;

            case GPUVertexFormat_Float:               return VK_FORMAT_R32_SFLOAT;
            case GPUVertexFormat_Float2:              return VK_FORMAT_R32G32_SFLOAT;
            case GPUVertexFormat_Float3:              return VK_FORMAT_R32G32B32_SFLOAT;
            case GPUVertexFormat_Float4:              return VK_FORMAT_R32G32B32A32_SFLOAT;

            case GPUVertexFormat_UInt:                return VK_FORMAT_R32_UINT;
            case GPUVertexFormat_UInt2:               return VK_FORMAT_R32G32_UINT;
            case GPUVertexFormat_UInt3:               return VK_FORMAT_R32G32B32_UINT;
            case GPUVertexFormat_UInt4:               return VK_FORMAT_R32G32B32A32_UINT;

            case GPUVertexFormat_Int:                 return VK_FORMAT_R32_SINT;
            case GPUVertexFormat_Int2:                return VK_FORMAT_R32G32_SINT;
            case GPUVertexFormat_Int3:                return VK_FORMAT_R32G32B32_SINT;
            case GPUVertexFormat_Int4:                return VK_FORMAT_R32G32B32A32_SINT;

            case GPUVertexFormat_Unorm10_10_10_2:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case GPUVertexFormat_Unorm8x4BGRA:   return VK_FORMAT_B8G8R8A8_UNORM;
                //case VertexFormat::RG11B10Float:            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                //case VertexFormat::RGB9E5Float:             return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

            default:
                ALIMER_UNREACHABLE();
        }
    }

    constexpr VkVertexInputRate ToVk(GPUVertexStepMode mode)
    {
        switch (mode)
        {
            case GPUVertexStepMode_Vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case GPUVertexStepMode_Instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;

            default:
                ALIMER_UNREACHABLE();
        }
    }

    constexpr VkImageAspectFlags GetImageAspectFlags(VkFormat format, GPUTextureAspect aspect)
    {
        switch (aspect)
        {
            case GPUTextureAspect_All:
                switch (format)
                {
                    case VK_FORMAT_D16_UNORM_S8_UINT:
                    case VK_FORMAT_D24_UNORM_S8_UINT:
                    case VK_FORMAT_D32_SFLOAT_S8_UINT:
                        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    case VK_FORMAT_D16_UNORM:
                    case VK_FORMAT_D32_SFLOAT:
                    case VK_FORMAT_X8_D24_UNORM_PACK32:
                        return VK_IMAGE_ASPECT_DEPTH_BIT;
                    case VK_FORMAT_S8_UINT:
                        return VK_IMAGE_ASPECT_STENCIL_BIT;
                    default:
                        return VK_IMAGE_ASPECT_COLOR_BIT;
                }
            case GPUTextureAspect_DepthOnly:
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            case GPUTextureAspect_StencilOnly:
                return VK_IMAGE_ASPECT_STENCIL_BIT;
            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    constexpr uint32_t MinImageCountForPresentMode(VkPresentModeKHR mode)
    {
        switch (mode)
        {
            case VK_PRESENT_MODE_FIFO_KHR:
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return 2;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return 3;
            default:
                return 2;
        }
    }

    [[nodiscard]] constexpr VkAttachmentLoadOp ToVk(GPULoadAction value)
    {
        switch (value)
        {
            case GPULoadAction_Discard:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            case GPULoadAction_Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case GPULoadAction_Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;

            default:
                ALIMER_UNREACHABLE();
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    [[nodiscard]] constexpr VkAttachmentStoreOp ToVk(GPUStoreAction value)
    {
        switch (value)
        {
            case GPUStoreAction_Discard:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            case GPUStoreAction_Store:
                return VK_ATTACHMENT_STORE_OP_STORE;

            default:
                ALIMER_UNREACHABLE();
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }

    [[nodiscard]] constexpr VkSampleCountFlagBits ToVkSampleCount(uint32_t sampleCount)
    {
        switch (sampleCount)
        {
            case 1:
                return VK_SAMPLE_COUNT_1_BIT;
            case 2:
                return VK_SAMPLE_COUNT_2_BIT;
            case 4:
                return VK_SAMPLE_COUNT_4_BIT;
            case 8:
                return VK_SAMPLE_COUNT_8_BIT;
            case 16:
                return VK_SAMPLE_COUNT_16_BIT;
            case 32:
                return VK_SAMPLE_COUNT_32_BIT;
            default:
                return VK_SAMPLE_COUNT_1_BIT;
        }
    }

    [[nodiscard]] constexpr VkCompareOp ToVk(GPUCompareFunction value)
    {
        switch (value)
        {
            case GPUCompareFunction_Never:        return VK_COMPARE_OP_NEVER;
            case GPUCompareFunction_Less:         return VK_COMPARE_OP_LESS;
            case GPUCompareFunction_Equal:        return VK_COMPARE_OP_EQUAL;
            case GPUCompareFunction_LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
            case GPUCompareFunction_Greater:      return VK_COMPARE_OP_GREATER;
            case GPUCompareFunction_NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
            case GPUCompareFunction_GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case GPUCompareFunction_Always:       return VK_COMPARE_OP_ALWAYS;
            default:
                ALIMER_UNREACHABLE();
                return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    [[nodiscard]] constexpr VkFilter ToVk(GPUSamplerMinMagFilter value)
    {
        switch (value)
        {
            default:
            case GPUSamplerMinMagFilter_Nearest:
                return VK_FILTER_NEAREST;

            case GPUSamplerMinMagFilter_Linear:
                return VK_FILTER_LINEAR;
        }
    }

    [[nodiscard]] constexpr VkSamplerMipmapMode ToVk(GPUSamplerMipFilter value)
    {
        switch (value)
        {
            default:
            case GPUSamplerMipFilter_Nearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;

            case GPUSamplerMipFilter_Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    [[nodiscard]] constexpr VkSamplerAddressMode ToVk(GPUSamplerAddressMode value, VkBool32 samplerMirrorClampToEdge)
    {
        switch (value)
        {
            default:
            case GPUSamplerAddressMode_ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            case GPUSamplerAddressMode_MirrorClampToEdge:
                if (samplerMirrorClampToEdge == VK_TRUE)
                    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

            case GPUSamplerAddressMode_Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;

            case GPUSamplerAddressMode_MirrorRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }
    }

    [[nodiscard]] constexpr VkShaderStageFlags ToVkShaderStageFlags(GPUShaderStage stage)
    {
        switch (stage)
        {
            case GPUShaderStage_Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
                //case GPUShaderStage_Hull:
                //    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                //case GPUShaderStage_Domain:
                //    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                //case GPUShaderStage_Geometry:
                //    return VK_SHADER_STAGE_GEOMETRY_BIT;
            case GPUShaderStage_Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case GPUShaderStage_Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;

            case GPUShaderStage_Amplification:
                return VK_SHADER_STAGE_TASK_BIT_EXT;
            case GPUShaderStage_Mesh:
                return VK_SHADER_STAGE_MESH_BIT_EXT;

            default:
                return 0;
        }
    }

    struct VkImageLayoutMapping final
    {
        VkImageLayout layout;
        VkPipelineStageFlags2 stageFlags;
        VkAccessFlags2 accessMask;

        VkImageLayoutMapping(VkImageLayout layout_, VkPipelineStageFlags2 stageFlags_, VkAccessFlags2 accessMask_)
            : layout(layout_)
            , stageFlags(stageFlags_)
            , accessMask(accessMask_)
        {
        }
    };

    VkImageLayoutMapping ConvertImageLayout(TextureLayout layout, bool depthOnlyFormat)
    {
        switch (layout)
        {
            case TextureLayout::Undefined:
                //case VK_IMAGE_LAYOUT_PREINITIALIZED:
                return { VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_NONE };

            case TextureLayout::CopySource:
                return { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_READ_BIT };

            case TextureLayout::CopyDest:
                return { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_WRITE_BIT };

            case TextureLayout::ResolveSource:
                return { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_READ_BIT };

            case TextureLayout::ResolveDest:
                return { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_WRITE_BIT };

            case TextureLayout::ShaderResource:
                //return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT };
                return { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT };

            case TextureLayout::UnorderedAccess:
                return { VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT };

            case TextureLayout::RenderTarget:
                return { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT };

            case TextureLayout::DepthWrite:
                return {
                    depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                };

            case TextureLayout::DepthRead:
                return {
                    depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                };

            case TextureLayout::Present:
                return { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,VK_ACCESS_2_MEMORY_READ_BIT };

            case TextureLayout::ShadingRateSurface:
                return { VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR };

#if TODO
                // TODO: Understand this
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                return {
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                };

                // TODO: Understand this
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                return {
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                };
#endif // TODO


            default:
                ALIMER_UNREACHABLE();
        }
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

static VulkanPhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
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

struct VulkanCommandBuffer;
struct VulkanQueue;
struct VulkanDevice;
struct VulkanSurface;
struct VulkanAdapter;
struct VulkanInstance;

struct VulkanBuffer final : public GPUBufferImpl
{
    VulkanDevice* device = nullptr;
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    uint64_t allocatedSize = 0;
    VkDeviceAddress deviceAddress = 0;
    void* pMappedData = nullptr;
    void* sharedHandle = nullptr;

    ~VulkanBuffer() override;
    void SetLabel(const char* label) override;
    GPUDeviceAddress GetDeviceAddress() const override { return deviceAddress; }
};

struct VulkanTexture final : public GPUTextureImpl
{
    VulkanDevice* device = nullptr;
    VkFormat vkFormat = VK_FORMAT_UNDEFINED;
    VkImage handle = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    uint32_t numSubResources = 0;
    mutable std::vector<TextureLayout> imageLayouts;
    mutable std::unordered_map<size_t, VkImageView> views;

    ~VulkanTexture() override;
    void SetLabel(const char* label) override;
    VkImageView GetView(uint32_t mipLevel) const;
};

struct VulkanSampler final : public GPUSamplerImpl
{
    VulkanDevice* device = nullptr;
    VkSampler handle = VK_NULL_HANDLE;

    ~VulkanSampler() override;
    void SetLabel(const char* label) override;
};

struct VulkanBindGroupLayout final : public GPUBindGroupLayoutImpl
{
    VulkanDevice* device = nullptr;
    VkDescriptorSetLayout handle = VK_NULL_HANDLE;

    ~VulkanBindGroupLayout() override;
    void SetLabel(const char* label) override;
};

struct VulkanPipelineLayout final : public GPUPipelineLayoutImpl
{
    VulkanDevice* device = nullptr;
    VkPipelineLayout handle = VK_NULL_HANDLE;

    ~VulkanPipelineLayout() override;
    void SetLabel(const char* label) override;
};

struct VulkanShaderModule final : public GPUShaderModuleImpl
{
    VulkanDevice* device = nullptr;
    VkShaderModule handle = VK_NULL_HANDLE;

    ~VulkanShaderModule() override;
    void SetLabel(const char* label) override;
};

struct VulkanComputePipeline final : public GPUComputePipelineImpl
{
    VulkanDevice* device = nullptr;
    VulkanPipelineLayout* layout = nullptr;
    VkPipeline handle = VK_NULL_HANDLE;

    ~VulkanComputePipeline() override;
    void SetLabel(const char* label) override;
};

struct VulkanRenderPipeline final : public GPURenderPipelineImpl
{
    VulkanDevice* device = nullptr;
    VulkanPipelineLayout* layout = nullptr;
    VkPipeline handle = VK_NULL_HANDLE;

    ~VulkanRenderPipeline() override;
    void SetLabel(const char* label) override;
};

struct VulkanComputePassEncoder final : public GPUComputePassEncoderImpl
{
    VulkanCommandBuffer* commandBuffer = nullptr;
    bool hasLabel = false;

    void Begin(const GPUComputePassDesc& desc);
    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void PrepareDispatch();
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset) override;
};

struct VulkanRenderPassEncoder final : public GPURenderPassEncoderImpl
{
    VulkanCommandBuffer* commandBuffer = nullptr;
    bool hasLabel = false;

    void Begin(const GPURenderPassDesc& desc);
    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void SetViewport(const GPUViewport* viewport) override;
    void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) override;
    void SetScissorRect(const GPUScissorRect* scissorRect) override;
    void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) override;
    void SetBlendColor(const float blendColor[4]) override;
    void SetStencilReference(uint32_t reference) override;

    void SetVertexBuffer(uint32_t slot, GPUBuffer buffer, uint64_t offset) override;
    void SetIndexBuffer(GPUBuffer buffer, GPUIndexFormat format, uint64_t offset) override;
    void SetPipeline(GPURenderPipeline pipeline) override;

    void PrepareDraw();
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
};

struct VulkanCommandBuffer final : public GPUCommandBufferImpl
{
    static constexpr uint32_t kMaxBarrierCount = 16;

    VulkanDevice* device = nullptr;
    VulkanQueue* queue = nullptr;
    uint32_t index = 0;
    bool hasLabel = false;
    bool encoderActive = false;
    VulkanComputePassEncoder* computePassEncoder = nullptr;
    VulkanRenderPassEncoder* renderPassEncoder = nullptr;

    VkCommandPool commandPools[GPU_MAX_INFLIGHT_FRAMES] = {};
    VkCommandBuffer commandBuffers[GPU_MAX_INFLIGHT_FRAMES] = {};
    VkCommandBuffer handle = VK_NULL_HANDLE;
    uint32_t numBarriersToCommit = 0;
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    std::vector<VulkanSurface*> presentSurfaces;

    ~VulkanCommandBuffer() override;
    void Clear();
    void Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc);
    VkCommandBuffer End();

    void TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, GPUTextureAspect aspect = GPUTextureAspect_All);
    void CommitBarriers();

    GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface surface, GPUTexture* surfaceTexture) override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    GPUComputePassEncoder BeginComputePass(const GPUComputePassDesc& desc) override;
    GPURenderPassEncoder BeginRenderPass(const GPURenderPassDesc& desc) override;
};

struct VulkanQueue final : public GPUQueueImpl
{
    VulkanDevice* device = nullptr;
    GPUQueueType queueType = GPUQueueType_Count;
    VkQueue handle = VK_NULL_HANDLE;
    VkFence frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};
    std::mutex mutex;

    std::vector<VulkanCommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUQueueType GetQueueType() const override { return queueType; }
    GPUCommandBuffer AcquireCommandBuffer(const GPUCommandBufferDesc* desc) override;
    void Submit(uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers) override;
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
    VulkanDevice* device = nullptr;
    std::mutex locker;
    std::vector<VulkanUploadContext> freeList;

    void Init(VulkanDevice* device);
    void Shutdown();
    VulkanUploadContext Allocate(uint64_t size);
    void Submit(VulkanUploadContext context);
};

struct VulkanDevice final : public GPUDeviceImpl
{
    VulkanAdapter* adapter = nullptr;
    VkDevice handle = VK_NULL_HANDLE;
    VulkanQueue queues[GPUQueueType_Count];
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VmaAllocator allocator = nullptr;
    VmaAllocator externalAllocator = nullptr;
    VulkanCopyAllocator copyAllocator;

    std::vector<VkDynamicState> psoDynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

    uint32_t maxFramesInFlight = 0;
    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;

    // Deletion queue objects
    std::mutex destroyMutex;
    std::deque<std::pair<VmaAllocation, uint64_t>> destroyedAllocations;
    std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyedImages;
    std::deque<std::pair<VkImageView, uint64_t>> destroyedImageViews;
    std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> destroyedBuffers;
    std::deque<std::pair<VkBufferView, uint64_t>> destroyedBufferViews;
    std::deque<std::pair<VkSampler, uint64_t>> destroyedSamplers;
    std::deque<std::pair<VkDescriptorSetLayout, uint64_t>> destroyedDescriptorSetLayouts;
    std::deque<std::pair<VkPipelineLayout, uint64_t>> destroyedPipelineLayouts;
    std::deque<std::pair<VkShaderModule, uint64_t>> destroyedShaderModules;
    std::deque<std::pair<VkPipeline, uint64_t>> destroyedPipelines;
    std::deque<std::pair<VkQueryPool, uint64_t>> destroyedQueryPools;
    std::deque<std::pair<VkSemaphore, uint64_t>> destroyedSemaphores;
    std::deque<std::pair<VkSwapchainKHR, uint64_t>> destroyedSwapchains;
    std::deque<std::pair<VkSurfaceKHR, uint64_t>> destroyedSurfaces;

#define VULKAN_DEVICE_FUNCTION(func) PFN_##func func;
#include "alimer_gpu_vulkan_funcs.h"

    ~VulkanDevice() override;
    void SetLabel(const char* label) override;
    bool HasFeature(GPUFeature feature) const override;
    GPUQueue GetQueue(GPUQueueType type) override;
    bool WaitIdle() override;
    uint64_t CommitFrame() override;
    void ProcessDeletionQueue(bool force);

    /* Resource creation */
    GPUBuffer CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) override;
    GPUTexture CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) override;
    GPUSampler CreateSampler(const GPUSamplerDesc& desc) override;
    GPUBindGroupLayout CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) override;
    GPUPipelineLayout CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) override;
    GPUShaderModule CreateShaderModule(const GPUShaderModuleDesc* desc) override;
    GPUComputePipeline CreateComputePipeline(const GPUComputePipelineDesc& desc) override;
    GPURenderPipeline CreateRenderPipeline(const GPURenderPipelineDesc& desc) override;

    void SetObjectName(VkObjectType type, uint64_t handle_, const char* label) const;
    void FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices) const;
    void FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices) const;
};

struct VulkanSurface final : public GPUSurfaceImpl
{
    VkInstance instance = VK_NULL_HANDLE;
    VulkanDevice* device = nullptr;
    VkSurfaceKHR handle = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent = {};
    uint32_t backBufferIndex = 0;
    std::vector<VulkanTexture*> backbufferTextures;
    std::mutex locker;
    size_t swapChainAcquireSemaphoreIndex = 0;
    std::vector<VkSemaphore> swapchainAcquireSemaphores;
    VkSemaphore swapchainReleaseSemaphore = VK_NULL_HANDLE;
    mutable std::vector<PixelFormat> supportedFormats;

    ~VulkanSurface() override;
    GPUResult GetCapabilities(GPUAdapter adapter, GPUSurfaceCapabilities* capabilities) const override;
    bool Configure(const GPUSurfaceConfig* config_) override;
    void Unconfigure() override;
};

struct VulkanAdapter final : public GPUAdapterImpl
{
    VulkanInstance* instance = nullptr;
    bool debugUtils = false;
    VkPhysicalDevice handle = nullptr;
    VulkanPhysicalDeviceExtensions extensions;
    VulkanQueueFamilyIndices queueFamilyIndices;
    //VkPhysicalDeviceProperties properties;
    bool synchronization2;
    bool dynamicRendering;
    std::string driverDescription;
    bool supportsDepth32Stencil8 = false;
    bool supportsDepth24Stencil8 = false;
    bool supportsStencil8 = false;

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
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProps = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
    VkPhysicalDeviceMemoryProperties2 memoryProperties2;

    GPUResult GetInfo(GPUAdapterInfo* info) const override;
    GPUResult GetLimits(GPULimits* limits) const override;
    bool HasFeature(GPUFeature feature) const override;
    bool IsDepthStencilFormatSupported(VkFormat format) const;
    VkFormat ToVkFormat(PixelFormat format) const;
    GPUDevice CreateDevice(const GPUDeviceDesc& desc) override;
};

struct VulkanInstance final : public GPUInstance
{
    bool debugUtils = false;
    bool headless = false;
    bool xcbSurface = false;
    bool xlibSurface = false;
    bool waylandSurface = false;

    VkInstance handle = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

    ~VulkanInstance() override;
    GPUSurface CreateSurface(Window* window) override;
    GPUAdapter RequestAdapter(const GPURequestAdapterOptions* options) override;
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

/* VulkanTexture */
VulkanTexture::~VulkanTexture()
{
    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();

    for (auto& it : views)
    {
        device->destroyedImageViews.push_back(std::make_pair(it.second, frameCount));
    }
    views.clear();

    if (allocation != VK_NULL_HANDLE)
    {
        if (handle)
        {
            device->destroyedImages.push_back(std::make_pair(std::make_pair(handle, allocation), frameCount));
        }
        //else if (stagingResource)
        //{
        //    device->destroyedBuffers.push_back(std::make_pair(std::make_pair(stagingResource, allocation), frameCount));
        //}
        else if (allocation)
        {
            device->destroyedAllocations.push_back(std::make_pair(allocation, frameCount));
        }
    }
    //stagingResource = VK_NULL_HANDLE;
    handle = VK_NULL_HANDLE;
    allocation = nullptr;

    device->destroyMutex.unlock();
}

void VulkanTexture::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(handle), label);
}

VkImageView VulkanTexture::GetView(uint32_t mipLevel) const
{
    size_t hash = 0;
    HashCombine(hash, mipLevel);

    auto it = views.find(hash);
    if (it == views.end())
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = handle;
        const bool isArray = desc.depthOrArrayLayers > 1;
        switch (desc.dimension)
        {
            case TextureDimension_1D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
                break;
            case TextureDimension_2D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
                break;
            case TextureDimension_3D:
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case TextureDimension_Cube:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
                break;
        }

        createInfo.format = vkFormat; // device->ToVkFormat(desc.format);
        createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        createInfo.subresourceRange.aspectMask = GetImageAspectFlags(createInfo.format, GPUTextureAspect_All);
        createInfo.subresourceRange.baseMipLevel = mipLevel;
        createInfo.subresourceRange.levelCount = desc.mipLevelCount;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = desc.depthOrArrayLayers;

        VkImageView newView = VK_NULL_HANDLE;
        const VkResult result = device->vkCreateImageView(
            device->handle,
            &createInfo,
            nullptr,
            &newView);
        if (result != VK_SUCCESS)
        {
            alimerLogError(LogCategory_GPU, "Vulkan: Failed to create ImageView, error: %s", VkResultToString(result));
            return VK_NULL_HANDLE;
        }

        views[hash] = newView;
        return newView;
    }

    return it->second;
}

/* VulkanSampler */
VulkanSampler::~VulkanSampler()
{
    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedSamplers.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanSampler::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanBindGroupLayout */
VulkanBindGroupLayout::~VulkanBindGroupLayout()
{
    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedDescriptorSetLayouts.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanBindGroupLayout::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanPipelineLayout */
VulkanPipelineLayout::~VulkanPipelineLayout()
{
    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedPipelineLayouts.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanPipelineLayout::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanShaderModule */
VulkanShaderModule::~VulkanShaderModule()
{
    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedShaderModules.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanShaderModule::SetLabel(const char* label)
{

}

/* VulkanComputePipeline */
VulkanComputePipeline::~VulkanComputePipeline()
{
    SAFE_RELEASE(layout);

    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedPipelines.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanComputePipeline::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanRenderPipeline */
VulkanRenderPipeline::~VulkanRenderPipeline()
{
    SAFE_RELEASE(layout);

    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedPipelines.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }
    device->destroyMutex.unlock();
}

void VulkanRenderPipeline::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanComputePassEncoder */
void VulkanComputePassEncoder::Begin(const GPUComputePassDesc& desc)
{
    if (desc.label)
    {
        PushDebugGroup(desc.label);
        hasLabel = true;
    }
}

void VulkanComputePassEncoder::EndEncoding()
{
    if (hasLabel)
    {
        PopDebugGroup();
    }

    commandBuffer->encoderActive = false;
    hasLabel = false;
}

void VulkanComputePassEncoder::PushDebugGroup(const char* groupLabel) const
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void VulkanComputePassEncoder::PopDebugGroup() const
{
    commandBuffer->PopDebugGroup();
}

void VulkanComputePassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

void VulkanComputePassEncoder::PrepareDispatch()
{

}

void VulkanComputePassEncoder::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    PrepareDispatch();

    commandBuffer->device->vkCmdDispatch(commandBuffer->handle, groupCountX, groupCountY, groupCountZ);
}

void VulkanComputePassEncoder::DispatchIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    PrepareDispatch();

    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(indirectBuffer);
    commandBuffer->device->vkCmdDispatchIndirect(commandBuffer->handle, backendBuffer->handle, indirectBufferOffset);
}

/* VulkanRenderPassEncoder */
void VulkanRenderPassEncoder::Begin(const GPURenderPassDesc& desc)
{
    if (desc.label)
    {
        PushDebugGroup(desc.label);
        hasLabel = true;
    }

    VkRect2D renderArea = {};
    renderArea.extent.width = commandBuffer->device->adapter->properties2.properties.limits.maxFramebufferWidth;
    renderArea.extent.height = commandBuffer->device->adapter->properties2.properties.limits.maxFramebufferHeight;
    uint32_t layerCount = commandBuffer->device->adapter->properties2.properties.limits.maxFramebufferLayers;

    uint32_t colorAttachmentCount = 0;
    VkRenderingAttachmentInfo colorAttachments[GPU_MAX_COLOR_ATTACHMENTS] = {};
    VkRenderingAttachmentInfo depthAttachment = {};
    VkRenderingAttachmentInfo stencilAttachment = {};

    for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        const GPURenderPassColorAttachment& attachment = desc.colorAttachments[i];
        if (!attachment.texture)
            continue;

        VulkanTexture* texture = static_cast<VulkanTexture*>(attachment.texture);

        renderArea.extent.width = std::min(renderArea.extent.width, std::max(texture->desc.width >> attachment.mipLevel, 1u));
        renderArea.extent.height = std::min(renderArea.extent.height, std::max(texture->desc.height >> attachment.mipLevel, 1u));
        layerCount = std::min(layerCount, texture->desc.depthOrArrayLayers);

        VkRenderingAttachmentInfo& attachmentInfo = colorAttachments[colorAttachmentCount++];
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.imageView = texture->GetView(attachment.mipLevel);
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.loadOp = ToVk(_ALIMER_DEF(attachment.loadAction, GPULoadAction_Load));
        attachmentInfo.storeOp = ToVk(_ALIMER_DEF(attachment.storeAction, GPUStoreAction_Store));
        attachmentInfo.clearValue.color.float32[0] = attachment.clearColor.r;
        attachmentInfo.clearValue.color.float32[1] = attachment.clearColor.g;
        attachmentInfo.clearValue.color.float32[2] = attachment.clearColor.b;
        attachmentInfo.clearValue.color.float32[3] = attachment.clearColor.a;

        // Barrier
        commandBuffer->TextureBarrier(texture, TextureLayout::RenderTarget, attachment.mipLevel, 1u, 0u, 1u);
    }

    const bool hasDepthOrStencil =
        desc.depthStencilAttachment != nullptr
        && desc.depthStencilAttachment->texture != nullptr;

    if (hasDepthOrStencil)
    {
        const GPURenderPassDepthStencilAttachment& attachment = *desc.depthStencilAttachment;

        VulkanTexture* texture = static_cast<VulkanTexture*>(attachment.texture);

        renderArea.extent.width = std::min(renderArea.extent.width, std::max(texture->desc.width >> attachment.mipLevel, 1u));
        renderArea.extent.height = std::min(renderArea.extent.height, std::max(texture->desc.height >> attachment.mipLevel, 1u));
        layerCount = std::min(layerCount, texture->desc.depthOrArrayLayers);

        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = texture->GetView(attachment.mipLevel);
        depthAttachment.imageLayout = attachment.depthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        depthAttachment.loadOp = ToVk(attachment.depthLoadAction);
        depthAttachment.storeOp = ToVk(attachment.depthStoreAction);
        depthAttachment.clearValue.depthStencil.depth = attachment.depthClearValue;

        // Barrier
        //commandBuffer->TextureBarrier(texture, depthAttachment.imageLayout, attachment.mipLevel, 1u, 0u, 1u);
    }
    commandBuffer->CommitBarriers();

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.flags = 0;
    renderingInfo.renderArea = renderArea;
    renderingInfo.layerCount = layerCount;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = colorAttachmentCount;
    renderingInfo.pColorAttachments = colorAttachmentCount > 0 ? colorAttachments : nullptr;
    renderingInfo.pDepthAttachment = hasDepthOrStencil ? &depthAttachment : nullptr;
    renderingInfo.pStencilAttachment = nullptr; // hasDepthOrStencil && !alimerPixelFormatIsDepthOnly(depthStencilFormat) ? &depthAttachment : nullptr;

    commandBuffer->device->vkCmdBeginRendering(commandBuffer->handle, &renderingInfo);

    // The viewport and scissor default to cover all of the attachments
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(renderArea.extent.height);
    viewport.width = static_cast<float>(renderArea.extent.width);
    viewport.height = -static_cast<float>(renderArea.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer->device->vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);

    VkRect2D scissorRect{};
    scissorRect.offset.x = 0;
    scissorRect.offset.y = 0;
    scissorRect.extent.width = renderArea.extent.width;
    scissorRect.extent.height = renderArea.extent.height;
    commandBuffer->device->vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissorRect);
}

void VulkanRenderPassEncoder::EndEncoding()
{
    commandBuffer->device->vkCmdEndRendering(commandBuffer->handle);

    if (hasLabel)
    {
        PopDebugGroup();
    }

    commandBuffer->encoderActive = false;
    hasLabel = false;
}

void VulkanRenderPassEncoder::PushDebugGroup(const char* groupLabel) const
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void VulkanRenderPassEncoder::PopDebugGroup() const
{
    commandBuffer->PopDebugGroup();
}

void VulkanRenderPassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

void VulkanRenderPassEncoder::SetViewport(const GPUViewport* viewport)
{
    // Flip viewport to match DirectX coordinate system
    VkViewport vkViewport{};
    vkViewport.x = viewport->x;
    vkViewport.y = viewport->height - viewport->y;
    vkViewport.width = viewport->width;
    vkViewport.height = -viewport->height;
    vkViewport.minDepth = viewport->minDepth;
    vkViewport.maxDepth = viewport->maxDepth;
    commandBuffer->device->vkCmdSetViewport(commandBuffer->handle, 0, 1, &vkViewport);
}

void VulkanRenderPassEncoder::SetViewports(uint32_t viewportCount, const GPUViewport* viewports)
{
    ALIMER_ASSERT(viewportCount < commandBuffer->device->adapter->properties2.properties.limits.maxViewports);

    VkViewport vkViewports[16] = {};
    for (uint32_t i = 0; i < viewportCount; ++i)
    {
        // Flip viewport to match DirectX coordinate system
        vkViewports[i].x = viewports[i].x;
        vkViewports[i].y = viewports[i].height - viewports[i].y;
        vkViewports[i].width = viewports[i].width;
        vkViewports[i].height = -viewports[i].height;
        vkViewports[i].minDepth = viewports[i].minDepth;
        vkViewports[i].maxDepth = viewports[i].maxDepth;
    }

    commandBuffer->device->vkCmdSetViewport(commandBuffer->handle, 0, viewportCount, vkViewports);
}

void VulkanRenderPassEncoder::SetScissorRect(const GPUScissorRect* scissorRect)
{
    commandBuffer->device->vkCmdSetScissor(commandBuffer->handle, 0, 1, (const VkRect2D*)scissorRect);
}

void VulkanRenderPassEncoder::SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects)
{
    ALIMER_ASSERT(scissorRects != nullptr);
    ALIMER_ASSERT(scissorCount < commandBuffer->device->adapter->properties2.properties.limits.maxViewports);

    commandBuffer->device->vkCmdSetScissor(commandBuffer->handle, 0, scissorCount, (const VkRect2D*)scissorRects);
}

void VulkanRenderPassEncoder::SetBlendColor(const float blendColor[4])
{
    commandBuffer->device->vkCmdSetBlendConstants(commandBuffer->handle, blendColor);
}

void VulkanRenderPassEncoder::SetStencilReference(uint32_t reference)
{
    commandBuffer->device->vkCmdSetStencilReference(commandBuffer->handle, VK_STENCIL_FRONT_AND_BACK, reference);
}

void VulkanRenderPassEncoder::SetVertexBuffer(uint32_t slot, GPUBuffer buffer, uint64_t offset)
{
    // TODO: Batch with 1 call
    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(buffer);

    commandBuffer->device->vkCmdBindVertexBuffers(commandBuffer->handle, slot, 1, &backendBuffer->handle, &offset);
}

void VulkanRenderPassEncoder::SetIndexBuffer(GPUBuffer buffer, GPUIndexFormat format, uint64_t offset)
{
    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(buffer);
    VkIndexType vkIndexType = (format == GPUIndexFormat_Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

    commandBuffer->device->vkCmdBindIndexBuffer(commandBuffer->handle, backendBuffer->handle, offset, vkIndexType);
}

void VulkanRenderPassEncoder::SetPipeline(GPURenderPipeline pipeline)
{
    VulkanRenderPipeline* backendPipeline = static_cast<VulkanRenderPipeline*>(pipeline);

    commandBuffer->device->vkCmdBindPipeline(commandBuffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, backendPipeline->handle);
}

void VulkanRenderPassEncoder::PrepareDraw()
{

}

void VulkanRenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    PrepareDraw();

    commandBuffer->device->vkCmdDraw(commandBuffer->handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

/* VulkanCommandBuffer */
VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Clear();

    for (uint32_t i = 0; i < queue->device->maxFramesInFlight; ++i)
    {
        queue->device->vkDestroyCommandPool(queue->device->handle, commandPools[i], nullptr);
    }

    //vkDestroySemaphore(device->device, semaphore, nullptr);

    delete computePassEncoder;
    delete renderPassEncoder;
}

void VulkanCommandBuffer::Clear()
{
    for (auto& surface : presentSurfaces)
    {
        surface->Release();
    }
    presentSurfaces.clear();
    memoryBarriers.clear();
    imageBarriers.clear();
    bufferBarriers.clear();
}

void VulkanCommandBuffer::Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc)
{
    //GraphicsContext::Reset(frameIndex);
    //waits.clear();
    //hasPendingWaits.store(false);
    //currentPipeline.Reset();
    //currentPipelineLayout.Reset();
    Clear();

    VK_CHECK(queue->device->vkResetCommandPool(queue->device->handle, commandPools[frameIndex], 0));
    handle = commandBuffers[frameIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK(queue->device->vkBeginCommandBuffer(handle, &beginInfo));

    if (desc && desc->label)
    {
        PushDebugGroup(desc->label);
        hasLabel = true;
    }

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

    if (queue->queueType == GPUQueueType_Graphics)
    {
        VkRect2D scissors[16];
        for (uint32_t i = 0; i < 16; ++i)
        {
            scissors[i].offset.x = 0;
            scissors[i].offset.y = 0;
            scissors[i].extent.width = 65535;
            scissors[i].extent.height = 65535;
        }
        queue->device->vkCmdSetScissor(handle, 0, 16, scissors);

        const float blendConstants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        queue->device->vkCmdSetBlendConstants(handle, blendConstants);
        queue->device->vkCmdSetStencilReference(handle, VK_STENCIL_FRONT_AND_BACK, ~0u);

#if TODO
        if (device->physicalDeviceFeatures2.features.depthBounds == VK_TRUE)
        {
            vkCmdSetDepthBounds(handle, 0.0f, 1.0f);
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

            vkCmdSetFragmentShadingRateKHR(handle, &fragmentSize, combiner);
        }
#endif // TODO
    }
}

VkCommandBuffer VulkanCommandBuffer::End()
{
    for (auto& surface : presentSurfaces)
    {
        VulkanTexture* swapChainTexture = surface->backbufferTextures[surface->backBufferIndex];
        TextureBarrier(swapChainTexture, TextureLayout::Present, 0, 1, 0, 1);
    }
    CommitBarriers();

    if (hasLabel)
    {
        PopDebugGroup();
    }

    VK_CHECK(queue->device->vkEndCommandBuffer(handle));
    return handle;
}

void VulkanCommandBuffer::TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, GPUTextureAspect aspect)
{
    const uint32_t mipLevelCount = texture->desc.mipLevelCount;
    const uint32_t subresource = CalculateSubresource(baseMiplevel, baseArrayLayer, mipLevelCount);
    TextureLayout currentLayout = texture->imageLayouts[subresource];
    if (currentLayout == newLayout)
        return;

    const bool depthOnlyFormat = alimerPixelFormatIsDepthOnly(texture->desc.format);

    VkImageSubresourceRange range{};
    range.aspectMask = GetImageAspectFlags(texture->vkFormat, aspect);
    range.baseMipLevel = baseMiplevel;
    range.levelCount = levelCount;
    range.baseArrayLayer = baseArrayLayer;
    range.layerCount = layerCount;

    if (device->adapter->synchronization2)
    {
        const VkImageLayoutMapping mappingBefore = ConvertImageLayout(currentLayout, depthOnlyFormat);
        const VkImageLayoutMapping mappingAfter = ConvertImageLayout(newLayout, depthOnlyFormat);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = mappingBefore.stageFlags;
        barrier.srcAccessMask = mappingBefore.accessMask;
        barrier.dstStageMask = mappingAfter.stageFlags;
        barrier.dstAccessMask = mappingAfter.accessMask;
        barrier.oldLayout = mappingBefore.layout;
        barrier.newLayout = mappingAfter.layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture->handle;
        barrier.subresourceRange = range;

        imageBarriers.push_back(barrier);
    }
    else
    {
    }

    if (numBarriersToCommit == kMaxBarrierCount)
        CommitBarriers();

    for (uint32_t arrayLayer = baseArrayLayer; arrayLayer < (baseArrayLayer + layerCount); arrayLayer++)
    {
        for (uint32_t mipLevel = baseMiplevel; mipLevel < (baseMiplevel + levelCount); mipLevel++)
        {
            const uint32_t iterSubresource = CalculateSubresource(mipLevel, arrayLayer, mipLevelCount);
            texture->imageLayouts[iterSubresource] = newLayout;
        }
    }
}

void VulkanCommandBuffer::CommitBarriers()
{
    if (!memoryBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
    {
        VkDependencyInfo dependencyInfo = {};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
        dependencyInfo.pMemoryBarriers = memoryBarriers.data();
        dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());
        dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
        dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
        dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
        device->vkCmdPipelineBarrier2(handle, &dependencyInfo);

        memoryBarriers.clear();
        imageBarriers.clear();
        bufferBarriers.clear();
    }

    numBarriersToCommit = 0;
}

GPUAcquireSurfaceResult VulkanCommandBuffer::AcquireSurfaceTexture(GPUSurface surface, GPUTexture* surfaceTexture)
{
    VulkanSurface* backendSurface = static_cast<VulkanSurface*>(surface);
    size_t swapChainAcquireSemaphoreIndex = backendSurface->swapChainAcquireSemaphoreIndex;

    backendSurface->locker.lock();
    VkResult result = queue->device->vkAcquireNextImageKHR(
        queue->device->handle,
        backendSurface->swapchain,
        UINT64_MAX,
        backendSurface->swapchainAcquireSemaphores[swapChainAcquireSemaphoreIndex],
        VK_NULL_HANDLE,
        &backendSurface->backBufferIndex
    );
    backendSurface->locker.unlock();

    if (result != VK_SUCCESS)
    {
        // Handle outdated error in acquire
        if (result == VK_SUBOPTIMAL_KHR
            || result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // we need to create a new semaphore or jump through a few hoops to
            // wait for the current one to be unsignalled before we can use it again
            // creating a new one is easiest. See also:
            // https://github.com/KhronosGroup/Vulkan-Docs/issues/152
            // https://www.khronos.org/blog/resolving-longstanding-issues-with-wsi
            {
                const uint64_t frameCount = queue->device->frameCount;
                std::scoped_lock lock(queue->device->destroyMutex);
                for (auto& x : backendSurface->swapchainAcquireSemaphores)
                {
                    queue->device->destroyedSemaphores.emplace_back(x, frameCount);
                }
            }
            backendSurface->swapchainAcquireSemaphores.clear();
        }
    }

    VulkanTexture* currentTexture = backendSurface->backbufferTextures[backendSurface->backBufferIndex];
    *surfaceTexture = currentTexture;

    // Barrier
    backendSurface->AddRef();
    presentSurfaces.push_back(backendSurface);

    return GPUAcquireSurfaceResult_SuccessOptimal;
}

void VulkanCommandBuffer::PushDebugGroup(const char* groupLabel) const
{
    if (!queue->device->adapter->debugUtils)
        return;

    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pNext = nullptr;
    label.pLabelName = groupLabel;
    label.color[0] = 0.0f;
    label.color[1] = 0.0f;
    label.color[2] = 0.0f;
    label.color[3] = 1.0f;
    vkCmdBeginDebugUtilsLabelEXT(handle, &label);
}

void VulkanCommandBuffer::PopDebugGroup() const
{
    if (!queue->device->adapter->debugUtils)
        return;

    vkCmdEndDebugUtilsLabelEXT(handle);
}

void VulkanCommandBuffer::InsertDebugMarker(const char* markerLabel) const
{
    if (!queue->device->adapter->debugUtils)
        return;

    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pNext = nullptr;
    label.pLabelName = markerLabel;
    label.color[0] = 0.0f;
    label.color[1] = 0.0f;
    label.color[2] = 0.0f;
    label.color[3] = 1.0f;
    vkCmdInsertDebugUtilsLabelEXT(handle, &label);
}

GPUComputePassEncoder VulkanCommandBuffer::BeginComputePass(const GPUComputePassDesc& desc)
{
    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    computePassEncoder->Begin(desc);
    encoderActive = true;
    return computePassEncoder;
}

GPURenderPassEncoder VulkanCommandBuffer::BeginRenderPass(const GPURenderPassDesc& desc)
{
    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    renderPassEncoder->Begin(desc);
    encoderActive = true;
    return renderPassEncoder;
}

/* VulkanQueue */
GPUCommandBuffer VulkanQueue::AcquireCommandBuffer(const GPUCommandBufferDesc* desc)
{
    cmdBuffersLocker.lock();
    uint32_t index = cmdBuffersCount++;
    if (index >= commandBuffers.size())
    {
        VulkanCommandBuffer* commandBuffer = new VulkanCommandBuffer();
        commandBuffer->device = device;
        commandBuffer->queue = this;
        commandBuffer->index = index;
        commandBuffer->computePassEncoder = new VulkanComputePassEncoder();
        commandBuffer->computePassEncoder->commandBuffer = commandBuffer;
        commandBuffer->renderPassEncoder = new VulkanRenderPassEncoder();
        commandBuffer->renderPassEncoder->commandBuffer = commandBuffer;

        for (uint32_t i = 0; i < device->maxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device->adapter->queueFamilyIndices.familyIndices[queueType];
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

void VulkanQueue::Submit(uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers)
{
    VkFence fence = VK_NULL_HANDLE;
    std::vector<VkSemaphoreSubmitInfo> submitWaitSemaphoreInfos;
    std::vector<VkSemaphoreSubmitInfo> submitSignalSemaphoreInfos;
    std::vector<VkCommandBufferSubmitInfo> submitCommandBufferInfos;
    bool submitPresent = false;

    for (uint32_t i = 0; i < numCommandBuffers; i++)
    {
        VulkanCommandBuffer* commandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffers[i]);

        VkCommandBufferSubmitInfo& commandBufferSubmitInfo = submitCommandBufferInfos.emplace_back();
        commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.commandBuffer = commandBuffer->End();

        for (auto& surface : commandBuffer->presentSurfaces)
        {
            VkSemaphoreSubmitInfo& waitSemaphore = submitWaitSemaphoreInfos.emplace_back();
            waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            waitSemaphore.semaphore = surface->swapchainAcquireSemaphores[surface->swapChainAcquireSemaphoreIndex];
            waitSemaphore.value = 0; // not a timeline semaphore
            waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSemaphoreSubmitInfo& signalSemaphore = submitSignalSemaphoreInfos.emplace_back();
            signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signalSemaphore.semaphore = surface->swapchainReleaseSemaphore;
            signalSemaphore.value = 0; // not a timeline semaphore

            submitPresent = true;
        }
    }

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = (uint32_t)submitWaitSemaphoreInfos.size();
    submitInfo.pWaitSemaphoreInfos = submitWaitSemaphoreInfos.data();
    submitInfo.commandBufferInfoCount = (uint32_t)submitCommandBufferInfos.size();
    submitInfo.pCommandBufferInfos = submitCommandBufferInfos.data();
    submitInfo.signalSemaphoreInfoCount = (uint32_t)submitSignalSemaphoreInfos.size();
    submitInfo.pSignalSemaphoreInfos = submitSignalSemaphoreInfos.data();
    VK_CHECK(device->vkQueueSubmit2(handle, 1, &submitInfo, fence));

    if (!submitPresent)
        return;

    // Present surfaces
    std::vector<VkSwapchainKHR> submitSwapchains;
    std::vector<uint32_t> submitSwapchainImageIndices;
    std::vector<VkSemaphore> submitSignalSemaphores;
    for (uint32_t i = 0; i < numCommandBuffers; i++)
    {
        VulkanCommandBuffer* commandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffers[i]);

        for (auto& surface : commandBuffer->presentSurfaces)
        {
            submitSwapchains.push_back(surface->swapchain);
            submitSwapchainImageIndices.push_back(surface->backBufferIndex);
            submitSignalSemaphores.push_back(surface->swapchainReleaseSemaphore);

            // Advance surface frame index
            surface->swapChainAcquireSemaphoreIndex = (surface->swapChainAcquireSemaphoreIndex + 1) % surface->swapchainAcquireSemaphores.size();
            surface->Release();
        }

        commandBuffer->presentSurfaces.clear();
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = (uint32_t)submitSignalSemaphores.size();
    presentInfo.pWaitSemaphores = submitSignalSemaphores.data();
    presentInfo.swapchainCount = (uint32_t)submitSwapchains.size();
    presentInfo.pSwapchains = submitSwapchains.data();
    presentInfo.pImageIndices = submitSwapchainImageIndices.data();

    const VkResult result = device->vkQueuePresentKHR(device->queues[GPUQueueType_Graphics].handle, &presentInfo);
    if (result != VK_SUCCESS)
    {
        // Handle outdated error in present
        if (result == VK_SUBOPTIMAL_KHR
            || result == VK_ERROR_OUT_OF_DATE_KHR)
        {
        }
        else
        {
            ALIMER_UNREACHABLE();
        }
    }
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
void VulkanCopyAllocator::Init(VulkanDevice* device_)
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
        context.uploadBuffer = static_cast<VulkanBuffer*>(device->CreateBuffer(uploadBufferDesc, nullptr));
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

/* VulkanDevice */
VulkanDevice::~VulkanDevice()
{
    VK_CHECK(vkDeviceWaitIdle(handle));

    for (uint32_t queueIndex = 0; queueIndex < GPUQueueType_Count; ++queueIndex)
    {
        if (queues[queueIndex].handle == VK_NULL_HANDLE)
            continue;

        for (uint32_t frameIndex = 0; frameIndex < maxFramesInFlight; ++frameIndex)
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

    SAFE_RELEASE(adapter);
}

void VulkanDevice::SetLabel(const char* label)
{
    SetObjectName(VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(handle), label);
}

bool VulkanDevice::HasFeature(GPUFeature feature) const
{
    return adapter->HasFeature(feature);
}

GPUQueue VulkanDevice::GetQueue(GPUQueueType type)
{
    return &queues[type];
}

bool VulkanDevice::WaitIdle()
{
    VkResult result = vkDeviceWaitIdle(handle);
    if (result != VK_SUCCESS)
        return false;

    ProcessDeletionQueue(true);
    return true;
}

uint64_t VulkanDevice::CommitFrame()
{
    // Final submits with fences.
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        queues[i].Submit(queues[i].frameFences[frameIndex]);
        queues[i].cmdBuffersCount = 0;
    }

    // Begin new frame
    frameCount++;
    frameIndex = frameCount % maxFramesInFlight;

    // Initiate stalling CPU when GPU is not yet finished with next frame
    if (frameCount >= maxFramesInFlight)
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

void VulkanDevice::ProcessDeletionQueue(bool force)
{
    const auto Destroy = [&](auto&& queue, auto&& handler) {
        while (!queue.empty()) {
            if (force || (queue.front().second + maxFramesInFlight < frameCount))
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
    Destroy(destroyedBufferViews, [&](auto& item) { vkDestroyBufferView(handle, item, nullptr); });
    Destroy(destroyedSamplers, [&](auto& item) { vkDestroySampler(handle, item, nullptr); });
    Destroy(destroyedDescriptorSetLayouts, [&](auto& item) { vkDestroyDescriptorSetLayout(handle, item, nullptr); });
    Destroy(destroyedPipelineLayouts, [&](auto& item) { vkDestroyPipelineLayout(handle, item, nullptr); });
    Destroy(destroyedShaderModules, [&](auto& item) { vkDestroyShaderModule(handle, item, nullptr); });
    Destroy(destroyedPipelines, [&](auto& item) { vkDestroyPipeline(handle, item, nullptr); });
    Destroy(destroyedQueryPools, [&](auto& item) { vkDestroyQueryPool(handle, item, nullptr); });
    Destroy(destroyedSemaphores, [&](auto& item) {vkDestroySemaphore(handle, item, nullptr); });
    Destroy(destroyedSwapchains, [&](auto& item) { vkDestroySwapchainKHR(handle, item, nullptr); });
    Destroy(destroyedSurfaces, [&](auto& item) { vkDestroySurfaceKHR(adapter->instance->handle, item, nullptr); });
    destroyMutex.unlock();
}

GPUBuffer VulkanDevice::CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData)
{
    VulkanBuffer* buffer = new VulkanBuffer();
    buffer->device = this;
    buffer->desc = desc;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = desc.size;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    bool needBufferDeviceAddress = false;
    if (desc.usage & GPUBufferUsage_Vertex)
    {
        createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if (desc.usage & GPUBufferUsage_Index)
    {
        createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if (desc.usage & GPUBufferUsage_Constant)
    {
        createInfo.size = VmaAlignUp(createInfo.size, adapter->properties2.properties.limits.minUniformBufferOffsetAlignment);
        createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (desc.usage & GPUBufferUsage_ShaderRead)
    {
        // Read only ByteAddressBuffer is also storage buffer
        createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }

    if (desc.usage & GPUBufferUsage_ShaderWrite)
    {
        createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    if (desc.usage & GPUBufferUsage_Indirect)
    {
        createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if ((desc.usage & GPUBufferUsage_Predication)
        /* && QueryFeatureSupport(RHIFeature::Predication)*/
        )
    {
        createInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }

    if ((desc.usage & GPUBufferUsage_RayTracing)
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
    if (desc.memoryType == GPUMemoryType_Readback)
    {
        memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    else if (desc.memoryType == GPUMemoryType_Upload)
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

    if (desc.label)
    {
        buffer->SetLabel(desc.label);
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
        if (desc.memoryType == GPUMemoryType_Upload)
        {
            pMappedData = buffer->pMappedData;
        }
        else
        {
            context = copyAllocator.Allocate(createInfo.size);
            pMappedData = context.uploadBufferData;
        }

        std::memcpy(pMappedData, pInitialData, desc.size);

        if (context.IsValid())
        {
            VkBufferCopy copyRegion = {};
            copyRegion.size = desc.size;
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

            if (desc.usage & GPUBufferUsage_Vertex)
            {
                barrier.dstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            }

            if (desc.usage & GPUBufferUsage_Index)
            {
                barrier.dstStageMask |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
            }

            if (desc.usage & GPUBufferUsage_Constant)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_UNIFORM_READ_BIT;
            }

            if (desc.usage & GPUBufferUsage_ShaderRead)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
            }

            if (desc.usage & GPUBufferUsage_ShaderWrite)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_WRITE_BIT;
            }

            if (desc.usage & GPUBufferUsage_Indirect)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            }

            if (desc.usage & GPUBufferUsage_Predication)
            {
                barrier.dstAccessMask |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
            }

            if (desc.usage & GPUBufferUsage_RayTracing)
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

GPUTexture VulkanDevice::CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData)
{
    const bool isDepthStencil = alimerPixelFormatIsDepthStencil(desc.format);

    VulkanTexture* texture = new VulkanTexture();
    texture->device = this;
    texture->desc = desc;
    texture->vkFormat = adapter->ToVkFormat(desc.format);

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format = texture->vkFormat;
    createInfo.extent.width = desc.width;
    createInfo.extent.height = 1;
    createInfo.extent.depth = 1;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    switch (desc.dimension)
    {
        case TextureDimension_1D:
            createInfo.imageType = VK_IMAGE_TYPE_1D;
            createInfo.arrayLayers = desc.depthOrArrayLayers;
            break;

        case TextureDimension_2D:
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.height = desc.height;
            createInfo.arrayLayers = desc.depthOrArrayLayers;
            break;

        case TextureDimension_3D:
            createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            createInfo.imageType = VK_IMAGE_TYPE_3D;
            createInfo.extent.height = desc.height;
            createInfo.extent.depth = desc.depthOrArrayLayers;
            createInfo.arrayLayers = 1u;
            break;

        case TextureDimension_Cube:
            createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.height = desc.height;
            createInfo.arrayLayers = desc.depthOrArrayLayers * 6;
            break;

        default:
            alimerLogError(LogCategory_GPU, "Invalid texture dimension");
            delete texture;
            return nullptr;
    }

    createInfo.mipLevels = desc.mipLevelCount;
    createInfo.samples = (VkSampleCountFlagBits)desc.sampleCount;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = 0u;

    if (desc.usage & GPUTextureUsage_Transient)
    {
        createInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    else
    {
        createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    TextureLayout currentLayout = TextureLayout::Undefined;

    if (desc.usage & GPUTextureUsage_ShaderRead)
    {
        createInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        currentLayout = TextureLayout::ShaderResource;
    }

    if (desc.usage & GPUTextureUsage_ShaderWrite)
    {
        createInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        currentLayout = TextureLayout::UnorderedAccess;
    }

    if (desc.usage & GPUTextureUsage_RenderTarget)
    {
        if (isDepthStencil)
        {
            createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            currentLayout = TextureLayout::DepthWrite;
        }
        else
        {
            createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            currentLayout = TextureLayout::RenderTarget;
        }
    }

    if (desc.usage & GPUTextureUsage_ShadingRate)
    {
        createInfo.usage |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }

    // If ShaderRead and RenderTarget add input attachment
    if (!isDepthStencil &&
        (desc.usage & (GPUTextureUsage_RenderTarget | GPUTextureUsage_ShaderRead)))
    {
        createInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    uint32_t sharingIndices[3];
    FillImageSharingIndices(createInfo, sharingIndices);

    VkResult result = VK_SUCCESS;

    VmaAllocationInfo allocationInfo{};
    VmaAllocationCreateInfo memoryInfo = {};
    memoryInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
    memoryInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    result = vmaCreateImage(allocator, &createInfo, &memoryInfo,
        &texture->handle,
        &texture->allocation,
        &allocationInfo);

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create image.");
        return nullptr;
    }

    if (desc.label)
    {
        texture->SetLabel(desc.label);
    }

    const bool depthOnlyFormat = alimerPixelFormatIsDepthOnly(desc.format);

    texture->numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
    texture->imageLayouts.resize(texture->numSubResources);
    for (uint32_t i = 0; i < texture->numSubResources; i++)
    {
        texture->imageLayouts[i] = currentLayout;
    }

    // Issue data copy on request or transition barrier
    VkImageSubresourceRange range{};
    range.aspectMask = GetImageAspectFlags(createInfo.format, GPUTextureAspect_All);
    range.baseMipLevel = 0;
    range.levelCount = createInfo.mipLevels;
    range.baseArrayLayer = 0;
    range.layerCount = createInfo.arrayLayers;

    if (pInitialData != nullptr)
    {
        VulkanUploadContext uploadContext;
        void* pMappedData = nullptr;
#if TEXTURE_STAGING
        if (desc.memoryType == MemoryType::Upload)
        {
            pMappedData = texture->pMappedData;
        }
        else
#endif // TEXTURE_STAGING

        {
            uploadContext = copyAllocator.Allocate(allocationInfo.size);
            pMappedData = uploadContext.uploadBuffer->pMappedData;
        }

        std::vector<VkBufferImageCopy> copyRegions;

        PixelFormatInfo formatInfo;
        alimerPixelFormatGetInfo(desc.format, &formatInfo);
        const uint32_t blockSize = formatInfo.blockWidth;

        VkDeviceSize copyOffset = 0;
        uint32_t initDataIndex = 0;
        for (uint32_t arrayIndex = 0; arrayIndex < createInfo.arrayLayers; ++arrayIndex)
        {
            uint32_t levelWidth = createInfo.extent.width;
            uint32_t levelHeight = createInfo.extent.height;
            uint32_t levelDepth = createInfo.extent.depth;

            for (uint32_t mipIndex = 0; mipIndex < createInfo.mipLevels; ++mipIndex)
            {
                const GPUTextureData& subresourceData = pInitialData[initDataIndex++];
                const uint32_t numBlocksX = std::max(1u, levelWidth / blockSize);
                const uint32_t numBlocksY = std::max(1u, levelHeight / blockSize);
                const uint32_t dstRowPitch = numBlocksX * formatInfo.bytesPerBlock;
                const uint32_t dstSlicePitch = dstRowPitch * numBlocksY;

                uint32_t srcRowPitch = subresourceData.rowPitch;
                uint32_t srcSlicePitch = subresourceData.slicePitch;
                //GetSurfaceInfo(desc.format, levelWidth, levelHeight, &srcRowPitch, &srcSlicePitch);

                for (uint32_t z = 0; z < levelDepth; ++z)
                {
                    uint8_t* dstSlice = (uint8_t*)pMappedData + copyOffset + dstSlicePitch * z;
                    uint8_t* srcSlice = (uint8_t*)subresourceData.pData + srcSlicePitch * z;
                    for (uint32_t y = 0; y < numBlocksY; ++y)
                    {
                        std::memcpy(
                            dstSlice + dstRowPitch * y,
                            srcSlice + srcRowPitch * y,
                            dstRowPitch
                        );
                    }
                }

                if (uploadContext.IsValid())
                {
                    VkBufferImageCopy copyRegion = {};
                    copyRegion.bufferOffset = copyOffset;
                    copyRegion.bufferRowLength = 0;
                    copyRegion.bufferImageHeight = 0;

                    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    copyRegion.imageSubresource.mipLevel = mipIndex;
                    copyRegion.imageSubresource.baseArrayLayer = arrayIndex;
                    copyRegion.imageSubresource.layerCount = 1;

                    copyRegion.imageOffset = { 0, 0, 0 };
                    copyRegion.imageExtent.width = levelWidth;
                    copyRegion.imageExtent.height = levelHeight;
                    copyRegion.imageExtent.depth = levelDepth;

                    copyRegions.push_back(copyRegion);
                }

                copyOffset += dstSlicePitch * levelDepth;

                levelWidth = std::max(1u, levelWidth / 2);
                levelHeight = std::max(1u, levelHeight / 2);
                levelDepth = std::max(1u, levelDepth / 2);
            }
        }

        if (uploadContext.IsValid())
        {
            const VkImageLayoutMapping mappingBefore = ConvertImageLayout(TextureLayout::CopyDest, depthOnlyFormat);

            VkImageMemoryBarrier2 barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstStageMask = mappingBefore.stageFlags;
            barrier.dstAccessMask = mappingBefore.accessMask;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = texture->handle;
            barrier.subresourceRange = range;

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &barrier;
            vkCmdPipelineBarrier2(uploadContext.transferCommandBuffer, &dependencyInfo);

            vkCmdCopyBufferToImage(
                uploadContext.transferCommandBuffer,
                uploadContext.uploadBuffer->handle,
                texture->handle,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                (uint32_t)copyRegions.size(),
                copyRegions.data()
            );

            const VkImageLayoutMapping mappingAfter = ConvertImageLayout(currentLayout, depthOnlyFormat);

            std::swap(barrier.srcStageMask, barrier.dstStageMask);
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = mappingAfter.layout;
            barrier.srcAccessMask = mappingBefore.accessMask;
            barrier.dstAccessMask = mappingAfter.accessMask;

            vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

            copyAllocator.Submit(uploadContext);
        }
    }
    else if (currentLayout != TextureLayout::Undefined)
    {
        VulkanUploadContext uploadContext = copyAllocator.Allocate(allocationInfo.size);

        const VkImageLayoutMapping mappingAfter = ConvertImageLayout(currentLayout, depthOnlyFormat);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.dstStageMask = mappingAfter.stageFlags;
        barrier.dstAccessMask = mappingAfter.accessMask;
        barrier.oldLayout = createInfo.initialLayout;
        barrier.newLayout = mappingAfter.layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture->handle;
        barrier.subresourceRange = range;

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

        copyAllocator.Submit(uploadContext);
    }

    return texture;
}

GPUSampler VulkanDevice::CreateSampler(const GPUSamplerDesc& desc)
{
    VulkanSampler* sampler = new VulkanSampler();
    sampler->device = this;

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.magFilter = ToVk(desc.magFilter);
    createInfo.minFilter = ToVk(desc.minFilter);
    createInfo.mipmapMode = ToVk(desc.mipFilter);
    createInfo.addressModeU = ToVk(desc.addressModeU, adapter->features12.samplerMirrorClampToEdge);
    createInfo.addressModeV = ToVk(desc.addressModeV, adapter->features12.samplerMirrorClampToEdge);
    createInfo.addressModeW = ToVk(desc.addressModeW, adapter->features12.samplerMirrorClampToEdge);
    createInfo.mipLodBias = 0.0f;
    uint16_t maxAnisotropy = desc.maxAnisotropy;
    if (adapter->features2.features.samplerAnisotropy == VK_TRUE && maxAnisotropy > 1)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = std::clamp(maxAnisotropy * 1.f, 1.f, adapter->properties2.properties.limits.maxSamplerAnisotropy);
    }
    else
    {
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1;
    }
    if (desc.compareFunction != GPUCompareFunction_Never)
    {
        createInfo.compareEnable = VK_TRUE;
        createInfo.compareOp = ToVk(desc.compareFunction);
    }
    else
    {
        createInfo.compareEnable = VK_FALSE;
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
    }
    createInfo.minLod = desc.lodMinClamp;
    createInfo.maxLod = desc.lodMaxClamp == GPU_LOD_CLAMP_NONE ? VK_LOD_CLAMP_NONE : desc.lodMaxClamp;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkResult result = vkCreateSampler(handle, &createInfo, nullptr, &sampler->handle);
    if (result != VK_SUCCESS)
    {
        delete sampler;
        VK_LOG_ERROR(result, "Failed to create Sampler");
        return nullptr;
    }

    return sampler;
}

GPUBindGroupLayout VulkanDevice::CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc)
{
    // https://developer.arm.com/documentation/101897/0303/CPU-overheads/Optimizing-descriptor-sets-and-layouts-for-Vulkan
    // For ease of programming, VkDescriptorSetLayoutBinding::stageFlags can always be set to VK_SHADER_STAGE_ALL with no performance loss.
    VulkanBindGroupLayout* layout = new VulkanBindGroupLayout();
    layout->device = this;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 0;
    createInfo.pBindings = nullptr;

    VkResult result = vkCreateDescriptorSetLayout(handle, &createInfo, nullptr, &layout->handle);
    if (result != VK_SUCCESS)
    {
        delete layout;
        VK_LOG_ERROR(result, "Failed to create BindGroupLayout");
        return nullptr;
    }

    if (desc.label)
    {
        layout->SetLabel(desc.label);
    }

    return layout;
}

GPUPipelineLayout VulkanDevice::CreatePipelineLayout(const GPUPipelineLayoutDesc& desc)
{
    VulkanPipelineLayout* layout = new VulkanPipelineLayout();
    layout->device = this;

    VkPushConstantRange range = {};
    range = {};
    range.stageFlags = VK_SHADER_STAGE_ALL;
    range.offset = 0;
    range.size = 32;

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 1u;
    createInfo.pPushConstantRanges = &range;

    VkResult result = vkCreatePipelineLayout(handle, &createInfo, nullptr, &layout->handle);
    if (result != VK_SUCCESS)
    {
        delete layout;
        VK_LOG_ERROR(result, "Failed to create PipelineLayout");
        return nullptr;
    }

    if (desc.label)
    {
        layout->SetLabel(desc.label);
    }

    return layout;
}

GPUShaderModule VulkanDevice::CreateShaderModule(const GPUShaderModuleDesc* desc)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = desc->bytecodeSize;
    createInfo.pCode = (const uint32_t*)desc->bytecode;

    VkShaderModule vk_handle;
    VkResult result = vkCreateShaderModule(handle, &createInfo, nullptr, &vk_handle);

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create a compute shader module");
        return nullptr;
    }

    VulkanShaderModule* shaderModule = new VulkanShaderModule();
    shaderModule->device = this;
    shaderModule->handle = vk_handle;
    return shaderModule;
}

GPUComputePipeline VulkanDevice::CreateComputePipeline(const GPUComputePipelineDesc& desc)
{
    // Create shader module first.
    VkPipelineShaderStageCreateInfo computeStage{};
    computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeStage.module = static_cast<VulkanShaderModule*>(desc.compute.module)->handle;
    computeStage.pName = desc.compute.entryPoint ? desc.compute.entryPoint : "main";

    VulkanComputePipeline* pipeline = new VulkanComputePipeline();
    pipeline->device = this;
    pipeline->layout = static_cast<VulkanPipelineLayout*>(desc.layout);
    pipeline->layout->AddRef();

    VkComputePipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = computeStage;
    createInfo.layout = pipeline->layout->handle;
    VkResult result = vkCreateComputePipelines(handle, pipelineCache, 1, &createInfo, nullptr, &pipeline->handle);

    if (result != VK_SUCCESS)
    {
        delete pipeline;
        VK_LOG_ERROR(result, "Failed to create Compute Pipeline");
        return nullptr;
    }

    if (desc.label)
    {
        pipeline->SetLabel(desc.label);
    }

    return pipeline;
}

GPURenderPipeline VulkanDevice::CreateRenderPipeline(const GPURenderPipelineDesc& desc)
{
    VulkanRenderPipeline* pipeline = new VulkanRenderPipeline();
    pipeline->device = this;
    pipeline->layout = static_cast<VulkanPipelineLayout*>(desc.layout);
    pipeline->layout->AddRef();

    // ShaderStages
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    VkPipelineShaderStageCreateInfo& vertexShaderStage = stages.emplace_back();
    vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage.module = static_cast<VulkanShaderModule*>(desc.vertex.module)->handle;
    vertexShaderStage.pName = desc.vertex.entryPoint ? desc.vertex.entryPoint : "main";

    // VertexInputState (need always be specified when using VertexShader)
    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;
    if (desc.vertex.bufferCount > 0)
    {
        vertexBindings.resize(desc.vertex.bufferCount);

        for (uint32_t bufferIndex = 0; bufferIndex < desc.vertex.bufferCount; ++bufferIndex)
        {
            const GPUVertexBufferLayout& layout = desc.vertex.buffers[bufferIndex];
            vertexBindings[bufferIndex].binding = bufferIndex;
            vertexBindings[bufferIndex].stride = layout.stride;
            vertexBindings[bufferIndex].inputRate = ToVk(layout.stepMode);

            // Compute stride from attributes
            if (vertexBindings[bufferIndex].stride == 0)
            {
                for (uint32_t attributeIndex = 0; attributeIndex < layout.attributeCount; ++attributeIndex)
                {
                    const GPUVertexAttribute& attribute = layout.attributes[attributeIndex];
                    vertexBindings[bufferIndex].stride += agpuVertexFormatGetByteSize(attribute.format);
                }
            }

            for (uint32_t attributeIndex = 0; attributeIndex < layout.attributeCount; ++attributeIndex)
            {
                const GPUVertexAttribute& attribute = layout.attributes[attributeIndex];

                VkVertexInputAttributeDescription& vertexAttribute = vertexAttributes.emplace_back();
                vertexAttribute.location = attribute.shaderLocation;
                vertexAttribute.binding = bufferIndex;
                vertexAttribute.format = ToVkVertexFormat(attribute.format);
                vertexAttribute.offset = attribute.offset;
            }
        }

        vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexBindings.size();
        vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributes.size();
        vertexInputState.pVertexAttributeDescriptions = vertexAttributes.data();
    }

    // InputAssemblyState
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    // TessellationState
    VkPipelineTessellationStateCreateInfo tessellationState{};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    tessellationState.patchControlPoints = 0;

    // ViewportState
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // RasterizationState
    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    // MultisampleState
    // VkPipelineSampleLocationsStateCreateInfoEXT sampleLocationsState = {VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT};
    //sampleLocationsState.sampleLocationsInfo.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = ToVkSampleCount(desc.rasterSampleCount);

    ALIMER_ASSERT(multisampleState.rasterizationSamples <= 32);
    if (multisampleState.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT)
    {
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 0.0f;
        multisampleState.alphaToCoverageEnable = desc.alphaToCoverageEnabled ? VK_TRUE : VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;
        multisampleState.pSampleMask = nullptr;
    }

    // DepthStencilState
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // BlendState
    uint32_t colorAttachmentCount = 0;
    VkPipelineColorBlendAttachmentState blendAttachmentStates[GPU_MAX_COLOR_ATTACHMENTS] = {};

    for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        if (desc.colorAttachmentFormats[i] == PixelFormat_Undefined)
            continue;

        blendAttachmentStates[colorAttachmentCount].blendEnable = VK_FALSE;
        blendAttachmentStates[colorAttachmentCount].colorWriteMask = 0xf;
        colorAttachmentCount++;
    }
    VkPipelineColorBlendStateCreateInfo blendState = {};
    blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.logicOpEnable = VK_FALSE;
    blendState.logicOp = VK_LOGIC_OP_CLEAR;
    blendState.attachmentCount = colorAttachmentCount;
    blendState.pAttachments = blendAttachmentStates;
    blendState.blendConstants[0] = 0.0f;
    blendState.blendConstants[1] = 0.0f;
    blendState.blendConstants[2] = 0.0f;
    blendState.blendConstants[3] = 0.0f;

    // RenderingInfo/ RenderPass
    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    VkFormat colorAttachmentFormats[GPU_MAX_COLOR_ATTACHMENTS];
    for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        if (desc.colorAttachmentFormats[i] == PixelFormat_Undefined)
            continue;

        colorAttachmentFormats[renderingInfo.colorAttachmentCount] = adapter->ToVkFormat(desc.colorAttachmentFormats[i]);
        renderingInfo.colorAttachmentCount++;
    }
    renderingInfo.pColorAttachmentFormats = colorAttachmentFormats;
    renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    if (desc.depthStencilAttachmentFormat != PixelFormat_Undefined)
    {
        renderingInfo.depthAttachmentFormat = adapter->ToVkFormat(desc.depthStencilAttachmentFormat);
        if (!alimerPixelFormatIsDepthOnly(desc.depthStencilAttachmentFormat))
        {
            renderingInfo.stencilAttachmentFormat = renderingInfo.depthAttachmentFormat;
        }
    }

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = &renderingInfo;
    createInfo.stageCount = (uint32_t)stages.size();
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pTessellationState = nullptr;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = nullptr;
    createInfo.pColorBlendState = &blendState;
    createInfo.pDynamicState = &dynamicStateInfo;
    createInfo.layout = pipeline->layout->handle;
    createInfo.renderPass = VK_NULL_HANDLE;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(handle, pipelineCache, 1, &createInfo, nullptr, &pipeline->handle);

    if (result != VK_SUCCESS)
    {
        delete pipeline;
        VK_LOG_ERROR(result, "Failed to create Render Pipeline");
        return nullptr;
    }

    if (desc.label)
    {
        pipeline->SetLabel(desc.label);
    }

    return pipeline;
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

void VulkanDevice::SetObjectName(VkObjectType type, uint64_t handle_, const char* label) const
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

void VulkanDevice::FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices) const
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

void VulkanDevice::FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices) const
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
    for (size_t i = 0; i < backbufferTextures.size(); ++i)
    {
        backbufferTextures[i]->Release();
    }

    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();

    for (size_t i = 0; i < backbufferTextures.size(); ++i)
    {
        device->destroyedSemaphores.push_back(std::make_pair(swapchainAcquireSemaphores[i], frameCount));
    }
    device->destroyedSemaphores.push_back(std::make_pair(swapchainReleaseSemaphore, frameCount));

    if (swapchain)
    {
        device->destroyedSwapchains.push_back(std::make_pair(swapchain, frameCount));
        swapchain = VK_NULL_HANDLE;
    }

    if (handle != VK_NULL_HANDLE)
    {
        device->destroyedSurfaces.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }


    device->destroyMutex.unlock();

    backBufferIndex = 0;
    backbufferTextures.clear();
    swapchainExtent = {};
    SAFE_RELEASE(device);
}

GPUResult VulkanSurface::GetCapabilities(GPUAdapter adapter, GPUSurfaceCapabilities* capabilities) const
{
    VulkanAdapter* backendAdapter = static_cast<VulkanAdapter*>(adapter);

    VkSurfaceCapabilitiesKHR caps;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(backendAdapter->handle, handle, &caps);
    if (result != VK_SUCCESS)
        return GPUResult_InvalidOperation;

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(backendAdapter->handle, handle, &formatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> vkFormats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(backendAdapter->handle, handle, &formatCount, vkFormats.data()));

    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(backendAdapter->handle, handle, &presentModeCount, nullptr));
    std::vector<VkPresentModeKHR> swapchainPresentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(backendAdapter->handle, handle, &presentModeCount, swapchainPresentModes.data()));

    // TODO: Add ColorSpace
    supportedFormats.clear();
    supportedFormats.reserve(formatCount);
    for (const auto& surfaceFormat : vkFormats)
    {
        PixelFormat format = alimerPixelFormatFromVkFormat(static_cast<uint32_t>(surfaceFormat.format));
        supportedFormats.push_back(format);
    }

    capabilities->preferredFormat = PixelFormat_BGRA8UnormSrgb;
    capabilities->supportedUsage = GPUTextureUsage_ShaderRead | GPUTextureUsage_RenderTarget;
    capabilities->formatCount = (uint32_t)supportedFormats.size();
    capabilities->formats = supportedFormats.data();
    return GPUResult_Success;
}

bool VulkanSurface::Configure(const GPUSurfaceConfig* config_)
{
    Unconfigure();

    config = *config_;
    device = static_cast<VulkanDevice*>(config.device);
    device->AddRef();

    VkSurfaceCapabilitiesKHR surfaceCaps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->adapter->handle, handle, &surfaceCaps));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device->adapter->handle, handle, &formatCount, nullptr));

    std::vector<VkSurfaceFormatKHR> swapchainFormats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device->adapter->handle, handle, &formatCount, swapchainFormats.data()));

    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device->adapter->handle, handle, &presentModeCount, nullptr));
    std::vector<VkPresentModeKHR> swapchainPresentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device->adapter->handle, handle, &presentModeCount, swapchainPresentModes.data()));

    VkPresentModeKHR vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Determine the number of images
    uint32_t imageCount = MinImageCountForPresentMode(vkPresentMode);
    if (surfaceCaps.maxImageCount != 0
        && imageCount > surfaceCaps.maxImageCount)
    {
        imageCount = surfaceCaps.maxImageCount;
    }

    if (imageCount < surfaceCaps.minImageCount)
    {
        imageCount = surfaceCaps.minImageCount;
    }

    if (imageCount > device->maxFramesInFlight)
    {
        imageCount = device->maxFramesInFlight;
    }

    // Format and color space
    VkSurfaceFormatKHR surfaceFormat = {};
    surfaceFormat.format = device->adapter->ToVkFormat(_ALIMER_DEF(config.format, PixelFormat_BGRA8UnormSrgb));
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    bool valid = false;
    bool allowHDR = true;
    for (const auto& format : swapchainFormats)
    {
        if (!allowHDR && format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            continue;

        if (format.format == surfaceFormat.format)
        {
            surfaceFormat = format;
            valid = true;
            break;
        }
    }

    if (!valid)
    {
        surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    if (surfaceCaps.currentExtent.width != 0xFFFFFFFF &&
        surfaceCaps.currentExtent.height != 0xFFFFFFFF)
    {
        swapchainExtent = surfaceCaps.currentExtent;
    }
    else
    {
        swapchainExtent.width = std::clamp(config.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        swapchainExtent.height = std::clamp(config.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = handle;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(device->adapter->handle, createInfo.imageFormat, &formatProps);

    if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) || (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
    {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        createInfo.preTransform = surfaceCaps.currentTransform;
    }

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = vkPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = swapchain;

    VK_CHECK(device->vkCreateSwapchainKHR(device->handle, &createInfo, nullptr, &swapchain));

    if (createInfo.oldSwapchain != VK_NULL_HANDLE)
    {
        device->vkDestroySwapchainKHR(device->handle, createInfo.oldSwapchain, nullptr);
    }

    VK_CHECK(device->vkGetSwapchainImagesKHR(device->handle, swapchain, &imageCount, nullptr));
    std::vector<VkImage> swapchainImages(imageCount);
    VK_CHECK(device->vkGetSwapchainImagesKHR(device->handle, swapchain, &imageCount, swapchainImages.data()));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (swapchainAcquireSemaphores.empty())
    {
        for (size_t i = 0; i < swapchainImages.size(); ++i)
        {
            VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &swapchainAcquireSemaphores.emplace_back()));
        }
    }

    if (swapchainReleaseSemaphore == VK_NULL_HANDLE)
    {
        VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &swapchainReleaseSemaphore));
    }

    backBufferIndex = 0;
    backbufferTextures.resize(imageCount);

    GPUTextureDesc textureDesc{};
    textureDesc.format = alimerPixelFormatFromVkFormat(createInfo.imageFormat);
    textureDesc.width = createInfo.imageExtent.width;
    textureDesc.height = createInfo.imageExtent.height;
    textureDesc.usage = GPUTextureUsage_RenderTarget;

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VulkanTexture* texture = new VulkanTexture();
        texture->device = device;
        texture->desc = textureDesc;
        texture->vkFormat = createInfo.imageFormat;
        texture->handle = swapchainImages[i];
        //texture->allocatedSize = 0;
        texture->numSubResources = 1;
        texture->imageLayouts.resize(1);
        texture->imageLayouts[0] = TextureLayout::Undefined;

        backbufferTextures[i] = texture;
    }

    return true;
}

void VulkanSurface::Unconfigure()
{
    if (device)
    {
        device->WaitIdle();
    }

    for (size_t i = 0; i < backbufferTextures.size(); ++i)
    {
        backbufferTextures[i]->Release();
    }

    backBufferIndex = 0;
    backbufferTextures.clear();
    swapchainExtent = {};
    SAFE_RELEASE(device);
}

/* VulkanAdapter */
GPUResult VulkanAdapter::GetInfo(GPUAdapterInfo* info) const
{
    memset(info, 0, sizeof(GPUAdapterInfo));

    info->deviceName = properties2.properties.deviceName;
    info->vendor = agpuGPUAdapterVendorFromID(properties2.properties.vendorID);
    info->vendorID = properties2.properties.vendorID;
    info->deviceID = properties2.properties.deviceID;

    uint32_t versionRaw = properties2.properties.driverVersion;

    switch (info->vendor)
    {
        case GPUAdapterVendor_NVIDIA:
            info->driverVersion[0] = static_cast<uint16_t>((versionRaw >> 22) & 0x3FF);
            info->driverVersion[1] = static_cast<uint16_t>((versionRaw >> 14) & 0x0FF);
            info->driverVersion[2] = static_cast<uint16_t>((versionRaw >> 6) & 0x0FF);
            info->driverVersion[3] = static_cast<uint16_t>(versionRaw & 0x003F);
            break;

        case GPUAdapterVendor_Intel:
#if ALIMER_PLATFORM_WINDOWS
            // Windows Vulkan driver releases together with D3D driver, so they share the same
            // version. But only CCC.DDDD is encoded in 32-bit driverVersion.
            info->driverVersion[0] = static_cast<uint16_t>(versionRaw >> 14);
            info->driverVersion[1] = static_cast<uint16_t>(versionRaw & 0x3FFF);
            break;
#endif

        default:
            // Use Vulkan driver conversions for other vendors
            info->driverVersion[0] = static_cast<uint16_t>(versionRaw >> 22);
            info->driverVersion[1] = static_cast<uint16_t>((versionRaw >> 12) & 0x3FF);
            info->driverVersion[2] = static_cast<uint16_t>(versionRaw & 0xFFF);
            break;
    }

    info->driverDescription = driverDescription.c_str();

    switch (properties2.properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            info->adapterType = GPUAdapterType_IntegratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            info->adapterType = GPUAdapterType_DiscreteGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            info->adapterType = GPUAdapterType_CPU;
            break;

        default:
            info->adapterType = GPUAdapterType_Unknown;
            break;
    }

    return GPUResult_Success;
}

GPUResult VulkanAdapter::GetLimits(GPULimits* limits) const
{
    limits->maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
    limits->maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
    limits->maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
    limits->maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
    limits->maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
    limits->maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
    limits->maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
    limits->minConstantBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minUniformBufferOffsetAlignment;
    limits->minStorageBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minStorageBufferOffsetAlignment;
    limits->maxBufferSize = properties13.maxBufferSize;
    limits->maxColorAttachments = properties2.properties.limits.maxColorAttachments;
    limits->maxViewports = properties2.properties.limits.maxViewports;
    limits->viewportBoundsMin = properties2.properties.limits.viewportBoundsRange[0];
    limits->viewportBoundsMax = properties2.properties.limits.viewportBoundsRange[1];

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
        }
        );

    if (HasFeature(GPUFeature_ConservativeRasterization))
    {
        limits->conservativeRasterizationTier = GPUConservativeRasterizationTier_1;

        if (conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
            limits->conservativeRasterizationTier = GPUConservativeRasterizationTier_2;
        if (conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
            limits->conservativeRasterizationTier = GPUConservativeRasterizationTier_3;
    }

    return GPUResult_Success;
}

bool VulkanAdapter::HasFeature(GPUFeature feature) const
{
    switch (feature)
    {
        case GPUFeature_DepthClipControl:
            return features2.features.depthClamp == VK_TRUE;

        case GPUFeature_Depth32FloatStencil8:
            return IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);

        case GPUFeature_TimestampQuery:
            return properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

        case GPUFeature_PipelineStatisticsQuery:
            return features2.features.pipelineStatisticsQuery == VK_TRUE;

        case GPUFeature_TextureCompressionBC:
            return features2.features.textureCompressionBC == VK_TRUE;

        case GPUFeature_TextureCompressionETC2:
            return features2.features.textureCompressionETC2 == VK_TRUE;

        case GPUFeature_TextureCompressionASTC:
            return features2.features.textureCompressionASTC_LDR == VK_TRUE;

        case GPUFeature_TextureCompressionASTC_HDR:
            return astcHdrFeatures.textureCompressionASTC_HDR == VK_TRUE;

        case GPUFeature_IndirectFirstInstance:
            return features2.features.drawIndirectFirstInstance == VK_TRUE;

        case GPUFeature_DualSourceBlending:
            return features2.features.dualSrcBlend == VK_TRUE;

        case GPUFeature_ShaderFloat16:
            // VK_KHR_16bit_storage core in 1.1
            // VK_KHR_shader_float16_int8 core in 1.2
            return true;

        case GPUFeature_GPUUploadHeapSupported:
            // https://github.com/KhronosGroup/Vulkan-Docs/issues/2096
                // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            return true;

        case GPUFeature_CopyQueueTimestampQueriesSupported:
            return true;

        case GPUFeature_CacheCoherentUMA:
            if (memoryProperties2.memoryProperties.memoryHeapCount == 1 &&
                memoryProperties2.memoryProperties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                return true;
            }

            return false;

        case GPUFeature_ShaderOutputViewportIndex:
            return features12.shaderOutputLayer == VK_TRUE && features12.shaderOutputViewportIndex == VK_TRUE;

        case GPUFeature_ConservativeRasterization:
            return extensions.conservativeRasterization;

        default:
            return false;
    }
}

bool VulkanAdapter::IsDepthStencilFormatSupported(VkFormat format) const
{
    ALIMER_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT
        || format == VK_FORMAT_D24_UNORM_S8_UINT
        || format == VK_FORMAT_D32_SFLOAT_S8_UINT
        || format == VK_FORMAT_S8_UINT);

    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(handle, format, &properties);
    return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

VkFormat VulkanAdapter::ToVkFormat(PixelFormat format) const
{
    //if (format == PixelFormat_Stencil8 && !supportsS8)
    //{
    //    return VK_FORMAT_D24_UNORM_S8_UINT;
    //}

    if (format == PixelFormat_Depth24UnormStencil8 && !supportsDepth24Stencil8)
    {
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    // https://github.com/doitsujin/dxvk/blob/master/src/dxgi/dxgi_format.cpp
    VkFormat vkFormat = static_cast<VkFormat>(alimerPixelFormatToVkFormat(format));
    return vkFormat;
}

GPUDevice VulkanAdapter::CreateDevice(const GPUDeviceDesc& desc)
{
    VulkanDevice* device = new VulkanDevice();
    device->adapter = this;
    device->adapter->AddRef();
    device->maxFramesInFlight = desc.maxFramesInFlight;

    std::vector<const char*> enabledDeviceExtensions;
    enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Core in 1.3
    if (properties2.properties.apiVersion < VK_API_VERSION_1_3)
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

    if (features13.dynamicRendering == VK_FALSE &&
        dynamicRenderingFeatures.dynamicRendering == VK_TRUE)
    {
        device->vkCmdBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(device->handle, "vkCmdBeginRenderingKHR");
        device->vkCmdEndRendering = (PFN_vkCmdEndRendering)vkGetDeviceProcAddr(device->handle, "vkCmdEndRenderingKHR");
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
            device->queues[i].queueType = (GPUQueueType)i;

            device->vkGetDeviceQueue(device->handle, queueFamilyIndices.familyIndices[i], queueFamilyIndices.queueIndices[i], &device->queues[i].handle);
            queueFamilyIndices.counts[i] = queueFamilyIndices.queueOffsets[queueFamilyIndices.familyIndices[i]];

            for (uint32_t frameIndex = 0; frameIndex < device->maxFramesInFlight; ++frameIndex)
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

    if (desc.label)
    {
        device->SetLabel(desc.label);
    }

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
        delete device;
        VK_LOG_ERROR(result, "Cannot create allocator");
        return nullptr;
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
            delete device;
            VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
            return nullptr;
        }
    }

    // Create pipeline cache 
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheInfo.pNext = nullptr;
    pipelineCacheInfo.flags = 0;
    pipelineCacheInfo.initialDataSize = 0;
    pipelineCacheInfo.pInitialData = nullptr;
    result = device->vkCreatePipelineCache(device->handle, &pipelineCacheInfo, nullptr, &device->pipelineCache);
    if (result != VK_SUCCESS)
    {
        delete device;
        VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
        return nullptr;
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

/* VulkanInstance */
VulkanInstance::~VulkanInstance()
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

GPUSurface VulkanInstance::CreateSurface(Window* window)
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

GPUAdapter VulkanInstance::RequestAdapter(const GPURequestAdapterOptions* options)
{
    // Enumerate physical device and detect best one.
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr));
    if (physicalDeviceCount == 0)
    {
        alimerLogDebug(LogCategory_GPU, "Vulkan: Failed to find GPUs with Vulkan support");
        return nullptr;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, physicalDevices.data()));

    // The result adapter
    VulkanAdapter* adapter = new VulkanAdapter();
    adapter->instance = this;
    adapter->debugUtils = debugUtils;

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
        adapter->conservativeRasterizationProps = {};
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

        if (adapter->extensions.conservativeRasterization)
        {
            adapter->conservativeRasterizationProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
            addToPropertiesChain(&adapter->conservativeRasterizationProps);
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

    adapter->memoryProperties2 = {};
    adapter->memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(adapter->handle, &adapter->memoryProperties2);

    adapter->driverDescription = adapter->properties12.driverName;
    if (adapter->properties12.driverInfo[0] != '\0')
    {
        adapter->driverDescription += std::string(": ") + adapter->properties12.driverInfo;
    }

    // The environment can request to various options for depth-stencil formats that could be
    // unavailable. Override the decision if it is not applicable.
    adapter->supportsDepth32Stencil8 = adapter->IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
    adapter->supportsDepth24Stencil8 = adapter->IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
    adapter->supportsStencil8 = adapter->IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

    ALIMER_ASSERT(adapter->supportsDepth32Stencil8 || adapter->supportsDepth24Stencil8);

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
    VulkanInstance* instance = new VulkanInstance();

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
