// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"

#if defined(ALIMER_RHI_VULKAN)
#include "Core/Log.h"
#include "Core/Vector.h"
#include "Core/UnorderedMap.h"
#include "Core/Hash.h"
#include "RHI/RHI.h"

#if defined(_WIN32)
// Use the C++ standard templated min/max
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#endif /* defined(_WIN32) */

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef VK_USE_PLATFORM_XLIB_KHR
#undef Always
#undef Bool
#undef False
#undef None
#undef True
#endif

ALIMER_DISABLE_WARNINGS()
#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"
ALIMER_ENABLE_WARNINGS()

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

#include <array>
#include <mutex>
#include <deque>
#include <memory>
#include <sstream>

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

#define VULKAN_GLOBAL_FUNCTION(name) static PFN_##name name = nullptr;
#include "RHI_Vulkan_Funcs.h"

#if ALIMER_ENABLE_ASSERT
/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err < 0) \
		{ \
			LOGE("Detected Vulkan error: {}", VkResultToString(err)); \
		} \
	} while (0)
#else
#define VK_CHECK(x) (void)(x)
#endif
#define VK_LOG_ERROR(result, message) LOGE("Vulkan: {}, error: {}", message, VkResultToString(result));

// Requires {}
#define VK_APPEND_EXT(desc) \
    *tail = &desc; \
    tail = &desc.pNext

namespace Alimer
{
    namespace
    {
        static_assert(sizeof(GPUAddress) == sizeof(VkDeviceAddress), "GPUAddress mismatch");

        static_assert(sizeof(ClearColorValue) == sizeof(VkClearColorValue), "ClearColorValue mismatch");
        static_assert(offsetof(ClearColorValue, float32) == offsetof(VkClearColorValue, float32), "ClearColorValue mismatch");
        static_assert(offsetof(ClearColorValue, int32) == offsetof(VkClearColorValue, int32), "ClearColorValue mismatch");
        static_assert(offsetof(ClearColorValue, uint32) == offsetof(VkClearColorValue, uint32), "ClearColorValue mismatch");

        static_assert(sizeof(ScissorRect) == sizeof(VkRect2D), "ScissorRect mismatch");
        static_assert(offsetof(ScissorRect, x) == offsetof(VkRect2D, offset.x), "ScissorRect layout mismatch");
        static_assert(offsetof(ScissorRect, y) == offsetof(VkRect2D, offset.y), "ScissorRect layout mismatch");
        static_assert(offsetof(ScissorRect, width) == offsetof(VkRect2D, extent.width), "ScissorRect layout mismatch");
        static_assert(offsetof(ScissorRect, height) == offsetof(VkRect2D, extent.height), "ScissorRect layout mismatch");

        static_assert(sizeof(Viewport) == sizeof(VkViewport), "Viewport mismatch");
        static_assert(offsetof(Viewport, x) == offsetof(VkViewport, x), "Viewport layout mismatch");
        static_assert(offsetof(Viewport, y) == offsetof(VkViewport, y), "Viewport layout mismatch");
        static_assert(offsetof(Viewport, width) == offsetof(VkViewport, width), "Viewport layout mismatch");
        static_assert(offsetof(Viewport, height) == offsetof(VkViewport, height), "Viewport layout mismatch");
        static_assert(offsetof(Viewport, minDepth) == offsetof(VkViewport, minDepth), "Viewport layout mismatch");
        static_assert(offsetof(Viewport, maxDepth) == offsetof(VkViewport, maxDepth), "Viewport layout mismatch");

        static_assert(sizeof(DispatchIndirectCommand) == sizeof(VkDispatchIndirectCommand), "DispatchIndirectCommand mismatch");
        static_assert(offsetof(DispatchIndirectCommand, x) == offsetof(VkDispatchIndirectCommand, x), "Layout mismatch");
        static_assert(offsetof(DispatchIndirectCommand, y) == offsetof(VkDispatchIndirectCommand, y), "Layout mismatch");
        static_assert(offsetof(DispatchIndirectCommand, z) == offsetof(VkDispatchIndirectCommand, z), "Layout mismatch");

