// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanTexture.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphics.h"

#include "Math/MathHelper.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <spirv_reflect.h>

namespace Alimer
{
    namespace
    {
        static_assert(sizeof(Alimer::DispatchIndirectCommand) == sizeof(VkDispatchIndirectCommand), "Size mismatch");
        static_assert(offsetof(Alimer::DispatchIndirectCommand, x) == offsetof(VkDispatchIndirectCommand, x), "Layout mismatch");
        static_assert(offsetof(Alimer::DispatchIndirectCommand, y) == offsetof(VkDispatchIndirectCommand, y), "Layout mismatch");
        static_assert(offsetof(Alimer::DispatchIndirectCommand, z) == offsetof(VkDispatchIndirectCommand, z), "Layout mismatch");

        static_assert(sizeof(Alimer::DrawIndexedIndirectCommand) == sizeof(VkDrawIndexedIndirectCommand), "Size mismatch");
        static_assert(offsetof(Alimer::DrawIndexedIndirectCommand, indexCount) == offsetof(VkDrawIndexedIndirectCommand, indexCount), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndexedIndirectCommand, instanceCount) == offsetof(VkDrawIndexedIndirectCommand, instanceCount), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndexedIndirectCommand, firstIndex) == offsetof(VkDrawIndexedIndirectCommand, firstIndex), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndexedIndirectCommand, vertexOffset) == offsetof(VkDrawIndexedIndirectCommand, vertexOffset), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndexedIndirectCommand, firstInstance) == offsetof(VkDrawIndexedIndirectCommand, firstInstance), "Layout mismatch");

        static_assert(sizeof(Alimer::DrawIndirectCommand) == sizeof(VkDrawIndirectCommand), "Size mismatch");
        static_assert(offsetof(Alimer::DrawIndirectCommand, vertexCount) == offsetof(VkDrawIndirectCommand, vertexCount), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndirectCommand, instanceCount) == offsetof(VkDrawIndirectCommand, instanceCount), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndirectCommand, firstVertex) == offsetof(VkDrawIndirectCommand, firstVertex), "Layout mismatch");
        static_assert(offsetof(Alimer::DrawIndirectCommand, firstInstance) == offsetof(VkDrawIndirectCommand, firstInstance), "Layout mismatch");

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

        struct PhysicalDeviceExtensions
        {
            bool swapchain;
            bool depth_clip_enable;
            bool memory_budget;
            bool performance_query;
            bool deferred_host_operations;
            bool accelerationStructure;
            bool raytracingPipeline;
            bool rayQuery;
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
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
                    extensions.deferred_host_operations = true;
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

        /* Sampler */
        [[nodiscard]] constexpr VkFilter ToVulkan(SamplerFilter mode)
        {
            switch (mode)
            {
                case SamplerFilter::Nearest:
                    return VK_FILTER_NEAREST;
                case SamplerFilter::Linear:
                    return VK_FILTER_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_FILTER_MAX_ENUM;
            }
        }

        [[nodiscard]] constexpr VkSamplerMipmapMode ToVulkanMipmapMode(SamplerFilter mode)
        {
            switch (mode)
            {
                case SamplerFilter::Nearest:
                    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
                case SamplerFilter::Linear:
                    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
            }
        }

        [[nodiscard]] constexpr VkSamplerAddressMode ToVulkan(SamplerAddressMode mode)
        {
            switch (mode)
            {
                case SamplerAddressMode::Wrap:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case SamplerAddressMode::Mirror:
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case SamplerAddressMode::Clamp:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case SamplerAddressMode::Border:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                case SamplerAddressMode::MirrorOnce:
                    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
            }
        }

        [[nodiscard]] constexpr VkBorderColor ToVulkan(SamplerBorderColor value)
        {
            switch (value)
            {
                case SamplerBorderColor::TransparentBlack:
                    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
                case SamplerBorderColor::OpaqueBlack:
                    return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
                case SamplerBorderColor::OpaqueWhite:
                    return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_BORDER_COLOR_MAX_ENUM;
            }
        }

        /* Shader */
        [[nodiscard]] constexpr ShaderStages ConvertShaderStage(SpvExecutionModel model)
        {
            switch (model)
            {
                case SpvExecutionModelVertex:
                    return ShaderStages::Vertex;
                    //case SpvExecutionModelTessellationControl:
                    //	return ShaderStages::TessControl;
                    //case SpvExecutionModelTessellationEvaluation:
                    //	return ShaderStages::TessEvaluation;
                    //case SpvExecutionModelGeometry:
                    //	return ShaderStages::Geometry;
                case SpvExecutionModelFragment:
                    return ShaderStages::Pixel;
                case SpvExecutionModelGLCompute:
                    return ShaderStages::Compute;
                    //case SpvExecutionModelRayGenerationNV:
                    //	return ShaderStage::RayGeneration;
                    //case SpvExecutionModelIntersectionNV:
                    //	return ShaderStage::Intersection;
                    //case SpvExecutionModelAnyHitNV:
                    //	return ShaderStage::AnyHit;
                    //case SpvExecutionModelClosestHitNV:
                    //	return ShaderStage::ClosestHit;
                    //case SpvExecutionModelMissNV:
                    //	return ShaderStage::Miss;
                    //case SpvExecutionModelCallableNV:
                    //	return ShaderStage::Callable;
                    //case SpvExecutionModelTaskNV:
                    //	return ShaderStage::Amplification;
                    //case SpvExecutionModelMeshNV:
                    //	return ShaderStage::Mesh;
            }

            ALIMER_UNREACHABLE();
            return ShaderStages::None;
        }

        [[nodiscard]] constexpr ShaderResourceType GetShaderResourceType(SpvReflectDescriptorType type)
        {
            switch (type)
            {
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    return ShaderResourceType::Sampler;
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    return ShaderResourceType::SampledTexture;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    return ShaderResourceType::StorageTexture;

                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    return ShaderResourceType::UniformBuffer;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    return ShaderResourceType::StorageBuffer;

                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    return ShaderResourceType::UniformBuffer;

                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                    break;
                default:
                    ALIMER_UNREACHABLE();
                    break;
            }

            ALIMER_UNREACHABLE();
        }

        /* DescriptorSetLayout/PipelineLayout */
        [[nodiscard]] constexpr VkDescriptorType ToVulkan(ShaderResourceType type, bool dynamic)
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

        /* Pipeline */

        [[nodiscard]] constexpr VkPrimitiveTopology ToVulkan(PrimitiveTopology topology)
        {
            switch (topology)
            {
                case PrimitiveTopology::PointList:
                    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case PrimitiveTopology::LineList:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopology::LineStrip:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                case PrimitiveTopology::TriangleList:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::TriangleStrip:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
            }
        }

        [[nodiscard]] constexpr VkBool32 EnablePrimitiveRestart(PrimitiveTopology topology)
        {
            switch (topology)
            {
                case PrimitiveTopology::LineStrip:
                case PrimitiveTopology::TriangleStrip:
                    return true;
                default:
                    return false;
            }
        }

        [[nodiscard]] constexpr VkVertexInputRate ToVulkan(VertexStepRate stepMode)
        {
            switch (stepMode)
            {
                case VertexStepRate::Instance:
                    return VK_VERTEX_INPUT_RATE_INSTANCE;
                default:
                    return VK_VERTEX_INPUT_RATE_VERTEX;
            }
        }

        [[nodiscard]] constexpr VkFormat ToVulkan(VertexFormat format)
        {
            switch (format)
            {
                case VertexFormat::Uint8x2:
                    return VK_FORMAT_R8G8_UINT;
                case VertexFormat::Uint8x4:
                    return VK_FORMAT_R8G8B8A8_UINT;
                case VertexFormat::Sint8x2:
                    return VK_FORMAT_R8G8_SINT;
                case VertexFormat::Sint8x4:
                    return VK_FORMAT_R8G8B8A8_SINT;
                case VertexFormat::Unorm8x2:
                    return VK_FORMAT_R8G8_UNORM;
                case VertexFormat::Unorm8x4:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                case VertexFormat::Snorm8x2:
                    return VK_FORMAT_R8G8_SNORM;
                case VertexFormat::Snorm8x4:
                    return VK_FORMAT_R8G8B8A8_SNORM;
                case VertexFormat::Uint16x2:
                    return VK_FORMAT_R16G16_UINT;
                case VertexFormat::Uint16x4:
                    return VK_FORMAT_R16G16B16A16_UINT;
                case VertexFormat::Sint16x2:
                    return VK_FORMAT_R16G16_SINT;
                case VertexFormat::Sint16x4:
                    return VK_FORMAT_R16G16B16A16_SINT;
                case VertexFormat::Unorm16x2:
                    return VK_FORMAT_R16G16_UNORM;
                case VertexFormat::Unorm16x4:
                    return VK_FORMAT_R16G16B16A16_UNORM;
                case VertexFormat::Snorm16x2:
                    return VK_FORMAT_R16G16_SNORM;
                case VertexFormat::Snorm16x4:
                    return VK_FORMAT_R16G16B16A16_SNORM;
                case VertexFormat::Float16x2:
                    return VK_FORMAT_R16G16_SFLOAT;
                case VertexFormat::Float16x4:
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                case VertexFormat::Float32:
                    return VK_FORMAT_R32_SFLOAT;
                case VertexFormat::Float32x2:
                    return VK_FORMAT_R32G32_SFLOAT;
                case VertexFormat::Float32x3:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexFormat::Float32x4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case VertexFormat::Uint32:
                    return VK_FORMAT_R32_UINT;
                case VertexFormat::Uint32x2:
                    return VK_FORMAT_R32G32_UINT;
                case VertexFormat::Uint32x3:
                    return VK_FORMAT_R32G32B32_UINT;
                case VertexFormat::Uint32x4:
                    return VK_FORMAT_R32G32B32A32_UINT;
                case VertexFormat::Sint32:
                    return VK_FORMAT_R32_SINT;
                case VertexFormat::Sint32x2:
                    return VK_FORMAT_R32G32_SINT;
                case VertexFormat::Sint32x3:
                    return VK_FORMAT_R32G32B32_SINT;
                case VertexFormat::Sint32x4:
                    return VK_FORMAT_R32G32B32A32_SINT;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr VkPolygonMode ToVulkan(FillMode mode)
        {
            switch (mode)
            {
                default:
                case FillMode::Solid:
                    return VK_POLYGON_MODE_FILL;
                case FillMode::Wireframe:
                    return VK_POLYGON_MODE_LINE;
            }
        }

        [[nodiscard]] constexpr VkCullModeFlagBits ToVulkan(CullMode mode)
        {
            switch (mode)
            {
                default:
                case CullMode::None:
                    return VK_CULL_MODE_NONE;
                case CullMode::Front:
                    return VK_CULL_MODE_FRONT_BIT;
                case CullMode::Back:
                    return VK_CULL_MODE_BACK_BIT;
            }
        }

        [[nodiscard]] constexpr VkFrontFace ToVulkan(FaceWinding winding)
        {
            switch (winding)
            {
                default:
                case FaceWinding::Clockwise:
                    return VK_FRONT_FACE_CLOCKWISE;
                case FaceWinding::CounterClockwise:
                    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            }
        }

        [[nodiscard]] constexpr VkStencilOp ToVulkan(StencilOperation op) {
            switch (op) {
                case StencilOperation::Keep:
                    return VK_STENCIL_OP_KEEP;
                case StencilOperation::Zero:
                    return VK_STENCIL_OP_ZERO;
                case StencilOperation::Replace:
                    return VK_STENCIL_OP_REPLACE;
                case StencilOperation::IncrementClamp:
                    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case StencilOperation::DecrementClamp:
                    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case StencilOperation::Invert:
                    return VK_STENCIL_OP_INVERT;
                case StencilOperation::IncrementWrap:
                    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case StencilOperation::DecrementWrap:
                    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr VkBlendFactor ToVulkan(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::Zero:
                    return VK_BLEND_FACTOR_ZERO;
                case BlendFactor::One:
                    return VK_BLEND_FACTOR_ONE;
                case BlendFactor::SourceColor:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case BlendFactor::OneMinusSourceColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case BlendFactor::SourceAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case BlendFactor::OneMinusSourceAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case BlendFactor::OneMinusDestinationColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case BlendFactor::DestinationAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case BlendFactor::OneMinusDestinationAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case BlendFactor::SourceAlphaSaturated:
                    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                case BlendFactor::BlendColor:
                    return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case BlendFactor::OneMinusBlendColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                case BlendFactor::Source1Color:
                    return VK_BLEND_FACTOR_SRC1_COLOR;
                case BlendFactor::OneMinusSource1Color:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
                case BlendFactor::Source1Alpha:
                    return VK_BLEND_FACTOR_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Alpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr VkBlendOp ToVulkan(BlendOperation operation)
        {
            switch (operation) {
                case BlendOperation::Add:
                    return VK_BLEND_OP_ADD;
                case BlendOperation::Subtract:
                    return VK_BLEND_OP_SUBTRACT;
                case BlendOperation::ReverseSubtract:
                    return VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendOperation::Min:
                    return VK_BLEND_OP_MIN;
                case BlendOperation::Max:
                    return VK_BLEND_OP_MAX;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        VkColorComponentFlags constexpr ToVulkan(ColorWriteMask mask)
        {
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Red) == VK_COLOR_COMPONENT_R_BIT, "Vulkan ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Green) == VK_COLOR_COMPONENT_G_BIT, "Vulkan ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Blue) == VK_COLOR_COMPONENT_B_BIT, "Vulkan ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Alpha) == VK_COLOR_COMPONENT_A_BIT, "Vulkan ColorWriteMask mismatch");
            return static_cast<VkColorComponentFlags>(mask);
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

        // Enumerate physical device and create logical device.
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

            std::vector<const char*> enabledExtensions;

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

                enabledExtensions.clear();
                enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                enabledExtensions.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

                if (physicalDeviceExt.memory_budget)
                {
                    enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
                }

                // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
                //if (physicalDeviceExt.performance_query &&
                //    physicalDeviceExt.host_query_reset)
                //{
                //    VkPhysicalDeviceFeatures2 physical_device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
                //    physical_device_features.pNext = &perf_counter_features;
                //    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);
                //
                //    physical_device_features.pNext = &host_query_reset_features;
                //    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);
                //
                //    if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset)
                //    {
                //        *features_chain = &perf_counter_features;
                //        features_chain = &perf_counter_features.pNext;
                //        *features_chain = &host_query_reset_features;
                //        features_chain = &host_query_reset_features.pNext;
                //
                //        enabledExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
                //        enabledExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
                //    }
                //}

                // Core 1.2.
                {
                    // Required by VK_KHR_spirv_1_4
                    enabledExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);

                    // Required for VK_KHR_ray_tracing_pipeline
                    enabledExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

                    enabledExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);

                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
                }

                if (physicalDeviceExt.accelerationStructure)
                {
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

                if (physicalDeviceExt.fragment_shading_rate)
                {
                    enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
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

            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

            ALIMER_ASSERT(properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE);
            ALIMER_ASSERT(features2.features.imageCubeArray == VK_TRUE);
            ALIMER_ASSERT(features2.features.independentBlend == VK_TRUE);
            ALIMER_ASSERT(features2.features.geometryShader == VK_TRUE);
            ALIMER_ASSERT(features2.features.samplerAnisotropy == VK_TRUE);
            ALIMER_ASSERT(features2.features.shaderClipDistance == VK_TRUE);
            ALIMER_ASSERT(features2.features.textureCompressionBC == VK_TRUE);
            ALIMER_ASSERT(features2.features.occlusionQueryPrecise == VK_TRUE);
            ALIMER_ASSERT(features_1_2.descriptorIndexing == VK_TRUE);

            caps.backendType = GraphicsAPI::Vulkan;
            caps.vendor = static_cast<GPUVendorId>(properties2.properties.vendorID);
            caps.adapterId = properties2.properties.deviceID;

            shaderFormat = ShaderFormat::SPIRV;

            switch (properties2.properties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    caps.adapterType = GPUAdapterType::Integrated;
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    caps.adapterType = GPUAdapterType::Discrete;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    caps.adapterType = GPUAdapterType::Software;
                    break;

                default:
                    caps.adapterType = GPUAdapterType::Unknown;
                    break;
            }

            caps.adapterName = properties2.properties.deviceName;
            //caps.blobType = ShaderBlobType::SPIRV;

            if (features2.features.tessellationShader == VK_TRUE)
            {
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
                features_1_2.bufferDeviceAddress == VK_TRUE)
            {
                caps.features.rayTracing = true;
                //SHADER_IDENTIFIER_SIZE = raytracing_properties.shaderGroupHandleSize;
            }

            if (mesh_shader_features.meshShader == VK_TRUE &&
                mesh_shader_features.taskShader == VK_TRUE)
            {
                caps.features.meshShader = true;
            }

            if (fragment_shading_rate_features.pipelineFragmentShadingRate == VK_TRUE)
            {
                caps.features.variableRateShading = true;
            }
            if (fragment_shading_rate_features.attachmentFragmentShadingRate == VK_TRUE)
            {
                caps.features.variableRateShadingTier2 = true;
                //VARIABLE_RATE_SHADING_TILE_SIZE = std::min(fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.width, fragment_shading_rate_properties.maxFragmentShadingRateAttachmentTexelSize.height);
            }

            VkFormatProperties formatProperties = {};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, ToVulkanFormat(PixelFormat::RG11B10Float), &formatProperties);
            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            {
                //capabilities |= GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT;
            }

            // Limits
            caps.limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
            caps.limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
            caps.limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
            caps.limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
            caps.limits.maxTextureArraySize = properties2.properties.limits.maxImageArrayLayers;
            caps.limits.minConstantBufferOffsetAlignment = properties2.properties.limits.minUniformBufferOffsetAlignment;
            caps.limits.minStorageBufferOffsetAlignment = properties2.properties.limits.minStorageBufferOffsetAlignment;
            caps.limits.maxDrawIndirectCount = properties2.properties.limits.maxDrawIndirectCount;

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

            vkGetDeviceQueue(device, graphicsQueueFamily, graphicsQueueIndex, &graphicsQueue);
            vkGetDeviceQueue(device, computeQueueFamily, computeQueueIndex, &computeQueue);
            vkGetDeviceQueue(device, copyQueueFamily, copyQueueIndex, &copyQueue);

            LOGI("Vendor : {}", ToString((GPUVendorId)properties2.properties.vendorID));
            LOGI("Name   : {}", properties2.properties.deviceName);
            LOGI("Type   : {}", ToString(caps.adapterType));
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

        // Command queues
        {
            queues[(uint8_t)QueueType::Graphics].queue = graphicsQueue;
            queues[(uint8_t)QueueType::Compute].queue = computeQueue;

            VkSemaphoreTypeCreateInfo timelineSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
            timelineSemaphoreInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineSemaphoreInfo.initialValue = 0;

            VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            semaphoreInfo.pNext = &timelineSemaphoreInfo;
            semaphoreInfo.flags = 0;

            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &queues[(uint8_t)QueueType::Graphics].semaphore));
            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &queues[(uint8_t)QueueType::Compute].semaphore));
        }

        // Create memory allocator
        {
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = physicalDevice;
            allocatorInfo.device = device;
            allocatorInfo.instance = instance;
            allocatorInfo.vulkanApiVersion = volkGetInstanceVersion();

            // Core in 1.1
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create allocator");
            }
        }

        // Init copy allocator.
        copyAllocator.Init(this);

        // Create frame resources
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            for (uint8_t queue = 0; queue < (uint8_t)QueueType::Count; ++queue)
            {
                const VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
                VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frames[i].fence[queue]));
            }

            // Create resources for transition command buffer:
            {
                VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
                poolInfo.queueFamilyIndex = graphicsQueueFamily;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].initCommandPool));

                VkCommandBufferAllocateInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
                commandBufferInfo.commandPool = frames[i].initCommandPool;
                commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                commandBufferInfo.commandBufferCount = 1;
                VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &frames[i].initCommandBuffer));

                VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = nullptr; // Optional
                VK_CHECK(vkBeginCommandBuffer(frames[i].initCommandBuffer, &beginInfo));
            }
        }

        // Dynamic PSO states:
        pso_dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        pso_dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        pso_dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        pso_dynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        if (caps.features.variableRateShading)
        {
            pso_dynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
        }
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = (uint32_t)pso_dynamicStates.size();
        dynamicStateInfo.pDynamicStates = pso_dynamicStates.data();

        // Create bindless
        bindlessSampledImages.Init(device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, properties_1_2.maxDescriptorSetUpdateAfterBindSampledImages / 4);
        bindlessUniformTexelBuffers.Init(device, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, properties_1_2.maxDescriptorSetUpdateAfterBindSampledImages / 4);
        bindlessStorageBuffers.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, properties_1_2.maxDescriptorSetUpdateAfterBindStorageBuffers / 4);
        bindlessStorageImages.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, properties_1_2.maxDescriptorSetUpdateAfterBindStorageImages / 4);
        bindlessStorageTexelBuffers.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, properties_1_2.maxDescriptorSetUpdateAfterBindStorageImages / 4);
        bindlessSamplers.Init(device, VK_DESCRIPTOR_TYPE_SAMPLER, 256);

        if (caps.features.rayTracing)
        {
            bindlessAccelerationStructures.Init(device, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 32);
        }

        // Null objects
        {
            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = 4;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.flags = 0;
            VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &nullBuffer, &nullBufferAllocation, nullptr));

            VkBufferViewCreateInfo bufferViewInfo{ VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
            bufferViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            bufferViewInfo.range = VK_WHOLE_SIZE;
            bufferViewInfo.buffer = nullBuffer;
            VK_CHECK(vkCreateBufferView(device, &bufferViewInfo, nullptr, &nullBufferView));

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
            pendingSubmitInits = true;
            initLocker.unlock();

            VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageViewInfo.image = nullImage1D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView1D));

            imageViewInfo.image = nullImage1D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView1DArray));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView2D));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView2DArray));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            imageViewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageViewCube));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            imageViewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageViewCubeArray));

            imageViewInfo.image = nullImage3D;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView3D));

            SamplerDesc samplerDesc{};
            nullSampler = CreateSampler(samplerDesc);
        }

        timestampFrequency = uint64_t(1.0 / double(properties2.properties.limits.timestampPeriod) * 1000 * 1000 * 1000);

        OnCreated();
        LOGI("Vulkan graphics backend initialized with success");
    }

    VulkanGraphics::~VulkanGraphics()
    {
        VK_CHECK(vkDeviceWaitIdle(device));

        for (uint8_t queue = 0; queue < (uint8_t)QueueType::Count; ++queue)
        {
            vkDestroySemaphore(device, queues[queue].semaphore, nullptr);
            for (uint32_t cmd = 0; cmd < kMaxCommandLists; ++cmd)
            {
                commandLists[cmd][queue].reset();
            }
        }

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

        // PipelineLayout cache
        {
            std::lock_guard<std::mutex> guard(pipelineLayoutCacheMutex);
            for (auto& it : pipelineLayoutCache)
            {
                vkDestroyPipelineLayout(device, it.second.pipelineLayout, nullptr);
                vkDestroyDescriptorSetLayout(device, it.second.descriptorSetLayout, nullptr);
            }
        }

        for (auto& frame : frames)
        {
            for (uint8_t queue = 0; queue < (uint8_t)QueueType::Count; ++queue)
            {
                vkDestroyFence(device, frame.fence[queue], nullptr);
            }
            vkDestroyCommandPool(device, frame.initCommandPool, nullptr);
        }

        // Shutdown copy allocator.
        copyAllocator.Shutdown();

        // Bindless data
        {
            bindlessSampledImages.Destroy(device);
            bindlessUniformTexelBuffers.Destroy(device);
            bindlessStorageBuffers.Destroy(device);
            bindlessStorageImages.Destroy(device);
            bindlessStorageTexelBuffers.Destroy(device);
            bindlessSamplers.Destroy(device);
            bindlessAccelerationStructures.Destroy(device);
        }

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
            nullSampler.Reset();
        }

        frameCount = UINT64_MAX;
        ProcessDeletionQueue();
        frameCount = 0;

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

    bool VulkanGraphics::BeginFrame()
    {
        cmdBuffersCount.store(0);
        return true;
    }

    void VulkanGraphics::EndFrame()
    {
        SubmitCommandBuffers();

        frameCount++;
        frameIndex = frameCount % kMaxFramesInFlight;

        // Begin next frame:
        {
            auto& frame = GetFrameResources();

            // Initiate stalling CPU when GPU is not yet finished with next frame:
            if (frameCount >= kMaxFramesInFlight)
            {
                for (uint8_t queue = 0; queue < (uint8_t)QueueType::Count; ++queue)
                {
                    VK_CHECK(vkWaitForFences(device, 1, &frame.fence[queue], VK_TRUE, UINT64_MAX));
                    VK_CHECK(vkResetFences(device, 1, &frame.fence[queue]));
                }
            }

            ProcessDeletionQueue();

            // Restart transition command buffers
            {
                initLocker.lock();
                VK_CHECK(vkResetCommandPool(device, frame.initCommandPool, 0));

                VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = nullptr; // Optional

                VK_CHECK(vkBeginCommandBuffer(frame.initCommandBuffer, &beginInfo));

                pendingSubmitInits = false;
                initLocker.unlock();
            }
        }


        // Reset frame command pools
        //ToVulkan(graphicsQueue)->Reset(frameIndex);
        //ToVulkan(computeQueue)->Reset(frameIndex);
    }

    void VulkanGraphics::SubmitCommandBuffers()
    {
        initLocker.lock();

        // Submit current frame.
        {
            auto& frame = GetFrameResources();

            QueueType submitQueue = QueueType::Count;

            // Transitions first.
            if (pendingSubmitInits)
            {
                VK_CHECK(vkEndCommandBuffer(frame.initCommandBuffer));
            }

            // Sync with copy queue
            uint64_t copySyncFence = copyAllocator.Flush();

            uint8_t cmd_last = cmdBuffersCount.load();
            cmdBuffersCount.store(0);
            for (uint8_t cmd = 0; cmd < cmd_last; ++cmd)
            {
                VulkanCommandBuffer* commandBuffer = GetCommandBuffer(cmd);
                VK_CHECK(vkEndCommandBuffer(commandBuffer->GetHandle()));

                const CommandListMetadata& meta = commandListMeta[cmd];
                if (submitQueue == QueueType::Count) // start first batch
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

                if (pendingSubmitInits)
                {
                    queues[(uint8_t)submitQueue].submit_cmds.push_back(frame.initCommandBuffer);
                    pendingSubmitInits = false;
                }

                for (auto& swapchain : commandBuffer->swapChains)
                {
                    queues[(uint8_t)submitQueue].submit_swapchains.push_back(swapchain);
                    queues[(uint8_t)submitQueue].submitVkSwapchains.push_back(swapchain->GetHandle());
                    queues[(uint8_t)submitQueue].submit_swapChainImageIndices.push_back(swapchain->GetBackBufferIndex());
                    queues[(uint8_t)submitQueue].submit_swapChainResults.push_back(VK_SUCCESS);
                    queues[(uint8_t)submitQueue].submit_waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    queues[(uint8_t)submitQueue].submit_waitSemaphores.push_back(swapchain->GetImageAvailableSemaphore());
                    queues[(uint8_t)submitQueue].submit_waitValues.push_back(0); // not a timeline semaphore
                    queues[(uint8_t)submitQueue].submit_signalSemaphores.push_back(swapchain->GetRenderCompleteSemaphore());
                    queues[(uint8_t)submitQueue].submit_signalValues.push_back(0); // not a timeline semaphore
                }

                queues[(uint8_t)submitQueue].submit_cmds.push_back(commandBuffer->GetHandle());
            }

            // Final submits with fences
            for (uint8_t queue = 0; queue < (uint8_t)QueueType::Count; ++queue)
            {
                queues[queue].Submit(frame.fence[queue]);
            }
        }

        pendingSubmitInits = false;
        initLocker.unlock();
    }

    CommandBuffer* VulkanGraphics::BeginCommandBuffer(QueueType queueType)
    {
        uint8_t cmd = cmdBuffersCount.fetch_add(1);
        assert(cmd < kMaxCommandLists);
        commandListMeta[cmd].queue = queueType;
        commandListMeta[cmd].waits.clear();

        if (GetCommandBuffer(cmd) == nullptr)
        {
            commandLists[cmd][(uint8_t)queueType] = std::make_unique<VulkanCommandBuffer>(*this, queueType, cmd);
        }

        GetCommandBuffer(cmd)->Reset(frameIndex);

        return GetCommandBuffer(cmd);
    }

    VulkanUploadContext VulkanGraphics::UploadBegin(uint64_t size)
    {
        return copyAllocator.Allocate(size);
    }

    void VulkanGraphics::UploadEnd(VulkanUploadContext context)
    {
        copyAllocator.Submit(context);
    }

    TextureRef VulkanGraphics::CreateTextureCore(const TextureCreateInfo& info, const TextureData* initialData)
    {
        auto result = new VulkanTexture(*this, info, nullptr, initialData);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    BufferRef VulkanGraphics::CreateBuffer(const BufferCreateInfo* info, const void* initialData)
    {
        RefPtr<VulkanBuffer> buffer(new VulkanBuffer(info));
        buffer->device = this;

        if (info->handle)
        {
            buffer->handle = (VkBuffer)info->handle;

            if (features_1_2.bufferDeviceAddress == VK_TRUE)
            {
                VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                info.buffer = buffer->handle;
                buffer->deviceAddress = vkGetBufferDeviceAddress(device, &info);
            }

            return buffer;
        }

        VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.size = info->size;
        createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if ((info->usage & BufferUsage::Vertex) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if ((info->usage & BufferUsage::Index) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if ((info->usage & BufferUsage::Constant) != 0)
        {
            // Align the buffer size to multiples of the dynamic uniform buffer minimum size
            uint64_t minAlignment = gGraphics().GetCaps().limits.minConstantBufferOffsetAlignment;
            createInfo.size = AlignUp(createInfo.size, minAlignment);

            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if ((info->usage & BufferUsage::ShaderRead) != 0)
        {
            if (info->format == PixelFormat::Undefined)
            {
                createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            else
            {
                createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            }
        }

        if ((info->usage & BufferUsage::ShaderWrite) != 0)
        {
            if (info->format == PixelFormat::Undefined)
            {
                createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            else
            {
                createInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            }
        }

        if ((info->usage & BufferUsage::Indirect) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
        if ((info->usage & BufferUsage::RayTracingAccelerationStructure) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        }
        if ((info->usage & BufferUsage::RayTracingShaderTable) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        if (features_1_2.bufferDeviceAddress == VK_TRUE)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.flags = 0;
        memoryInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;

        switch (info->cpuAccess)
        {
            case CpuAccessMode::Write:
                memoryInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                memoryInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                memoryInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case CpuAccessMode::Read:
                memoryInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                memoryInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                memoryInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                break;

            case CpuAccessMode::None:
            default:
                memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                memoryInfo.requiredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                break;
        }

        uint32_t sharingIndices[3];
        if (graphicsQueueFamily != computeQueueFamily
            || graphicsQueueFamily != copyQueueFamily)
        {
            // For buffers, always just use CONCURRENT access modes,
            // so we don't have to deal with acquire/release barriers in async compute.
            createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

            sharingIndices[createInfo.queueFamilyIndexCount++] = graphicsQueueFamily;

            if (graphicsQueueFamily != computeQueueFamily)
            {
                sharingIndices[createInfo.queueFamilyIndexCount++] = computeQueueFamily;
            }

            if (graphicsQueueFamily != copyQueueFamily
                && computeQueueFamily != copyQueueFamily)
            {
                sharingIndices[createInfo.queueFamilyIndexCount++] = copyQueueFamily;
            }

            createInfo.pQueueFamilyIndices = sharingIndices;
        }

        VmaAllocationInfo allocationInfo{};
        VkResult result = vmaCreateBuffer(allocator,
            &createInfo, &memoryInfo,
            &buffer->handle, &buffer->allocation, &allocationInfo);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create buffer.");
            return nullptr;
        }

        if (info->label != nullptr)
        {
            SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer->handle, info->label);
        }

        buffer->allocatedSize = allocationInfo.size;

        if (createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            info.buffer = buffer->handle;
            buffer->deviceAddress = vkGetBufferDeviceAddress(device, &info);
        }

        if (memoryInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
        {
            buffer->mappedData = static_cast<uint8_t*>(allocationInfo.pMappedData);
        }

        if (initialData != nullptr)
        {
            VulkanUploadContext context = copyAllocator.Allocate(info->size);
            memcpy(context.data, initialData, info->size);

            VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = buffer->handle;
            barrier.offset = 0;
            barrier.size = VK_WHOLE_SIZE;

            vkCmdPipelineBarrier(
                context.commandBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                1, &barrier,
                0, nullptr
            );

            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = info->size;

            vkCmdCopyBuffer(
                context.commandBuffer,
                ToVulkan(context.uploadBuffer.Get())->handle,
                buffer->handle,
                1,
                &copyRegion
            );

            VkAccessFlags tmp = barrier.srcAccessMask;
            barrier.srcAccessMask = barrier.dstAccessMask;
            barrier.dstAccessMask = 0;

            if ((info->usage & BufferUsage::Vertex) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            }
            if ((info->usage & BufferUsage::Index) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
            }
            if ((info->usage & BufferUsage::Constant) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
            }
            if ((info->usage & BufferUsage::ShaderRead) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
            }
            if ((info->usage & BufferUsage::ShaderWrite) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
            }
            if ((info->usage & BufferUsage::Indirect) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            }
            if ((info->usage & BufferUsage::RayTracingAccelerationStructure) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            }
            if ((info->usage & BufferUsage::RayTracingShaderTable) != 0)
            {
                barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            }

            vkCmdPipelineBarrier(
                context.commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                1, &barrier,
                0, nullptr
            );

            copyAllocator.Submit(context);
        }

        return buffer;
    }

    ShaderRef VulkanGraphics::CreateShader(ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint)
    {
        RefPtr<VulkanShader> shader(new VulkanShader(stage, entryPoint));
        shader->device = this;

        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(byteCodeLength, byteCode, &module);
        ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

        const SpvReflectEntryPoint* spvEntryPoint = spvReflectGetEntryPoint(&module, entryPoint.c_str());
        ALIMER_ASSERT(spvEntryPoint != nullptr);
        ALIMER_ASSERT(module.entry_point_count == 1);

        //stage = ConvertShaderStage(spvEntryPoint->spirv_execution_model);

        // Bindings
        {
            uint32_t count = 0;
            result = spvReflectEnumerateEntryPointDescriptorBindings(&module, entryPoint.c_str(), &count, NULL);
            ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorBinding*> bindings(count);
            result = spvReflectEnumerateEntryPointDescriptorBindings(&module, entryPoint.c_str(), &count, bindings.data());
            ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

            const uint32_t bindlessSet = 1;
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (auto& binding : bindings)
            {
                const bool bindless = binding->set > 0;

                ShaderResource resource{};
                resource.type = GetShaderResourceType(binding->descriptor_type);
                resource.stages |= stage;
                resource.name = binding->name;

                resource.arraySize = binding->count;
                for (uint32_t i_dim = 0; i_dim < binding->array.dims_count; ++i_dim) {
                    resource.arraySize *= binding->array.dims[i_dim];
                }
                resource.set = binding->set;
                resource.binding = binding->binding;

                if (binding->resource_type == SPV_REFLECT_RESOURCE_FLAG_CBV)
                {
                    resource.offset = binding->block.offset;
                    resource.size = binding->block.size;
                }

                if (!bindless)
                {
                    switch (binding->resource_type)
                    {
                        case SPV_REFLECT_RESOURCE_FLAG_SAMPLER:
                            spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_Sampler, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
                            break;

                        case SPV_REFLECT_RESOURCE_FLAG_CBV: // Unchanged
                            spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_CBV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
                            break;

                        case SPV_REFLECT_RESOURCE_FLAG_SRV:
                            spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_SRV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
                            break;
                        case SPV_REFLECT_RESOURCE_FLAG_UAV:
                            spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_UAV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
                            break;
                    }
                }

                resource.backend_binding = binding->binding;
                shader->resources.push_back(resource);

                // WIP:
                if (bindless)
                {
                    // There can be padding between bindless spaces because sets need to be bound contiguously
                    shader->bindlessBindings.resize(Max(shader->bindlessBindings.size(), (size_t)binding->set));
                }

                auto& descriptor = bindless ? shader->bindlessBindings[binding->set - 1] : shader->layoutBindings.emplace_back();
                descriptor.stageFlags = (VkShaderStageFlags)module.shader_stage;
                descriptor.binding = binding->binding;
                descriptor.descriptorCount = binding->count;
                descriptor.descriptorType = (VkDescriptorType)binding->descriptor_type;

                if (bindless)
                    continue;

                auto& imageViewType = shader->imageViewTypes.emplace_back();
                imageViewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

                if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
                {
                    // TODO: static samplers
                }

                switch (binding->descriptor_type)
                {
                    default:
                        break;
                    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        switch (binding->image.dim)
                        {
                            default:
                            case SpvDim1D:
                                if (binding->image.arrayed == 0)
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_1D;
                                }
                                else
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                                }
                                break;
                            case SpvDim2D:
                                if (binding->image.arrayed == 0)
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_2D;
                                }
                                else
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                                }
                                break;
                            case SpvDim3D:
                                imageViewType = VK_IMAGE_VIEW_TYPE_3D;
                                break;
                            case SpvDimCube:
                                if (binding->image.arrayed == 0)
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
                                }
                                else
                                {
                                    imageViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                                }
                                break;
                        }
                        break;
                }
            }
        }

        // Push constants
        {
            uint32_t pushCount = 0;
            result = spvReflectEnumeratePushConstantBlocks(&module, &pushCount, nullptr);
            ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectBlockVariable*> pushConstants(pushCount);
            result = spvReflectEnumeratePushConstantBlocks(&module, &pushCount, pushConstants.data());
            ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

            for (auto& pushConstant : pushConstants)
            {
                ShaderResource resource{};
                resource.type = ShaderResourceType::PushConstant;
                resource.stages |= stage;
                resource.name = pushConstant->name;
                resource.offset = pushConstant->offset;
                resource.size = pushConstant->size;
                shader->resources.push_back(resource);

                // WIP
                VkPushConstantRange& pushConstantRange = shader->pushConstantRanges;
                pushConstantRange.stageFlags = (VkShaderStageFlags)module.shader_stage;
                pushConstantRange.offset = pushConstant->offset;
                pushConstantRange.size = pushConstant->size;
            }
        }

        std::vector<uint8_t> modifiedByteCode(spvReflectGetCodeSize(&module));
        memcpy(modifiedByteCode.data(), spvReflectGetCode(&module), modifiedByteCode.size());
        spvReflectDestroyShaderModule(&module);

        // Create the Vulkan ShaderModule.
        {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = modifiedByteCode.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(modifiedByteCode.data());
            VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shader->handle);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create ShaderModule");
                return nullptr;
            }
        }

        std::hash<std::string> hasher{};
        shader->hash = hasher(std::string{ modifiedByteCode.cbegin(), modifiedByteCode.cend() });
        return shader;
    }

    SamplerRef VulkanGraphics::CreateSampler(const SamplerDesc& desc)
    {
        VkSamplerCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.magFilter = ToVulkan(desc.magFilter);
        createInfo.minFilter = ToVulkan(desc.minFilter);
        createInfo.mipmapMode = ToVulkanMipmapMode(desc.mipFilter);
        createInfo.addressModeU = ToVulkan(desc.addressModeU);
        createInfo.addressModeV = ToVulkan(desc.addressModeV);
        createInfo.addressModeW = ToVulkan(desc.addressModeW);
        createInfo.mipLodBias = desc.mipLodBias;

        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
        if (desc.maxAnisotropy > 1)
        {
            createInfo.anisotropyEnable = VK_TRUE;
            createInfo.maxAnisotropy = Min(static_cast<float>(desc.maxAnisotropy), properties2.properties.limits.maxSamplerAnisotropy);
        }
        else
        {
            createInfo.anisotropyEnable = VK_FALSE;
            createInfo.maxAnisotropy = 1;
        }

        if (desc.compareFunction != CompareFunction::Undefined)
        {
            createInfo.compareOp = ToVkCompareOp(desc.compareFunction);
            createInfo.compareEnable = VK_TRUE;
        }
        else
        {
            createInfo.compareOp = VK_COMPARE_OP_NEVER;
            createInfo.compareEnable = VK_FALSE;
        }

        createInfo.minLod = desc.minLod;
        createInfo.maxLod = desc.maxLod;
        createInfo.borderColor = ToVulkan(desc.borderColor);
        createInfo.unnormalizedCoordinates = VK_FALSE;

        // Reduction
        VkSamplerReductionModeCreateInfo samplerReductionCreateInfo{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO };
        if (features_1_2.samplerFilterMinmax)
        {
            samplerReductionCreateInfo.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
            createInfo.pNext = &samplerReductionCreateInfo;
        }

        VkSampler handle;
        VkResult result = vkCreateSampler(device, &createInfo, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create sampler.");
            return nullptr;
        }

        if (desc.label != nullptr)
        {
            SetObjectName(VkObjectType::VK_OBJECT_TYPE_SAMPLER, (uint64_t)handle, desc.label);
        }

        auto sampler = new VulkanSampler();
        sampler->device = this;
        sampler->handle = handle;

        sampler->bindlessIndex = bindlessSamplers.Allocate();
        if (sampler->bindlessIndex == kInvalidBindlessIndex)
        {
            LOGE("Vulkan: Cannot allocate bindless index for sampler");
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = handle;

        VkWriteDescriptorSet descriptorSetWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorSetWrite.dstBinding = 0;
        descriptorSetWrite.dstArrayElement = sampler->bindlessIndex;
        descriptorSetWrite.descriptorCount = 1;
        descriptorSetWrite.dstSet = bindlessSamplers.descriptorSet;
        descriptorSetWrite.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(device, 1, &descriptorSetWrite, 0, nullptr);

        return SamplerRef(sampler);
    }

    PipelineRef VulkanGraphics::CreateRenderPipeline(const RenderPipelineStateCreateInfo* info)
    {
        RefPtr<VulkanPipeline> pipeline(new VulkanPipeline());
        pipeline->device = this;
        pipeline->type = PipelineType::Render;

        auto InsertShader = [&](const Shader* shader) {
            if (shader == nullptr)
                return;

            const VulkanShader* shaderVulkan = checked_cast<const VulkanShader*>(shader);

            uint32_t i = 0;
            size_t check_max = shaderVulkan->layoutBindings.size(); // dont't check for duplicates within self table
            for (const VkDescriptorSetLayoutBinding& shaderLayoutBinding : shaderVulkan->layoutBindings)
            {
                bool found = false;
                size_t j = 0;
                for (VkDescriptorSetLayoutBinding& pipelineLayoutBinding : pipeline->layoutBindings)
                {
                    if (shaderLayoutBinding.binding == pipelineLayoutBinding.binding)
                    {
                        // If the asserts fire, it means there are overlapping bindings between shader stages
                        //	This is not supported now for performance reasons (less binding management)!
                        //	(Overlaps between s/b/t bind points are not a problem because those are shifted by the compiler)
                        ALIMER_ASSERT(shaderLayoutBinding.descriptorCount == pipelineLayoutBinding.descriptorCount);
                        ALIMER_ASSERT(shaderLayoutBinding.descriptorType == pipelineLayoutBinding.descriptorType);
                        found = true;
                        pipelineLayoutBinding.stageFlags |= shaderLayoutBinding.stageFlags;
                        break;
                    }
                    if (j++ >= check_max)
                        break;
                }

                if (!found)
                {
                    pipeline->layoutBindings.push_back(shaderLayoutBinding);
                    pipeline->imageViewTypes.push_back(shaderVulkan->imageViewTypes[i]);
                }
                i++;
            }

            if (shaderVulkan->pushConstantRanges.size > 0)
            {
                pipeline->pushConstantRanges.offset = Min(pipeline->pushConstantRanges.offset, shaderVulkan->pushConstantRanges.offset);
                pipeline->pushConstantRanges.size = Max(pipeline->pushConstantRanges.size, shaderVulkan->pushConstantRanges.size);
                pipeline->pushConstantRanges.stageFlags |= shaderVulkan->pushConstantRanges.stageFlags;
            }
        };

        auto InsertShaderBindless = [&](const Shader* shader) {
            if (shader == nullptr)
                return;

            const VulkanShader* shaderVulkan = checked_cast<const VulkanShader*>(shader);

            pipeline->bindlessBindings.resize(Max(pipeline->bindlessBindings.size(), shaderVulkan->bindlessBindings.size()));

            int i = 0;
            for (const VkDescriptorSetLayoutBinding& shaderLayoutBinding : shaderVulkan->bindlessBindings)
            {
                if (pipeline->bindlessBindings[i].descriptorType != shaderLayoutBinding.descriptorType)
                {
                    pipeline->bindlessBindings[i] = shaderLayoutBinding;
                }
                else
                {
                    pipeline->bindlessBindings[i].stageFlags |= shaderLayoutBinding.stageFlags;
                }
                i++;
            }
        };

        //InsertShader(pDesc->ms);
        //InsertShader(pDesc->as);
        InsertShader(info->vertexShader);
        //InsertShader(pDesc->hs);
        //InsertShader(pDesc->ds);
        //InsertShader(pDesc->gs);
        InsertShader(info->pixelShader);

        //InsertShaderBindless(pDesc->ms);
        //InsertShaderBindless(pDesc->as);
        InsertShaderBindless(info->vertexShader);
        //InsertShaderBindless(pDesc->hs);
        //InsertShaderBindless(pDesc->ds);
        //InsertShaderBindless(pDesc->gs);
        InsertShaderBindless(info->pixelShader);

        std::vector<VulkanShader*> shaders;
        shaders.push_back(ToVulkan(info->vertexShader));

        if (info->pixelShader != nullptr)
        {
            shaders.push_back(ToVulkan(info->pixelShader));
        }

        pipeline->layoutBindingHash = 0;
        size_t i = 0;
        for (auto& x : pipeline->layoutBindings)
        {
            HashCombine(pipeline->layoutBindingHash, x.binding);
            HashCombine(pipeline->layoutBindingHash, x.descriptorType);
            HashCombine(pipeline->layoutBindingHash, x.descriptorCount);
            HashCombine(pipeline->layoutBindingHash, x.stageFlags);
            HashCombine(pipeline->layoutBindingHash, pipeline->imageViewTypes[i++]);
        }
        for (auto& x : pipeline->bindlessBindings)
        {
            HashCombine(pipeline->layoutBindingHash, x.binding);
            HashCombine(pipeline->layoutBindingHash, x.descriptorType);
            HashCombine(pipeline->layoutBindingHash, x.descriptorCount);
            HashCombine(pipeline->layoutBindingHash, x.stageFlags);
        }
        HashCombine(pipeline->layoutBindingHash, pipeline->pushConstantRanges.offset);
        HashCombine(pipeline->layoutBindingHash, pipeline->pushConstantRanges.size);
        HashCombine(pipeline->layoutBindingHash, pipeline->pushConstantRanges.stageFlags);

        // Request DescriptorSetLayout (+ bindless) + pipeline layout
        pipelineLayoutCacheMutex.lock();
        if (pipelineLayoutCache[pipeline->layoutBindingHash].pipelineLayout == VK_NULL_HANDLE)
        {
            // Not found -> create new
            std::vector<VkDescriptorSetLayout> setLayouts;
            {
                VkDescriptorSetLayoutCreateInfo descriptorSetlayoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                descriptorSetlayoutInfo.pBindings = pipeline->layoutBindings.data();
                descriptorSetlayoutInfo.bindingCount = static_cast<uint32_t>(pipeline->layoutBindings.size());

                VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptorSetlayoutInfo, nullptr, &pipeline->descriptorSetLayout));
                pipelineLayoutCache[pipeline->layoutBindingHash].descriptorSetLayout = pipeline->descriptorSetLayout;
                setLayouts.push_back(pipeline->descriptorSetLayout);
            }

            pipelineLayoutCache[pipeline->layoutBindingHash].bindlessFirstSet = (uint32_t)setLayouts.size();
            for (const VkDescriptorSetLayoutBinding& x : pipeline->bindlessBindings)
            {
                switch (x.descriptorType)
                {
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        assert(0); // not supported, use the raw buffers for same functionality
                        break;
                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        setLayouts.push_back(bindlessSampledImages.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessSampledImages.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        setLayouts.push_back(bindlessUniformTexelBuffers.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessUniformTexelBuffers.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        setLayouts.push_back(bindlessStorageBuffers.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessStorageBuffers.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        setLayouts.push_back(bindlessStorageImages.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessStorageImages.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        setLayouts.push_back(bindlessStorageTexelBuffers.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessStorageTexelBuffers.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        setLayouts.push_back(bindlessSamplers.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessSamplers.descriptorSet);
                        break;
                    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        setLayouts.push_back(bindlessAccelerationStructures.descriptorSetLayout);
                        pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets.push_back(bindlessAccelerationStructures.descriptorSet);
                        break;
                    default:
                        break;
                }
            }

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
            pipelineLayoutInfo.pSetLayouts = setLayouts.data();
            if (pipeline->pushConstantRanges.size > 0)
            {
                pipelineLayoutInfo.pushConstantRangeCount = 1;
                pipelineLayoutInfo.pPushConstantRanges = &pipeline->pushConstantRanges;
            }
            
            VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline->pipelineLayout));
            pipelineLayoutCache[pipeline->layoutBindingHash].pipelineLayout = pipeline->pipelineLayout;
        }
        pipeline->descriptorSetLayout = pipelineLayoutCache[pipeline->layoutBindingHash].descriptorSetLayout;
        pipeline->pipelineLayout = pipelineLayoutCache[pipeline->layoutBindingHash].pipelineLayout;
        pipeline->bindlessSets = pipelineLayoutCache[pipeline->layoutBindingHash].bindlessSets;
        pipeline->bindlessFirstSet = pipelineLayoutCache[pipeline->layoutBindingHash].bindlessFirstSet;
        pipelineLayoutCacheMutex.unlock();

        uint32_t vertexBindingCount = 0;
        uint32_t vertexAttributeCount = 0;
        VkVertexInputBindingDescription vertexBindings[kMaxVertexBufferBindings];
        VkVertexInputAttributeDescription vertexAttributes[kMaxVertexAttributes];

        for (uint32_t index = 0; index < kMaxVertexBufferBindings; ++index)
        {
            if (!info->vertexLayout.buffers[index].stride)
                break;

            const VertexBufferLayout* layout = &info->vertexLayout.buffers[index];
            VkVertexInputBindingDescription* bindingDesc = &vertexBindings[vertexBindingCount++];
            bindingDesc->binding = index;
            bindingDesc->stride = layout->stride;
            bindingDesc->inputRate = ToVulkan(layout->stepRate);
        }

        for (uint32_t index = 0; index < kMaxVertexAttributes; index++) {
            const VertexAttribute* attribute = &info->vertexLayout.attributes[index];
            if (attribute->format == VertexFormat::Undefined) {
                continue;
            }

            VkVertexInputAttributeDescription* attributeDesc = &vertexAttributes[vertexAttributeCount++];
            attributeDesc->location = index;
            attributeDesc->binding = attribute->bufferIndex;
            attributeDesc->format = ToVulkan(attribute->format);
            attributeDesc->offset = attribute->offset;
        }

        VulkanRenderPassKey renderPassKey;
        renderPassKey.depthStencilAttachment.format = info->depthStencilFormat;
        renderPassKey.depthStencilAttachment.loadAction = LoadAction::Discard;
        renderPassKey.depthStencilAttachment.storeAction = StoreAction::Discard;
        renderPassKey.sampleCount = info->sampleCount;

        // BlendState
        std::array<VkPipelineColorBlendAttachmentState, kMaxColorAttachments> blendAttachmentState = {};

        VkPipelineColorBlendStateCreateInfo blendState{};
        blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendState.logicOpEnable = VK_FALSE;
        blendState.logicOp = VK_LOGIC_OP_CLEAR;
        blendState.attachmentCount = 0;
        blendState.pAttachments = blendAttachmentState.data();

        for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
        {
            if (info->colorFormats[i] == PixelFormat::Undefined)
                break;

            uint32_t rtIndex = 0;
            if (info->blendState.independentBlendEnable)
                rtIndex = i;

            const RenderTargetBlendState& renderTarget = info->blendState.renderTargets[rtIndex];

            blendAttachmentState[blendState.attachmentCount].blendEnable = EnableBlend(renderTarget) ? VK_TRUE : VK_FALSE;
            blendAttachmentState[blendState.attachmentCount].srcColorBlendFactor = ToVulkan(renderTarget.srcBlend);
            blendAttachmentState[blendState.attachmentCount].dstColorBlendFactor = ToVulkan(renderTarget.destBlend);
            blendAttachmentState[blendState.attachmentCount].colorBlendOp = ToVulkan(renderTarget.blendOperation);
            blendAttachmentState[blendState.attachmentCount].srcAlphaBlendFactor = ToVulkan(renderTarget.srcBlendAlpha);
            blendAttachmentState[blendState.attachmentCount].dstAlphaBlendFactor = ToVulkan(renderTarget.destBlendAlpha);
            blendAttachmentState[blendState.attachmentCount].alphaBlendOp = ToVulkan(renderTarget.blendOperationAlpha);
            blendAttachmentState[blendState.attachmentCount].colorWriteMask = ToVulkan(renderTarget.writeMask);
            blendState.attachmentCount++;

            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].format = info->colorFormats[i];
            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].loadAction = LoadAction::Discard;
            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].storeAction = StoreAction::Store;
            renderPassKey.colorAttachmentCount++;
        }

        // Rasterization state
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.pNext = nullptr;
        rasterizationState.flags = 0u;
        rasterizationState.depthClampEnable = VK_TRUE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = ToVulkan(info->rasterizerState.fillMode);
        rasterizationState.cullMode = ToVulkan(info->rasterizerState.cullMode);
        rasterizationState.frontFace = ToVulkan(info->rasterizerState.frontFace);
        rasterizationState.depthBiasEnable = info->rasterizerState.depthBias != 0 || info->rasterizerState.depthBiasSlopeScale != 0;
        rasterizationState.depthBiasConstantFactor = info->rasterizerState.depthBias;
        rasterizationState.depthBiasClamp = info->rasterizerState.depthBiasClamp;
        rasterizationState.depthBiasSlopeFactor = info->rasterizerState.depthBiasSlopeScale;
        rasterizationState.lineWidth = 1.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.pNext = nullptr;
        depthStencilState.flags = 0;
        // Depth writes only occur if depth is enabled
        depthStencilState.depthTestEnable = (info->depthStencilState.depthCompare != CompareFunction::Always || info->depthStencilState.depthWriteEnabled)
            ? VK_TRUE
            : VK_FALSE;

        depthStencilState.depthWriteEnable = info->depthStencilState.depthWriteEnabled ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = ToVkCompareOp(info->depthStencilState.depthCompare);
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.stencilTestEnable = StencilTestEnabled(&info->depthStencilState) ? VK_TRUE : VK_FALSE;

        depthStencilState.front.failOp = ToVulkan(info->depthStencilState.frontFace.failOperation);
        depthStencilState.front.passOp = ToVulkan(info->depthStencilState.frontFace.passOperation);
        depthStencilState.front.depthFailOp = ToVulkan(info->depthStencilState.frontFace.depthFailOperation);
        depthStencilState.front.compareOp = ToVkCompareOp(info->depthStencilState.frontFace.compareFunction);
        depthStencilState.front.compareMask = info->depthStencilState.stencilReadMask;
        depthStencilState.front.writeMask = info->depthStencilState.stencilWriteMask;
        depthStencilState.front.reference = 0; // The stencil reference is always dynamic

        depthStencilState.back.failOp = ToVulkan(info->depthStencilState.backFace.failOperation);
        depthStencilState.back.passOp = ToVulkan(info->depthStencilState.backFace.passOperation);
        depthStencilState.back.depthFailOp = ToVulkan(info->depthStencilState.backFace.depthFailOperation);
        depthStencilState.back.compareOp = ToVkCompareOp(info->depthStencilState.backFace.compareFunction);
        depthStencilState.back.compareMask = info->depthStencilState.stencilReadMask;
        depthStencilState.back.writeMask = info->depthStencilState.stencilWriteMask;
        depthStencilState.back.reference = 0; // The stencil reference is always dynamic

        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        std::vector<VkPipelineShaderStageCreateInfo> stages;
        for (VulkanShader* shader : shaders)
        {
            VkPipelineShaderStageCreateInfo stageCreateInfo{};
            stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageCreateInfo.stage = (VkShaderStageFlagBits)ToVulkan(shader->GetStage());
            stageCreateInfo.module = shader->handle;
            stageCreateInfo.pName = shader->GetEntryPoint().c_str();
            stageCreateInfo.pSpecializationInfo = nullptr;
            stages.push_back(stageCreateInfo);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputState.vertexBindingDescriptionCount = vertexBindingCount;
        vertexInputState.pVertexBindingDescriptions = vertexBindings;
        vertexInputState.vertexAttributeDescriptionCount = vertexAttributeCount;
        vertexInputState.pVertexAttributeDescriptions = vertexAttributes;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        inputAssemblyState.topology = ToVulkan(info->primitiveTopology);
        inputAssemblyState.primitiveRestartEnable = EnablePrimitiveRestart(info->primitiveTopology);

        VkPipelineTessellationStateCreateInfo tessellationState;
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = nullptr;
        tessellationState.flags = 0;
        tessellationState.patchControlPoints = info->patchControlPoints;

        VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisampleState;
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.pNext = nullptr;
        multisampleState.flags = 0;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // static_cast<VkSampleCountFlagBits>(info.sampleCount);
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 0.0f;
        //ALIMER_ASSERT(multisampleState.rasterizationSamples <= 32);
        //VkSampleMask sampleMask = info->sampleMask;
        //multisampleState.pSampleMask = &sampleMask;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = info->blendState.alphaToCoverageEnable;
        multisampleState.alphaToOneEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.stageCount = static_cast<uint32_t>(stages.size());
        createInfo.pStages = stages.data();
        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pInputAssemblyState = &inputAssemblyState;
        createInfo.pTessellationState = &tessellationState;
        createInfo.pViewportState = &viewportState;
        createInfo.pRasterizationState = &rasterizationState;
        createInfo.pMultisampleState = &multisampleState;
        createInfo.pDepthStencilState = &depthStencilState;
        createInfo.pColorBlendState = &blendState;
        createInfo.pDynamicState = &dynamicStateInfo;
        createInfo.layout = pipeline->pipelineLayout;
        createInfo.renderPass = GetVkRenderPass(renderPassKey);
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = 0;

        VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
            1, &createInfo,
            nullptr,
            &pipeline->handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create pipeline");
            return nullptr;
        }

        //OnCreated();

        if (info->label != nullptr)
        {
            SetObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline->handle, info->label);
        }

        return pipeline;
    }

    PipelineRef VulkanGraphics::CreateComputePipeline(const ComputePipelineCreateInfo* info)
    {
        RefPtr<VulkanPipeline> pipeline(new VulkanPipeline());
        pipeline->device = this;
        pipeline->type = PipelineType::Compute;

        VulkanShader* shader = ToVulkan(info->shader);
        //VulkanPipelineLayout* pipelineLayout = RequestPipelineLayout({ shader });

        VkComputePipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage.stage = (VkShaderStageFlagBits)ToVulkan(shader->GetStage());
        createInfo.stage.module = shader->handle;
        createInfo.stage.pName = shader->GetEntryPoint().c_str();
        createInfo.stage.pSpecializationInfo = nullptr;
        //createInfo.layout = pipelineLayout->GetHandle();

        VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE,
            1, &createInfo,
            nullptr, &pipeline->handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create compute pipeline");
            return nullptr;
        }

        //OnCreated();

        if (info->label != nullptr)
        {
            SetObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline->handle, info->label);
        }

        return pipeline;
    }

    SwapChainRef VulkanGraphics::CreateSwapChain(void* windowHandle, const SwapChainCreateInfo& info)
    {
        auto result = new VulkanSwapChain(*this, windowHandle, info);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
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

    void VulkanGraphics::DeferDestroy(VkSampler resource, uint32_t bindlessIndex)
    {
        destroyMutex.lock();
        if (resource != VK_NULL_HANDLE)
        {
            destroyedSamplers.push_back(std::make_pair(resource, frameCount));
        }

        if (bindlessIndex != kInvalidBindlessIndex)
        {
            destroyedBindlessSamplers.push_back(std::make_pair(bindlessIndex, frameCount));
        }
        destroyMutex.unlock();
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
        destroyMutex.lock();
        deletionDescriptorPoolQueue.push_back(std::make_pair(resource, frameCount));
        destroyMutex.unlock();
    }

    void VulkanGraphics::ProcessDeletionQueue()
    {
        destroyMutex.lock();
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

        while (!destroyedSamplers.empty())
        {
            if (destroyedSamplers.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = destroyedSamplers.front();
                destroyedSamplers.pop_front();
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
        while (!destroyedBindlessSampledImages.empty())
        {
            if (destroyedBindlessSampledImages.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessSampledImages.front().first;
                destroyedBindlessSampledImages.pop_front();
                bindlessSampledImages.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessUniformTexelBuffers.empty())
        {
            if (destroyedBindlessUniformTexelBuffers.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessUniformTexelBuffers.front().first;
                destroyedBindlessUniformTexelBuffers.pop_front();
                bindlessUniformTexelBuffers.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessStorageBuffers.empty())
        {
            if (destroyedBindlessStorageBuffers.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessStorageBuffers.front().first;
                destroyedBindlessStorageBuffers.pop_front();
                bindlessStorageBuffers.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessStorageImages.empty())
        {
            if (destroyedBindlessStorageImages.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessStorageImages.front().first;
                destroyedBindlessStorageImages.pop_front();
                bindlessStorageImages.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessStorageTexelBuffers.empty())
        {
            if (destroyedBindlessStorageTexelBuffers.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessStorageTexelBuffers.front().first;
                destroyedBindlessStorageTexelBuffers.pop_front();
                bindlessStorageTexelBuffers.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessSamplers.empty())
        {
            if (destroyedBindlessSamplers.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessSamplers.front().first;
                destroyedBindlessSamplers.pop_front();
                bindlessSamplers.Free(index);
            }
            else
            {
                break;
            }
        }
        while (!destroyedBindlessAccelerationStructures.empty())
        {
            if (destroyedBindlessAccelerationStructures.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessAccelerationStructures.front().first;
                destroyedBindlessAccelerationStructures.pop_front();
                bindlessAccelerationStructures.Free(index);
            }
            else
            {
                break;
            }
        }
        destroyMutex.unlock();
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
                attachmentDesc.initialLayout = key.colorAttachments[i].initialLayout;
                attachmentDesc.finalLayout = key.colorAttachments[i].finalLayout;

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
            std::array<VkSubpassDependency, 2> dependencies;
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
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            // Finally, create the renderpass.
            VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
            createInfo.attachmentCount = attachmentCount;
            createInfo.pAttachments = attachmentDescs.data();
            createInfo.subpassCount = 1;
            createInfo.pSubpasses = &subpassDesc;
            createInfo.dependencyCount = 0;
            createInfo.pDependencies = nullptr;
            createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            createInfo.pDependencies = dependencies.data();

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
    
    /* Copy Allocator */
    void VulkanGraphics::CopyAllocator::Init(VulkanGraphics* device_)
    {
        device = device_;

        VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
        timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreateInfo.initialValue = 0;

        VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        createInfo.pNext = &timelineCreateInfo;
        createInfo.flags = 0;
        VK_CHECK(vkCreateSemaphore(device->device, &createInfo, nullptr, &semaphore));
    }

    void VulkanGraphics::CopyAllocator::Shutdown()
    {
        VK_CHECK(vkQueueWaitIdle(device->copyQueue));
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

            VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device->copyQueueFamily;

            VK_CHECK(vkCreateCommandPool(device->device, &poolInfo, nullptr, &context.commandPool));

            VkCommandBufferAllocateInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
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
            const uint64_t bufferSize = NextPowerOfTwo(size);
            context.uploadBuffer = Buffer::CreateUpload(bufferSize);
            context.data = context.uploadBuffer->MappedData();
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
            VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
            submitInfo.pCommandBuffers = submitCommandBuffers.data();
            submitInfo.pSignalSemaphores = &semaphore;
            submitInfo.signalSemaphoreCount = 1;

            VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
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