        static_assert(sizeof(DrawIndirectCommand) == sizeof(VkDrawIndirectCommand), "DrawIndirectCommand mismatch");
        static_assert(offsetof(DrawIndirectCommand, vertexCount) == offsetof(VkDrawIndirectCommand, vertexCount), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, instanceCount) == offsetof(VkDrawIndirectCommand, instanceCount), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, firstVertex) == offsetof(VkDrawIndirectCommand, firstVertex), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, firstInstance) == offsetof(VkDrawIndirectCommand, firstInstance), "Layout mismatch");

        static_assert(sizeof(DrawIndexedIndirectCommand) == sizeof(VkDrawIndexedIndirectCommand), "DrawIndexedIndirectCommand mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, indexCount) == offsetof(VkDrawIndexedIndirectCommand, indexCount), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, instanceCount) == offsetof(VkDrawIndexedIndirectCommand, instanceCount), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, firstIndex) == offsetof(VkDrawIndexedIndirectCommand, firstIndex), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, baseVertex) == offsetof(VkDrawIndexedIndirectCommand, vertexOffset), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, firstInstance) == offsetof(VkDrawIndexedIndirectCommand, firstInstance), "Layout mismatch");

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

        template<typename MainT, typename NewT>
        inline void PnextChainPushBack(MainT* mainStruct, NewT* newStruct)
        {
            struct VkAnyStruct
            {
                VkStructureType sType;
                void* pNext;
            };
            VkAnyStruct* lastStruct = (VkAnyStruct*)mainStruct;
            while (lastStruct->pNext != nullptr)
            {
                lastStruct = (VkAnyStruct*)lastStruct->pNext;
            }
            newStruct->pNext = nullptr;
            lastStruct->pNext = newStruct;
        }

        constexpr VkFormat ToVkVertexFormat(VertexAttributeFormat format)
        {
            switch (format)
            {
                case VertexAttributeFormat::Uint8:              return VK_FORMAT_R8_UINT;
                case VertexAttributeFormat::Uint8x2:            return VK_FORMAT_R8G8_UINT;
                case VertexAttributeFormat::Uint8x4:            return VK_FORMAT_R8G8B8A8_UINT;
                case VertexAttributeFormat::Sint8:              return VK_FORMAT_R8_SINT;
                case VertexAttributeFormat::Sint8x2:            return VK_FORMAT_R8G8_SINT;
                case VertexAttributeFormat::Sint8x4:            return VK_FORMAT_R8G8B8A8_SINT;
                case VertexAttributeFormat::Unorm8:             return VK_FORMAT_R8_UNORM;
                case VertexAttributeFormat::Unorm8x2:           return VK_FORMAT_R8G8_UNORM;
                case VertexAttributeFormat::Unorm8x4:           return VK_FORMAT_R8G8B8A8_UNORM;
                case VertexAttributeFormat::Snorm8:             return VK_FORMAT_R8_SNORM;
                case VertexAttributeFormat::Snorm8x2:           return VK_FORMAT_R8G8_SNORM;
                case VertexAttributeFormat::Snorm8x4:           return VK_FORMAT_R8G8B8A8_SNORM;

                case VertexAttributeFormat::Uint16:             return VK_FORMAT_R16_UINT;
                case VertexAttributeFormat::Uint16x2:           return VK_FORMAT_R16G16_UINT;
                case VertexAttributeFormat::Uint16x4:           return VK_FORMAT_R16G16B16A16_UINT;
                case VertexAttributeFormat::Sint16:             return VK_FORMAT_R16_SINT;
                case VertexAttributeFormat::Sint16x2:           return VK_FORMAT_R16G16_SINT;
                case VertexAttributeFormat::Sint16x4:           return VK_FORMAT_R16G16B16A16_SINT;
                case VertexAttributeFormat::Unorm16:            return VK_FORMAT_R16_UNORM;
                case VertexAttributeFormat::Unorm16x2:          return VK_FORMAT_R16G16_UNORM;
                case VertexAttributeFormat::Unorm16x4:          return VK_FORMAT_R16G16B16A16_UNORM;
                case VertexAttributeFormat::Snorm16:            return VK_FORMAT_R16_SNORM;
                case VertexAttributeFormat::Snorm16x2:          return VK_FORMAT_R16G16_SNORM;
                case VertexAttributeFormat::Snorm16x4:          return VK_FORMAT_R16G16B16A16_SNORM;
                case VertexAttributeFormat::Float16:            return VK_FORMAT_R16_SFLOAT;
                case VertexAttributeFormat::Float16x2:          return VK_FORMAT_R16G16_SFLOAT;
                case VertexAttributeFormat::Float16x4:          return VK_FORMAT_R16G16B16A16_SFLOAT;

                case VertexAttributeFormat::Float32:            return VK_FORMAT_R32_SFLOAT;
                case VertexAttributeFormat::Float32x2:          return VK_FORMAT_R32G32_SFLOAT;
                case VertexAttributeFormat::Float32x3:          return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexAttributeFormat::Float32x4:          return VK_FORMAT_R32G32B32A32_SFLOAT;

                case VertexAttributeFormat::Uint32:             return VK_FORMAT_R32_UINT;
                case VertexAttributeFormat::Uint32x2:           return VK_FORMAT_R32G32_UINT;
                case VertexAttributeFormat::Uint32x3:           return VK_FORMAT_R32G32B32_UINT;
                case VertexAttributeFormat::Uint32x4:           return VK_FORMAT_R32G32B32A32_UINT;

                case VertexAttributeFormat::Sint32:             return VK_FORMAT_R32_SINT;
                case VertexAttributeFormat::Sint32x2:           return VK_FORMAT_R32G32_SINT;
                case VertexAttributeFormat::Sint32x3:           return VK_FORMAT_R32G32B32_SINT;
                case VertexAttributeFormat::Sint32x4:           return VK_FORMAT_R32G32B32A32_SINT;

                    //case VertexFormat.Int1010102Normalized: return VkFormat.A2B10G10R10SnormPack32;
                case VertexAttributeFormat::Unorm10_10_10_2:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                case VertexAttributeFormat::Unorm8x4BGRA:   return VK_FORMAT_B8G8R8A8_UNORM;
                    //case VertexFormat::RG11B10Float:            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                    //case VertexFormat::RGB9E5Float:             return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkImageAspectFlags GetImageAspectFlags(VkFormat format, TextureAspect aspect)
        {
            switch (aspect)
            {
                case TextureAspect::All:
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
                case TextureAspect::DepthOnly:
                    return VK_IMAGE_ASPECT_DEPTH_BIT;
                case TextureAspect::StencilOnly:
                    return VK_IMAGE_ASPECT_STENCIL_BIT;
                default:
                    return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }

        constexpr VkShaderStageFlags ToVkShaderStageFlags(ShaderStages stage)
        {
            if (stage == ShaderStages::All)
                return VK_SHADER_STAGE_ALL;

            VkShaderStageFlags flags = 0;
            if (CheckBitsAny(stage, ShaderStages::Vertex))
                flags |= VK_SHADER_STAGE_VERTEX_BIT;
            if (CheckBitsAny(stage, ShaderStages::Fragment))
                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            if (CheckBitsAny(stage, ShaderStages::Compute))
                flags |= VK_SHADER_STAGE_COMPUTE_BIT;
            if (CheckBitsAny(stage, ShaderStages::Amplification))
                flags |= VK_SHADER_STAGE_TASK_BIT_EXT;
            if (CheckBitsAny(stage, ShaderStages::Mesh))
                flags |= VK_SHADER_STAGE_MESH_BIT_EXT;
            return flags;
        }

        constexpr VkVertexInputRate ToVk(VertexStepMode mode)
        {
            switch (mode)
            {
                case VertexStepMode::Vertex:
                    return VK_VERTEX_INPUT_RATE_VERTEX;
                case VertexStepMode::Instance:
                    return VK_VERTEX_INPUT_RATE_INSTANCE;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkCompareOp ToVk(CompareFunction function)
        {
            switch (function)
            {
                case CompareFunction::Never:        return VK_COMPARE_OP_NEVER;
                case CompareFunction::Less:         return VK_COMPARE_OP_LESS;
                case CompareFunction::Equal:        return VK_COMPARE_OP_EQUAL;
                case CompareFunction::LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
                case CompareFunction::Greater:      return VK_COMPARE_OP_GREATER;
                case CompareFunction::NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
                case CompareFunction::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
                case CompareFunction::Always:       return VK_COMPARE_OP_ALWAYS;
                default:
                    return VK_COMPARE_OP_MAX_ENUM;
            }
        }

        constexpr VkStencilOp ToVk(StencilOperation op)
        {
            switch (op)
            {
                case StencilOperation::Keep:            return VK_STENCIL_OP_KEEP;
                case StencilOperation::Zero:            return VK_STENCIL_OP_ZERO;
                case StencilOperation::Replace:         return VK_STENCIL_OP_REPLACE;
                case StencilOperation::Invert:          return VK_STENCIL_OP_INVERT;
                case StencilOperation::IncrementClamp:  return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case StencilOperation::DecrementClamp:  return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case StencilOperation::IncrementWrap:   return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case StencilOperation::DecrementWrap:   return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkPolygonMode ToVk(FillMode value, bool fillModeNonSolid)
        {
            switch (value)
            {
                default:
                case FillMode::Solid:
                    return VK_POLYGON_MODE_FILL;

                case FillMode::Wireframe:
                    if (!fillModeNonSolid)
                    {
                        LOGW("Vulkan: Wireframe fill mode is being used but it's not supported on this device");
                        return VK_POLYGON_MODE_FILL;
                    }

                    return VK_POLYGON_MODE_LINE;
            }
        }

        constexpr VkCullModeFlags ToVk(CullMode value)
        {
            switch (value)
            {
                default:
                case CullMode::Back:
                    return VK_CULL_MODE_BACK_BIT;
                case CullMode::None:
                    return VK_CULL_MODE_NONE;
                case CullMode::Front:
                    return VK_CULL_MODE_FRONT_BIT;
            }
        }

        constexpr VkFrontFace ToVk(FrontFace value)
        {
            switch (value)
            {
                default:
                case FrontFace::CounterClockwise:
                    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
                case FrontFace::Clockwise:
                    return VK_FRONT_FACE_CLOCKWISE;
            }
        }

        constexpr VkFilter ToVk(SamplerMinMagFilter filter)
        {
            switch (filter)
            {
                case SamplerMinMagFilter::Point:
                    return VK_FILTER_NEAREST;
                case SamplerMinMagFilter::Linear:
                    return VK_FILTER_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_FILTER_NEAREST;
            }
        }

        constexpr VkSamplerMipmapMode ToVk(SamplerMipFilter filter)
        {
            switch (filter)
            {
                case SamplerMipFilter::Point:
                    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
                case SamplerMipFilter::Linear:
                    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            }
        }

        constexpr VkSamplerAddressMode ToVk(SamplerAddressMode mode, bool samplerMirrorClampToEdge)
        {
            switch (mode)
            {
                case SamplerAddressMode::Clamp:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case SamplerAddressMode::Wrap:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case SamplerAddressMode::Mirror:
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case SamplerAddressMode::MirrorOnce:
                    if (samplerMirrorClampToEdge)
                    {
                        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                    }
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

                case SamplerAddressMode::Border:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        }

        constexpr VkBorderColor ToVk(SamplerBorderColor color)
        {
            switch (color)
            {
                case SamplerBorderColor::FloatTransparentBlack:
                    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
                case SamplerBorderColor::FloatOpaqueBlack:
                    return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
                case SamplerBorderColor::FloatOpaqueWhite:
                    return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                case SamplerBorderColor::UintTransparentBlack:
                    return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
                case SamplerBorderColor::UintOpaqueBlack:
                    return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                case SamplerBorderColor::UintOpaqueWhite:
                    return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            }
        }

        constexpr VkSamplerReductionMode ToVk(SamplerReductionType value)
        {
            switch (value)
            {
                case SamplerReductionType::Minimum:
                    return VK_SAMPLER_REDUCTION_MODE_MIN;
                case SamplerReductionType::Maximum:
                    return VK_SAMPLER_REDUCTION_MODE_MAX;
                default:
                    return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
            }
        }

        constexpr VkBlendFactor ToVk(BlendFactor value)
        {
            switch (value)
            {
                case BlendFactor::Zero:                         return VK_BLEND_FACTOR_ZERO;
                case BlendFactor::One:                          return VK_BLEND_FACTOR_ONE;
                case BlendFactor::SourceColor:                  return VK_BLEND_FACTOR_SRC_COLOR;
                case BlendFactor::OneMinusSourceColor:          return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case BlendFactor::SourceAlpha:                  return VK_BLEND_FACTOR_SRC_ALPHA;
                case BlendFactor::OneMinusSourceAlpha:          return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::DestinationColor:             return VK_BLEND_FACTOR_DST_COLOR;
                case BlendFactor::OneMinusDestinationColor:     return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case BlendFactor::DestinationAlpha:             return VK_BLEND_FACTOR_DST_ALPHA;
                case BlendFactor::OneMinusDestinationAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case BlendFactor::SourceAlphaSaturate:          return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                case BlendFactor::BlendColor:                   return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case BlendFactor::OneMinusBlendColor:           return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                case BlendFactor::Source1Color:                 return VK_BLEND_FACTOR_SRC1_COLOR;
                case BlendFactor::OneMinusSource1Color:         return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
                case BlendFactor::Source1Alpha:                 return VK_BLEND_FACTOR_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Alpha:         return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkBlendOp ToVk(BlendOperation value)
        {
            switch (value)
            {
                case BlendOperation::Add:              return VK_BLEND_OP_ADD;
                case BlendOperation::Subtract:         return VK_BLEND_OP_SUBTRACT;
                case BlendOperation::ReverseSubtract:  return VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendOperation::Min:              return VK_BLEND_OP_MIN;
                case BlendOperation::Max:              return VK_BLEND_OP_MAX;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkColorComponentFlags ToVk(ColorWriteMask value)
        {
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Red) == VK_COLOR_COMPONENT_R_BIT, "ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Green) == VK_COLOR_COMPONENT_G_BIT, "ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Blue) == VK_COLOR_COMPONENT_B_BIT, "ColorWriteMask mismatch");
            static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Alpha) == VK_COLOR_COMPONENT_A_BIT, "ColorWriteMask mismatch");
            return static_cast<VkColorComponentFlags>(value);
        }

        constexpr VkSampleCountFlagBits ToVk(TextureSampleCount count)
        {
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count1) == VK_SAMPLE_COUNT_1_BIT, "TextureSampleCount missmatch");
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count2) == VK_SAMPLE_COUNT_2_BIT, "TextureSampleCount missmatch");
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count4) == VK_SAMPLE_COUNT_4_BIT, "TextureSampleCount missmatch");
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count8) == VK_SAMPLE_COUNT_8_BIT, "TextureSampleCount missmatch");
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count16) == VK_SAMPLE_COUNT_16_BIT, "TextureSampleCount missmatch");
            static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count32) == VK_SAMPLE_COUNT_32_BIT, "TextureSampleCount missmatch");
            //static_assert(static_cast<VkSampleCountFlagBits>(TextureSampleCount::Count64) == VK_SAMPLE_COUNT_64_BIT, "TextureSampleCount missmatch");

            return static_cast<VkSampleCountFlagBits>(count);
        }

        constexpr VkAttachmentLoadOp ToVk(LoadAction op)
        {
            switch (op)
            {
                case LoadAction::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case LoadAction::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case LoadAction::DontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                default:
                    ALIMER_UNREACHABLE();
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
        }

        constexpr VkAttachmentStoreOp ToVk(StoreAction op)
        {
            switch (op)
            {
                case StoreAction::Store:
                    return VK_ATTACHMENT_STORE_OP_STORE;
                case StoreAction::DontCare:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;

                default:
                    ALIMER_UNREACHABLE();
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
        }

        constexpr VkPrimitiveTopology ToVk(PrimitiveTopology type)
        {
            switch (type)
            {
                case PrimitiveTopology::PointList:      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case PrimitiveTopology::LineList:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopology::LineStrip:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                case PrimitiveTopology::TriangleList:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::TriangleStrip:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
            }
        }

        constexpr VkPresentModeKHR ToVk(PresentMode mode)
        {
            switch (mode)
            {
                case PresentMode::Fifo:
                    return VK_PRESENT_MODE_FIFO_KHR;
                case PresentMode::Immediate:
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case PresentMode::Mailbox:
                    return VK_PRESENT_MODE_MAILBOX_KHR;
                default:
                    return VK_PRESENT_MODE_FIFO_KHR;
            }
        }

        constexpr VkQueryType ToVkQueryType(QueryType type)
        {
            switch (type)
            {
                case QueryType::Occlusion:
                case QueryType::BinaryOcclusion:
                    return VK_QUERY_TYPE_OCCLUSION;

                case QueryType::Timestamp:
                    return VK_QUERY_TYPE_TIMESTAMP;

                case QueryType::PipelineStatistics:
                    return VK_QUERY_TYPE_PIPELINE_STATISTICS;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr uint32_t GetQueryResultSize(QueryType type)
        {
            constexpr uint32_t highestBitInPipelineStatsBits = 11;

            switch (type)
            {
                case QueryType::Occlusion:
                case QueryType::BinaryOcclusion:
                case QueryType::Timestamp:
                    return sizeof(uint64_t);

                case QueryType::PipelineStatistics:
                    return highestBitInPipelineStatsBits * sizeof(uint64_t);

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr VkComponentSwizzle ToVkTextureSwizzle(TextureSwizzle value)
        {
            switch (value)
            {
                default:
                    return VK_COMPONENT_SWIZZLE_IDENTITY;
                case TextureSwizzle::Red:
                    return VK_COMPONENT_SWIZZLE_R;
                case TextureSwizzle::Green:
                    return VK_COMPONENT_SWIZZLE_G;
                case TextureSwizzle::Blue:
                    return VK_COMPONENT_SWIZZLE_B;
                case TextureSwizzle::Alpha:
                    return VK_COMPONENT_SWIZZLE_A;
                case TextureSwizzle::Zero:
                    return VK_COMPONENT_SWIZZLE_ZERO;
                case TextureSwizzle::One:
                    return VK_COMPONENT_SWIZZLE_ONE;
            }
        }

        constexpr VkComponentMapping ToVkSwizzle(TextureSwizzleChannels value)
        {
            VkComponentMapping mapping = {};
            mapping.r = ToVkTextureSwizzle(value.red);
            mapping.g = ToVkTextureSwizzle(value.green);
            mapping.b = ToVkTextureSwizzle(value.blue);
            mapping.a = ToVkTextureSwizzle(value.alpha);
            return mapping;
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
                LOGW("Vulkan - {}: {}", messageTypeStr, pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LOGE("Vulkan - {}: {}", messageTypeStr, pCallbackData->pMessage);
#if defined(_DEBUG)
                ALIMER_BREAKPOINT;
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
                    LOGW("Validation Layer '{}' not found", layer);
                    return false;
                }
            }

            return true;
        }

        std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& availableInstanceLayers)
        {
            std::vector<std::vector<const char*>> validationLayerPriorityList =
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

            for (auto& validation_layers : validationLayerPriorityList)
            {
                if (ValidateLayers(validation_layers, availableInstanceLayers))
                {
                    return validation_layers;
                }

                LOGW("Couldn't enable validation layers (see log for error) - falling back");
            }

            // Else return nothing
            return {};
        }

        struct VkBufferStateMapping final
        {
            BufferStates state = BufferStates::Undefined;
            VkPipelineStageFlags2 stageFlags = 0;
            VkAccessFlags2 accessMask = 0;

            VkBufferStateMapping() = default;

            VkBufferStateMapping(BufferStates state_, VkPipelineStageFlags2 stageFlags_, VkAccessFlags2 accessMask_)
                : state(state_)
                , stageFlags(stageFlags_)
                , accessMask(accessMask_)
            {}
        };

        struct VkImageLayoutMapping final
        {
            VkImageLayout layout;
            VkPipelineStageFlags2 stageFlags;
            VkAccessFlags2 accessMask;

            VkImageLayoutMapping(VkImageLayout layout_, VkPipelineStageFlags2 stageFlags_, VkAccessFlags2 accessMask_)
                : layout(layout_)
                , stageFlags(stageFlags_)
                , accessMask(accessMask_)
            {}
        };

        static const VkBufferStateMapping g_BufferStateMap[] =
        {
            { BufferStates::CopyDest, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT },
            { BufferStates::CopySource, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT },
            { BufferStates::ShaderResource, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT },
            { BufferStates::UnorderedAccess, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT },
            { BufferStates::VertexBuffer, VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT },
            { BufferStates::IndexBuffer, VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT },
            { BufferStates::ConstantBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_UNIFORM_READ_BIT },
            { BufferStates::Predication, VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT },
            #if TODO
            { ResourceStates::IndirectArgument, VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT },
            { ResourceStates::StreamOut, VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT },
            { ResourceStates::AccelerationStructureRead, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR },
            { ResourceStates::AccelerationStructureWrite, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR },
            { ResourceStates::AccelerationStructureBuildInput, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR},
            { ResourceStates::OpacityMicromapWrite, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT },
            { ResourceStates::OpacityMicromapBuildInput, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_SHADER_READ_BIT },
             #endif // TODO
        };

        VkBufferStateMapping ConvertBufferState(BufferStates state)
        {
            VkBufferStateMapping result = {};

            constexpr uint32_t numStateBits = sizeof(g_BufferStateMap) / sizeof(g_BufferStateMap[0]);

            uint32_t stateTmp = uint32_t(state);
            uint32_t bitIndex = 0;

            while (stateTmp != 0 && bitIndex < numStateBits)
            {
                uint32_t bit = (1 << bitIndex);

                if (stateTmp & bit)
                {
                    const VkBufferStateMapping& mapping = g_BufferStateMap[bitIndex];

                    ALIMER_ASSERT(uint32_t(mapping.state) == bit);

                    result.state |= mapping.state;
                    result.accessMask |= mapping.accessMask;
                    result.stageFlags |= mapping.stageFlags;

                    stateTmp &= ~bit;
                }

                bitIndex++;
            }

            ALIMER_ASSERT(result.state == state);

            return result;
        }

        VkImageLayoutMapping ConvertImageLayout(TextureLayout layout, bool depthOnlyFormat)
        {
            switch (layout)
            {
                case TextureLayout::Undefined:
                    return {
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_2_NONE
                    };

                case TextureLayout::CopySource:
                    return {
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_READ_BIT
                    };

                case TextureLayout::CopyDest:
                    return {
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_WRITE_BIT
                    };

                case TextureLayout::ResolveSource:
                    return {
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_READ_BIT
                    };

                case TextureLayout::ResolveDest:
                    return {
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_WRITE_BIT
                    };

                case TextureLayout::ShaderResource:
                    //return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT };
                    return {
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                        VK_ACCESS_2_SHADER_READ_BIT
                    };

                case TextureLayout::UnorderedAccess:
                    return {
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT
                    };

                case TextureLayout::RenderTarget:
                    return {
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
                    };

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
                    return {
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        VK_ACCESS_2_MEMORY_READ_BIT
                    };

                case TextureLayout::ShadingRateSurface:
                    return {
                        VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
                        VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                        VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR
                    };

                default:
                    ALIMER_UNREACHABLE();
            }
        }
    }

    /* Forward vulkan type declarations */
    class VulkanDevice;
    class VulkanRHIAdapter;
    class VulkanRHIFactory;

    struct VulkanBuffer final : public RHIBuffer
    {
    private:
        VulkanDevice* device = nullptr;

    public:
        VkBuffer handle = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;

        uint64_t allocatedSize{};
        VkDeviceAddress deviceAddress{};
        void* pMappedData{ nullptr };
#if defined(_WIN32)
        void* sharedHandle = nullptr;
#else
        int sharedHandle = 0;
#endif

        explicit VulkanBuffer(VulkanDevice* device_, const BufferDesc& desc_)
            : RHIBuffer(desc_)
            , device(device_)
        {

        }

        ~VulkanBuffer() override;

        uint64_t GetAllocatedSize() const { return allocatedSize; }
        void SetLabel(const char* label) override;
        void* GetMappedData() const override { return pMappedData; }
        GPUAddress GetGPUAddress() const override { return deviceAddress; }
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
        void SetCurrentState(BufferStates state) const
        {
            currentState = state;
        }
    };

    struct VulkanTexture final : public RHITexture
    {
        VulkanDevice* device = nullptr;

        VkImage handle = VK_NULL_HANDLE;
        VkBuffer stagingResource = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkFormat vkFormat = VK_FORMAT_UNDEFINED;

        uint64_t allocatedSize{};
        //void* pMappedData{ nullptr };
        uint32_t numSubResources = 0;
        mutable std::vector<TextureLayout> imageLayouts;
#if defined(_WIN32)
        void* sharedHandle = nullptr;
#else
        int sharedHandle = 0;
#endif

        explicit VulkanTexture(VulkanDevice* device_, const TextureDescriptor& desc)
            : RHITexture(desc)
            , device(device_)
        {

        }

        ~VulkanTexture() override;

        uint64_t GetAllocatedSize() const { return allocatedSize; }
        void SetLabel(const char* label) override;
        RHITextureViewRef CreateView(const TextureViewDesc& desc) const override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
    };

    struct VulkanTextureView final : public RHITextureView
    {
        VulkanDevice* device;
        VkImageView handle = nullptr;

        explicit VulkanTextureView(const VulkanTexture* texture_, const TextureViewDesc& desc)
            : RHITextureView(texture_, desc)
            , device(texture_->device)
        {

        }

        ~VulkanTextureView() override;

        void SetLabel(const char* label) override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
    };

    struct VulkanSampler final : public RHISampler
    {
        VulkanDevice* device = nullptr;
        SamplerDesc desc;
        VkSampler handle = VK_NULL_HANDLE;

        ~VulkanSampler() override;
        const SamplerDesc& GetDesc() const override { return desc; }
        void SetLabel(const char* label) override;
    };

    struct VulkanShaderModule final : public RHIShaderModule
    {
        VulkanDevice* device = nullptr;
        std::string entryPoint;
        VkPipelineShaderStageCreateInfo stageInfo = {};

        ~VulkanShaderModule() override;
        void SetLabel(const char* label) override;
    };

    struct VulkanComputePipeline final : public RHIComputePipeline
    {
        VulkanDevice* device = nullptr;
        VkPipeline handle = VK_NULL_HANDLE;

        ~VulkanComputePipeline() override;
        void SetLabel(const char* label) override;
    };

    struct VulkanRenderPipeline final : public RHIRenderPipeline
    {
        VulkanDevice* device = nullptr;
        //VulkanPipelineLayoutReflection reflection{};

        VkPipeline handle = VK_NULL_HANDLE;

        ~VulkanRenderPipeline() override;
        void SetLabel(const char* label) override;
    };

    struct VulkanQueryHeap final : public RHIQueryHeap
    {
        VulkanDevice* device = nullptr;
        QueryHeapDesc desc;
        VkQueryPool handle = VK_NULL_HANDLE;
        uint32_t queryResultSize = 0;

        ~VulkanQueryHeap() override;
        void SetLabel(const char* label) override;

        uint32_t GetCount() const override { return desc.count; }
        QueryType GetType() const override { return desc.type; }
    };

    struct VulkanRHISurface final : public RHISurface
    {
        VulkanRHIFactory* factory = nullptr;
        VkSurfaceKHR handle = VK_NULL_HANDLE;

        ~VulkanRHISurface() override;
    };

    struct VulkanSwapChain final : public RHISwapChain
    {
        VulkanDevice* device = nullptr;
        std::mutex locker;

        SharedPtr<VulkanRHISurface> surface;
        VkSwapchainKHR handle = VK_NULL_HANDLE;

        uint32_t imageIndex = 0;
        VkExtent2D extent{};
        uint32_t queuePresentSupport = 0;
        PixelFormat colorFormat = PixelFormat::Undefined;
        PresentMode presentMode = PresentMode::Immediate;
        std::vector<SharedPtr<VulkanTexture>> backbufferTextures;
        size_t acquireSemaphoreIndex = 0;
        std::vector<VkSemaphore> acquireSemaphores;
        std::vector<VkSemaphore> releaseSemaphores;

        ~VulkanSwapChain() override;

        RHISurface* GetSurface() const { return surface; }
        PixelFormat GetColorFormat() const override { return colorFormat; }
        void SetLabel(const char* label) override;
    };

    struct VulkanQueue final
    {
        VkQueue queue = VK_NULL_HANDLE;

        std::vector<SharedPtr<VulkanSwapChain>> swapchainUpdates;
        std::vector<VkSwapchainKHR> submitSwapchains;
        std::vector<uint32_t> submitSwapchainImageIndices;
        std::vector<VkSemaphore> submitSignalSemaphores;
        std::vector<VkSemaphoreSubmitInfo> submitWaitSemaphoreInfos;
        std::vector<VkSemaphoreSubmitInfo> submitSignalSemaphoreInfos;
        std::vector<VkCommandBufferSubmitInfo> submitCommandBufferInfos;

        bool sparseBindingSupported = false;
        std::mutex locker;

        VkFence frameFences[kNumFramesInFlight] = {};

        void Submit(VulkanDevice* device, VkFence fence);
    };

    struct VulkanUploadContext final
    {
        VkCommandPool transferCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer transferCommandBuffer = VK_NULL_HANDLE;
        VkCommandPool transitionCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer transitionCommandBuffer = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkSemaphore semaphores[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE }; // graphics, compute, video
        SharedPtr<VulkanBuffer> uploadBuffer;
        void* uploadBufferData = nullptr;
        uint64_t uploadBufferSize = 0;

        inline bool IsValid() const { return transferCommandBuffer != VK_NULL_HANDLE; }
    };

    class VulkanCommandBuffer;

    class VulkanComputePassEncoder final : public RHIComputePassEncoder
    {
        friend class VulkanCommandBuffer;

    public:
        VulkanComputePassEncoder(VulkanDevice* device, VulkanCommandBuffer* commandBuffer);
        ~VulkanComputePassEncoder() override;

        void Reset(VkCommandBuffer commandBuffer);
        void Begin(const ComputePassDescriptor& descriptor);

        void PushDebugGroup(std::string_view groupLabel) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view markerLabel) override;

        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) override;
        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) override;

        void SetPipeline(RHIComputePipeline* pipeline) override;
        void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) override;
        void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void DispatchIndirectCore(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;

        void End() override;
        RHICommandBuffer* GetCommandBuffer() const override;

    private:
        void ClearState();
        void PrepareDispatch();

        VulkanDevice* _device;
        VulkanCommandBuffer* _commandBuffer;
        VkCommandBuffer _vkCommandBuffer = VK_NULL_HANDLE;
        bool _hasLabel{ false };
        SharedPtr<VulkanComputePipeline> _currentPipeline;
    };

    class VulkanRenderPassEncoder final : public RHIRenderPassEncoder
    {
        friend class VulkanCommandBuffer;

    public:
        VulkanRenderPassEncoder(VulkanDevice* device, VulkanCommandBuffer* commandBuffer);
        ~VulkanRenderPassEncoder() override;

        void Reset(VkCommandBuffer commandBuffer);
        void Begin(const RenderPassDesc& descriptor);

        void PushDebugGroup(std::string_view groupLabel) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view markerLabel) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(const Viewport* viewports, uint32_t count) override;
        void SetScissorRect(const ScissorRect& rect) override;
        void SetScissorRects(const ScissorRect* scissorRects, uint32_t count) override;
        void SetStencilReference(uint32_t referenceValue) override;
        void SetBlendColor(const Color& color) override;
        void SetShadingRate(ShadingRate rate) override;
        void SetDepthBounds(float minBounds, float maxBounds) override;

        void SetPipeline(RHIRenderPipeline* pipeline) override;
        void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) override;

        void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset) override;
        void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) override;
        void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexFormat format) override;

        void PrepareDraw();
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
        void DrawIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
        void DrawMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount) override;

        void End() override;

        RHICommandBuffer* GetCommandBuffer() const override;

    private:
        void ClearState();

        VulkanDevice* _device;
        VulkanCommandBuffer* _commandBuffer;
        VkCommandBuffer _vkCommandBuffer = VK_NULL_HANDLE;
        bool _hasLabel{ false };
        ShadingRate _currentShadingRate{ ShadingRate::Invalid };
        SharedPtr<VulkanRenderPipeline> _currentPipeline;
    };

    class VulkanCommandBuffer final : public RHICommandBuffer
    {
        friend class VulkanDevice;

    public:
        VulkanCommandBuffer(VulkanDevice* device, QueueType queueType, uint32_t id);
        ~VulkanCommandBuffer() override;

        void Begin(uint32_t frameIndex, std::string_view label);
        VkCommandBuffer End();
        void EndEncoding();

        RHIDevice* GetDevice() const override;

        void PushDebugGroup(std::string_view name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view name) override;

        void BufferBarrier(const VulkanBuffer* buffer, BufferStates newState);
        void TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, TextureAspect aspect = TextureAspect::All);
        void TextureBarrier(const VulkanTextureView* view, TextureLayout newLayout);
        void CommitBarriers();

        void FlushBindGroups();

        void BeginQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void EndQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) override;
        void ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count) override;

        /* GraphicsContext */
        RHITexture* AcquireSwapChainTexture(RHISwapChain* swapChain) override;

        RHIComputePassEncoder* BeginComputePassCore(const ComputePassDescriptor& descriptor) override;
        RHIRenderPassEncoder* BeginRenderPassCore(const RenderPassDesc& desc) override;

        void BeginPredication(const RHIBuffer* buffer, uint64_t offset, PredicationOperation operation) override;
        void EndPredication() override;

    private:
        static constexpr uint32_t kMaxBarrierCount = 16;

        VulkanDevice* device;
        QueueType queueType;
        uint32_t id;

        VkCommandPool commandPools[kNumFramesInFlight] = {};
        VkCommandBuffer commandBuffers[kNumFramesInFlight] = {};
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkSemaphore semaphore = VK_NULL_HANDLE;

        std::vector<VulkanCommandBuffer*> waits;
        std::atomic_bool hasPendingWaits{ false };
        bool hasLabel = false;

        uint32_t numBarriersToCommit = 0;
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        VulkanRenderPassEncoder* _renderPassEncoder;
        VulkanComputePassEncoder* _computePassEncoder;

        std::vector<SharedPtr<VulkanSwapChain>> presentSwapChains;
    };

    struct VulkanBindlessDescriptorHeap final
    {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        Vector<BindlessIndex> freeList;
        std::mutex locker;

        void Init(VulkanDevice* device, VkDescriptorType type, uint32_t descriptorCount);
        void Destroy(VulkanDevice* device);

        BindlessIndex Allocate()
        {
            std::scoped_lock lck(locker);
            if (!freeList.empty())
            {
                BindlessIndex index = freeList.back();
                freeList.pop_back();
                return index;
            }

            return kInvalidBindlessIndex;
        }

        void Free(BindlessIndex index)
        {
            if (index < 0)
                return;

            std::scoped_lock lck(locker);
            freeList.push_back(index);
        }
    };

    struct VulkanBindlessManager final
    {
    public:
        enum DESCRIPTOR_SET
        {
            DESCRIPTOR_SET_BINDINGS,
            DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE,

            DESCRIPTOR_SET_COUNT,
        };

        VulkanDevice* device;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        bool mutableDescriptorType;
        VulkanBindlessDescriptorHeap bindlessSampledImages;

        VulkanBindlessManager(VulkanDevice* device_);
        ~VulkanBindlessManager();

        void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

    private:
        VkDescriptorSetLayout bindingsSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSets[DESCRIPTOR_SET_COUNT] = {};
    };

    struct PhysicalDeviceVideoExtensions final
    {
        bool queue;
        bool decode_queue;
        bool decode_h264;
        bool decode_h265;
        bool encode_queue;
        bool encode_h264;
        bool encode_h265;
    };

    struct PhysicalDeviceExtensions final
    {
        // Core 1.4
        bool pushDescriptor;

        // Core 1.3
        bool maintenance4;
        bool dynamicRendering;
        bool synchronization2;
        bool extendedDynamicState;
        bool extendedDynamicState2;
        bool pipelineCreationCacheControl;
        bool formatFeatureFlags2;
        bool copyCommands2;

        // Extensions
        bool swapchain;
        bool memoryBudget;
        bool AMD_device_coherent_memory;
        bool EXT_memory_priority;
        bool deferredHostOperations;
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
        bool unifiedImageLayouts;
        bool mutableDescriptorType;
        bool descriptorHeap;

        PhysicalDeviceVideoExtensions video{};
        bool win32_full_screen_exclusive;
    };

    struct QueueFamilyIndices final
    {
        uint32_t queueFamilyCount = 0;

        uint32_t familyIndices[ecast(QueueType::Count)];
        uint32_t queueIndices[ecast(QueueType::Count)] = {};
        uint32_t counts[ecast(QueueType::Count)] = {};

        uint32_t timestampValidBits = 0;

        std::vector<uint32_t> queueOffsets;
        std::vector<std::vector<float>> queuePriorities;

        QueueFamilyIndices()
        {
            for (auto& index : familyIndices)
            {
                index = VK_QUEUE_FAMILY_IGNORED;
            }
        }

        bool IsComplete() const
        {
            return familyIndices[ecast(QueueType::Graphics)] != VK_QUEUE_FAMILY_IGNORED;
        }
    };

    class VulkanDevice final : public RHIDevice
    {
        friend class VulkanCommandBuffer;
        friend class VulkanComputePassEncoder;
        friend class VulkanRenderPassEncoder;
        friend struct VulkanBindlessManager;
        friend struct VulkanQueue;
        friend struct VulkanBindGroup;

    public:
#define VULKAN_DEVICE_FUNCTION(func) PFN_##func func;
#include "RHI_Vulkan_Funcs.h"

        VulkanDevice(VulkanRHIAdapter* adapter, const RHIDeviceDesc& desc);
        ~VulkanDevice() override;

        RHIBackendType GetBackend() const override { return RHIBackendType::Vulkan; }
        void SetLabel(const char* label) override;
        bool WaitIdle() override;
        uint64_t CommitFrame() override;

        RHIBufferRef CreateBufferCore(const BufferDesc& desc, const void* initialData) override;
        RHITextureRef CreateTextureCore(const TextureDescriptor& desc, const TextureData* initialData) override;
        RHITextureRef CreateTextureFromNativeHandleCore(RHINativeHandle handle, const TextureDescriptor& desc) override;
        RHISamplerRef CreateSamplerCore(const SamplerDesc& desc) override;

        VkSampler GetOrCreateVulkanSampler(const SamplerDesc* desc);
        VkDescriptorPool CreateDescriptorSetPool();

        RHIShaderModuleRef CreateShaderModuleCore(const ShaderModuleDesc& desc) override;
        RHIComputePipelineRef CreateComputePipelineCore(const ComputePipelineDescriptor& desc) override;
        RHIRenderPipelineRef CreateRenderPipelineCore(const RenderPipelineDescriptor& desc) override;
        RHIQueryHeapRef CreateQueryHeapCore(const QueryHeapDesc& desc) override;
        RHISwapChainRef CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc) override;
        void UpdateSwapChain(VulkanSwapChain* swapChain);

        void WriteShadingRateValue(ShadingRate rate, void* dest) const override;

        RHICommandBuffer* BeginCommandBuffer(QueueType queue, std::string_view label = "") override;

        void FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices);
        void FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices);
        void SetObjectName(VkObjectType objectType, uint64_t objectHandle, const char* label);
        bool GetImageFormatProperties(const VkImageCreateInfo& createInfo, const void* pNext, VkImageFormatProperties2* properties2) const;
        bool GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, const void* pNext, VkImageFormatProperties2* properties2) const;

        void ProcessDeletionQueue(bool force);

        bool QueryFeatureSupport(RHIFeature feature) override;
        PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) override;
        bool QueryVertexFormatSupport(VertexAttributeFormat format);
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
        VkFormat ToVkFormat(PixelFormat format);
        bool IsDepthStencilFormatSupported(VkFormat format) const;

        RHIAdapter* GetAdapter() const override;

        VkDevice GetHandle() const { return handle; }
        VulkanQueue& GetGraphicsQueue() { return queues[ecast(QueueType::Graphics)]; }
        VulkanQueue& GetComputeQueue() { return queues[ecast(QueueType::Compute)]; }
        VulkanQueue& GetCopyQueue() { return queues[ecast(QueueType::Copy)]; }

        /* Null resources */
        VkBuffer		nullBuffer = VK_NULL_HANDLE;
        VmaAllocation	nullBufferAllocation = VK_NULL_HANDLE;
        VkBufferView	nullBufferView = VK_NULL_HANDLE;
        VkSampler		nullSampler = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation1D = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation2D = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation3D = VK_NULL_HANDLE;
        VkImage			nullImage1D = VK_NULL_HANDLE;
        VkImage			nullImage2D = VK_NULL_HANDLE;
        VkImage			nullImage3D = VK_NULL_HANDLE;
        VkImageView		nullImageView1D = VK_NULL_HANDLE;
        VkImageView		nullImageView1DArray = VK_NULL_HANDLE;
        VkImageView		nullImageView2D = VK_NULL_HANDLE;
        VkImageView		nullImageView2DArray = VK_NULL_HANDLE;
        VkImageView		nullImageViewCube = VK_NULL_HANDLE;
        VkImageView		nullImageViewCubeArray = VK_NULL_HANDLE;
        VkImageView		nullImageView3D = VK_NULL_HANDLE;
        std::vector<VkSampler> vkStaticSamplers;

        // Deletion queue objects
        std::mutex destroyMutex;
        std::deque<std::pair<VmaAllocation, uint64_t>> destroyedAllocations;
        std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyedImages;
        std::deque<std::pair<VkImageView, uint64_t>> destroyedImageViews;
        std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> destroyedBuffers;
        std::deque<std::pair<VkBufferView, uint64_t>> destroyedBufferViews;
        std::deque<std::pair<VkShaderModule, uint64_t>> destroyedShaderModules;
        std::deque<std::pair<VkDescriptorSetLayout, uint64_t>> destroyedDescriptorSetLayouts;
        std::deque<std::pair<VkPipelineLayout, uint64_t>> destroyedPipelineLayouts;
        std::deque<std::pair<VkPipeline, uint64_t>> destroyedPipelines;
        std::deque<std::pair<VkQueryPool, uint64_t>> destroyedQueryPools;
        std::deque<std::pair<VkSemaphore, uint64_t>> destroyedSemaphores;
        std::deque<std::pair<VkSwapchainKHR, uint64_t>> destroyedSwapchains;
        //std::deque<std::pair<VkSurfaceKHR, uint64_t>> destroyedSurfaces;
        std::deque<std::pair<std::pair<VkDescriptorPool, VkDescriptorSet>, uint64_t>> destroyedDescriptorSets;

    private:
        bool shuttingDown{ false };
        VulkanRHIAdapter* _adapter;
        VkDevice handle = VK_NULL_HANDLE;
        VulkanQueue queues[ecast(QueueType::Count)];

        VmaAllocator allocator{ VK_NULL_HANDLE };
        VmaAllocator externalAllocator{ VK_NULL_HANDLE };

        struct CopyAllocator final
        {
            VulkanDevice* device = nullptr;
            std::mutex locker;

            std::vector<VulkanUploadContext> freeList;

            void Init(VulkanDevice* device);
            void Shutdown();
            VulkanUploadContext Allocate(uint64_t size);
            void Submit(VulkanUploadContext context);
        };
        mutable CopyAllocator copyAllocator;

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        VulkanBindlessManager* bindlessManager;

        std::vector<VkDynamicState> psoDynamicStates;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

        // Caches
        Vector<VkDescriptorPool> descriptorSetPools;
        UnorderedMap<size_t, VkSampler> samplerCache;

        std::vector<std::unique_ptr<VulkanCommandBuffer>> commandBuffers;
        uint32_t cmdBuffersCount = 0;
        std::mutex cmdBuffersLocker;
    };

    class VulkanRHIAdapter final : public RHIAdapter
    {
    public:
        VulkanRHIFactory* factory;
        bool debugUtils;
        VkPhysicalDevice handle;
        PhysicalDeviceExtensions extensions;
        QueueFamilyIndices queueFamilyIndices;
        bool synchronization2{ false };
        bool dynamicRendering{ false };
        bool supportsDepth32Stencil8{ false };
        bool supportsDepth24Stencil8{ false };
        bool supportsStencil8{ false };


        // Features
        VkPhysicalDeviceFeatures2 features2 = {};
        VkPhysicalDeviceVulkan11Features features11 = {};
        VkPhysicalDeviceVulkan12Features features12 = {};
        VkPhysicalDeviceVulkan13Features features13 = {};
        VkPhysicalDeviceVulkan14Features features14 = {};

        // Core 1.3
        VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
        VkPhysicalDeviceMaintenance4Properties maintenance4Properties = {};
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};

        // Core 1.4
        VkPhysicalDeviceMaintenance5Features maintenance5Features = {};
        VkPhysicalDeviceMaintenance6Features maintenance6Features = {};
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = {};
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProps = {};

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
        VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeatures{};
        VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeaturesEXT{};

        // Properties
        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDeviceVulkan11Properties properties11 = {};
        VkPhysicalDeviceVulkan12Properties properties12 = {};
        VkPhysicalDeviceVulkan13Properties properties13 = {};
        VkPhysicalDeviceVulkan14Properties properties14 = {};
        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = {};
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = {};
        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProps = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
        VkPhysicalDeviceMemoryProperties2 memoryProperties2 = {};
        VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptorHeapPropertiesEXT = {};

        VulkanRHIAdapter(VulkanRHIFactory* factory_, VkPhysicalDevice handle_);
        bool Init();

        RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) override;

        VkFormat ToVkFormat(PixelFormat format);
        bool IsDepthStencilFormatSupported(VkFormat format) const;

        RHIAdapterType GetType() const override { return _type; }

    private:
        RHIAdapterType _type = RHIAdapterType::Other;
    };

    class VulkanRHIFactory final : public RHIFactory
    {
    public:
#define VULKAN_INSTANCE_FUNCTION(name) PFN_##name name = nullptr;
#include "RHI_Vulkan_Funcs.h"

        bool debugUtils{ false };
        bool headless{ false };
        bool xlib_surface{ false };
        bool wayland_surface{ false };
        VkInstance handle = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
        Vector<VkSurfaceKHR> destroyedSurfaces;

        static bool IsSupported();

        VulkanRHIFactory(const RHIFactoryDesc& desc);
        ~VulkanRHIFactory() override;

        RHIBackendType GetBackend() const override { return RHIBackendType::Vulkan; }

        RHISurfaceRef CreateSurface(void* window, void* display) override;

        PhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice);
        QueueFamilyIndices QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue);
        bool GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
    };

    /* VulkanBuffer */
    VulkanBuffer::~VulkanBuffer()
    {
        const uint64_t frameCount = device->GetFrameCount();

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
        //allocation->SetName(device->allocator, newLabel.data());
    }

    RHINativeHandle VulkanBuffer::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            case RHINativeHandleType::VK_Buffer:
                return RHINativeHandle(handle);
            case RHINativeHandleType::SharedHandle:
                return RHINativeHandle(sharedHandle);
            default:
                return nullptr;
        }
    }

    /* VulkanTexture */
    VulkanTexture::~VulkanTexture()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();

        if (allocation != VK_NULL_HANDLE)
        {
            if (handle)
            {
                device->destroyedImages.push_back(std::make_pair(std::make_pair(handle, allocation), frameCount));
            }
            else if (stagingResource)
            {
                device->destroyedBuffers.push_back(std::make_pair(std::make_pair(stagingResource, allocation), frameCount));
            }
            else if (allocation)
            {
                device->destroyedAllocations.push_back(std::make_pair(allocation, frameCount));
            }
        }
        stagingResource = VK_NULL_HANDLE;
        handle = VK_NULL_HANDLE;
        allocation = nullptr;

        device->destroyMutex.unlock();
    }

    void VulkanTexture::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(handle), label);
        //allocation->SetName(device->allocator, newLabel.data());
    }

    RHINativeHandle VulkanTexture::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            case RHINativeHandleType::VK_Image:
                return RHINativeHandle(handle);
            case RHINativeHandleType::SharedHandle:
                return RHINativeHandle(sharedHandle);
            default:
                return nullptr;
        }
    }

    RHITextureViewRef VulkanTexture::CreateView(const TextureViewDesc& desc) const
    {
        SharedPtr<VulkanTextureView> textureView(new VulkanTextureView(this, desc));

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = handle;
        const bool isArray = depthOrArrayLayers > 1;
        // Should we expose TextureViewType enum?
        switch (dimension)
        {
            case TextureDimension::Texture1D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
                break;
            case TextureDimension::Texture2D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
                break;
            case TextureDimension::Texture3D:
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case TextureDimension::TextureCube:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
                break;
        }

        createInfo.format = device->ToVkFormat(desc.format);
        createInfo.components = ToVkSwizzle(desc.swizzle);
        createInfo.subresourceRange.aspectMask = GetImageAspectFlags(createInfo.format, desc.aspect);
        createInfo.subresourceRange.baseMipLevel = desc.baseMipLevel;
        createInfo.subresourceRange.levelCount = desc.mipLevelCount;
        createInfo.subresourceRange.baseArrayLayer = desc.baseArrayLayer;
        createInfo.subresourceRange.layerCount = desc.arrayLayerCount;

        const VkResult result = device->vkCreateImageView(device->GetHandle(), &createInfo, nullptr, &textureView->handle);
        if (result != VK_SUCCESS)
        {
            LOGE("Vulkan: Failed to create ImageView, error: {}", VkResultToString(result));
            return VK_NULL_HANDLE;
        }

        if (desc.label)
        {
            textureView->SetLabel(desc.label);
        }

        return textureView;
    }

    /* VulkanTexture */
    VulkanTextureView::~VulkanTextureView()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        device->destroyedImageViews.push_back(std::make_pair(handle, frameCount));
        device->destroyMutex.unlock();
    }

    void VulkanTextureView::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(handle), label);
    }

    RHINativeHandle VulkanTextureView::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            case RHINativeHandleType::VK_ImageView:
                return RHINativeHandle(handle);
            default:
                return nullptr;
        }
    }

    /* VulkanSampler */
    VulkanSampler::~VulkanSampler()
    {}

    void VulkanSampler::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanShaderModule */
    VulkanShaderModule::~VulkanShaderModule()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (stageInfo.module)
        {
            device->destroyedShaderModules.push_back(std::make_pair(stageInfo.module, frameCount));
        }
        stageInfo = {};
        device->destroyMutex.unlock();
    }

    void VulkanShaderModule::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(stageInfo.module), label);
    }

    /* VulkanComputePipeline */
    VulkanComputePipeline::~VulkanComputePipeline()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (handle)
        {
            device->destroyedPipelines.push_back(std::make_pair(handle, frameCount));
        }
        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanComputePipeline::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanRenderPipeline */
    VulkanRenderPipeline::~VulkanRenderPipeline()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (handle)
        {
            device->destroyedPipelines.push_back(std::make_pair(handle, frameCount));
        }
        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanRenderPipeline::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanQueryHeap */
    VulkanQueryHeap::~VulkanQueryHeap()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (handle)
        {
            device->destroyedQueryPools.push_back(std::make_pair(handle, frameCount));
        }
        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanQueryHeap::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_QUERY_POOL, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanSwapChain */
    VulkanRHISurface::~VulkanRHISurface()
    {
        if (handle != VK_NULL_HANDLE)
        {
            factory->destroyedSurfaces.push_back(handle);
            handle = VK_NULL_HANDLE;
        }
    }

    /* VulkanSwapChain */
    VulkanSwapChain::~VulkanSwapChain()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();

        if (handle)
        {
            device->destroyedSwapchains.push_back(std::make_pair(handle, frameCount));
        }

        for (size_t i = 0; i < backbufferTextures.size(); ++i)
        {
            device->destroyedSemaphores.push_back(std::make_pair(acquireSemaphores[i], frameCount));
            device->destroyedSemaphores.push_back(std::make_pair(releaseSemaphores[i], frameCount));
        }

        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanSwapChain::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanComputePassEncoder */
    VulkanComputePassEncoder::VulkanComputePassEncoder(VulkanDevice* device, VulkanCommandBuffer* commandBuffer)
        : _device(device)
        , _commandBuffer(commandBuffer)
    {

    }

    VulkanComputePassEncoder::~VulkanComputePassEncoder()
    {

    }

    void VulkanComputePassEncoder::Reset(VkCommandBuffer commandBuffer)
    {
        _vkCommandBuffer = commandBuffer;
        ClearState();
    }

    void VulkanComputePassEncoder::Begin(const ComputePassDescriptor& descriptor)
    {
        ClearState();

        if (descriptor.label)
        {
            _commandBuffer->PushDebugGroup(descriptor.label);
            _hasLabel = true;
        }
        else
        {
            _hasLabel = false;
        }
    }

    void VulkanComputePassEncoder::PushDebugGroup(std::string_view groupLabel)
    {
        _commandBuffer->PushDebugGroup(groupLabel);
    }

    void VulkanComputePassEncoder::PopDebugGroup()
    {
        _commandBuffer->PopDebugGroup();
    }

    void VulkanComputePassEncoder::InsertDebugMarker(std::string_view markerLabel)
    {
        _commandBuffer->InsertDebugMarker(markerLabel);
    }

    void VulkanComputePassEncoder::CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer)
    {
        auto backendSrcBuffer = static_cast<const VulkanBuffer*>(sourceBuffer);
        auto backendDestBuffer = static_cast<const VulkanBuffer*>(destinationBuffer);

        _commandBuffer->BufferBarrier(backendSrcBuffer, BufferStates::CopySource);
        _commandBuffer->BufferBarrier(backendDestBuffer, BufferStates::CopyDest);
        _commandBuffer->CommitBarriers();

        VkBufferCopy copy = {};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = Min(backendSrcBuffer->GetSize(), backendDestBuffer->GetSize());

        _device->vkCmdCopyBuffer(_vkCommandBuffer,
            backendSrcBuffer->handle,
            backendDestBuffer->handle,
            1, &copy
        );
    }

    void VulkanComputePassEncoder::CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size)
    {
        auto backendSrcBuffer = static_cast<const VulkanBuffer*>(sourceBuffer);
        auto backendDestBuffer = static_cast<const VulkanBuffer*>(destinationBuffer);

        _commandBuffer->BufferBarrier(backendSrcBuffer, BufferStates::CopySource);
        _commandBuffer->BufferBarrier(backendDestBuffer, BufferStates::CopyDest);
        _commandBuffer->CommitBarriers();

        VkBufferCopy copy = {};
        copy.srcOffset = sourceOffset;
        copy.dstOffset = destinationOffset;
        copy.size = size;

        _device->vkCmdCopyBuffer(_vkCommandBuffer,
            backendSrcBuffer->handle,
            backendDestBuffer->handle,
            1, &copy
        );
    }

    void VulkanComputePassEncoder::SetPipeline(RHIComputePipeline* pipeline)
    {
        if (_currentPipeline.Get() == pipeline)
            return;

        VulkanComputePipeline* backendPipeline = static_cast<VulkanComputePipeline*>(pipeline);
        _device->vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, backendPipeline->handle);
        _device->bindlessManager->BindDescriptorSets(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);

        _currentPipeline = backendPipeline;
    }

    void VulkanComputePassEncoder::SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset)
    {
        _device->vkCmdPushConstants(_vkCommandBuffer,
            _device->bindlessManager->pipelineLayout,
            VK_SHADER_STAGE_ALL,
            offset,
            size,
            data
        );
    }

    void VulkanComputePassEncoder::ClearState()
    {
        _hasLabel = false;
        _currentPipeline.Reset();
    }

    void VulkanComputePassEncoder::PrepareDispatch()
    {
        _commandBuffer->FlushBindGroups();
    }

    void VulkanComputePassEncoder::DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        PrepareDispatch();

        _device->vkCmdDispatch(_vkCommandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    void VulkanComputePassEncoder::DispatchIndirectCore(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
        PrepareDispatch();

        auto backendIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        _device->vkCmdDispatchIndirect(_vkCommandBuffer, backendIndirectBuffer->handle, indirectBufferOffset);
    }


    void VulkanComputePassEncoder::End()
    {
        if (_hasLabel)
        {
            PopDebugGroup();
            _hasLabel = false;
        }

        _commandBuffer->EndEncoding();
        ClearState();
    }

    RHICommandBuffer* VulkanComputePassEncoder::GetCommandBuffer() const
    {
        return _commandBuffer;
    }

    /* VulkanRenderPassEncoder */
    VulkanRenderPassEncoder::VulkanRenderPassEncoder(VulkanDevice* device, VulkanCommandBuffer* commandBuffer)
        : _device(device)
        , _commandBuffer(commandBuffer)
    {

    }

    VulkanRenderPassEncoder::~VulkanRenderPassEncoder()
    {

    }

    void VulkanRenderPassEncoder::ClearState()
    {
        _hasLabel = false;
        _currentShadingRate = ShadingRate::Invalid;
        _currentPipeline.Reset();
    }

    void VulkanRenderPassEncoder::Reset(VkCommandBuffer commandBuffer)
    {
        _vkCommandBuffer = commandBuffer;
        ClearState();
    }

    void VulkanRenderPassEncoder::Begin(const RenderPassDesc& descriptor)
    {
        if (descriptor.label)
        {
            _commandBuffer->PushDebugGroup(descriptor.label);
            _hasLabel = true;
        }
        else
        {
            _hasLabel = false;
        }

        VkRect2D renderArea = {};
        renderArea.extent.width = _device->_adapter->properties2.properties.limits.maxFramebufferWidth;
        renderArea.extent.height = _device->_adapter->properties2.properties.limits.maxFramebufferHeight;
        uint32_t layerCount = _device->_adapter->properties2.properties.limits.maxFramebufferLayers;

        uint32_t colorAttachmentCount = 0;
        VkRenderingAttachmentInfo colorAttachments[kMaxColorAttachments] = {};
        VkRenderingAttachmentInfo depthAttachment = {};
        VkRenderingAttachmentInfo stencilAttachment = {};

        PixelFormat depthStencilFormat = descriptor.depthStencilAttachment != nullptr ? descriptor.depthStencilAttachment->view->GetFormat() : PixelFormat::Undefined;
        const bool hasDepthOrStencil = descriptor.depthStencilAttachment != nullptr;

        for (uint32_t i = 0; i < descriptor.colorAttachmentCount; ++i)
        {
            ALIMER_VERIFY(descriptor.colorAttachments[i].view != nullptr);

            const RenderPassColorAttachment& attachment = descriptor.colorAttachments[i];
            VulkanTextureView* view = static_cast<VulkanTextureView*>(attachment.view);

            renderArea.extent.width = std::min(renderArea.extent.width, view->GetWidth());
            renderArea.extent.height = std::min(renderArea.extent.height, view->GetHeight());
            layerCount = std::min(layerCount, view->GetArrayLayerCount());

            VkRenderingAttachmentInfo& attachmentInfo = colorAttachments[colorAttachmentCount++];
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = view->handle;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = ToVk(attachment.loadAction);
            attachmentInfo.storeOp = ToVk(attachment.storeAction);
            attachmentInfo.clearValue.color.float32[0] = attachment.clearColor.float32[0];
            attachmentInfo.clearValue.color.float32[1] = attachment.clearColor.float32[1];
            attachmentInfo.clearValue.color.float32[2] = attachment.clearColor.float32[2];
            attachmentInfo.clearValue.color.float32[3] = attachment.clearColor.float32[3];

            // Barrier
            _commandBuffer->TextureBarrier(view, TextureLayout::RenderTarget);
        }

        if (hasDepthOrStencil)
        {
            const RenderPassDepthStencilAttachment& attachment = *descriptor.depthStencilAttachment;

            VulkanTextureView* view = static_cast<VulkanTextureView*>(attachment.view);

            renderArea.extent.width = std::min(renderArea.extent.width, view->GetWidth());
            renderArea.extent.height = std::min(renderArea.extent.height, view->GetHeight());
            layerCount = std::min(layerCount, view->GetTexture()->GetArrayLayers());

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = view->handle;
            depthAttachment.imageLayout = attachment.depthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.loadOp = ToVk(attachment.depthLoadAction);
            depthAttachment.storeOp = ToVk(attachment.depthStoreAction);
            depthAttachment.clearValue.depthStencil.depth = attachment.depthClearValue;

            // Barrier
            _commandBuffer->TextureBarrier(view, attachment.depthReadOnly ? TextureLayout::DepthRead : TextureLayout::DepthWrite);
        }
        _commandBuffer->CommitBarriers();

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
        renderingInfo.pStencilAttachment = hasDepthOrStencil && !IsDepthOnlyFormat(depthStencilFormat) ? &depthAttachment : nullptr;

        _device->vkCmdBeginRendering(_vkCommandBuffer, &renderingInfo);

        // The viewport and scissor default to cover all of the attachments
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(renderArea.extent.height);
        viewport.width = static_cast<float>(renderArea.extent.width);
        viewport.height = -static_cast<float>(renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        _device->vkCmdSetViewport(_vkCommandBuffer, 0, 1, &viewport);

        VkRect2D scissorRect{};
        scissorRect.offset.x = 0;
        scissorRect.offset.y = 0;
        scissorRect.extent.width = renderArea.extent.width;
        scissorRect.extent.height = renderArea.extent.height;
        _device->vkCmdSetScissor(_vkCommandBuffer, 0, 1, &scissorRect);
    }

    void VulkanRenderPassEncoder::PushDebugGroup(std::string_view groupLabel)
    {
        _commandBuffer->PushDebugGroup(groupLabel);
    }

    void VulkanRenderPassEncoder::PopDebugGroup()
    {
        _commandBuffer->PopDebugGroup();
    }

    void VulkanRenderPassEncoder::InsertDebugMarker(std::string_view markerLabel)
    {
        _commandBuffer->InsertDebugMarker(markerLabel);
    }

    void VulkanRenderPassEncoder::SetViewport(const Viewport& viewport)
    {
        // Flip viewport to match DirectX coordinate system
        VkViewport vkViewport{};
        vkViewport.x = viewport.x;
        vkViewport.y = viewport.height - viewport.y;
        vkViewport.width = viewport.width;
        vkViewport.height = -viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        _device->vkCmdSetViewport(_vkCommandBuffer, 0, 1, &vkViewport);
    }

    void VulkanRenderPassEncoder::SetViewports(const Viewport* viewports, uint32_t count)
    {
        ALIMER_ASSERT(viewports != nullptr);
        ALIMER_ASSERT(count < _device->GetLimits().maxViewports);

        VkViewport vkViewports[kMaxViewportsAndScissors] = {};
        for (uint32_t i = 0; i < count; ++i)
        {
            // Flip viewport to match DirectX coordinate system
            vkViewports[i].x = viewports[i].x;
            vkViewports[i].y = viewports[i].height - viewports[i].y;
            vkViewports[i].width = viewports[i].width;
            vkViewports[i].height = -viewports[i].height;
            vkViewports[i].minDepth = viewports[i].minDepth;
            vkViewports[i].maxDepth = viewports[i].maxDepth;
        }

        _device->vkCmdSetViewport(_vkCommandBuffer, 0, count, vkViewports);
    }

    void VulkanRenderPassEncoder::SetScissorRect(const ScissorRect& rect)
    {
        _device->vkCmdSetScissor(_vkCommandBuffer, 0, 1, (VkRect2D*)&rect);
    }

    void VulkanRenderPassEncoder::SetScissorRects(const ScissorRect* scissorRects, uint32_t count)
    {
        ALIMER_ASSERT(scissorRects != nullptr);
        ALIMER_ASSERT(count < _device->GetLimits().maxViewports);

        _device->vkCmdSetScissor(_vkCommandBuffer, 0, count, (const VkRect2D*)scissorRects);
    }


    void VulkanRenderPassEncoder::SetStencilReference(uint32_t reference)
    {
        _device->vkCmdSetStencilReference(_vkCommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, reference);
    }

    void VulkanRenderPassEncoder::SetBlendColor(const Color& color)
    {
        const float blendColor[4] = { color.r, color.g, color.b, color.a };
        _device->vkCmdSetBlendConstants(_vkCommandBuffer, blendColor);
    }

    void VulkanRenderPassEncoder::SetShadingRate(ShadingRate rate)
    {
        if (_device->_adapter->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE
            && _currentShadingRate != rate)
        {
            _currentShadingRate = rate;

            VkExtent2D fragmentSize;
            switch (rate)
            {
                case ShadingRate::Rate1x1:
                    fragmentSize.width = 1;
                    fragmentSize.height = 1;
                    break;
                case ShadingRate::Rate1x2:
                    fragmentSize.width = 1;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate::Rate2x1:
                    fragmentSize.width = 2;
                    fragmentSize.height = 1;
                    break;
                case ShadingRate::Rate2x2:
                    fragmentSize.width = 2;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate::Rate2x4:
                    fragmentSize.width = 2;
                    fragmentSize.height = 4;
                    break;
                case ShadingRate::Rate4x2:
                    fragmentSize.width = 4;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate::Rate4x4:
                    fragmentSize.width = 4;
                    fragmentSize.height = 4;
                    break;
                default:
                    break;
            }

            VkFragmentShadingRateCombinerOpKHR combiner[] = {
                VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
            };

            if (_device->_adapter->fragmentShadingRateProperties.fragmentShadingRateNonTrivialCombinerOps == VK_TRUE)
            {
                if (_device->_adapter->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
                {
                    combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
                }
                if (_device->_adapter->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
                {
                    combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
                }
            }
            else
            {
                if (_device->_adapter->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
                {
                    combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
                }
                if (_device->_adapter->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
                {
                    combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
                }
            }

            _device->vkCmdSetFragmentShadingRateKHR(_vkCommandBuffer, &fragmentSize, combiner);
        }
    }

    void VulkanRenderPassEncoder::SetDepthBounds(float minBounds, float maxBounds)
    {
        if (_device->_adapter->features2.features.depthBounds == VK_TRUE)
        {
            _device->vkCmdSetDepthBounds(_vkCommandBuffer, minBounds, maxBounds);
        }
        else
        {
            LOGW("DepthBounds is not supported");
        }
    }

    void VulkanRenderPassEncoder::SetPipeline(RHIRenderPipeline* pipeline)
    {
        if (_currentPipeline.Get() == pipeline)
            return;

        VulkanRenderPipeline* newPipeline = static_cast<VulkanRenderPipeline*>(pipeline);
        _device->vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, newPipeline->handle);
        _device->bindlessManager->BindDescriptorSets(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

        _currentPipeline = newPipeline;
    }

    void VulkanRenderPassEncoder::SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset)
    {
        _device->vkCmdPushConstants(_vkCommandBuffer,
            _device->bindlessManager->pipelineLayout,
            VK_SHADER_STAGE_ALL,
            offset,
            size,
            data
        );
    }

    void VulkanRenderPassEncoder::SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset)
    {
        auto backendBuffer = static_cast<const VulkanBuffer*>(buffer);

        _device->vkCmdBindVertexBuffers(_vkCommandBuffer, slot, 1u, &backendBuffer->handle, &offset);
    }

    void VulkanRenderPassEncoder::SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets)
    {
        ALIMER_ASSERT(buffers != nullptr);
        ALIMER_ASSERT(count <= ALIMER_STATIC_ARRAY_SIZE(buffers));
        ALIMER_ASSERT(count <= _device->_adapter->properties2.properties.limits.maxVertexInputBindings);

        VkBuffer vkBuffers[kMaxVertexBuffers];
        for (uint32_t i = 0; i < count; ++i)
        {
            if (buffers[i] == nullptr)
            {
                vkBuffers[i] = _device->nullBuffer;
            }
            else
            {
                vkBuffers[i] = static_cast<const VulkanBuffer*>(buffers[i])->handle;
            }
        }

        _device->vkCmdBindVertexBuffers(_vkCommandBuffer, slot, count, vkBuffers, offsets);
    }

    void VulkanRenderPassEncoder::SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexFormat format)
    {
        if (format == IndexFormat::Undefined)
        {
            LOGE("Invalid index format, cannot be IndexFormat.Undefined.");
            return;
        }

        auto backendBuffer = static_cast<const VulkanBuffer*>(buffer);
        const VkIndexType vkIndexType = (format == IndexFormat::Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

        _device->vkCmdBindIndexBuffer(_vkCommandBuffer, backendBuffer->handle, offset, vkIndexType);
    }

    void VulkanRenderPassEncoder::PrepareDraw()
    {
        _commandBuffer->FlushBindGroups();
    }

    void VulkanRenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        PrepareDraw();

        _device->vkCmdDraw(_vkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanRenderPassEncoder::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        PrepareDraw();

        _device->vkCmdDrawIndexed(_vkCommandBuffer, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void VulkanRenderPassEncoder::DrawIndirect(const RHIBuffer* buffer, uint64_t offset)
    {
        ALIMER_ASSERT(buffer);
        PrepareDraw();

        auto vulkanBuffer = static_cast<const VulkanBuffer*>(buffer);
        _device->vkCmdDrawIndirect(_vkCommandBuffer, vulkanBuffer->handle, offset, 1, (uint32_t)sizeof(DrawIndirectCommand));
    }

    void VulkanRenderPassEncoder::DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
        ALIMER_ASSERT(indirectBuffer);
        PrepareDraw();

        auto backendBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        _device->vkCmdDrawIndexedIndirect(_vkCommandBuffer, backendBuffer->handle, indirectBufferOffset, 1, (uint32_t)sizeof(DrawIndexedIndirectCommand));
    }

    void VulkanRenderPassEncoder::DrawMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        PrepareDraw();

        _device->vkCmdDrawMeshTasksEXT(_vkCommandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    void VulkanRenderPassEncoder::DrawMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
        ALIMER_ASSERT(indirectBuffer);
        PrepareDraw();

        auto backendIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        _device->vkCmdDrawMeshTasksIndirectEXT(_vkCommandBuffer,
            backendIndirectBuffer->handle,
            indirectBufferOffset,
            1,
            sizeof(DispatchIndirectCommand)
        );
    }

    void VulkanRenderPassEncoder::DrawMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount)
    {
        ALIMER_ASSERT(indirectBuffer);
        ALIMER_ASSERT(countBuffer);

        auto backendIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        auto vulkanCountBuffer = static_cast<const VulkanBuffer*>(countBuffer);

        PrepareDraw();
        _device->vkCmdDrawMeshTasksIndirectCountEXT(_vkCommandBuffer,
            backendIndirectBuffer->handle, indirectBufferOffset,
            vulkanCountBuffer->handle, countBufferOffset,
            maxCount, sizeof(DispatchIndirectCommand));
    }

    void VulkanRenderPassEncoder::End()
    {
        _device->vkCmdEndRendering(_vkCommandBuffer);

        if (_hasLabel)
        {
            PopDebugGroup();
            _hasLabel = false;
        }

        _commandBuffer->EndEncoding();
        ClearState();
    }

    RHICommandBuffer* VulkanRenderPassEncoder::GetCommandBuffer() const
    {
        return _commandBuffer;
    }

    /* VulkanCommandBuffer */
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device_, QueueType queueType_, uint32_t id_)
        : device(device_)
        , queueType(queueType_)
        , id(id_)
    {
        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device_->_adapter->queueFamilyIndices.familyIndices[ecast(queueType_)];

            VK_CHECK(device->vkCreateCommandPool(device->handle, &poolInfo, nullptr, &commandPools[i]));

            VkCommandBufferAllocateInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferInfo.commandPool = commandPools[i];
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferInfo.commandBufferCount = 1;
            VK_CHECK(device->vkAllocateCommandBuffers(device->handle, &commandBufferInfo, &commandBuffers[i]));
        }

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(device->vkCreateSemaphore(device->handle, &semaphoreInfo, nullptr, &semaphore));

        _renderPassEncoder = new VulkanRenderPassEncoder(device, this);
        _computePassEncoder = new VulkanComputePassEncoder(device, this);
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        SafeDelete(_renderPassEncoder);
        SafeDelete(_computePassEncoder);

        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            device->vkDestroyCommandPool(device->handle, commandPools[i], nullptr);
        }

        device->vkDestroySemaphore(device->handle, semaphore, nullptr);
    }

    void VulkanCommandBuffer::Begin(uint32_t frameIndex, std::string_view label)
    {
        RHICommandBuffer::Reset(frameIndex);
        waits.clear();
        hasPendingWaits.store(false);
        presentSwapChains.clear();
        memoryBarriers.clear();
        imageBarriers.clear();
        bufferBarriers.clear();

        VK_CHECK(device->vkResetCommandPool(device->handle, commandPools[frameIndex], 0));
        commandBuffer = commandBuffers[frameIndex];

        const VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(device->vkBeginCommandBuffer(commandBuffer, &beginInfo));

        if (queueType == QueueType::Graphics)
        {
            VkRect2D scissors[16];
            for (uint32_t i = 0; i < ALIMER_STATIC_ARRAY_SIZE(scissors); ++i)
            {
                scissors[i].offset.x = 0;
                scissors[i].offset.y = 0;
                scissors[i].extent.width = 65535;
                scissors[i].extent.height = 65535;
            }
            device->vkCmdSetScissor(commandBuffer, 0, ALIMER_STATIC_ARRAY_SIZE(scissors), scissors);

            const float blendConstants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            device->vkCmdSetBlendConstants(commandBuffer, blendConstants);
            device->vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FRONT_AND_BACK, ~0u);

            if (device->_adapter->features2.features.depthBounds == VK_TRUE)
            {
                device->vkCmdSetDepthBounds(commandBuffer, 0.0f, 1.0f);
            }

            // Silence validation about uninitialized stride:
            //const VkDeviceSize zero = {};
            //device->vkCmdBindVertexBuffers2(commandBuffer, 0, 1, &nullBuffer, &zero, &zero, &zero);

            if (device->_adapter->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            {
                VkExtent2D fragmentSize = {};
                fragmentSize.width = 1;
                fragmentSize.height = 1;

                VkFragmentShadingRateCombinerOpKHR combiner[] = {
                    VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                    VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
                };

                device->vkCmdSetFragmentShadingRateKHR(commandBuffer, &fragmentSize, combiner);
            }
        }

        _renderPassEncoder->Reset(commandBuffer);
        _computePassEncoder->Reset(commandBuffer);

        hasLabel = !label.empty();
        if (hasLabel)
        {
            PushDebugGroup(label);
            hasLabel = true;
        }
    }

    VkCommandBuffer VulkanCommandBuffer::End()
    {
        for (auto& swapChain : presentSwapChains)
        {
            VulkanTexture* swapChainTexture = swapChain->backbufferTextures[swapChain->imageIndex].Get();
            TextureBarrier(swapChainTexture, TextureLayout::Present, 0, 1, 0, 1);
        }
        CommitBarriers();

        if (hasLabel)
        {
            PopDebugGroup();
        }

        VK_CHECK(device->vkEndCommandBuffer(commandBuffer));
        return commandBuffer;
    }

    void VulkanCommandBuffer::EndEncoding()
    {
        _encoderActive = false;
    }

    RHIDevice* VulkanCommandBuffer::GetDevice() const
    {
        return device;
    }

    void VulkanCommandBuffer::PushDebugGroup(std::string_view name)
    {
        if (!device->_adapter->debugUtils)
            return;

        VkDebugUtilsLabelEXT label = {};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        device->_adapter->factory->vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
    }

    void VulkanCommandBuffer::PopDebugGroup()
    {
        if (!device->_adapter->debugUtils)
            return;

        device->_adapter->factory->vkCmdEndDebugUtilsLabelEXT(commandBuffer);
    }

    void VulkanCommandBuffer::InsertDebugMarker(std::string_view name)
    {
        if (!device->_adapter->debugUtils)
            return;

        VkDebugUtilsLabelEXT label = {};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        device->_adapter->factory->vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &label);
    }

    void VulkanCommandBuffer::BufferBarrier(const VulkanBuffer* buffer, BufferStates newState)
    {
        BufferStates currentState = buffer->GetCurrentState();
        if (currentState == newState)
            return;

        VkBufferStateMapping before = ConvertBufferState(currentState);
        VkBufferStateMapping after = ConvertBufferState(newState);

        VkBufferMemoryBarrier2 barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.srcStageMask = before.stageFlags;
        barrier.srcAccessMask = before.accessMask;
        barrier.dstStageMask = after.stageFlags;
        barrier.dstAccessMask = after.accessMask;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = buffer->handle;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        bufferBarriers.push_back(barrier);

        // Update the buffer state
        buffer->SetCurrentState(newState);
        numBarriersToCommit++;

        if (numBarriersToCommit >= kMaxBarrierCount)
            CommitBarriers();
    }

    void VulkanCommandBuffer::TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, TextureAspect aspect)
    {
        const uint32_t mipLevelCount = texture->GetMipLevelCount();
        const uint32_t subresource = CalculateSubresource(baseMiplevel, baseArrayLayer, mipLevelCount);
        TextureLayout currentLayout = texture->imageLayouts[subresource];
        if (currentLayout == newLayout)
            return;

        const bool depthOnlyFormat = IsDepthOnlyFormat(texture->GetFormat());
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
        barrier.subresourceRange.aspectMask = GetImageAspectFlags(texture->vkFormat, aspect);
        barrier.subresourceRange.baseMipLevel = baseMiplevel;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount = layerCount;

        imageBarriers.push_back(barrier);


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

    void VulkanCommandBuffer::TextureBarrier(const VulkanTextureView* view, TextureLayout newLayout)
    {
        const VulkanTexture* backendTexture = static_cast<const VulkanTexture*>(view->GetTexture());
        TextureBarrier(backendTexture, newLayout,
            view->GetBaseMipLevel(), view->GetMipLevelCount(),
            view->GetBaseArrayLayer(), view->GetArrayLayerCount(),
            view->GetAspect()
        );
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
            device->vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

            memoryBarriers.clear();
            imageBarriers.clear();
            bufferBarriers.clear();
        }

        numBarriersToCommit = 0;
    }

    void VulkanCommandBuffer::FlushBindGroups()
    {
#if TODO
        if (!currentPipelineLayout)
            return;

        ALIMER_ASSERT_MSG(currentPipelineLayout.Get() != nullptr, "No PipelineLayout bound");
        ALIMER_ASSERT_MSG(currentPipeline.Get() != nullptr, "No Pipeline bound");

        if (!bindGroupsDirty || !currentPipelineLayout->bindGroupLayoutCount)
            return;

        device->vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            currentPipelineLayout->handle,
            0u,
            currentPipelineLayout->bindGroupLayoutCount,
            descriptorSets,
            0, nullptr
        );
        bindGroupsDirty = false;
#endif

    }

    void VulkanCommandBuffer::BeginQuery(const RHIQueryHeap* heap, uint32_t index)
    {
        auto vulkanHeap = static_cast<const VulkanQueryHeap*>(heap);

        switch (vulkanHeap->desc.type)
        {
            case QueryType::Occlusion:
                device->vkCmdBeginQuery(commandBuffer, vulkanHeap->handle, index, VK_QUERY_CONTROL_PRECISE_BIT);
                break;
            case QueryType::BinaryOcclusion:
            case QueryType::PipelineStatistics:
                device->vkCmdBeginQuery(commandBuffer, vulkanHeap->handle, index, 0);
                break;

            default:
            case QueryType::Timestamp:
                break;
        }
    }

    void VulkanCommandBuffer::EndQuery(const RHIQueryHeap* heap, uint32_t index)
    {
        auto vulkanHeap = static_cast<const VulkanQueryHeap*>(heap);

        switch (vulkanHeap->desc.type)
        {
            case QueryType::Timestamp:
                // Should be VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT?
                device->vkCmdWriteTimestamp2(commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, vulkanHeap->handle, index);
                break;
            case QueryType::Occlusion:
            case QueryType::BinaryOcclusion:
            case QueryType::PipelineStatistics:
                device->vkCmdEndQuery(commandBuffer, vulkanHeap->handle, index);
                break;
        }
    }

    void VulkanCommandBuffer::ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset)
    {
        auto vulkanHeap = static_cast<const VulkanQueryHeap*>(heap);
        auto vulkanDestBuffer = static_cast<const VulkanBuffer*>(destinationBuffer);

        VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

        switch (vulkanHeap->desc.type)
        {
            case QueryType::BinaryOcclusion:
                flags |= VK_QUERY_RESULT_PARTIAL_BIT;
                break;
            default:
                break;
        }

        device->vkCmdCopyQueryPoolResults(
            commandBuffer,
            vulkanHeap->handle,
            index,
            count,
            vulkanDestBuffer->handle,
            destinationOffset,
            vulkanHeap->queryResultSize,
            flags
        );
    }

    void VulkanCommandBuffer::ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count)
    {
        auto vulkanHeap = static_cast<const VulkanQueryHeap*>(heap);
        device->vkCmdResetQueryPool(commandBuffer, vulkanHeap->handle, index, count);
    }

    RHITexture* VulkanCommandBuffer::AcquireSwapChainTexture(RHISwapChain* swapChain)
    {
        VulkanSwapChain* backendSwapChain = (VulkanSwapChain*)swapChain;
        const size_t swapChainAcquireSemaphoreIndex = backendSwapChain->acquireSemaphoreIndex;

        backendSwapChain->locker.lock();
        VkResult result = device->vkAcquireNextImageKHR(
            device->handle,
            backendSwapChain->handle,
            UINT64_MAX,
            backendSwapChain->acquireSemaphores[swapChainAcquireSemaphoreIndex],
            VK_NULL_HANDLE,
            &backendSwapChain->imageIndex
        );
        backendSwapChain->locker.unlock();

        if (result != VK_SUCCESS)
        {
            // Handle outdated error in acquire
            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                // we need to create a new semaphore or jump through a few hoops to
                // wait for the current one to be unsignalled before we can use it again
                // creating a new one is easiest. See also:
                // https://github.com/KhronosGroup/Vulkan-Docs/issues/152
                // https://www.khronos.org/blog/resolving-longstanding-issues-with-wsi
                {
                    std::scoped_lock lock(device->destroyMutex);
                    for (auto& x : backendSwapChain->acquireSemaphores)
                    {
                        device->destroyedSemaphores.emplace_back(x, device->_frameCount);
                    }
                }
                backendSwapChain->acquireSemaphores.clear();

                //device->WaitIdle();
                device->UpdateSwapChain(backendSwapChain);
                return AcquireSwapChainTexture(backendSwapChain);
            }
        }

        VulkanTexture* swapChainTexture = backendSwapChain->backbufferTextures[backendSwapChain->imageIndex].Get();

        presentSwapChains.push_back(SharedPtr<VulkanSwapChain>(backendSwapChain));
        return swapChainTexture;
    }

    RHIComputePassEncoder* VulkanCommandBuffer::BeginComputePassCore(const ComputePassDescriptor& descriptor)
    {
        _computePassEncoder->Begin(descriptor);
        return _computePassEncoder;
    }

    RHIRenderPassEncoder* VulkanCommandBuffer::BeginRenderPassCore(const RenderPassDesc& descriptor)
    {
        _renderPassEncoder->Begin(descriptor);
        return _renderPassEncoder;
    }

    void VulkanCommandBuffer::BeginPredication(const RHIBuffer* buffer, uint64_t offset, PredicationOperation operation)
    {
        if (device->_adapter->conditionalRenderingFeatures.conditionalRendering == VK_TRUE)
        {
            auto vulkanBuffer = static_cast<const VulkanBuffer*>(buffer);
            BufferBarrier(vulkanBuffer, BufferStates::Predication);
            CommitBarriers();

            VkConditionalRenderingBeginInfoEXT info = {};
            info.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
            if (operation == PredicationOperation::NotEqualZero)
            {
                info.flags = VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;
            }
            info.buffer = vulkanBuffer->handle;
            info.offset = offset;

            device->vkCmdBeginConditionalRenderingEXT(commandBuffer, &info);
        }
    }

    void VulkanCommandBuffer::EndPredication()
    {
        if (device->_adapter->conditionalRenderingFeatures.conditionalRendering == VK_TRUE)
        {
            device->vkCmdEndConditionalRenderingEXT(commandBuffer);
        }
    }

    void VulkanBindlessDescriptorHeap::Init(VulkanDevice* device, VkDescriptorType type, uint32_t descriptorCount)
    {
        descriptorCount = std::min(descriptorCount, 500000u);

        const VkDescriptorPoolSize poolSize = {
            .type = type,
            .descriptorCount = descriptorCount
        };

        const VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };
        VK_CHECK(device->vkCreateDescriptorPool(device->GetHandle(), &poolCreateInfo, nullptr, &descriptorPool));

        const VkDescriptorSetLayoutBinding setLayoutBinding = {
            .binding = 0,
            .descriptorType = type,
            .descriptorCount = descriptorCount,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        const VkDescriptorBindingFlags bindingFlags =
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        const VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlagsCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1,
            .pBindingFlags = &bindingFlags
        };

        const VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &setLayoutBindingFlagsCreateInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1,
            .pBindings = &setLayoutBinding,
        };
        VK_CHECK(device->vkCreateDescriptorSetLayout(device->GetHandle(), &setLayoutCreateInfo, nullptr, &descriptorSetLayout));

        const VkDescriptorSetVariableDescriptorCountAllocateInfo setVariableDescriptorCountAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &descriptorCount
        };

        const VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = &setVariableDescriptorCountAllocateInfo,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
        };
        VK_CHECK(device->vkAllocateDescriptorSets(device->GetHandle(), &allocateInfo, &descriptorSet));

        for (uint32_t i = 0; i < descriptorCount; ++i)
        {
            freeList.push_back((BindlessIndex)(descriptorCount - i - 1));
        }

        // Descriptor safety feature:
        //	We init null descriptors for bindless index = 0 for access safety
        //	Because shader compiler sometimes incorrectly loads descriptor outside of safety branch
        //	Note: these are never freed, this is intentional
        if (type != VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
        {
            BindlessIndex index = Allocate();
            ALIMER_ASSERT_MSG(index == 0, "Bindless descriptor safety feature error: descriptor index must be 0!");

            VkWriteDescriptorSet writeDescriptorSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSet,
                .dstBinding = 0,
                .dstArrayElement = (uint32_t)index,
                .descriptorCount = 1,
                .descriptorType = type,
            };

            VkDescriptorImageInfo imageInfo = {};
            VkDescriptorBufferInfo bufferInfo = {};

            switch (type)
            {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    imageInfo.imageView = device->nullImageView2D;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    writeDescriptorSet.pImageInfo = &imageInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    writeDescriptorSet.pTexelBufferView = &device->nullBufferView;
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    bufferInfo.buffer = device->nullBuffer;
                    bufferInfo.range = VK_WHOLE_SIZE;
                    writeDescriptorSet.pBufferInfo = &bufferInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    imageInfo.imageView = device->nullImageView2D;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    writeDescriptorSet.pImageInfo = &imageInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    imageInfo.sampler = device->nullSampler;
                    writeDescriptorSet.pImageInfo = &imageInfo;
                    break;
                default:
                    ALIMER_ASSERT_FAIL("Bindless: Descriptor safety feature error: descriptor type not handled!");
                    break;
            }

            device->vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptorSet, 0, nullptr);
        }
    }

    void VulkanBindlessDescriptorHeap::Destroy(VulkanDevice* device)
    {
        if (descriptorSetLayout != VK_NULL_HANDLE)
        {
            device->vkDestroyDescriptorSetLayout(device->GetHandle(), descriptorSetLayout, nullptr);
            descriptorSetLayout = VK_NULL_HANDLE;
        }

        if (descriptorPool != VK_NULL_HANDLE)
        {
            device->vkDestroyDescriptorPool(device->GetHandle(), descriptorPool, nullptr);
            descriptorPool = VK_NULL_HANDLE;
        }
    }

    /* VulkanBindlessManager */
    VulkanBindlessManager::VulkanBindlessManager(VulkanDevice* device_)
        : device(device_)
    {
        // TODO: Handle MutableDescriptorType
        mutableDescriptorType = false;

        std::vector<VkDescriptorSetLayout> setLayouts;
        if (mutableDescriptorType)
        {
        }
        else
        {
            // First set (space0)
            Vector<VkDescriptorSetLayoutBinding> layoutBindings;
            Vector<VkDescriptorBindingFlags> layoutBindingsFlags;

            const VkDescriptorBindingFlags bindingFlags =
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
                | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                ;

            for (size_t i = 0; i < layoutBindings.size(); ++i)
            {
                layoutBindingsFlags.push_back(bindingFlags);
            }

            // Create entries for static samples
            for (size_t samplerIndex = 0; samplerIndex < device->vkStaticSamplers.size(); samplerIndex++)
            {
                VkSampler sampler = device->vkStaticSamplers[samplerIndex];

                uint32_t binding = kImmutableSamplerSlotBegin + static_cast<uint32_t>(samplerIndex);

                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = binding + VulkanRegisterShift::kSampler;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                layoutBinding.descriptorCount = 1;
                layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
                layoutBinding.pImmutableSamplers = &sampler;

                layoutBindings.push_back(layoutBinding);
                //layout->layoutBindingsOriginal.push_back(binding);
            }

            const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = (uint32_t)layoutBindingsFlags.size(),
                .pBindingFlags = layoutBindingsFlags.data()
            };

            const VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &bindingFlagsInfo,
                .bindingCount = (uint32_t)layoutBindings.size(),
                .pBindings = layoutBindings.data()
            };

            VK_CHECK(device->vkCreateDescriptorSetLayout(device->GetHandle(), &layoutInfo, nullptr, &bindingsSetLayout));

            // Bindless now
            VkPhysicalDeviceVulkan12Properties properties12 = device_->_adapter->properties12;
            bindlessSampledImages.Init(device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min(kBindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages / 2));

            // Setup set layouts
            setLayouts.push_back(bindingsSetLayout);
            setLayouts.push_back(bindlessSampledImages.descriptorSetLayout);

            // Setup descriptor sets
            const VkDescriptorSetAllocateInfo allocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = device_->descriptorSetPools.back(),
                .descriptorSetCount = 1,
                .pSetLayouts = &bindingsSetLayout,
            };
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VK_CHECK(device->vkAllocateDescriptorSets(device->GetHandle(), &allocateInfo, &descriptorSet));

            descriptorSets[DESCRIPTOR_SET_BINDINGS] = descriptorSet;
            descriptorSets[DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE] = bindlessSampledImages.descriptorSet;
        }

        // Pipeline layout
        const VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = kMaxPushConstantsSize
        };

        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = (uint32_t)setLayouts.size(),
            .pSetLayouts = setLayouts.data(),
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges = &pushConstantRange
        };

        const VkResult result = device->vkCreatePipelineLayout(device->GetHandle(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
        if (result != VK_SUCCESS)
        {
            LOGE("Vulkan: Failed to create PipelineLayout, error: {}", VkResultToString(result));
            return;
        }

        device->SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(pipelineLayout), "Bindless Pipeline Layout");
    }

    VulkanBindlessManager::~VulkanBindlessManager()
    {
        if (!mutableDescriptorType)
        {
            bindlessSampledImages.Destroy(device);
        }

        if (bindingsSetLayout != VK_NULL_HANDLE)
        {
            device->vkDestroyDescriptorSetLayout(device->GetHandle(), bindingsSetLayout, nullptr);
            bindingsSetLayout = VK_NULL_HANDLE;
        }

        device->vkDestroyPipelineLayout(device->GetHandle(), pipelineLayout, nullptr);
    }

    void VulkanBindlessManager::BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
    {
        device->vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            ALIMER_STATIC_ARRAY_SIZE(descriptorSets),
            descriptorSets,
            0, nullptr
        );
    }

    /* VulkanDevice */
    VulkanDevice::VulkanDevice(VulkanRHIAdapter* adapter, const RHIDeviceDesc& desc)
        : _adapter(adapter)
    {
        std::vector<const char*> enabledDeviceExtensions;
        enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        const PhysicalDeviceExtensions& extensions = adapter->extensions;

        // Core in 1.3
        if (adapter->properties2.properties.apiVersion < VK_API_VERSION_1_3)
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

            if (extensions.pipelineCreationCacheControl)
            {
                enabledDeviceExtensions.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
            }

            if (extensions.textureCompressionAstcHdr)
            {
                enabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);
            }
        }
        else
        {
            // Core in 1.4
            if (adapter->properties2.properties.apiVersion < VK_API_VERSION_1_4)
            {
                if (extensions.maintenance5)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
                }

                if (extensions.maintenance6)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
                }

                if (extensions.pushDescriptor)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
                }
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

        if (extensions.depthClipEnable)
        {
            enabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
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

        if (extensions.unifiedImageLayouts)
        {
            enabledDeviceExtensions.push_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
        }

        if (extensions.descriptorHeap)
        {
            // Promoted to 1.4 but needed by VK_EXT_descriptor_heap
            enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
            enabledDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
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

                if (extensions.video.encode_h264)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME);
                }

                if (extensions.video.encode_h265)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME);
                }
            }
#endif // RHI_VIDEO_ENCODE
        }

        if (!adapter->features2.features.textureCompressionBC &&
            !(adapter->features2.features.textureCompressionETC2 && adapter->features2.features.textureCompressionASTC_LDR))
        {
            LOGE("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
            return;
        }

        // Bindless (https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs)
        ALIMER_VERIFY(adapter->features12.descriptorIndexing == VK_TRUE);
        ALIMER_VERIFY(adapter->features12.runtimeDescriptorArray == VK_TRUE);
        ALIMER_VERIFY(adapter->features12.descriptorBindingPartiallyBound == VK_TRUE);
        ALIMER_VERIFY(adapter->features12.descriptorBindingVariableDescriptorCount == VK_TRUE);
        ALIMER_VERIFY(adapter->features12.shaderSampledImageArrayNonUniformIndexing == VK_TRUE);

        ALIMER_VERIFY(adapter->properties2.properties.limits.maxPushConstantsSize >= kMaxPushConstantsSize);

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (uint32_t familyIndex = 0; familyIndex < adapter->queueFamilyIndices.queueFamilyCount; familyIndex++)
        {
            if (adapter->queueFamilyIndices.queueOffsets[familyIndex] == 0)
                continue;

            VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            info.queueFamilyIndex = familyIndex;
            info.queueCount = adapter->queueFamilyIndices.queueOffsets[familyIndex];
            info.pQueuePriorities = adapter->queueFamilyIndices.queuePriorities[familyIndex].data();
            queueInfos.push_back(info);
        }

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &adapter->features2;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        createInfo.pQueueCreateInfos = queueInfos.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

        VkResult result = adapter->factory->vkCreateDevice(adapter->handle, &createInfo, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Cannot create device");
            return;
        }

        if (desc.label)
        {
            SetLabel(desc.label);
        }

#ifdef _DEBUG
        LOGI("Enabled {} Device Extensions:", createInfo.enabledExtensionCount);
        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
        {
            LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
        }
#endif

#define VULKAN_DEVICE_FUNCTION(func) func = (PFN_##func) adapter->factory->vkGetDeviceProcAddr(handle, #func);
#include "RHI_Vulkan_Funcs.h"

        // VK_KHR_dynamic_rendering
        if (adapter->features13.dynamicRendering == VK_FALSE &&
            adapter->dynamicRenderingFeatures.dynamicRendering == VK_TRUE)
        {
            vkCmdBeginRendering = (PFN_vkCmdBeginRendering)adapter->factory->vkGetDeviceProcAddr(handle, "vkCmdBeginRenderingKHR");
            vkCmdEndRendering = (PFN_vkCmdEndRendering)adapter->factory->vkGetDeviceProcAddr(handle, "vkCmdEndRenderingKHR");
        }

        // VK_KHR_synchronization2
        if (adapter->features13.synchronization2 == VK_FALSE &&
            adapter->synchronization2Features.synchronization2 == VK_TRUE)
        {
            vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)adapter->factory->vkGetDeviceProcAddr(handle, "vkCmdPipelineBarrier2KHR");
            vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)adapter->factory->vkGetDeviceProcAddr(handle, "vkCmdWriteTimestamp2KHR");
            vkQueueSubmit2 = (PFN_vkQueueSubmit2)adapter->factory->vkGetDeviceProcAddr(handle, "vkQueueSubmit2KHR");
        }

        // VK_KHR_push_descriptor
        if (adapter->features14.pushDescriptor == VK_FALSE &&
            extensions.pushDescriptor == true)
        {
            vkCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)adapter->factory->vkGetDeviceProcAddr(handle, "vkCmdPushDescriptorSetKHR");
        }

        // Init limits
        {
            const VkPhysicalDeviceLimits& vkLimits = adapter->properties2.properties.limits;

            _limits.maxTextureDimension1D = vkLimits.maxImageDimension1D;
            _limits.maxTextureDimension2D = vkLimits.maxImageDimension2D;
            _limits.maxTextureDimension3D = vkLimits.maxImageDimension3D;
            _limits.maxTextureDimensionCube = vkLimits.maxImageDimensionCube;
            _limits.maxTextureArrayLayers = vkLimits.maxImageArrayLayers;
            _limits.maxBindGroups = vkLimits.maxBoundDescriptorSets;
            _limits.maxConstantBufferBindingSize = vkLimits.maxUniformBufferRange;
            _limits.maxStorageBufferBindingSize = vkLimits.maxStorageBufferRange;
            _limits.minConstantBufferOffsetAlignment = (uint32_t)vkLimits.minUniformBufferOffsetAlignment;
            _limits.minStorageBufferOffsetAlignment = (uint32_t)vkLimits.minStorageBufferOffsetAlignment;
            //_limits.maxPushConstantsSize = properties2.properties.limits.maxPushConstantsSize;
            [[maybe_unused]] const uint32_t maxPushDescriptors = adapter->pushDescriptorProps.maxPushDescriptors;
            _limits.maxBufferSize = adapter->properties13.maxBufferSize;
            _limits.maxColorAttachments = vkLimits.maxColorAttachments;
            _limits.maxViewports = vkLimits.maxViewports;
            //_limits.viewportBoundsMin = properties2.properties.limits.viewportBoundsRange[0];
            //_limits.viewportBoundsMax = properties2.properties.limits.viewportBoundsRange[1];

            /* Compute */
            _limits.maxComputeWorkgroupStorageSize = vkLimits.maxComputeSharedMemorySize;
            _limits.maxComputeInvocationsPerWorkgroup = vkLimits.maxComputeWorkGroupInvocations;

            _limits.maxComputeWorkgroupSizeX = vkLimits.maxComputeWorkGroupSize[0];
            _limits.maxComputeWorkgroupSizeY = vkLimits.maxComputeWorkGroupSize[1];
            _limits.maxComputeWorkgroupSizeZ = vkLimits.maxComputeWorkGroupSize[2];

            _limits.maxComputeWorkgroupsPerDimension = std::min({ vkLimits.maxComputeWorkGroupCount[0],vkLimits.maxComputeWorkGroupCount[1],vkLimits.maxComputeWorkGroupCount[2], });

            _limits.conservativeRasterizationTier = ConservativeRasterizationTier::NotSupported;
            if (extensions.conservativeRasterization)
            {
                _limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier1;

                if (adapter->conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && adapter->conservativeRasterizationProps.degenerateTrianglesRasterized)
                    _limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier2;
                if (adapter->conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && adapter->conservativeRasterizationProps.degenerateTrianglesRasterized)
                    _limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier3;
            }

            _limits.variableShadingRateTier = VariableRateShadingTier::NotSupported;
            if (extensions.fragmentShadingRate)
            {
                if (adapter->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
                {
                    _limits.variableShadingRateTier = VariableRateShadingTier::Tier1;
                }

                if (adapter->fragmentShadingRateFeatures.primitiveFragmentShadingRate &&
                    adapter->fragmentShadingRateFeatures.attachmentFragmentShadingRate)
                {
                    _limits.variableShadingRateTier = VariableRateShadingTier::Tier2;
                }

                const auto& tileExtent = adapter->fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize;
                _limits.variableShadingRateImageTileSize = std::max(tileExtent.width, tileExtent.height);
                _limits.isAdditionalVariableShadingRatesSupported = adapter->fragmentShadingRateProperties.maxFragmentSize.height > 2 || adapter->fragmentShadingRateProperties.maxFragmentSize.width > 2;
            }

            // Ray tracing
            _limits.rayTracingTier = RayTracingTier::NotSupported;
            if (adapter->features12.bufferDeviceAddress == VK_TRUE &&
                adapter->accelerationStructureFeatures.accelerationStructure == VK_TRUE &&
                adapter->rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE)
            {
                _limits.rayTracingTier = RayTracingTier::Tier1;

                if (adapter->rayQueryFeatures.rayQuery == VK_TRUE)
                {
                    _limits.rayTracingTier = RayTracingTier::Tier2;
                }

                //if (OpacityMicromapFeatures.micromap)
                //    m_Desc.tiers.rayTracing++;

                _limits.rayTracingShaderGroupIdentifierSize = adapter->rayTracingPipelineProperties.shaderGroupHandleSize;
                _limits.rayTracingShaderTableAlignment = adapter->rayTracingPipelineProperties.shaderGroupBaseAlignment;
                _limits.rayTracingShaderTableMaxStride = adapter->rayTracingPipelineProperties.maxShaderGroupStride;
                _limits.rayTracingShaderRecursionMaxDepth = adapter->rayTracingPipelineProperties.maxRayRecursionDepth;
                _limits.rayTracingMaxGeometryCount = (uint32_t)adapter->accelerationStructureProperties.maxGeometryCount;
                _limits.rayTracingScratchAlignment = adapter->accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
            }

            // Mesh shader
            _limits.meshShaderTier = MeshShaderTier::NotSupported;
            if (adapter->meshShaderFeatures.meshShader == VK_TRUE &&
                adapter->meshShaderFeatures.taskShader == VK_TRUE)
            {
                _limits.meshShaderTier = MeshShaderTier::Tier1;
            }

            const bool atomicsI64 = (adapter->features12.shaderBufferInt64Atomics || adapter->features12.shaderSharedInt64Atomics) ? true : false;
            //const bool atomicsF64 = (adapter->ShaderAtomicFloatFeatures.shaderBufferFloat64Atomics || ShaderAtomicFloatFeatures.shaderSharedFloat64Atomics) ? true : false;

            // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage
            _limits.highestShaderModel = ShaderModel::Model_6_0;
            if (adapter->features11.multiview)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_1;
            }

            if (adapter->features12.shaderFloat16 || adapter->features2.features.shaderInt16)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_2;
            }

            if (_limits.rayTracingTier != RayTracingTier::NotSupported)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_3;
            }

            if (_limits.variableShadingRateTier >= VariableRateShadingTier::Tier2)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_4;
            }

            if (_limits.meshShaderTier != MeshShaderTier::NotSupported || _limits.rayTracingTier >= RayTracingTier::Tier2)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_5;
            }

            // Check mutableDescriptorType for D3D12 ultimate bindless model
            if (atomicsI64)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_6;
            }

            if (adapter->features2.features.shaderStorageImageMultisample)
            {
                _limits.highestShaderModel = ShaderModel::Model_6_7;
            }

            // TODO: add SM 6.8 and 6.9 detection

            _timestampFrequency = uint64_t(1.0 / double(_adapter->properties2.properties.limits.timestampPeriod) * 1000 * 1000 * 1000);
        }

        // Create comamnd queues
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        for (uint8_t i = 0; i < ecast(QueueType::Count); i++)
        {
            if (_adapter->queueFamilyIndices.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
            {
                VkDeviceQueueInfo2 queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
                queueInfo.queueFamilyIndex = _adapter->queueFamilyIndices.familyIndices[i];
                queueInfo.queueIndex = _adapter->queueFamilyIndices.queueIndices[i];
                vkGetDeviceQueue2(handle, &queueInfo, &queues[i].queue);

                _adapter->queueFamilyIndices.counts[i] = _adapter->queueFamilyIndices.queueOffsets[_adapter->queueFamilyIndices.familyIndices[i]];

                for (uint32_t frameIndex = 0; frameIndex < kNumFramesInFlight; ++frameIndex)
                {
                    VK_CHECK(vkCreateFence(handle, &fenceInfo, nullptr, &queues[i].frameFences[frameIndex]));
                }
            }
            else
            {
                queues[i].queue = VK_NULL_HANDLE;
            }
        }

        // Create memory allocator
        {
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = _adapter->handle;
            allocatorInfo.device = handle;
            allocatorInfo.instance = _adapter->factory->handle;
            allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

            // Core in 1.1
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;

            if (_adapter->extensions.memoryBudget)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
            }

            if (_adapter->extensions.AMD_device_coherent_memory)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
            }

            if (_adapter->features12.bufferDeviceAddress == VK_TRUE)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            if (_adapter->extensions.EXT_memory_priority)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
            }

            // Core in 1.3
            if (_adapter->properties2.properties.apiVersion >= VK_API_VERSION_1_3
                || _adapter->extensions.maintenance4)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
            }

            if (_adapter->features14.maintenance5 == VK_TRUE
                || _adapter->maintenance5Features.maintenance5 == VK_TRUE)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
            }

            if (_adapter->extensions.externalMemory)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
            }

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
            static VmaVulkanFunctions vulkanFunctions = {};
            vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
            vulkanFunctions.vkGetDeviceProcAddr = _adapter->factory->vkGetDeviceProcAddr;
            allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

            result = vmaCreateAllocator(&allocatorInfo, &allocator);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create allocator");
            }

            if (_adapter->extensions.externalMemory)
            {
                std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
#if defined(_WIN32)
                externalMemoryHandleTypes.resize(_adapter->memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
                externalMemoryHandleTypes.resize(_adapter->memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif

                allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
                result = vmaCreateAllocator(&allocatorInfo, &externalAllocator);
                if (result != VK_SUCCESS)
                {
                    VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
                }
            }

            copyAllocator.Init(this);

            // Pipeline Cache
            {
                // TODO: Add cache from disk
                const VkPipelineCacheCreateInfo pipelineCacheCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                };

                result = vkCreatePipelineCache(handle, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
                if (result != VK_SUCCESS)
                {
                    VK_LOG_ERROR(result, "Failed to create pipeline cache");
                }
            }

            // Dynamic PSO states
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
            if (_adapter->features2.features.depthBounds == VK_TRUE)
            {
                psoDynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
            }
            if (_adapter->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            {
                psoDynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
            }
            //psoDynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);

            dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateInfo.dynamicStateCount = (uint32_t)psoDynamicStates.size();
            dynamicStateInfo.pDynamicStates = psoDynamicStates.data();
        }

        // Create default null descriptors.
        {
            VmaAllocationCreateInfo memoryInfo = {};
            memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = 4;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.flags = 0;
            VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo, &nullBuffer, &nullBufferAllocation, nullptr));

            VkBufferViewCreateInfo bufferViewInfo = {};
            bufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            bufferViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            bufferViewInfo.range = VK_WHOLE_SIZE;
            bufferViewInfo.buffer = nullBuffer;
            VK_CHECK(vkCreateBufferView(handle, &bufferViewInfo, nullptr, &nullBufferView));

            // Images
            memoryInfo = {};
            memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
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
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &nullImage1D, &nullImageAllocation1D, nullptr));

            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            imageInfo.arrayLayers = 6;
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &nullImage2D, &nullImageAllocation2D, nullptr));

            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.flags = 0;
            imageInfo.arrayLayers = 1;
            VK_CHECK(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &nullImage3D, &nullImageAllocation3D, nullptr));

            // Transitions:
            {
                VulkanUploadContext uploadContext = copyAllocator.Allocate(0);

                VkImageMemoryBarrier2 barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                barrier.srcAccessMask = 0;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                barrier.oldLayout = imageInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = nullImage1D;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;

                VkDependencyInfo dependencyInfo = {};
                dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                dependencyInfo.imageMemoryBarrierCount = 1;
                dependencyInfo.pImageMemoryBarriers = &barrier;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

                barrier.image = nullImage2D;
                barrier.subresourceRange.layerCount = 6;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

                barrier.image = nullImage3D;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
                copyAllocator.Submit(uploadContext);
            }

            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = nullImage1D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageView1D));

            imageViewInfo.image = nullImage1D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageView1DArray));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageView2D));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageView2DArray));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            imageViewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageViewCube));

            imageViewInfo.image = nullImage2D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            imageViewInfo.subresourceRange.layerCount = 6;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageViewCubeArray));

            imageViewInfo.image = nullImage3D;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            imageViewInfo.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(handle, &imageViewInfo, nullptr, &nullImageView3D));
        }

        // Null and immutable samplers
        {
            SamplerDesc samplerDesc{};
            nullSampler = GetOrCreateVulkanSampler(&samplerDesc);

            // Init common
            InitResources();
            for (size_t samplerIndex = 0; samplerIndex < staticSamplers.size(); samplerIndex++)
            {
                SharedPtr<VulkanSampler> sampler = StaticCast<VulkanSampler>(staticSamplers[samplerIndex]);
                vkStaticSamplers.push_back(sampler->handle);
            }
        }

        // Allocate at least one descriptor pool.
        descriptorSetPools.emplace_back(CreateDescriptorSetPool());

        // Init bindless manager
        bindlessManager = new VulkanBindlessManager(this);

        LOGI("Vulkan: Initialized with success");
    }

    VulkanDevice::~VulkanDevice()
    {
        WaitIdle();
        shuttingDown = true;

        for (uint8_t i = 0; i < ecast(QueueType::Count); ++i)
        {
            if (queues[i].queue == VK_NULL_HANDLE)
                continue;

            for (uint32_t frameIndex = 0; frameIndex < kNumFramesInFlight; ++frameIndex)
            {
                vkDestroyFence(handle, queues[i].frameFences[frameIndex], nullptr);
            }
        }

        SafeDelete(bindlessManager);
        copyAllocator.Shutdown();

        vmaDestroyBuffer(allocator, nullBuffer, nullBufferAllocation);
        vkDestroyBufferView(handle, nullBufferView, nullptr);
        vmaDestroyImage(allocator, nullImage1D, nullImageAllocation1D);
        vmaDestroyImage(allocator, nullImage2D, nullImageAllocation2D);
        vmaDestroyImage(allocator, nullImage3D, nullImageAllocation3D);
        vkDestroyImageView(handle, nullImageView1D, nullptr);
        vkDestroyImageView(handle, nullImageView1DArray, nullptr);
        vkDestroyImageView(handle, nullImageView2D, nullptr);
        vkDestroyImageView(handle, nullImageView2DArray, nullptr);
        vkDestroyImageView(handle, nullImageViewCube, nullptr);
        vkDestroyImageView(handle, nullImageViewCubeArray, nullptr);
        vkDestroyImageView(handle, nullImageView3D, nullptr);

        commandBuffers.clear();
        cmdBuffersCount = 0;

        // Destory pending objects.
        ProcessDeletionQueue(true);
        _frameCount = 0;

        // Release caches
        {
            // Samplers
            for (auto& it : samplerCache)
            {
                vkDestroySampler(handle, it.second, nullptr);
            }
            samplerCache.clear();
            staticSamplers.clear();

            // Destroy Descriptor Pools
            for (VkDescriptorPool descriptorPool : descriptorSetPools)
            {
                vkDestroyDescriptorPool(handle, descriptorPool, nullptr);
            }
            descriptorSetPools.clear();
        }

        if (allocator != VK_NULL_HANDLE)
        {
#if defined(_DEBUG)
            VmaTotalStatistics stats;
            vmaCalculateStatistics(allocator, &stats);

            if (stats.total.statistics.allocationBytes > 0)
            {
                LOGW("Total device memory leaked:  {} bytes.", stats.total.statistics.allocationBytes);
            }
#endif

            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }

        if (externalAllocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(externalAllocator);
            externalAllocator = VK_NULL_HANDLE;
        }

        if (pipelineCache != VK_NULL_HANDLE)
        {
            // Destroy Vulkan pipeline cache
            vkDestroyPipelineCache(handle, pipelineCache, nullptr);
            pipelineCache = VK_NULL_HANDLE;
        }

        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyDevice(handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }

    void VulkanDevice::SetLabel(const char* label)
    {
        SetObjectName(VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(handle), label);
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
        // Submit current frame.
        {
            uint32_t cmd_last = cmdBuffersCount;
            cmdBuffersCount = 0;
            for (uint32_t cmd = 0; cmd < cmd_last; ++cmd)
            {
                VulkanCommandBuffer& commandBuffer = *commandBuffers[cmd].get();
                VkCommandBuffer vkCommandBuffer = commandBuffer.End();

                VulkanQueue& queue = queues[ecast(commandBuffer.queueType)];

                VkCommandBufferSubmitInfo& commandBufferSubmitInfo = queue.submitCommandBufferInfos.emplace_back();
                commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
                commandBufferSubmitInfo.commandBuffer = vkCommandBuffer;

                queue.swapchainUpdates = commandBuffer.presentSwapChains;
                for (auto& swapchain : commandBuffer.presentSwapChains)
                {
                    queue.submitSwapchains.push_back(swapchain->handle);
                    queue.submitSwapchainImageIndices.push_back(swapchain->imageIndex);

                    VkSemaphoreSubmitInfo& waitSemaphore = queue.submitWaitSemaphoreInfos.emplace_back();
                    waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    waitSemaphore.semaphore = swapchain->acquireSemaphores[swapchain->acquireSemaphoreIndex];
                    waitSemaphore.value = 0; // not a timeline semaphore
                    waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

                    VkSemaphoreSubmitInfo& signalSemaphore = queue.submitSignalSemaphoreInfos.emplace_back();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.semaphore = swapchain->releaseSemaphores[swapchain->imageIndex];
                    signalSemaphore.value = 0; // not a timeline semaphore

                    queue.submitSignalSemaphores.push_back(signalSemaphore.semaphore);

                    // Advance surface frame index
                    swapchain->acquireSemaphoreIndex = (swapchain->acquireSemaphoreIndex + 1) % swapchain->acquireSemaphores.size();
                }

                if (commandBuffer.hasPendingWaits.load() || !commandBuffer.waits.empty())
                {
                    for (auto& waitCommandBuffer : commandBuffer.waits)
                    {
                        // Wait for command list dependency.
                        VkSemaphoreSubmitInfo& waitSemaphore = queue.submitWaitSemaphoreInfos.emplace_back();
                        waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                        waitSemaphore.semaphore = waitCommandBuffer->semaphore;
                        waitSemaphore.value = 0; // Not a timeline semaphore
                        waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    }

                    if (commandBuffer.hasPendingWaits.load())
                    {
                        // Signal this command list's completion
                        VkSemaphoreSubmitInfo& signalSemaphore = queue.submitSignalSemaphoreInfos.emplace_back();
                        signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                        signalSemaphore.semaphore = commandBuffer.semaphore;
                        signalSemaphore.value = 0; // not a timeline semaphore

                        queue.submitSignalSemaphores.push_back(commandBuffer.semaphore);
                    }

                    queue.Submit(this, VK_NULL_HANDLE);
                }
            }

            // Final submits with fences.
            for (uint8_t i = 0; i < ecast(QueueType::Count); ++i)
            {
                queues[i].Submit(this, queues[i].frameFences[_frameIndex]);
            }
        }

        // Begin new frame
        _frameAllocators[_frameIndex].Reset();
        _frameCount++;
        _frameIndex = _frameCount % kNumFramesInFlight;

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= kNumFramesInFlight)
        {
            for (uint8_t i = 0; i < ecast(QueueType::Count); ++i)
            {
                if (queues[i].queue == VK_NULL_HANDLE)
                    continue;

                VK_CHECK(vkWaitForFences(handle, 1, &queues[i].frameFences[_frameIndex], true, 0xFFFFFFFFFFFFFFFF));
                VK_CHECK(vkResetFences(handle, 1, &queues[i].frameFences[_frameIndex]));
            }
        }

        ProcessDeletionQueue(false);
        return _frameCount;
    }

    RHIBufferRef VulkanDevice::CreateBufferCore(const BufferDesc& desc, const void* initialData)
    {
        SharedPtr<VulkanBuffer> buffer(new VulkanBuffer(this, desc));

        if (desc.existingHandle)
        {
            buffer->handle = static_cast<VkBuffer>(desc.existingHandle);

            if (desc.label)
            {
                buffer->SetLabel(desc.label);
            }

            return buffer;
        }

        VkBufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = desc.size;
        createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        bool needBufferDeviceAddress = false;
        if (CheckBitsAny(desc.usage, BufferUsage::Vertex))
        {
            createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            needBufferDeviceAddress = true;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::Index))
        {
            createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            needBufferDeviceAddress = true;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::Constant))
        {
            createInfo.size = VmaAlignUp(desc.size, _adapter->properties2.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::ShaderRead))
        {
            // Read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::ShaderReadWrite))
        {
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::Indirect))
        {
            createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            needBufferDeviceAddress = true;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::Predication) && QueryFeatureSupport(RHIFeature::Predication))
        {
            createInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::RayTracing)
            && _limits.rayTracingTier != RayTracingTier::NotSupported)
        {
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
            needBufferDeviceAddress = true;
        }

        // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
        if (_adapter->features12.bufferDeviceAddress == VK_TRUE && needBufferDeviceAddress)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        uint32_t sharingIndices[ecast(QueueType::Count)];
        FillBufferSharingIndices(createInfo, sharingIndices);

        VmaAllocationCreateInfo memoryInfo = {};
        memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
        if (desc.memoryType == MemoryType::Readback)
        {
            memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        else if (desc.memoryType == MemoryType::Upload)
        {
            createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VkBufferUsageFlags2CreateInfoKHR bufUsageFlags2 = {};
        if (_adapter->extensions.maintenance5)
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
        if (initialData != nullptr)
        {
            VulkanUploadContext context;
            void* pMappedData = nullptr;
            if (desc.memoryType == MemoryType::Upload)
            {
                pMappedData = buffer->pMappedData;
            }
            else
            {
                context = copyAllocator.Allocate(createInfo.size);
                pMappedData = context.uploadBufferData;
            }

            std::memcpy(pMappedData, initialData, desc.size);

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

                if (CheckBitsAny(desc.usage, BufferUsage::Vertex))
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::Index))
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::Constant))
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_UNIFORM_READ_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::ShaderRead))
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::ShaderReadWrite))
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_WRITE_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::Indirect))
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::Predication))
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
                }

                if (CheckBitsAny(desc.usage, BufferUsage::RayTracing))
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

    RHITextureRef VulkanDevice::CreateTextureCore(const TextureDescriptor& desc, const TextureData* initialData)
    {
        const bool isDepthStencil = IsDepthStencilFormat(desc.format);

        SharedPtr<VulkanTexture> texture(new VulkanTexture(this, desc));
        texture->vkFormat = ToVkFormat(desc.format);

        VkImageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.format = texture->vkFormat;
        createInfo.extent.width = desc.width;
        createInfo.extent.height = 1;
        createInfo.extent.depth = 1;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (desc.dimension)
        {
            case TextureDimension::Texture1D:
                createInfo.imageType = VK_IMAGE_TYPE_1D;
                createInfo.arrayLayers = desc.depthOrArrayLayers;
                break;

            case TextureDimension::Texture2D:
                createInfo.imageType = VK_IMAGE_TYPE_2D;
                createInfo.extent.height = desc.height;
                createInfo.arrayLayers = desc.depthOrArrayLayers;
                break;

            case TextureDimension::Texture3D:
                // When using 3D textures as render targets, we need to be able to create 2d array views.
                if (!isDepthStencil &&
                    CheckBitsAny(desc.usage, TextureUsage::RenderTarget))
                {
                    createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
                }

                createInfo.imageType = VK_IMAGE_TYPE_3D;
                createInfo.extent.height = desc.height;
                createInfo.extent.depth = desc.depthOrArrayLayers;
                createInfo.arrayLayers = 1u;
                break;

            case TextureDimension::TextureCube:
                createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                createInfo.imageType = VK_IMAGE_TYPE_2D;
                createInfo.extent.height = desc.height;
                createInfo.arrayLayers = desc.depthOrArrayLayers * 6;
                break;

            default:
                if (Alimer::Assert::ReportFailure(0, __FILE__, __LINE__, "Invalid texture dimension") == Alimer::Assert::FailBehavior::Halt)
                {
                    ALIMER_BREAKPOINT;
                }

                ALIMER_ASSERT_FAIL("Invalid texture dimension");
                break;
        }

        createInfo.mipLevels = desc.mipLevelCount;
        createInfo.samples = ToVk(desc.sampleCount);
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = 0u;

        if (CheckBitsAny(desc.usage, TextureUsage::Transient))
        {
            createInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }
        else
        {
            createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        TextureLayout initialLayout = TextureLayout::Undefined;

        if (CheckBitsAny(desc.usage, TextureUsage::ShaderRead))
        {
            createInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            initialLayout = TextureLayout::ShaderResource;
        }

        if (CheckBitsAny(desc.usage, TextureUsage::ShaderWrite))
        {
            createInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
            initialLayout = TextureLayout::UnorderedAccess;
        }

        if (CheckBitsAny(desc.usage, TextureUsage::RenderTarget))
        {
            if (isDepthStencil)
            {
                createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                initialLayout = TextureLayout::DepthWrite;
            }
            else
            {
                createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                initialLayout = TextureLayout::RenderTarget;
            }
        }

        if (CheckBitsAny(desc.usage, TextureUsage::ShadingRate))
        {
            createInfo.usage |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        }

        // If ShaderRead and RenderTarget add input attachment
        if (!IsDepthStencilFormat(desc.format) &&
            CheckBitsAll(desc.usage, TextureUsage::RenderTarget | TextureUsage::ShaderRead))
        {
            createInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }

        uint32_t sharingIndices[ecast(QueueType::Count)];
        FillImageSharingIndices(createInfo, sharingIndices);

        VkResult result = VK_SUCCESS;

        VmaAllocationInfo allocationInfo{};
        VmaAllocationCreateInfo memoryInfo = {};
        memoryInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        memoryInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VkExternalMemoryImageCreateInfo externalInfo = {};
        bool isShared = false;

#if TEXTURE_STAGING
        if (desc.memoryType == MemoryType::Upload ||
            desc.memoryType == MemoryType::Readback)
        {
            // Staging texture
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = createInfo.extent.width * createInfo.extent.height * createInfo.extent.depth
                * createInfo.arrayLayers * GetFormatBytesPerBlock(desc.format);

            memoryInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            if (desc.memoryType == MemoryType::Readback)
            {
                memoryInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            else
            {
                memoryInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }

            VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
                &texture->stagingResource,
                &texture->allocation,
                &allocationInfo);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create staging image buffer.");
                return nullptr;
            }

            createInfo.tiling = VK_IMAGE_TILING_LINEAR;
            VkImage tempImage;
            result = vkCreateImage(device, &createInfo, nullptr, &tempImage);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create staging image.");
                return nullptr;
            }

            VkImageSubresource subresource = {};
            subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            VkSubresourceLayout subresourcelayout{};
            vkGetImageSubresourceLayout(device, tempImage, &subresource, &subresourcelayout);

            //texture->pMappedData = allocationInfo.pMappedData;
            //texture->rowPitch = (uint32_t)subresourcelayout.rowPitch;

            vkDestroyImage(device, tempImage, nullptr);
            return texture;
        }
        else
#endif // TEXTURE_STAGING

            if (CheckBitsAny(desc.usage, TextureUsage::Shared))
            {
                // Ensure that the handle type is supported.
                VkImageFormatProperties2 props2 = {};
                props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
                VkExternalImageFormatProperties externalProps = {};
                externalProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;

                VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo = {};
                externalFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
#if defined(_WIN32)
                externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
                externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

                props2.pNext = &externalProps;
                if (!GetImageFormatProperties(createInfo, &externalFormatInfo, &props2))
                {
                    LOGE("Image format is not supported for external memory type {}.", (uint32_t)externalFormatInfo.handleType);
                    return nullptr;
                }

                externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#if defined(_WIN32)
                externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
                externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

                const bool supportsImport = (externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
                const bool supportsExport = (externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) != 0;

                createInfo.pNext = &externalInfo;

                // We have to use a dedicated allocator for external handles that has been created with VkExportMemoryAllocateInfo
                allocator = externalAllocator;

                // Dedicated memory
                memoryInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                isShared = true;
            }

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

        if (isShared)
        {
#if defined(_WIN32)
            VkMemoryGetWin32HandleInfoKHR getWin32HandleInfoKHR = {};
            getWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
            getWin32HandleInfoKHR.pNext = nullptr;
            getWin32HandleInfoKHR.memory = allocationInfo.deviceMemory;
            getWin32HandleInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
            VK_CHECK(vkGetMemoryWin32HandleKHR(handle, &getWin32HandleInfoKHR, &texture->sharedHandle));
#elif defined(__linux__)
            VkMemoryGetFdInfoKHR memoryGetFdInfoKHR = {};
            memoryGetFdInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
            memoryGetFdInfoKHR.pNext = nullptr;
            memoryGetFdInfoKHR.memory = allocationInfo.deviceMemory;
            memoryGetFdInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
            VK_CHECK(vkGetMemoryFdKHR(handle, &memoryGetFdInfoKHR, &texture->sharedHandle));
#endif
        }

        const bool depthOnlyFormat = IsDepthOnlyFormat(desc.format);

        texture->numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
        texture->imageLayouts.resize(texture->numSubResources);
        for (uint32_t i = 0; i < texture->numSubResources; i++)
        {
            texture->imageLayouts[i] = initialLayout;
        }

        // Issue data copy on request
        VkImageSubresourceRange range{};
        range.aspectMask = GetImageAspectFlags(createInfo.format, TextureAspect::All);
        range.baseMipLevel = 0;
        range.levelCount = createInfo.mipLevels;
        range.baseArrayLayer = 0;
        range.layerCount = createInfo.arrayLayers;

        if (initialData != nullptr)
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

            const PixelFormatInfo& formatInfo = GetPixelFormatInfo(desc.format);
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
                    const TextureData& subresourceData = initialData[initDataIndex++];
                    const uint32_t numBlocksX = Max(1u, levelWidth / blockSize);
                    const uint32_t numBlocksY = Max(1u, levelHeight / blockSize);
                    const uint32_t dstRowPitch = numBlocksX * formatInfo.bytesPerBlock;
                    const uint32_t dstSlicePitch = dstRowPitch * numBlocksY;

                    uint32_t srcRowPitch = subresourceData.rowPitch;
                    uint32_t srcSlicePitch = subresourceData.slicePitch;
                    //GetSurfaceInfo(desc.format, levelWidth, levelHeight, &srcRowPitch, &srcSlicePitch);

                    for (uint32_t z = 0; z < levelDepth; ++z)
                    {
                        uint8_t* dstSlice = (uint8_t*)uploadContext.uploadBufferData + copyOffset + dstSlicePitch * z;
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

                    levelWidth = Max(1u, levelWidth / 2);
                    levelHeight = Max(1u, levelHeight / 2);
                    levelDepth = Max(1u, levelDepth / 2);
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

                const VkImageLayoutMapping mappingAfter = ConvertImageLayout(initialLayout, depthOnlyFormat);

                std::swap(barrier.srcStageMask, barrier.dstStageMask);
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = mappingAfter.layout;
                barrier.srcAccessMask = mappingBefore.accessMask;
                barrier.dstAccessMask = mappingAfter.accessMask;

                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

                copyAllocator.Submit(uploadContext);
            }
        }
        else if (initialLayout != TextureLayout::Undefined)
        {
            const VkImageLayoutMapping mappingAfter = ConvertImageLayout(initialLayout, depthOnlyFormat);

            VulkanUploadContext uploadContext = copyAllocator.Allocate(allocationInfo.size);

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

    RHITextureRef VulkanDevice::CreateTextureFromNativeHandleCore(RHINativeHandle handle, const TextureDescriptor& desc)
    {
        if (handle.type != RHINativeHandleType::VK_Image)
            return nullptr;

        SharedPtr<VulkanTexture> texture(new VulkanTexture(this, desc));
        texture->vkFormat = ToVkFormat(desc.format);

        texture->handle = (VkImage)handle.integer;

        if (desc.label)
        {
            texture->SetLabel(desc.label);
        }

        return texture;
    }

    RHISamplerRef VulkanDevice::CreateSamplerCore(const SamplerDesc& desc)
    {
        SharedPtr<VulkanSampler> sampler(new VulkanSampler());
        sampler->device = this;
        sampler->desc = desc;
        sampler->handle = GetOrCreateVulkanSampler(&desc);
        if (desc.label)
        {
            sampler->SetLabel(desc.label);
        }

        return sampler;
    }

    VkSampler VulkanDevice::GetOrCreateVulkanSampler(const SamplerDesc* desc)
    {
        size_t hash = 0;

        HashCombine(hash, (uint32_t)desc->minFilter);
        HashCombine(hash, (uint32_t)desc->magFilter);
        HashCombine(hash, (uint32_t)desc->mipFilter);
        HashCombine(hash, (uint32_t)desc->addressModeU);
        HashCombine(hash, (uint32_t)desc->addressModeV);
        HashCombine(hash, (uint32_t)desc->addressModeW);
        HashCombine(hash, (uint32_t)desc->reductionType);
        HashCombine(hash, (uint32_t)desc->borderColor);
        HashCombine(hash, desc->lodMinClamp);
        HashCombine(hash, desc->lodMaxClamp);
        HashCombine(hash, (uint32_t)desc->compareFunction);
        HashCombine(hash, desc->maxAnisotropy);

        auto it = samplerCache.find(hash);
        if (it == samplerCache.end())
        {
            VkSamplerCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            createInfo.flags = 0;
            createInfo.pNext = nullptr;
            createInfo.magFilter = ToVk(desc->magFilter);
            createInfo.minFilter = ToVk(desc->minFilter);
            createInfo.mipmapMode = ToVk(desc->mipFilter);
            createInfo.addressModeU = ToVk(desc->addressModeU, _adapter->features12.samplerMirrorClampToEdge);
            createInfo.addressModeV = ToVk(desc->addressModeU, _adapter->features12.samplerMirrorClampToEdge);
            createInfo.addressModeW = ToVk(desc->addressModeU, _adapter->features12.samplerMirrorClampToEdge);
            createInfo.mipLodBias = 0.0f;

            uint16_t maxAnisotropy = desc->maxAnisotropy;
            if (_adapter->features2.features.samplerAnisotropy == VK_TRUE && maxAnisotropy > 1)
            {
                createInfo.anisotropyEnable = VK_TRUE;
                createInfo.maxAnisotropy = std::clamp(maxAnisotropy * 1.f, 1.f, _adapter->properties2.properties.limits.maxSamplerAnisotropy);
            }
            else
            {
                createInfo.anisotropyEnable = VK_FALSE;
                createInfo.maxAnisotropy = 1;
            }

            if (desc->reductionType == SamplerReductionType::Comparison)
            {
                createInfo.compareOp = ToVk(desc->compareFunction);
                createInfo.compareEnable = VK_TRUE;
            }
            else
            {
                createInfo.compareOp = VK_COMPARE_OP_NEVER;
                createInfo.compareEnable = VK_FALSE;
            }

            createInfo.minLod = desc->lodMinClamp;
            createInfo.maxLod = (desc->lodMaxClamp == FLT_MAX) ? VK_LOD_CLAMP_NONE : desc->lodMaxClamp;
            createInfo.borderColor = ToVk(desc->borderColor);
            createInfo.unnormalizedCoordinates = VK_FALSE;

            VkSamplerReductionModeCreateInfo samplerReductionModeInfo = {};
            if (_adapter->features12.samplerFilterMinmax == VK_TRUE &&
                (desc->reductionType == SamplerReductionType::Minimum || desc->reductionType == SamplerReductionType::Maximum))
            {
                samplerReductionModeInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
                samplerReductionModeInfo.reductionMode = ToVk(desc->reductionType);
                createInfo.pNext = &samplerReductionModeInfo;
            }

            VkSampler newSampler;
            VkResult result = vkCreateSampler(handle, &createInfo, nullptr, &newSampler);
            if (result != VK_SUCCESS)
            {
                LOGE("Vulkan: Failed to create Sampler, error: {}", VkResultToString(result));
                return VK_NULL_HANDLE;
            }

            samplerCache[hash] = newSampler;
            return newSampler;
        }

        return it->second;
    }

    VkDescriptorPool VulkanDevice::CreateDescriptorSetPool()
    {
        const std::vector<VkDescriptorPoolSize> poolSizes{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 512 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128 },
            { VK_DESCRIPTOR_TYPE_SAMPLER, 16 }, /* Static samplers are 10 */
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1024;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT (bindless)
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorPool(handle, &poolInfo, nullptr, &pool);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Error when creating descriptor pool: {}");
            return VK_NULL_HANDLE;
        }

        return pool;
    }

    RHIShaderModuleRef VulkanDevice::CreateShaderModuleCore(const ShaderModuleDesc& desc)
    {
        SharedPtr<VulkanShaderModule> module(new VulkanShaderModule());
        module->device = this;
        module->stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        SpvReflectShaderModule reflectModule;
        SpvReflectResult reflectResult = spvReflectCreateShaderModule(desc.byteCodeSize, desc.byteCode, &reflectModule);
        if (reflectResult != SPV_REFLECT_RESULT_SUCCESS)
        {
            return nullptr;
        }

        if (reflectModule.entry_point_count > 1)
        {
            LOGE("Too many entry points in SPIR-V module: Expected 1, but got {}", reflectModule.entry_point_count);
            return nullptr;
        }

        module->entryPoint = reflectModule.entry_point_name;
        module->stageInfo.stage = static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
        module->stageInfo.pName = module->entryPoint.c_str();

        // TODO: Shift sets here instead in shader compiler
        const VkShaderModuleCreateInfo moduleInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = desc.byteCodeSize,
            .pCode = (const uint32_t*)desc.byteCode
        };
        VkResult result = vkCreateShaderModule(handle, &moduleInfo, nullptr, &module->stageInfo.module);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create a shader module");
            return nullptr;
        }

        spvReflectDestroyShaderModule(&reflectModule);
        return module;
    }

    RHIComputePipelineRef VulkanDevice::CreateComputePipelineCore(const ComputePipelineDescriptor& desc)
    {
        SharedPtr<VulkanComputePipeline> pipeline(new VulkanComputePipeline());
        pipeline->device = this;

        VkPipelineShaderStageCreateInfo stage = StaticCast<VulkanShaderModule>(desc.shader)->stageInfo;
        ALIMER_ASSERT(stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

        const VkComputePipelineCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .flags = 0,
            .stage = stage,
            .layout = bindlessManager->pipelineLayout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        };

        VkResult result = vkCreateComputePipelines(handle, pipelineCache, 1, &createInfo, nullptr, &pipeline->handle);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Compute Pipeline");
            return nullptr;
        }

        if (desc.label)
        {
            pipeline->SetLabel(desc.label);
        }

        return pipeline;
    }

    RHIRenderPipelineRef VulkanDevice::CreateRenderPipelineCore(const RenderPipelineDescriptor& desc)
    {
        SharedPtr<VulkanRenderPipeline> pipeline(new VulkanRenderPipeline());
        pipeline->device = this;

        // ShaderStages
        std::vector<VkPipelineShaderStageCreateInfo> stages;

        // Mesh Pipeline (D3DX12_MESH_SHADER_PIPELINE_STATE_DESC)
        if (desc.amplificationShader != nullptr)
        {
            stages.push_back(StaticCast<VulkanShaderModule>(desc.amplificationShader)->stageInfo);
            stages.push_back(StaticCast<VulkanShaderModule>(desc.meshShader)->stageInfo);

            if (desc.fragmentShader)
                stages.push_back(StaticCast<VulkanShaderModule>(desc.fragmentShader)->stageInfo);
        }
        else
        {
            stages.push_back(StaticCast<VulkanShaderModule>(desc.vertexShader)->stageInfo);

            if (desc.fragmentShader)
                stages.push_back(StaticCast<VulkanShaderModule>(desc.fragmentShader)->stageInfo);
        }

        // VertexInputState (need always be specified when using VertexShader)
        VkPipelineVertexInputStateCreateInfo vertexInputState{};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        std::vector<VkVertexInputBindingDescription> vertexBindings;
        std::vector<VkVertexInputAttributeDescription> vertexAttributes;

        uint32_t attributeLocation = 0;
        if (desc.vertexBufferLayoutCount > 0)
        {
            for (uint32_t bufferIndex = 0; bufferIndex < desc.vertexBufferLayoutCount; ++bufferIndex)
            {
                const VertexBufferLayout& vertexBufferLayout = desc.vertexBufferLayouts[bufferIndex];

                VkVertexInputBindingDescription& vertexBinding = vertexBindings.emplace_back();
                vertexBinding.binding = bufferIndex;
                vertexBinding.stride = vertexBufferLayout.stride;
                vertexBinding.inputRate = ToVk(vertexBufferLayout.stepMode);

                uint32_t currentOffset = 0;
                const bool computeStride = vertexBinding.stride == 0;
                for (uint32_t attributeIndex = 0; attributeIndex < vertexBufferLayout.attributeCount; ++attributeIndex)
                {
                    const VertexAttribute& attribute = vertexBufferLayout.attributes[attributeIndex];

                    VkVertexInputAttributeDescription& vertexAttribute = vertexAttributes.emplace_back();
                    vertexAttribute.location = attributeLocation;
                    vertexAttribute.binding = bufferIndex;
                    vertexAttribute.format = ToVkVertexFormat(attribute.format);
                    vertexAttribute.offset = attribute.offset != 0 ? attribute.offset : currentOffset;

                    const VertexAttributeFormatInfo& formatInfo = GetVertexAttributeFormatInfo(attribute.format);
                    currentOffset += formatInfo.byteSize;
                    attributeLocation++;

                    // Compute stride from attributes if necessary
                    if (computeStride)
                    {
                        vertexBinding.stride += formatInfo.byteSize;
                    }
                }
            }

            vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
            vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
            vertexInputState.pVertexAttributeDescriptions = vertexAttributes.data();
        }

        // InputAssemblyState
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = ToVk(desc.primitiveTopology);
        switch (desc.primitiveTopology)
        {
            case PrimitiveTopology::LineStrip:
            case PrimitiveTopology::TriangleStrip:
                inputAssemblyState.primitiveRestartEnable = VK_TRUE;
                break;
            default:
                inputAssemblyState.primitiveRestartEnable = VK_FALSE;
                break;
        }

        // ViewportState
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // RasterizationState
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.depthClampEnable = (desc.rasterizerState.depthClipMode == DepthClipMode::Clamp) ? VK_TRUE : VK_FALSE;

        VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipStateInfo = {};
        depthClipStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;

        const void** tail = &rasterizationState.pNext;
        if (_adapter->depthClipEnableFeatures.depthClipEnable == VK_TRUE)
        {
            depthClipStateInfo.depthClipEnable = (desc.rasterizerState.depthClipMode == DepthClipMode::Clip) ? VK_TRUE : VK_FALSE;

            rasterizationState.depthClampEnable = VK_TRUE;
            rasterizationState.pNext = &depthClipStateInfo;

            VK_APPEND_EXT(depthClipStateInfo);
        }

        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = ToVk(desc.rasterizerState.fillMode, _adapter->features2.features.fillModeNonSolid);
        rasterizationState.cullMode = ToVk(desc.rasterizerState.cullMode);
        rasterizationState.frontFace = ToVk(desc.rasterizerState.frontFace);
        // Can be managed by command buffer
        rasterizationState.depthBiasEnable = desc.rasterizerState.depthBias != 0.0f || desc.rasterizerState.depthBiasSlopeScale != 0.0f;
        rasterizationState.depthBiasConstantFactor = desc.rasterizerState.depthBias;
        rasterizationState.depthBiasClamp = desc.rasterizerState.depthBiasClamp;
        rasterizationState.depthBiasSlopeFactor = desc.rasterizerState.depthBiasSlopeScale;
        rasterizationState.lineWidth = 1.0f;

        VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeState = {};
        if (_adapter->extensions.conservativeRasterization &&
            desc.rasterizerState.conservativeRasterEnable)
        {
            rasterizationConservativeState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
            rasterizationConservativeState.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
            //rasterizationConservativeState.extraPrimitiveOverestimationSize = conservativeRasterProperties.maxExtraPrimitiveOverestimationSize;

            VK_APPEND_EXT(rasterizationConservativeState);
        }

        // MultisampleState
        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = ToVk(desc.sampleCount);

        ALIMER_ASSERT(multisampleState.rasterizationSamples <= 32);
        if (multisampleState.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT)
        {
            multisampleState.alphaToCoverageEnable = desc.blendState.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
            multisampleState.alphaToOneEnable = VK_FALSE;
            multisampleState.sampleShadingEnable = VK_FALSE;
            multisampleState.minSampleShading = 1.0f;
        }
        const VkSampleMask sampleMask = UINT_MAX;
        multisampleState.pSampleMask = &sampleMask;

        // DepthStencilState
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = (desc.depthStencilState.depthCompare != CompareFunction::Always || desc.depthStencilState.depthWriteEnabled) ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = desc.depthStencilState.depthWriteEnabled ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = ToVk(desc.depthStencilState.depthCompare);
        if (_adapter->features2.features.depthBounds == VK_TRUE)
        {
            depthStencilState.depthBoundsTestEnable = desc.depthStencilState.depthBoundsTestEnable ? VK_TRUE : VK_FALSE;
        }
        else
        {
            depthStencilState.depthBoundsTestEnable = false;
        }
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        depthStencilState.stencilTestEnable = StencilTestEnabled(&desc.depthStencilState) ? VK_TRUE : VK_FALSE;
        depthStencilState.front.failOp = ToVk(desc.depthStencilState.frontFace.failOp);
        depthStencilState.front.passOp = ToVk(desc.depthStencilState.frontFace.passOp);
        depthStencilState.front.depthFailOp = ToVk(desc.depthStencilState.frontFace.depthFailOp);
        depthStencilState.front.compareOp = ToVk(desc.depthStencilState.frontFace.compareFunc);
        depthStencilState.front.compareMask = desc.depthStencilState.stencilReadMask;
        depthStencilState.front.writeMask = desc.depthStencilState.stencilWriteMask;
        depthStencilState.front.reference = 0;

        depthStencilState.back.failOp = ToVk(desc.depthStencilState.backFace.failOp);
        depthStencilState.back.passOp = ToVk(desc.depthStencilState.backFace.passOp);
        depthStencilState.back.depthFailOp = ToVk(desc.depthStencilState.backFace.depthFailOp);
        depthStencilState.back.compareOp = ToVk(desc.depthStencilState.backFace.compareFunc);
        depthStencilState.back.compareMask = desc.depthStencilState.stencilReadMask;
        depthStencilState.back.writeMask = desc.depthStencilState.stencilWriteMask;
        depthStencilState.back.reference = 0;

        // BlendState
        uint32_t colorAttachmentCount = 0;
        VkPipelineColorBlendAttachmentState blendAttachmentStates[kMaxColorAttachments] = {};

        for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            if (desc.colorAttachmentFormats[i] == PixelFormat::Undefined)
                continue;

            uint32_t attachmentIndex = 0;
            if (desc.blendState.independentBlendEnable)
                attachmentIndex = i;

            const RenderTargetBlendState& attachment = desc.blendState.renderTargets[attachmentIndex];

            blendAttachmentStates[colorAttachmentCount].blendEnable = BlendEnabled(&attachment) ? VK_TRUE : VK_FALSE;
            blendAttachmentStates[colorAttachmentCount].srcColorBlendFactor = ToVk(attachment.srcColorBlendFactor);
            blendAttachmentStates[colorAttachmentCount].dstColorBlendFactor = ToVk(attachment.destColorBlendFactor);
            blendAttachmentStates[colorAttachmentCount].colorBlendOp = ToVk(attachment.colorBlendOp);
            blendAttachmentStates[colorAttachmentCount].srcAlphaBlendFactor = ToVk(attachment.srcAlphaBlendFactor);
            blendAttachmentStates[colorAttachmentCount].dstAlphaBlendFactor = ToVk(attachment.destAlphaBlendFactor);
            blendAttachmentStates[colorAttachmentCount].alphaBlendOp = ToVk(attachment.alphaBlendOp);
            blendAttachmentStates[colorAttachmentCount].colorWriteMask = ToVk(attachment.colorWriteMask);
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

        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        VkFormat colorAttachmentFormats[kMaxColorAttachments];
        for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            if (desc.colorAttachmentFormats[i] == PixelFormat::Undefined)
                continue;

            colorAttachmentFormats[renderingInfo.colorAttachmentCount] = ToVkFormat(desc.colorAttachmentFormats[i]);
            renderingInfo.colorAttachmentCount++;
        }
        renderingInfo.pColorAttachmentFormats = colorAttachmentFormats;
        renderingInfo.depthAttachmentFormat = ToVkFormat(desc.depthStencilFormat);
        if (!IsDepthOnlyFormat(desc.depthStencilFormat))
        {
            renderingInfo.stencilAttachmentFormat = ToVkFormat(desc.depthStencilFormat);
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
        if (desc.depthStencilFormat == PixelFormat::Undefined)
        {
            createInfo.pDepthStencilState = nullptr;
        }
        else
        {
            createInfo.pDepthStencilState = &depthStencilState;
        }
        createInfo.pColorBlendState = &blendState;
        createInfo.pDynamicState = &dynamicStateInfo;
        createInfo.layout = bindlessManager->pipelineLayout;
        createInfo.renderPass = VK_NULL_HANDLE;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        VkResult result = vkCreateGraphicsPipelines(handle,
            pipelineCache,
            1, &createInfo,
            nullptr,
            &pipeline->handle);

        if (result != VK_SUCCESS)
        {
            LOGE("Vulkan: Failed to create RenderPipeline");
            return nullptr;
        }

        if (desc.label)
        {
            pipeline->SetLabel(desc.label);
        }

        return pipeline;
    }

    RHIQueryHeapRef VulkanDevice::CreateQueryHeapCore(const QueryHeapDesc& desc)
    {
        VkQueryPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = ToVkQueryType(desc.type);
        createInfo.queryCount = desc.count;

        if (desc.type == QueryType::PipelineStatistics)
        {
            VkQueryPipelineStatisticFlags pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
                ;

            createInfo.pipelineStatistics = pipelineStatistics;
        }

        VkQueryPool queryPool = VK_NULL_HANDLE;
        VkResult result = vkCreateQueryPool(handle, &createInfo, nullptr, &queryPool);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        SharedPtr<VulkanQueryHeap> resource(new VulkanQueryHeap());
        resource->device = this;
        resource->desc = desc;
        resource->handle = queryPool;
        resource->queryResultSize = GetQueryResultSize(desc.type);

        if (desc.label)
        {
            resource->SetLabel(desc.label);
        }

        return resource;
    }

    RHISwapChainRef VulkanDevice::CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc)
    {
        VulkanRHISurface* backendSurface = static_cast<VulkanRHISurface*>(surface);
        const QueueFamilyIndices& queueFamilyIndices = _adapter->queueFamilyIndices;

        VkBool32 presentSupport = false;
        uint32_t queuePresentSupport = 0;

        for (auto& index : queueFamilyIndices.familyIndices)
        {
            if (index == VK_QUEUE_FAMILY_IGNORED)
            {
                continue;
            }

            if (_adapter->factory->vkGetPhysicalDeviceSurfaceSupportKHR(_adapter->handle,
                index,
                backendSurface->handle,
                &presentSupport) == VK_SUCCESS
                && presentSupport)
            {
                queuePresentSupport |= 1u << index;
            }
        }

        // Present family not found, we cannot create SwapChain
        if ((queuePresentSupport & (1u << queueFamilyIndices.familyIndices[ecast(QueueType::Graphics)])) == 0)
        {
            LOGE("Vulkan: No presentation queue found for GPU.");
            return nullptr;
        }

        SharedPtr<VulkanSwapChain> swapChain(new VulkanSwapChain());
        swapChain->device = this;
        swapChain->surface.Reset(backendSurface);
        swapChain->queuePresentSupport = queuePresentSupport;
        swapChain->colorFormat = desc.colorFormat;
        swapChain->presentMode = desc.presentMode;
        UpdateSwapChain(swapChain.Get());

        return swapChain;
    }

    void VulkanDevice::UpdateSwapChain(VulkanSwapChain* swapChain)
    {
        // In vulkan, the swapchain recreate can happen whenever it gets outdated, it's not in application's control so we have to be extra careful.
        std::scoped_lock lock(swapChain->locker);

        VkResult result = VK_SUCCESS;
        VkSurfaceCapabilitiesKHR caps;
        VK_CHECK(_adapter->factory->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_adapter->handle, swapChain->surface->handle, &caps));

        uint32_t formatCount;
        VK_CHECK(_adapter->factory->vkGetPhysicalDeviceSurfaceFormatsKHR(_adapter->handle, swapChain->surface->handle, &formatCount, nullptr));

        std::vector<VkSurfaceFormatKHR> swapchainFormats(formatCount);
        VK_CHECK(_adapter->factory->vkGetPhysicalDeviceSurfaceFormatsKHR(_adapter->handle, swapChain->surface->handle, &formatCount, swapchainFormats.data()));

        uint32_t presentModeCount;
        VK_CHECK(_adapter->factory->vkGetPhysicalDeviceSurfacePresentModesKHR(_adapter->handle, swapChain->surface->handle, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> swapchainPresentModes(presentModeCount);
        VK_CHECK(_adapter->factory->vkGetPhysicalDeviceSurfacePresentModesKHR(_adapter->handle, swapChain->surface->handle, &presentModeCount, swapchainPresentModes.data()));

        VkSurfaceFormatKHR surfaceFormat = {};
        surfaceFormat.format = ToVkFormat(swapChain->colorFormat);
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

        if (caps.currentExtent.width != 0xFFFFFFFF &&
            caps.currentExtent.height != 0xFFFFFFFF)
        {
            swapChain->extent = caps.currentExtent;
        }
        else
        {
            swapChain->extent.width = Max(caps.minImageExtent.width, Min(caps.maxImageExtent.width, swapChain->extent.width));
            swapChain->extent.height = Max(caps.minImageExtent.height, Min(caps.maxImageExtent.height, swapChain->extent.height));
        }

        VkPresentModeKHR vkPresentMode = ToVk(swapChain->presentMode);

        {

            auto HasPresentMode = [](const std::vector<VkPresentModeKHR>& modes,
                VkPresentModeKHR target) -> bool {
                    return std::find(modes.begin(), modes.end(), target) != modes.end();
                };
            const std::array<VkPresentModeKHR, 3> kPresentModeFallbacks = {
                VK_PRESENT_MODE_IMMEDIATE_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_FIFO_KHR,
            };

            // Go to the target mode.
            size_t modeIndex = 0;
            while (kPresentModeFallbacks[modeIndex] != vkPresentMode) {
                modeIndex++;
            }

            // Find the first available fallback.
            while (!HasPresentMode(swapchainPresentModes, kPresentModeFallbacks[modeIndex]))
            {
                modeIndex++;
            }

            ALIMER_ASSERT(modeIndex < kPresentModeFallbacks.size());
            vkPresentMode = kPresentModeFallbacks[modeIndex];
        }

        // Determine the number of images
        uint32_t imageCount = MinImageCountForPresentMode(vkPresentMode);
        imageCount = Max(imageCount, caps.minImageCount);
        if (caps.maxImageCount != 0)
        {
            imageCount = Min(imageCount, caps.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = swapChain->surface->handle;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = { swapChain->extent.width, swapChain->extent.height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkFormatProperties2 formatProps2 = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        _adapter->factory->vkGetPhysicalDeviceFormatProperties2(_adapter->handle, createInfo.imageFormat, &formatProps2);

        if ((formatProps2.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR)
            || (formatProps2.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = caps.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = vkPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = swapChain->handle;

        VK_CHECK(vkCreateSwapchainKHR(handle, &createInfo, nullptr, &swapChain->handle));

        if (createInfo.oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(handle, createInfo.oldSwapchain, nullptr);
        }

        VK_CHECK(vkGetSwapchainImagesKHR(handle, swapChain->handle, &imageCount, nullptr));
        std::vector<VkImage> swapchainImages(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(handle, swapChain->handle, &imageCount, swapchainImages.data()));

        swapChain->imageIndex = 0;
        swapChain->backbufferTextures.resize(imageCount);
        swapChain->colorFormat = FromVkFormat(createInfo.imageFormat);
        swapChain->extent = createInfo.imageExtent;

        TextureDescriptor textureDesc{};
        textureDesc.format = swapChain->colorFormat;
        textureDesc.width = createInfo.imageExtent.width;
        textureDesc.height = createInfo.imageExtent.height;
        textureDesc.usage = TextureUsage::RenderTarget;

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            SharedPtr<VulkanTexture> texture(new VulkanTexture(this, textureDesc));
            texture->vkFormat = createInfo.imageFormat;
            texture->handle = swapchainImages[i];
            texture->allocatedSize = 0;
            texture->numSubResources = 1;
            texture->imageLayouts.resize(1);
            texture->imageLayouts[0] = TextureLayout::Undefined;

            swapChain->backbufferTextures[i] = texture;
        }

        destroyMutex.lock();
        for (auto& x : swapChain->acquireSemaphores)
        {
            destroyedSemaphores.push_back(std::make_pair(x, _frameCount));
        }
        for (auto& x : swapChain->releaseSemaphores)
        {
            destroyedSemaphores.push_back(std::make_pair(x, _frameCount));
        }
        swapChain->acquireSemaphores.clear();
        swapChain->releaseSemaphores.clear();
        destroyMutex.unlock();

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (size_t i = 0; i < imageCount; ++i)
        {
            VK_CHECK(
                vkCreateSemaphore(handle, &semaphoreInfo, nullptr, &swapChain->acquireSemaphores.emplace_back())
            );
        }

        for (size_t i = 0; i < imageCount; ++i)
        {
            VK_CHECK(
                vkCreateSemaphore(handle, &semaphoreInfo, nullptr, &swapChain->releaseSemaphores.emplace_back())
            );
        }
    }

    void VulkanDevice::WriteShadingRateValue(ShadingRate rate, void* dest) const
    {
        // How to compute shading rate value texel data:
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#primsrast-fragment-shading-rate-attachment

        switch (rate)
        {
            default:
            case ShadingRate::Rate1x1:
                *(uint8_t*)dest = 0;
                break;
            case ShadingRate::Rate1x2:
                *(uint8_t*)dest = 0x1;
                break;
            case ShadingRate::Rate2x1:
                *(uint8_t*)dest = 0x4;
                break;
            case ShadingRate::Rate2x2:
                *(uint8_t*)dest = 0x5;
                break;
            case ShadingRate::Rate2x4:
                *(uint8_t*)dest = 0x6;
                break;
            case ShadingRate::Rate4x2:
                *(uint8_t*)dest = 0x9;
                break;
            case ShadingRate::Rate4x4:
                *(uint8_t*)dest = 0xa;
                break;
        }
    }

    RHICommandBuffer* VulkanDevice::BeginCommandBuffer(QueueType queue, std::string_view label)
    {
        cmdBuffersLocker.lock();
        uint32_t index = cmdBuffersCount++;
        if (index >= commandBuffers.size())
        {
            commandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(this, queue, index));
        }
        cmdBuffersLocker.unlock();

        commandBuffers[index]->Begin(_frameIndex, label);

        return commandBuffers[index].get();
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

    void VulkanDevice::FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices)
    {
        for (auto& i : _adapter->queueFamilyIndices.familyIndices)
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

    void VulkanDevice::FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices)
    {
        for (auto& i : _adapter->queueFamilyIndices.familyIndices)
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

    void VulkanDevice::SetObjectName(VkObjectType objectType, uint64_t objectHandle, const char* label)
    {
        if (!_adapter->debugUtils)
            return;

        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = objectHandle;
        nameInfo.pObjectName = label;
        _adapter->factory->vkSetDebugUtilsObjectNameEXT(handle, &nameInfo);
    }

    bool VulkanDevice::GetImageFormatProperties(const VkImageCreateInfo& createInfo, const void* pNext, VkImageFormatProperties2* properties2) const
    {
        return GetImageFormatProperties(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags, pNext, properties2);
    }

    bool VulkanDevice::GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, const void* pNext, VkImageFormatProperties2* properties2) const
    {
        ALIMER_ASSERT(properties2->sType == VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2);

        VkPhysicalDeviceImageFormatInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        info.pNext = pNext;
        info.format = format;
        info.type = type;
        info.tiling = tiling;
        info.usage = usage;
        info.flags = flags;

        const VkResult result = _adapter->factory->vkGetPhysicalDeviceImageFormatProperties2(
            _adapter->handle,
            &info,
            properties2
        );
        return result == VK_SUCCESS;
    }

    void VulkanDevice::ProcessDeletionQueue(bool force)
    {
        const auto Destroy = [&](auto&& queue, auto&& handler) {
            while (!queue.empty()) {
                if (force || (queue.front().second + kNumFramesInFlight < _frameCount))
                {
                    auto& item = queue.front();
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
        Destroy(destroyedShaderModules, [&](auto& item) { vkDestroyShaderModule(handle, item, nullptr); });
        Destroy(destroyedDescriptorSetLayouts, [&](auto& item) { vkDestroyDescriptorSetLayout(handle, item, nullptr); });
        Destroy(destroyedPipelineLayouts, [&](auto& item) { vkDestroyPipelineLayout(handle, item, nullptr); });
        Destroy(destroyedPipelines, [&](auto& item) { vkDestroyPipeline(handle, item, nullptr); });
        Destroy(destroyedQueryPools, [&](auto& item) { vkDestroyQueryPool(handle, item, nullptr); });
        Destroy(destroyedSemaphores, [&](auto& item) {vkDestroySemaphore(handle, item, nullptr); });
        Destroy(destroyedSwapchains, [&](auto& item) { vkDestroySwapchainKHR(handle, item, nullptr); });
        //Destroy(destroyedSurfaces, [&](auto& item) { _adapter->factory->vkDestroySurfaceKHR(_adapter->factory->handle, item, nullptr); });
        Destroy(destroyedDescriptorSets, [&](auto& item) { vkFreeDescriptorSets(handle, item.first, 1u, &item.second); });
        destroyMutex.unlock();
    }

    bool VulkanDevice::QueryFeatureSupport(RHIFeature feature)
    {
        switch (feature)  // NOLINT(clang-diagnostic-switch-enum)
        {
            case RHIFeature::TimestampQuery:
                return _adapter->properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

            case RHIFeature::PipelineStatisticsQuery:
                return _adapter->features2.features.pipelineStatisticsQuery == VK_TRUE;

            case RHIFeature::TextureCompressionBC:
                return _adapter->features2.features.textureCompressionBC == VK_TRUE;

            case RHIFeature::TextureCompressionETC2:
                return _adapter->features2.features.textureCompressionETC2 == VK_TRUE;

            case RHIFeature::TextureCompressionASTC_HDR:
                return _adapter->features13.textureCompressionASTC_HDR == VK_TRUE || _adapter->astcHdrFeatures.textureCompressionASTC_HDR == VK_TRUE;

            case RHIFeature::TextureCompressionASTC:
                return _adapter->features2.features.textureCompressionASTC_LDR == VK_TRUE;

            case RHIFeature::IndirectFirstInstance:
                return _adapter->features2.features.drawIndirectFirstInstance == VK_TRUE;

            case RHIFeature::ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case RHIFeature::GPUUploadHeapSupported:
                // https://github.com/KhronosGroup/Vulkan-Docs/issues/2096
                // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                return true;

            case RHIFeature::CopyQueueTimestampQuery:
                return _adapter->properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

            case RHIFeature::TessellationShader:
                return _adapter->features2.features.tessellationShader == VK_TRUE;

            case RHIFeature::DepthBoundsTest:
                return _adapter->features2.features.depthBounds == VK_TRUE;

            case RHIFeature::SamplerMirrorOnce:
                return _adapter->features12.samplerMirrorClampToEdge == VK_TRUE;

            case RHIFeature::SamplerBorder:
                return true;

            case RHIFeature::SamplerMinMax:
                return _adapter->features12.samplerFilterMinmax == VK_TRUE;

            case RHIFeature::Bindless:
                // https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs
                return _adapter->features12.descriptorIndexing == VK_TRUE
                    && _adapter->features12.runtimeDescriptorArray == VK_TRUE
                    && _adapter->features12.descriptorBindingPartiallyBound == VK_TRUE
                    && _adapter->features12.descriptorBindingVariableDescriptorCount == VK_TRUE
                    && _adapter->features12.shaderSampledImageArrayNonUniformIndexing == VK_TRUE
                    ;

            case RHIFeature::DepthResolveMinMax:
                return
                    (_adapter->depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                    (_adapter->depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MAX_BIT);

            case RHIFeature::StencilResolveMinMax:
                return
                    (_adapter->depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                    (_adapter->depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MAX_BIT);

            case RHIFeature::ShaderOutputViewportIndex:
                return _adapter->features12.shaderOutputLayer == VK_TRUE && _adapter->features12.shaderOutputViewportIndex == VK_TRUE;

            case RHIFeature::CacheCoherentUMA:
                if (_adapter->memoryProperties2.memoryProperties.memoryHeapCount == 1 &&
                    _adapter->memoryProperties2.memoryProperties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    return true;
                }

                return false;

            case RHIFeature::Predication:
                return _adapter->conditionalRenderingFeatures.conditionalRendering == VK_TRUE;

            default:
                return false;
        }
    }

    PixelFormatSupport VulkanDevice::QueryPixelFormatSupport(PixelFormat format)
    {
        PixelFormatSupport result = PixelFormatSupport::None;
        const VkFormat vkFormat = ToVkFormat(format);
        if (vkFormat == VK_FORMAT_UNDEFINED)
            return result;

        VkFormatProperties2 props = {};
        props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
        _adapter->factory->vkGetPhysicalDeviceFormatProperties2(_adapter->handle, vkFormat, &props);

        constexpr uint32_t transferBits = VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;

        if (props.formatProperties.optimalTilingFeatures & transferBits)
            result |= PixelFormatSupport::Texture;

        if (props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            result |= PixelFormatSupport::DepthStencil;

        if (props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            result |= PixelFormatSupport::RenderTarget;

        if (props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
            result |= PixelFormatSupport::Blendable;

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ||
            (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
        {
            result |= PixelFormatSupport::ShaderLoad;
        }

        if (props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
            result |= PixelFormatSupport::ShaderSample;

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) ||
            (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
        {
            result |= PixelFormatSupport::ShaderUavLoad;
            result |= PixelFormatSupport::ShaderUavStore;
        }

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) ||
            (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
        {
            result |= PixelFormatSupport::ShaderAtomic;
        }

        // Ensure that the handle type is supported.
        VkImageFormatProperties2 props2 = {};
        props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_1D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture1D/Texture1DArray
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture2D/Texture2DArray
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_3D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture3D
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, nullptr, &props2))
        {
            // TextureCube/TextureCubeArray
        }

        props2 = {};
        props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        TextureSampleCount supportedSampleCount = TextureSampleCount::Count1;
        if (IsDepthStencilFormat(format))
        {
            if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, nullptr, &props2))
            {
                supportedSampleCount = static_cast<TextureSampleCount>(props2.imageFormatProperties.sampleCounts);
            }
        }
        else
        {
            if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, 0, nullptr, &props2))
            {
                supportedSampleCount = static_cast<TextureSampleCount>(props2.imageFormatProperties.sampleCounts);
            }
        }

        return result;
    }

    bool VulkanDevice::QueryVertexFormatSupport(VertexAttributeFormat format)
    {
        const VkFormat vkFormat = ToVkVertexFormat(format);
        if (vkFormat == VK_FORMAT_UNDEFINED)
            return false;

        VkFormatProperties2 props = {};
        props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
        _adapter->factory->vkGetPhysicalDeviceFormatProperties2(_adapter->handle, vkFormat, &props);

        if (!props.formatProperties.bufferFeatures)
        {
            return false;
        }

#if TODO
        bool ShaderLoad = false;
        bool storage = false;
        bool storageAtomic = false;
        bool vertex = false;
        bool accelerationStructure = false;

        if (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)
            ShaderLoad = true;

        if (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)
            storage = true;

        if (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)
            storageAtomic = true;

        if (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
            vertex = true;

        if (props.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)
            accelerationStructure = true;

#endif // TODO

        return true;
    }

    RHINativeHandle VulkanDevice::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            case RHINativeHandleType::VK_Instance:
                return RHINativeHandle(_adapter->factory->handle);
            case RHINativeHandleType::VK_PhysicalDevice:
                return RHINativeHandle(_adapter->handle);
            case RHINativeHandleType::VK_Device:
                return RHINativeHandle(handle);
            default:
                return nullptr;
        }
    }

    VkFormat VulkanDevice::ToVkFormat(PixelFormat format)
    {
        return _adapter->ToVkFormat(format);
    }

    bool VulkanDevice::IsDepthStencilFormatSupported(VkFormat format) const
    {
        return _adapter->IsDepthStencilFormatSupported(format);
    }

    RHIAdapter* VulkanDevice::GetAdapter() const
    {
        return _adapter;
    }

    void VulkanQueue::Submit(VulkanDevice* device, VkFence fence)
    {
        if (queue == VK_NULL_HANDLE)
            return;

        ALIMER_ASSERT(submitSignalSemaphores.size() == submitSignalSemaphoreInfos.size());

        std::scoped_lock lock(locker);
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

            const VkResult result = device->vkQueuePresentKHR(queue, &presentInfo);
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

        swapchainUpdates.clear();
        submitSwapchains.clear();
        submitSwapchainImageIndices.clear();
        submitSignalSemaphores.clear();
        // KHR_synchronization2
        submitWaitSemaphoreInfos.clear();
        submitSignalSemaphoreInfos.clear();
        submitCommandBufferInfos.clear();
    }

    void VulkanDevice::CopyAllocator::Init(VulkanDevice* device_)
    {
        device = device_;
    }

    void VulkanDevice::CopyAllocator::Shutdown()
    {
        device->vkQueueWaitIdle(device->queues[ecast(QueueType::Copy)].queue);
        for (auto& context : freeList)
        {
            device->vkDestroyCommandPool(device->handle, context.transferCommandPool, nullptr);
            device->vkDestroyCommandPool(device->handle, context.transitionCommandPool, nullptr);
            device->vkDestroySemaphore(device->handle, context.semaphores[0], nullptr);
            device->vkDestroySemaphore(device->handle, context.semaphores[1], nullptr);
            device->vkDestroySemaphore(device->handle, context.semaphores[2], nullptr);
            device->vkDestroyFence(device->handle, context.fence, nullptr);

            context.uploadBuffer.Reset();
            context.uploadBufferData = nullptr;
        }
    }

    VulkanUploadContext VulkanDevice::CopyAllocator::Allocate(uint64_t size)
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
            poolCreateInfo.queueFamilyIndex = device->_adapter->queueFamilyIndices.familyIndices[ecast(QueueType::Copy)];
            VK_CHECK(device->vkCreateCommandPool(device->handle, &poolCreateInfo, nullptr, &context.transferCommandPool));

            poolCreateInfo.queueFamilyIndex = device->_adapter->queueFamilyIndices.familyIndices[ecast(QueueType::Graphics)];
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
            context.uploadBufferSize = Max(context.uploadBufferSize, uint64_t(65536));

            BufferDesc uploadBufferDesc;
            uploadBufferDesc.label = "CopyAllocator::UploadBuffer";
            uploadBufferDesc.size = context.uploadBufferSize;
            uploadBufferDesc.memoryType = MemoryType::Upload;

            context.uploadBuffer = StaticCast<VulkanBuffer>(device->CreateBuffer(uploadBufferDesc, nullptr));
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

    void VulkanDevice::CopyAllocator::Submit(VulkanUploadContext context)
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

            std::scoped_lock lock(device->queues[ecast(QueueType::Copy)].locker);
            VK_CHECK(device->vkQueueSubmit2(device->queues[ecast(QueueType::Copy)].queue, 1, &submitInfo, VK_NULL_HANDLE));
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

            std::scoped_lock lock(device->queues[ecast(QueueType::Graphics)].locker);
            VK_CHECK(device->vkQueueSubmit2(device->queues[ecast(QueueType::Graphics)].queue, 1, &submitInfo, VK_NULL_HANDLE));
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
            std::scoped_lock lock(device->queues[ecast(QueueType::Compute)].locker);
            VK_CHECK(device->vkQueueSubmit2(device->queues[ecast(QueueType::Compute)].queue, 1, &submitInfo, context.fence));
        }

        std::scoped_lock lock(locker);
        freeList.push_back(context);
    }

    /* VulkanRHIAdapter */
    VulkanRHIAdapter::VulkanRHIAdapter(VulkanRHIFactory* factory_, VkPhysicalDevice handle_)
        : factory(factory_)
        , handle(handle_)
        , debugUtils(factory_->debugUtils)
    {

    }

    bool VulkanRHIAdapter::Init()
    {
        extensions = factory->QueryPhysicalDeviceExtensions(handle);
        queueFamilyIndices = factory->QueryQueueFamilies(handle, extensions.video.queue);

        // Get current base properties
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        factory->vkGetPhysicalDeviceProperties2(handle, &properties2);

        VkBaseOutStructure* featureChainCurrent{ nullptr };
        auto addToFeatureChain = [&featureChainCurrent](auto* next) {
            auto n = reinterpret_cast<VkBaseOutStructure*>(next);
            featureChainCurrent->pNext = n;
            featureChainCurrent = n;
            };

        VkBaseOutStructure* propertiesChainCurrent{ nullptr };
        auto addToPropertiesChain = [&propertiesChainCurrent](auto* next) {
            auto n = reinterpret_cast<VkBaseOutStructure*>(next);
            propertiesChainCurrent->pNext = n;
            propertiesChainCurrent = n;
            };

        // Features
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;

        featureChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&features2);
        propertiesChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&properties2);

        addToFeatureChain(&features11);
        addToFeatureChain(&features12);
        addToPropertiesChain(&properties11);
        addToPropertiesChain(&properties12);

        if (properties2.properties.apiVersion >= VK_API_VERSION_1_3)
        {
            features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;

            addToFeatureChain(&features13);
            addToPropertiesChain(&properties13);
        }

        if (properties2.properties.apiVersion >= VK_API_VERSION_1_4)
        {
            features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
            properties14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;

            addToFeatureChain(&features14);
            addToPropertiesChain(&properties14);
        }

        // Properties
        samplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
        depthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;

        addToPropertiesChain(&samplerFilterMinmaxProperties);
        addToPropertiesChain(&depthStencilResolveProperties);

        // Core in 1.3
        if (properties2.properties.apiVersion < VK_API_VERSION_1_3)
        {
            if (extensions.maintenance4)
            {
                maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
                maintenance4Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;

                addToFeatureChain(&maintenance4Features);
                addToPropertiesChain(&maintenance4Properties);
            }

            if (extensions.dynamicRendering)
            {
                dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                addToFeatureChain(&dynamicRenderingFeatures);
            }

            if (extensions.synchronization2)
            {
                synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                addToFeatureChain(&synchronization2Features);
            }

            if (extensions.extendedDynamicState)
            {
                extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
                addToFeatureChain(&extendedDynamicStateFeatures);
            }

            if (extensions.extendedDynamicState2)
            {
                extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
                addToFeatureChain(&extendedDynamicState2Features);
            }

            if (extensions.textureCompressionAstcHdr)
            {
                astcHdrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
                addToFeatureChain(&astcHdrFeatures);
            }
        }
        else
        {
            // Core in 1.4
            if (properties2.properties.apiVersion < VK_API_VERSION_1_4)
            {
                if (extensions.maintenance5)
                {
                    maintenance5Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
                    addToFeatureChain(&maintenance5Features);
                }

                if (extensions.maintenance6)
                {
                    maintenance6Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES;
                    maintenance6Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES;

                    addToFeatureChain(&maintenance6Features);
                    addToPropertiesChain(&maintenance6Properties);
                }

                if (extensions.pushDescriptor)
                {
                    pushDescriptorProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES;
                    addToPropertiesChain(&pushDescriptorProps);
                }
            }
        }

        if (extensions.conservativeRasterization)
        {
            conservativeRasterizationProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
            addToPropertiesChain(&conservativeRasterizationProps);
        }

        if (extensions.depthClipEnable)
        {
            depthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
            addToFeatureChain(&depthClipEnableFeatures);
        }

        if (extensions.accelerationStructure)
        {
            ALIMER_ASSERT(extensions.deferredHostOperations);

            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

            addToFeatureChain(&accelerationStructureFeatures);
            addToPropertiesChain(&accelerationStructureProperties);

            if (extensions.raytracingPipeline)
            {
                rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

                addToFeatureChain(&rayTracingPipelineFeatures);
                addToPropertiesChain(&rayTracingPipelineProperties);
            }

            if (extensions.rayQuery)
            {
                rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                addToFeatureChain(&rayQueryFeatures);
            }
        }

        if (extensions.fragmentShadingRate)
        {
            fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
            fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

            addToFeatureChain(&fragmentShadingRateFeatures);
            addToPropertiesChain(&fragmentShadingRateProperties);
        }

        if (extensions.meshShader)
        {
            meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
            meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

            addToFeatureChain(&meshShaderFeatures);
            addToPropertiesChain(&meshShaderProperties);
        }

        if (extensions.conditionalRendering)
        {
            conditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
            addToFeatureChain(&conditionalRenderingFeatures);
        }

        if (extensions.unifiedImageLayouts)
        {
            unifiedImageLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR;
            addToFeatureChain(&unifiedImageLayoutsFeatures);
        }

        if (extensions.descriptorHeap)
        {
            descriptorHeapFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
            addToFeatureChain(&descriptorHeapFeaturesEXT);

            descriptorHeapPropertiesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT;
            addToPropertiesChain(&descriptorHeapPropertiesEXT);
        }

        factory->vkGetPhysicalDeviceFeatures2(handle, &features2);
        factory->vkGetPhysicalDeviceProperties2(handle, &properties2);

        synchronization2 = features13.synchronization2 == VK_TRUE || synchronization2Features.synchronization2 == VK_TRUE;
        dynamicRendering = features13.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE;

        // Memory properties
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        factory->vkGetPhysicalDeviceMemoryProperties2(handle, &memoryProperties2);

        _properties.vendorID = properties2.properties.vendorID;
        _properties.deviceID = properties2.properties.deviceID;
        _properties.deviceName = properties2.properties.deviceName;
        _properties.driverDescription = properties12.driverName;
        if (properties12.driverInfo[0] != '\0')
        {
            _properties.driverDescription += std::string(": ") + properties12.driverInfo;
        }

        switch (properties2.properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                _type = RHIAdapterType::IntegratedGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                _type = RHIAdapterType::DiscreteGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                _type = RHIAdapterType::VirtualGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                _type = RHIAdapterType::Cpu;
                break;
            default:
                _type = RHIAdapterType::Other;
                break;
        }

        static_assert(kUUIDSize == sizeof(properties11.deviceUUID));
        memcpy(_properties.uuid, properties11.deviceUUID, kUUIDSize);

        if (properties11.deviceLUIDValid)
        {
            static_assert(kLUIDSize == sizeof(properties11.deviceLUID));
            memcpy(_properties.luid, properties11.deviceLUID, kLUIDSize);
        }

        // Go through the memory types to figure out the amount of VRAM on this physical device.
        for (uint32_t heapIndex = 0; heapIndex < memoryProperties2.memoryProperties.memoryHeapCount; ++heapIndex)
        {
            VkMemoryHeap const& heap = memoryProperties2.memoryProperties.memoryHeaps[heapIndex];
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                _properties.videoMemorySize += heap.size;
            }
            else
            {
                _properties.systemMemorySize += heap.size;
            }
        }

        // The environment can request to various options for depth-stencil formats that could be
        // unavailable. Override the decision if it is not applicable.
        supportsDepth32Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
        supportsDepth24Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
        supportsStencil8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

        return true;
    }

    RHIDeviceRef VulkanRHIAdapter::CreateDevice(const RHIDeviceDesc& desc)
    {
        SharedPtr<VulkanDevice> device(new VulkanDevice(this, desc));
        if (!device->GetHandle())
        {
            return nullptr;
        }

        return device;
    }

    VkFormat VulkanRHIAdapter::ToVkFormat(PixelFormat format)
    {
        if (format == PixelFormat::Stencil8 && !supportsStencil8)
        {
            return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        if (format == PixelFormat::Depth24UnormStencil8 && !supportsDepth24Stencil8)
        {
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        // https://github.com/doitsujin/dxvk/blob/master/src/dxgi/dxgi_format.cpp
        VkFormat vkFormat = static_cast<VkFormat>(Alimer::ToVkFormat(format));
        return vkFormat;
    }

    bool VulkanRHIAdapter::IsDepthStencilFormatSupported(VkFormat format) const
    {
        ALIMER_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

        VkFormatProperties2 properties2 = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        factory->vkGetPhysicalDeviceFormatProperties2(handle, format, &properties2);
        return properties2.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    /* VulkanRHIFactory */
    bool VulkanRHIFactory::IsSupported()
    {
        static bool available_initialized = false;
        static bool available = false;

        if (available_initialized) {
            return available;
        }

        available_initialized = true;
#if defined(_WIN32)
        HMODULE module = LoadLibrary(L"vulkan-1.dll");
        if (!module)
            return false;

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
        void* module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
        if (!module)
            module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
        if (!module)
            module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
        // Add support for using Vulkan and MoltenVK in a Framework. App store rules for iOS
        // strictly enforce no .dylib's. If they aren't found it just falls through
        if (!module)
            module = dlopen("vulkan.framework/vulkan", RTLD_NOW | RTLD_LOCAL);
        if (!module)
            module = dlopen("MoltenVK.framework/MoltenVK", RTLD_NOW | RTLD_LOCAL);
        // modern versions of macOS don't search /usr/local/lib automatically contrary to what man dlopen says
        // Vulkan SDK uses this as the system-wide installation location, so we're going to fallback to this if all else fails
        if (!module && getenv("DYLD_FALLBACK_LIBRARY_PATH") == NULL)
            module = dlopen("/usr/local/lib/libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
        if (!module)
            return false;

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#else
        void* module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!module) {
            module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
        }
        if (!module) {
            return false;
        }
        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#endif

#define VULKAN_GLOBAL_FUNCTION(name) \
    name = (PFN_##name)vkGetInstanceProcAddr(VK_NULL_HANDLE, #name); \
    if (name == nullptr) { \
        return false; \
    }
#include "RHI_Vulkan_Funcs.h"

        // We require vulkan 1.2
        uint32_t apiVersion;
        if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS)
            return false;

        // Check if the Vulkan API version is sufficient.
        static constexpr uint32_t kMinimumVulkanVersion = VK_API_VERSION_1_2;
        if (apiVersion < kMinimumVulkanVersion)
        {
            LOGW("The Vulkan API version supported on the system ({}.{}.{}) is too low, at least {}.{}.{} is required.",
                VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion),
                VK_API_VERSION_MAJOR(kMinimumVulkanVersion), VK_API_VERSION_MINOR(kMinimumVulkanVersion), VK_API_VERSION_PATCH(kMinimumVulkanVersion)
            );
            return false;
        }

        // Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it.
        if (VK_API_VERSION_VARIANT(apiVersion) != 0)
        {
            LOGW("The Vulkan API supported on the system uses an unexpected variant: {}.", VK_API_VERSION_VARIANT(apiVersion));
            return false;
        }

        available = true;
        return true;
    }

    VulkanRHIFactory::VulkanRHIFactory(const RHIFactoryDesc& desc)
    {
        // Enumerate available layers and extensions
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
                debugUtils = true;
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
                headless = true;
                instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
            }
#if defined(VK_USE_PLATFORM_XLIB_KHR)
            else if (strcmp(availableExtension.extensionName, VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0)
            {
                xlib_surface = true;
            }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
            else if (strcmp(availableExtension.extensionName, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME) == 0)
            {
                wayland_surface = true;
            }
#endif
        }

        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        // Enable surface extensions depending on os
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
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

#if defined(VK_USE_PLATFORM_XLIB_KHR)
        if (xlib_surface)
        {
            instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        }
#endif /* defined(VK_USE_PLATFORM_XLIB_KHR) */

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        if (wayland_surface)
        {
            instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        }
#endif
#endif

        if (desc.validationMode != RHIValidationMode::Disabled)
        {
            // Determine the optimal validation layers to enable that are necessary for useful debugging
            std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

        bool validationFeatures = false;
        if (desc.validationMode == RHIValidationMode::GPU)
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

        uint32_t instanceApiVersion;
        VK_CHECK(vkEnumerateInstanceVersion(&instanceApiVersion));

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = desc.label.data();
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = ENGINE_NAME;
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        // Target Vulkan 1.4 if available.
        appInfo.apiVersion = std::max(VK_API_VERSION_1_3, std::min(VK_API_VERSION_1_4, instanceApiVersion));

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

        if (desc.validationMode != RHIValidationMode::Disabled && debugUtils)
        {
            debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType =
                //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            if (desc.validationMode == RHIValidationMode::Verbose)
            {
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            }

            debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
            createInfo.pNext = &debugUtilsCreateInfo;
        }

        VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
        if (desc.validationMode == RHIValidationMode::GPU && validationFeatures)
        {
            static const VkValidationFeatureEnableEXT enable_features[2] = {
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            };
            validationFeaturesInfo.enabledValidationFeatureCount = 2;
            validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
            PnextChainPushFront(&createInfo, &validationFeaturesInfo);
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
            return;
        }

#define VULKAN_INSTANCE_FUNCTION(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(handle, #fn);
#include "RHI_Vulkan_Funcs.h"

        if (desc.validationMode != RHIValidationMode::Disabled && debugUtils)
        {
            result = vkCreateDebugUtilsMessengerEXT(handle, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Could not create debug utils messenger");
            }
        }

#ifdef _DEBUG
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
#endif

        // Enumerate physical device and detect best one.
        uint32_t physicalDeviceCount = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr));
        if (physicalDeviceCount == 0)
        {
            LOGE("Vulkan: Failed to find GPUs with Vulkan support");
            return;
        }

        _adapters.reserve(physicalDeviceCount);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, physicalDevices.data()));

        for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        {
            // We require minimum 1.2
            VkPhysicalDeviceProperties2 physicalDeviceProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
            if (physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_2)
            {
                continue;
            }

            VkPhysicalDeviceFeatures2 physicalDeviceFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

            // Check required features
            if (physicalDeviceFeatures.features.robustBufferAccess != VK_TRUE
                || physicalDeviceFeatures.features.fullDrawIndexUint32 != VK_TRUE
                || physicalDeviceFeatures.features.depthClamp != VK_TRUE
                || physicalDeviceFeatures.features.depthBiasClamp != VK_TRUE
                || physicalDeviceFeatures.features.fragmentStoresAndAtomics != VK_TRUE
                || physicalDeviceFeatures.features.imageCubeArray != VK_TRUE
                || physicalDeviceFeatures.features.independentBlend != VK_TRUE
                || physicalDeviceFeatures.features.sampleRateShading != VK_TRUE
                || physicalDeviceFeatures.features.shaderClipDistance != VK_TRUE
                || physicalDeviceFeatures.features.occlusionQueryPrecise != VK_TRUE)
            {
                continue;
            }

            PhysicalDeviceExtensions physicalDeviceExt = QueryPhysicalDeviceExtensions(physicalDevice);
            if (!physicalDeviceExt.swapchain)
            {
                continue;
            }

            QueueFamilyIndices queueFamilyIndices = QueryQueueFamilies(physicalDevice, physicalDeviceExt.video.queue);
            if (!queueFamilyIndices.IsComplete())
            {
                continue;
            }

            VulkanRHIAdapter* adapter = new VulkanRHIAdapter(this, physicalDevice);
            if (!adapter->Init())
            {
                delete adapter;
                continue;
            }

            _adapters.push_back(adapter);
        }
    }

    VulkanRHIFactory::~VulkanRHIFactory()
    {
        // Delete adapters
        for (size_t i = 0, count = _adapters.size(); i < count; ++i)
        {
            VulkanRHIAdapter* adapter = static_cast<VulkanRHIAdapter*>(_adapters[i]);
            SafeDelete(adapter);
        }
        _adapters.clear();

        for (auto& surface : destroyedSurfaces)
        {
            vkDestroySurfaceKHR(handle, surface, nullptr);
        }
        destroyedSurfaces.clear();

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

    RHISurfaceRef VulkanRHIFactory::CreateSurface(void* window, void* display)
    {
        SharedPtr<VulkanRHISurface> surface(new VulkanRHISurface());
        surface->factory = this;

        VkResult result = VK_SUCCESS;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        HWND hwnd = (HWND)window;
        ALIMER_ASSERT(IsWindow(hwnd));

        const VkWin32SurfaceCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = hwnd,
        };
        result = vkCreateWin32SurfaceKHR(handle, &createInfo, nullptr, &surface->handle);
#elif defined(__ANDROID__)
        const VkAndroidSurfaceCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = (ANativeWindow*)window
        };
        result = vkCreateAndroidSurfaceKHR(handle, &createInfo, nullptr, &surface->handle);
#elif defined(__APPLE__)
        const VkMetalSurfaceCreateInfoEXT createInfo = {
            .sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .pLayer = (CAMetalLayer*)window
        };

        result = vkCreateMetalSurfaceEXT(handle, &createInfo, nullptr, &surface->handle);
#endif

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Vulkan surface.");
            return nullptr;
        }

        return surface;
    }

    PhysicalDeviceExtensions VulkanRHIFactory::QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t count = 0;
        VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
        if (result != VK_SUCCESS)
            return {};

        std::vector<VkExtensionProperties> vk_extensions(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data());

        PhysicalDeviceExtensions extensions{};

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
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME) == 0)
            {
                extensions.copyCommands2 = true;
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
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
            {
                extensions.deferredHostOperations = true;
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
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) == 0)
            {
                extensions.unifiedImageLayouts = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME) == 0)
            {
                extensions.mutableDescriptorType = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME) == 0)
            {
                extensions.descriptorHeap = true;
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

#if defined(VK_USE_PLATFORM_WIN32_KHR)
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

        VkPhysicalDeviceProperties2 properties2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

        // Core 1.4
        if (properties2.properties.apiVersion >= VK_API_VERSION_1_4)
        {
            extensions.maintenance5 = true;
            extensions.maintenance6 = true;
            extensions.pushDescriptor = true;
        }

        // Core 1.3
        if (properties2.properties.apiVersion >= VK_API_VERSION_1_3)
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

    QueueFamilyIndices VulkanRHIFactory::QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue)
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

        QueueFamilyIndices indices;
        indices.queueFamilyCount = queueFamilyCount;
        indices.queueOffsets.resize(queueFamilyCount);
        indices.queuePriorities.resize(queueFamilyCount);

        const auto FindVacantQueue = [&](QueueType type, VkQueueFlags required, VkQueueFlags ignore_flags,
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
                        indices.familyIndices[ecast(type)] = familyIndex;
                        queueFamilies[familyIndex].queueFamilyProperties.queueCount--;
                        indices.queueIndices[ecast(type)] = indices.queueOffsets[familyIndex]++;
                        indices.queuePriorities[familyIndex].push_back(priority);
                        return true;
                    }
                }

                return false;
            };

        if (!FindVacantQueue(QueueType::Graphics, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
        {
            LOGE("Vulkan: Could not find suitable graphics queue.");
            return indices;
        }

        // XXX: This assumes timestamp valid bits is the same for all queue types.
        indices.timestampValidBits = queueFamilies[indices.familyIndices[ecast(QueueType::Graphics)]].queueFamilyProperties.timestampValidBits;

        // Prefer standalone compute queue. If not, fall back to another graphics queue.
        if (!FindVacantQueue(QueueType::Compute, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f)
            && !FindVacantQueue(QueueType::Compute, VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
        {
            // Fallback to the graphics queue if we must.
            indices.familyIndices[ecast(QueueType::Compute)] = indices.familyIndices[ecast(QueueType::Graphics)];
            indices.queueIndices[ecast(QueueType::Compute)] = indices.queueIndices[ecast(QueueType::Graphics)];
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(QueueType::Copy, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f)
            && !FindVacantQueue(QueueType::Copy, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
        {
            indices.familyIndices[ecast(QueueType::Copy)] = indices.familyIndices[ecast(QueueType::Compute)];
            indices.queueIndices[ecast(QueueType::Copy)] = indices.queueIndices[ecast(QueueType::Compute)];
        }

        if (supportsVideoQueue)
        {
            if (!FindVacantQueue(QueueType::VideoDecode, VK_QUEUE_VIDEO_DECODE_BIT_KHR, 0, 0.5f))
            {
                indices.familyIndices[ecast(QueueType::VideoDecode)] = VK_QUEUE_FAMILY_IGNORED;
                indices.queueIndices[ecast(QueueType::VideoDecode)] = UINT32_MAX;
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

    bool VulkanRHIFactory::GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
    {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex) == VK_TRUE;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        return true;
#elif defined(__APPLE__)
        return true;
#else
        // Linux
        //if (vkGetPhysicalDeviceXcbPresentationSupportKHR)
        //{
        //
        //}
        //else
        //{
        //    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, _this->internal->display);
        //}

        return true;
#endif
    }

    bool Vulkan_IsSupported()
    {
        return VulkanRHIFactory::IsSupported();
    }

    RHIFactoryRef Vulkan_CreateFactory(const RHIFactoryDesc& desc)
    {
        if (!VulkanRHIFactory::IsSupported())
        {
            LOGE("Vulkan is not supported on this system.");
            return nullptr;
        }

        SharedPtr<VulkanRHIFactory> factory(new VulkanRHIFactory(desc));
        return factory;
    }
}
#endif /* defined(ALIMER_RHI_VULKAN) */
