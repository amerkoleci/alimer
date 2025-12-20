// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"

#if defined(ALIMER_RHI_VULKAN)
#include "Core/Log.h"
#include "Core/Vector.h"
#include "Core/UnorderedMap.h"
#include "Core/Hash.h"
#include "RHI/RHI.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

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
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#elif defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#else

#endif

typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_visualid_t;

//#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan_xcb.h>

struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>

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
// TODO: remove once migrate to RHIFactory
#define VULKAN_INSTANCE_FUNCTION(name) static PFN_##name name = nullptr;
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

        constexpr VkFormat ToVkVertexFormat(VertexFormat format)
        {
            switch (format)
            {
                case VertexFormat::UByte:               return VK_FORMAT_R8_UINT;
                case VertexFormat::UByte2:              return VK_FORMAT_R8G8_UINT;
                case VertexFormat::UByte4:              return VK_FORMAT_R8G8B8A8_UINT;
                case VertexFormat::Byte:                return VK_FORMAT_R8_SINT;
                case VertexFormat::Byte2:               return VK_FORMAT_R8G8_SINT;
                case VertexFormat::Byte4:               return VK_FORMAT_R8G8B8A8_SINT;
                case VertexFormat::UByteNormalized:     return VK_FORMAT_R8_UNORM;
                case VertexFormat::UByte2Normalized:    return VK_FORMAT_R8G8_UNORM;
                case VertexFormat::UByte4Normalized:    return VK_FORMAT_R8G8B8A8_UNORM;
                case VertexFormat::ByteNormalized:      return VK_FORMAT_R8_SNORM;
                case VertexFormat::Byte2Normalized:     return VK_FORMAT_R8G8_SNORM;
                case VertexFormat::Byte4Normalized:     return VK_FORMAT_R8G8B8A8_SNORM;

                case VertexFormat::UShort:              return VK_FORMAT_R16_UINT;
                case VertexFormat::UShort2:             return VK_FORMAT_R16G16_UINT;
                case VertexFormat::UShort4:             return VK_FORMAT_R16G16B16A16_UINT;
                case VertexFormat::Short:               return VK_FORMAT_R16_SINT;
                case VertexFormat::Short2:              return VK_FORMAT_R16G16_SINT;
                case VertexFormat::Short4:              return VK_FORMAT_R16G16B16A16_SINT;
                case VertexFormat::UShortNormalized:    return VK_FORMAT_R16_UNORM;
                case VertexFormat::UShort2Normalized:   return VK_FORMAT_R16G16_UNORM;
                case VertexFormat::UShort4Normalized:   return VK_FORMAT_R16G16B16A16_UNORM;
                case VertexFormat::ShortNormalized:     return VK_FORMAT_R16_SNORM;
                case VertexFormat::Short2Normalized:    return VK_FORMAT_R16G16_SNORM;
                case VertexFormat::Short4Normalized:    return VK_FORMAT_R16G16B16A16_SNORM;
                case VertexFormat::Half:                return VK_FORMAT_R16_SFLOAT;
                case VertexFormat::Half2:               return VK_FORMAT_R16G16_SFLOAT;
                case VertexFormat::Half4:               return VK_FORMAT_R16G16B16A16_SFLOAT;

                case VertexFormat::Float:               return VK_FORMAT_R32_SFLOAT;
                case VertexFormat::Float2:              return VK_FORMAT_R32G32_SFLOAT;
                case VertexFormat::Float3:              return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexFormat::Float4:              return VK_FORMAT_R32G32B32A32_SFLOAT;

                case VertexFormat::UInt:                return VK_FORMAT_R32_UINT;
                case VertexFormat::UInt2:               return VK_FORMAT_R32G32_UINT;
                case VertexFormat::UInt3:               return VK_FORMAT_R32G32B32_UINT;
                case VertexFormat::UInt4:               return VK_FORMAT_R32G32B32A32_UINT;

                case VertexFormat::Int:                 return VK_FORMAT_R32_SINT;
                case VertexFormat::Int2:                return VK_FORMAT_R32G32_SINT;
                case VertexFormat::Int3:                return VK_FORMAT_R32G32B32_SINT;
                case VertexFormat::Int4:                return VK_FORMAT_R32G32B32A32_SINT;

                case VertexFormat::UInt1010102Normalized:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
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
                case PrimitiveTopology::PatchList:      return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
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
            {
            }
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
            {
            }
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

    class VulkanDevice;

    struct VulkanBuffer final : public RHIBuffer
    {
    private:
        VulkanDevice* device = nullptr;

    public:
        VkBuffer handle = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        mutable BufferStates currentState = BufferStates::Undefined;

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
        DeviceAddress GetDeviceAddress() const override { return deviceAddress; }
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
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

        explicit VulkanTexture(VulkanDevice* device_, const TextureDesc& desc)
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

    struct VulkanBindGroupLayout final : public RHIBindGroupLayout
    {
        VulkanDevice* device = nullptr;

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        std::vector<uint32_t> layoutBindingsOriginal;
        VkDescriptorSetLayout handle = VK_NULL_HANDLE;
        bool isBindless = false;

        ~VulkanBindGroupLayout() override;
        void SetLabel(const char* label) override;
    };

    struct VulkanPipelineLayout final : public RHIPipelineLayout
    {
        VulkanDevice* device = nullptr;
        VkPipelineLayout handle = VK_NULL_HANDLE;

        uint32_t bindGroupLayoutCount = 0;
        VkPushConstantRange pushConstantRange = {};

        ~VulkanPipelineLayout() override;
        void SetLabel(const char* label) override;
    };

    struct VulkanBindGroup final : public RHIBindGroup
    {
        VulkanDevice* device = nullptr;
        SharedPtr<VulkanBindGroupLayout> bindGroupLayout;

        VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

        ~VulkanBindGroup() override;
        RHIBindGroupLayout* GetBindGroupLayout() const override { return bindGroupLayout.Get(); }

        void SetLabel(const char* label) override;
        void Update(size_t entryCount, const BindGroupEntry* entries) override;
    };

    struct VulkanBindingUsage final
    {
        bool used = false;
        VkDescriptorSetLayoutBinding binding = {};
    };

    struct VulkanDescriptorSetLayout final
    {
        //uint32_t set = 0;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkImageViewType> imageViewTypes;
    };

    struct VulkanPipelineLayoutReflection final
    {
        UnorderedMap<uint32_t, VulkanDescriptorSetLayout> layoutBindings;
        VkPushConstantRange pushConstantRange = {};
        //std::vector<VkDeviceSize> uniformBufferSizes;
        UnorderedMap<uint32_t, VkDeviceSize> uniformBufferSizes;
        std::vector<uint32_t> uniformBufferDynamicSlots;

        std::vector<VulkanBindingUsage> bindlessBindings;
        std::vector<VkDescriptorSet> bindlessSets;
        uint32_t bindlessFirstSet = 0;
    };

    struct VulkanPipeline final : public RHIPipeline
    {
        VulkanDevice* device = nullptr;
        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        Type type = Type::Render;

        VulkanPipelineLayoutReflection reflection{};

        SharedPtr<VulkanPipelineLayout> layout;
        VkPipeline handle = VK_NULL_HANDLE;

        ~VulkanPipeline() override;

        RHIPipeline::Type GetType() const override { return type; }
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

    struct VulkanSwapChain final : public RHISwapChain
    {
        VulkanDevice* device = nullptr;
        std::mutex locker;
        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
        VkSwapchainKHR handle = VK_NULL_HANDLE;

        uint32_t imageIndex = 0;
        VkExtent2D extent{};
        PixelFormat colorFormat = PixelFormat::Undefined;
        PresentMode presentMode = PresentMode::Immediate;
        std::vector<SharedPtr<VulkanTexture>> backbufferTextures;
        size_t acquireSemaphoreIndex = 0;
        std::vector<VkSemaphore> acquireSemaphores;
        std::vector<VkSemaphore> releaseSemaphores;

        explicit VulkanSwapChain(RHISurface* surface);
        ~VulkanSwapChain() override;

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

    class VulkanCommandContext final : public GraphicsContext
    {
        friend class VulkanDevice;

    public:
        VulkanCommandContext(VulkanDevice* device, QueueType queueType, uint32_t id);
        ~VulkanCommandContext() override;

        void Begin(uint32_t frameIndex, const std::string& label);
        VkCommandBuffer End();

        void PushDebugGroup(std::string_view name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view name) override;

        void BufferBarrier(const VulkanBuffer* buffer, BufferStates newState);
        void TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, TextureAspect aspect = TextureAspect::All);
        void TextureBarrier(const VulkanTextureView* view, TextureLayout newLayout);
        void CommitBarriers();

        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) override;
        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) override;

        /* ComputeContext */
        void SetPipeline(RHIPipeline* pipeline) override;
        void SetPushConstants(const void* data, uint32_t size, uint32_t offset = 0) override;
        void SetBindGroup(uint32_t groupIndex, RHIBindGroup* bindGroup) override;
        void FlushBindGroups();

        void PrepareDispatch();
        void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void BeginQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void EndQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) override;
        void ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count) override;

        /* GraphicsContext */
        RHITexture* AcquireSwapChainTexture(RHISwapChain* swapChain) override;

        void BeginRenderPassCore(const RenderPassDesc& desc) override;
        void EndRenderPassCore() override;
        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) override;
        //void SetViewport(const Viewport& viewport) override;
        //void SetViewports(uint32_t count, const Viewport* viewports) override;
        void SetScissorRect(int32_t x, int32_t y, int32_t width, int32_t height) override;
        //void SetScissorRect(const Rect& rect) override;
        //void SetScissorRects(uint32_t count, const Rect* scissorRects) override;
        void SetStencilReference(uint32_t reference) override;
        void SetBlendColor(float red, float green, float blue, float alpha) override;
        //void SetBlendColor(const Color& color) override;
        void SetShadingRate(ShadingRate rate) override;
        void SetDepthBounds(float minBounds, float maxBounds) override;

        void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset) override;
        void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) override;
        void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexType indexType) override;

        void PrepareDraw();
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
        void DrawIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
        void DispatchMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DispatchMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount) override;

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

        std::vector<VulkanCommandContext*> waits;
        std::atomic_bool hasPendingWaits{ false };
        bool hasLabel = false;

        uint32_t numBarriersToCommit = 0;
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;

        bool bindGroupsDirty{ false };
        uint32_t numBoundBindGroups{ 0 };
        SharedPtr<VulkanBindGroup> boundBindGroups[kMaxBindGroups] = {};
        VkDescriptorSet descriptorSets[kMaxBindGroups] = {};

        SharedPtr<VulkanPipeline> currentPipeline;
        SharedPtr<VulkanPipelineLayout> currentPipelineLayout;
        std::vector<SharedPtr<VulkanSwapChain>> presentSwapChains;
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
        bool unifiedImageLayouts;
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

    inline PhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
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
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) == 0)
            {
                extensions.unifiedImageLayouts = true;
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

    class VulkanDevice final : public RHIDevice
    {
        friend class VulkanCommandContext;
        friend struct VulkanQueue;
        friend struct VulkanBindGroup;

    public:
#define VULKAN_DEVICE_FUNCTION(func) PFN_##func func;
#include "RHI_Vulkan_Funcs.h"

        VulkanDevice(const std::string& appName, const RHIDeviceDesc& desc);
        ~VulkanDevice() override;

        bool WaitIdle() override;
        uint64_t CommitFrame() override;

        RHIBufferRef CreateBufferCore(const BufferDesc& desc, const void* initialData) override;
        RHITextureRef CreateTextureCore(const TextureDesc& desc, const TextureData* initialData) override;
        RHITextureRef CreateTextureFromNativeHandleCore(RHINativeHandle handle, const TextureDesc& desc) override;
        RHISamplerRef CreateSamplerCore(const SamplerDesc& desc) override;

        VkDescriptorSetLayout GetOrCreateDescriptorSetLayout(const VulkanDescriptorSetLayout& setLayout);
        VkPipelineLayout GetOrCreatePipelineLayout(const VulkanPipelineLayoutReflection& reflection);
        VkSampler GetOrCreateVulkanSampler(const SamplerDesc* desc);
        VkDescriptorPool CreateDescriptorSetPool();

        RHIShaderModuleRef CreateShaderModuleCore(const ShaderModuleDesc& desc) override;
        RHIBindGroupLayoutRef CreateBindGroupLayoutCore(const BindGroupLayoutDesc& desc) override;

        RHIPipelineLayoutRef CreatePipelineLayoutCore(const PipelineLayoutDesc& desc) override;
        RHIBindGroupRef CreateBindGroupCore(RHIBindGroupLayout* layout, const BindGroupDesc& desc) override;
        RHIPipelineRef CreateRenderPipelineCore(const RenderPipelineDesc& desc) override;
        RHIPipelineRef CreateComputePipelineCore(const ComputePipelineDesc& desc) override;
        RHIPipelineRef CreateRayTracingPipelineCore(const RayTracingPipelineDesc& desc) override;
        RHIQueryHeapRef CreateQueryHeapCore(const QueryHeapDesc& desc) override;
        RHISwapChainRef CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc) override;
        void UpdateSwapChain(VulkanSwapChain* swapChain);

        void WriteShadingRateValue(ShadingRate rate, void* dest) const override;

        GraphicsContext* BeginGraphicsContext(const std::string& label = "") override;
        ComputeContext* BeginComputeContext(const std::string& label = "") override;
        GraphicsContext* BeginCommandContext(QueueType queue, const std::string& label);

        QueueFamilyIndices QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue);
        bool GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
        void FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices);
        void FillImageSharingIndices(VkImageCreateInfo& info, uint32_t* sharingIndices);
        void SetObjectName(VkObjectType type, uint64_t handle, const char* label);
        bool GetImageFormatProperties(const VkImageCreateInfo& createInfo, const void* pNext, VkImageFormatProperties2* properties2) const;
        bool GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, const void* pNext, VkImageFormatProperties2* properties2) const;

        void ProcessDeletionQueue(bool force);

        bool QueryFeatureSupport(RHIFeature feature) override;
        PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) override;
        bool QueryVertexFormatSupport(VertexFormat format);
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
        VkFormat ToVkFormat(PixelFormat format);
        bool IsDepthStencilFormatSupported(VkFormat format) const;

        GraphicsAPI GetGraphicsAPI() const override { return GraphicsAPI::Vulkan; }

        VkDevice GetHandle() const { return device; }
        VulkanQueue& GetGraphicsQueue() { return queues[ecast(QueueType::Graphics)]; }
        VulkanQueue& GetComputeQueue() { return queues[ecast(QueueType::Compute)]; }
        VulkanQueue& GetCopyQueue() { return queues[ecast(QueueType::Copy)]; }

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
        std::deque<std::pair<VkSurfaceKHR, uint64_t>> destroyedSurfaces;
        std::deque<std::pair<std::pair<VkDescriptorPool, VkDescriptorSet>, uint64_t>> destroyedDescriptorSets;

    private:
        bool shuttingDown{ false };
        bool debugUtils{ false };
        bool headless{ false };
        bool xcb_surface{ false };
        bool xlib_surface{ false };
        bool wayland_surface{ false };
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

        // Features
        VkPhysicalDeviceFeatures2 features2 = {};
        VkPhysicalDeviceVulkan11Features features11 = {};
        VkPhysicalDeviceVulkan12Features features12 = {};
        VkPhysicalDeviceVulkan13Features features13 = {};
        VkPhysicalDeviceVulkan14Features features14 = {};

        // Core 1.4
        VkPhysicalDeviceMaintenance6Features maintenance6Features = {};
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = {};
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProps = {};

        // Core in 1.3
        VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
        VkPhysicalDeviceMaintenance4Properties maintenance4Properties = {};
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};
        VkPhysicalDevicePipelineCreationCacheControlFeatures pipelineCreationCacheControlFeatures = {};

        VkPhysicalDeviceMaintenance5Features maintenance5Features = {};
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures{};
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
        VkPhysicalDeviceVulkan14Properties properties14 = {};

        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = {};
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = {};
        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProps = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
        VkPhysicalDeviceMemoryProperties2 memoryProperties2 = {};

        PhysicalDeviceExtensions physicalDeviceExtensions{};
        QueueFamilyIndices queueFamilyIndices{};
        bool supportsS8 = false;
        bool supportsD24S8 = false;
        bool supportsD32S8 = false;

        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

        VkDevice device = VK_NULL_HANDLE;
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

        std::vector<VkDynamicState> psoDynamicStates;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

        // Caches
        UnorderedMap<size_t, VkDescriptorSetLayout> descriptorSetLayoutCache;
        UnorderedMap<size_t, VkPipelineLayout> pipelineLayoutCache;
        Vector<VkDescriptorPool> descriptorSetPools;
        UnorderedMap<size_t, VkSampler> samplerCache;

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

        std::vector<std::unique_ptr<VulkanCommandContext>> commandBuffers;
        uint32_t cmdBuffersCount = 0;
        std::mutex cmdBuffersLocker;
    };

    class VulkanRHIFactory;

    class VulkanRHIAdapter final : public RHIAdapter
    {
    public:
        VulkanRHIFactory* factory;
        VkPhysicalDevice handle;
        PhysicalDeviceExtensions extensions;
        QueueFamilyIndices queueFamilyIndices;
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
        VkPhysicalDeviceMemoryProperties2 memoryProperties2;

        VulkanRHIAdapter(VulkanRHIFactory* factory_, VkPhysicalDevice handle_);
        bool Init();

        RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) override;

        VkFormat ToVkFormat(PixelFormat format);
        bool IsDepthStencilFormatSupported(VkFormat format) const;

        AdapterType GetType() const override { return _type; }

    private:
        AdapterType _type = AdapterType::Other;
    };

    class VulkanRHIFactory final : public RHIFactory
    {
    public:
#define VULKAN_INSTANCE_FUNCTION(name) PFN_##name name = nullptr;
#include "RHI_Vulkan_Funcs.h"

        bool debugUtils{ false };
        bool headless{ false };
        bool xcb_surface{ false };
        bool xlib_surface{ false };
        bool wayland_surface{ false };
        VkInstance handle = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

        static bool IsSupported();

        VulkanRHIFactory(const RHIFactoryDesc& desc);
        ~VulkanRHIFactory() override;

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
        switch (type)
        {
            case TextureType::Texture1D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
                break;
            case TextureType::Texture2D:
                createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
                break;
            case TextureType::Texture3D:
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case TextureType::TextureCube:
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
    {
    }

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

    /* VulkanBindGroupLayout */
    VulkanBindGroupLayout::~VulkanBindGroupLayout()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (handle)
        {
            device->destroyedDescriptorSetLayouts.push_back(std::make_pair(handle, frameCount));
        }
        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanBindGroupLayout::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanPipelineLayout */
    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        if (handle)
        {
            device->destroyedPipelineLayouts.push_back(std::make_pair(handle, frameCount));
        }
        handle = VK_NULL_HANDLE;
        device->destroyMutex.unlock();
    }

    void VulkanPipelineLayout::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanBindGroup */
    VulkanBindGroup::~VulkanBindGroup()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();
        device->destroyedDescriptorSets.push_back(std::make_pair(std::make_pair(descriptorPool, descriptorSet), frameCount));
        device->destroyMutex.unlock();
    }

    void VulkanBindGroup::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(descriptorSet), label);
    }

    void VulkanBindGroup::Update(size_t entryCount, const BindGroupEntry* entries)
    {
        // collect all of the descriptor write data
        const size_t layoutBindingCount = bindGroupLayout->layoutBindings.size();
        uint32_t descriptorWriteCount = 0;
        std::vector<VkWriteDescriptorSet> descriptorWrites(layoutBindingCount);
        std::vector<VkDescriptorImageInfo> descriptorImageInfo;
        std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
        //std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelStructWriteInfo;

        descriptorImageInfo.reserve(layoutBindingCount);
        descriptorBufferInfo.reserve(layoutBindingCount);

        // Generates a VkWriteDescriptorSet in descriptorWriteInfo
        auto generateWriteDescriptorData =
            [&](uint32_t bindingLocation,
                VkDescriptorType descriptorType,
                VkDescriptorImageInfo* imageInfo,
                VkDescriptorBufferInfo* bufferInfo,
                VkBufferView* bufferView,
                const void* pNext = nullptr)
            {
                descriptorWrites[descriptorWriteCount].pNext = pNext;
                descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[descriptorWriteCount].dstSet = descriptorSet;
                descriptorWrites[descriptorWriteCount].dstBinding = bindingLocation;
                descriptorWrites[descriptorWriteCount].dstArrayElement = 0;
                descriptorWrites[descriptorWriteCount].descriptorCount = 1;
                descriptorWrites[descriptorWriteCount].descriptorType = descriptorType;
                descriptorWrites[descriptorWriteCount].pImageInfo = imageInfo;
                descriptorWrites[descriptorWriteCount].pBufferInfo = bufferInfo;
                descriptorWrites[descriptorWriteCount].pTexelBufferView = bufferView;

                descriptorWriteCount++;
            };

        // TODO: Handle null descriptors
        for (size_t bindingIndex = 0; bindingIndex < layoutBindingCount; ++bindingIndex)
        {
            const VkDescriptorSetLayoutBinding& layoutBinding = bindGroupLayout->layoutBindings[bindingIndex];

            if (layoutBinding.pImmutableSamplers != nullptr)
                continue;

            VkDescriptorType descriptorType = layoutBinding.descriptorType;

            SharedPtr<VulkanBuffer> backendBuffer;
            SharedPtr<VulkanTextureView> backendTextureView;
            SharedPtr<VulkanSampler> backendSampler;

            const BindGroupEntry* foundEntry = nullptr;
            for (size_t i = 0; i < entryCount; ++i)
            {
                const BindGroupEntry& entry = entries[i];

                uint32_t originalBinding = bindGroupLayout->layoutBindingsOriginal[bindingIndex]; // layoutBinding.binding - registerOffset;

                if (entry.binding != originalBinding)
                    continue;

                switch (layoutBinding.descriptorType)
                {
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        backendSampler = entry.sampler ? StaticCast<VulkanSampler>(entry.sampler) : nullptr;
                        if (backendSampler == nullptr)
                            continue;
                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        backendTextureView = entry.textureView ? StaticCast<VulkanTextureView>(entry.textureView) : nullptr;
                        if (backendTextureView == nullptr)
                            continue;
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        backendBuffer = entry.buffer ? StaticCast<VulkanBuffer>(entry.buffer) : nullptr;
                        if (backendBuffer == nullptr || !CheckBitsAny(backendBuffer->GetUsage(), BufferUsage::ShaderReadWrite))
                            continue;
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        backendBuffer = entry.buffer ? StaticCast<VulkanBuffer>(entry.buffer) : nullptr;
                        if (backendBuffer == nullptr)
                            continue;
                        break;

                        //case VkDescriptorType.AccelerationStructureKHR:
                        //    return shaderResource;

                    default:
                        break;
                }

                foundEntry = &entry;
                break;
            }

            switch (layoutBinding.descriptorType)
            {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                {
                    auto& samplerImageInfo = descriptorImageInfo.emplace_back();
                    samplerImageInfo.sampler = foundEntry != nullptr ? backendSampler->handle : device->nullSampler;

                    ALIMER_ASSERT(samplerImageInfo.sampler != VK_NULL_HANDLE);
                    generateWriteDescriptorData(layoutBinding.binding,
                        layoutBinding.descriptorType,
                        &samplerImageInfo, nullptr, nullptr);
                }
                break;

                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                {
                    auto& imageInfo = descriptorImageInfo.emplace_back();
                    if (backendTextureView)
                    {
                        imageInfo.imageView = backendTextureView->handle;
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                    else
                    {
                        imageInfo.imageView = device->nullImageView2D;
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }

                    ALIMER_ASSERT(imageInfo.imageView != VK_NULL_HANDLE);
                    generateWriteDescriptorData(layoutBinding.binding,
                        layoutBinding.descriptorType,
                        &imageInfo, nullptr, nullptr);
                }
                break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                {
                    VkDescriptorBufferInfo& bufferInfo = descriptorBufferInfo.emplace_back();
                    if (backendBuffer != nullptr)
                    {
                        bufferInfo.buffer = backendBuffer->handle;
                        bufferInfo.offset = Min(foundEntry->offset, backendBuffer->GetSize());
                        bufferInfo.range = foundEntry->size;
                        if (bufferInfo.range == 0)
                        {
                            //bufferInfo.range = backendBuffer->GetSize() - bufferInfo.offset;
                            bufferInfo.range = VK_WHOLE_SIZE;
                        }
                        else
                        {
                            bufferInfo.range = Min(bufferInfo.range, backendBuffer->GetSize() - bufferInfo.offset);
                        }
                    }
                    else
                    {
                        bufferInfo.buffer = device->nullBuffer;
                        bufferInfo.range = VK_WHOLE_SIZE;
                    }

                    ALIMER_ASSERT(bufferInfo.buffer != VK_NULL_HANDLE);
                    generateWriteDescriptorData(layoutBinding.binding,
                        layoutBinding.descriptorType,
                        nullptr, &bufferInfo, nullptr);
                }
                break;
            }
        }

        device->vkUpdateDescriptorSets(
            device->device,
            descriptorWriteCount,
            descriptorWrites.data(),
            0,
            nullptr
        );
    }

    /* VulkanPipeline */
    VulkanPipeline::~VulkanPipeline()
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

    void VulkanPipeline::SetLabel(const char* label)
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
    VulkanSwapChain::VulkanSwapChain(RHISurface* surface_)
        : RHISwapChain(surface_)
    {
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        const uint64_t frameCount = device->GetFrameCount();
        device->destroyMutex.lock();

        if (handle)
        {
            device->destroyedSwapchains.push_back(std::make_pair(handle, frameCount));
        }

        if (vkSurface)
        {
            device->destroyedSurfaces.push_back(std::make_pair(vkSurface, frameCount));
        }

        for (size_t i = 0; i < backbufferTextures.size(); ++i)
        {
            device->destroyedSemaphores.push_back(std::make_pair(acquireSemaphores[i], frameCount));
            device->destroyedSemaphores.push_back(std::make_pair(releaseSemaphores[i], frameCount));
        }

        handle = VK_NULL_HANDLE;
        vkSurface = VK_NULL_HANDLE;

        device->destroyMutex.unlock();
    }

    void VulkanSwapChain::SetLabel(const char* label)
    {
        device->SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, reinterpret_cast<uint64_t>(handle), label);
    }

    /* VulkanCommandContext */
    VulkanCommandContext::VulkanCommandContext(VulkanDevice* device_, QueueType queueType_, uint32_t id_)
        : device(device_)
        , queueType(queueType_)
        , id(id_)
    {
        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = device_->queueFamilyIndices.familyIndices[ecast(queueType_)];

            VK_CHECK(device->vkCreateCommandPool(device->device, &poolInfo, nullptr, &commandPools[i]));

            VkCommandBufferAllocateInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferInfo.commandPool = commandPools[i];
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferInfo.commandBufferCount = 1;
            VK_CHECK(device->vkAllocateCommandBuffers(device->device, &commandBufferInfo, &commandBuffers[i]));
        }

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(device->vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &semaphore));
    }

    VulkanCommandContext::~VulkanCommandContext()
    {
        for (uint32_t i = 0; i < kMaxBindGroups; ++i)
        {
            boundBindGroups[i].Reset();
            descriptorSets[i] = VK_NULL_HANDLE;
        }

        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            device->vkDestroyCommandPool(device->device, commandPools[i], nullptr);
        }

        device->vkDestroySemaphore(device->device, semaphore, nullptr);
    }

    void VulkanCommandContext::Begin(uint32_t frameIndex, const std::string& label)
    {
        GraphicsContext::Reset(frameIndex);
        waits.clear();
        hasPendingWaits.store(false);
        currentPipeline.Reset();
        currentPipelineLayout.Reset();
        presentSwapChains.clear();
        memoryBarriers.clear();
        imageBarriers.clear();
        bufferBarriers.clear();

        VK_CHECK(device->vkResetCommandPool(device->device, commandPools[frameIndex], 0));
        commandBuffer = commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional
        VK_CHECK(device->vkBeginCommandBuffer(commandBuffer, &beginInfo));

        if (queueType != QueueType::Copy)
        {
            bindGroupsDirty = false;
            numBoundBindGroups = 0;
            for (uint32_t i = 0; i < kMaxBindGroups; ++i)
            {
                boundBindGroups[i].Reset();
                descriptorSets[i] = VK_NULL_HANDLE;
            }
        }

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

            if (device->features2.features.depthBounds == VK_TRUE)
            {
                device->vkCmdSetDepthBounds(commandBuffer, 0.0f, 1.0f);
            }

            // Silence validation about uninitialized stride:
            //const VkDeviceSize zero = {};
            //device->vkCmdBindVertexBuffers2(commandBuffer, 0, 1, &nullBuffer, &zero, &zero, &zero);

            if (device->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
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

        hasLabel = !label.empty();
        if (hasLabel)
        {
            PushDebugGroup(label);
            hasLabel = true;
        }
    }

    VkCommandBuffer VulkanCommandContext::End()
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

    void VulkanCommandContext::PushDebugGroup(std::string_view name)
    {
        if (!device->debugUtils)
            return;

        VkDebugUtilsLabelEXT label = {};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
    }

    void VulkanCommandContext::PopDebugGroup()
    {
        if (!device->debugUtils)
            return;

        vkCmdEndDebugUtilsLabelEXT(commandBuffer);
    }

    void VulkanCommandContext::InsertDebugMarker(std::string_view name)
    {
        if (!device->debugUtils)
            return;

        VkDebugUtilsLabelEXT label = {};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &label);
    }

    void VulkanCommandContext::BufferBarrier(const VulkanBuffer* buffer, BufferStates newState)
    {
        if (buffer->currentState == newState)
            return;

        VkBufferStateMapping before = ConvertBufferState(buffer->currentState);
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
        buffer->currentState = newState;
        numBarriersToCommit++;

        if (numBarriersToCommit >= kMaxBarrierCount)
            CommitBarriers();
    }

    void VulkanCommandContext::TextureBarrier(const VulkanTexture* texture, TextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, TextureAspect aspect)
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

        for (uint arrayLayer = baseArrayLayer; arrayLayer < (baseArrayLayer + layerCount); arrayLayer++)
        {
            for (uint mipLevel = baseMiplevel; mipLevel < (baseMiplevel + levelCount); mipLevel++)
            {
                const uint32_t iterSubresource = CalculateSubresource(mipLevel, arrayLayer, mipLevelCount);
                texture->imageLayouts[iterSubresource] = newLayout;
            }
        }
    }

    void VulkanCommandContext::TextureBarrier(const VulkanTextureView* view, TextureLayout newLayout)
    {
        const VulkanTexture* backendTexture = static_cast<const VulkanTexture*>(view->GetTexture());
        TextureBarrier(backendTexture, newLayout,
            view->GetBaseMipLevel(), view->GetMipLevelCount(),
            view->GetBaseArrayLayer(), view->GetArrayLayerCount(),
            view->GetAspect()
        );
    }

    void VulkanCommandContext::CommitBarriers()
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

    void VulkanCommandContext::CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer)
    {
        auto backendSrcBuffer = static_cast<const VulkanBuffer*>(sourceBuffer);
        auto backendDestBuffer = static_cast<const VulkanBuffer*>(destinationBuffer);

        BufferBarrier(backendSrcBuffer, BufferStates::CopySource);
        BufferBarrier(backendDestBuffer, BufferStates::CopyDest);
        CommitBarriers();

        VkBufferCopy copy = {};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = Min(backendSrcBuffer->GetSize(), backendDestBuffer->GetSize());

        device->vkCmdCopyBuffer(commandBuffer,
            backendSrcBuffer->handle,
            backendDestBuffer->handle,
            1, &copy
        );
    }

    void VulkanCommandContext::CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size)
    {
        auto backendSrcBuffer = static_cast<const VulkanBuffer*>(sourceBuffer);
        auto backendDestBuffer = static_cast<const VulkanBuffer*>(destinationBuffer);

        BufferBarrier(backendSrcBuffer, BufferStates::CopySource);
        BufferBarrier(backendDestBuffer, BufferStates::CopyDest);
        CommitBarriers();

        VkBufferCopy copy = {};
        copy.srcOffset = sourceOffset;
        copy.dstOffset = destinationOffset;
        copy.size = size;

        device->vkCmdCopyBuffer(commandBuffer,
            backendSrcBuffer->handle,
            backendDestBuffer->handle,
            1, &copy
        );
    }

    void VulkanCommandContext::SetPipeline(RHIPipeline* pipeline)
    {
        if (currentPipeline.Get() == pipeline)
            return;

        VulkanPipeline* newPipeline = static_cast<VulkanPipeline*>(pipeline);
        VulkanPipelineLayout* newPipelineLayout = newPipeline->layout.Get();

        if (currentPipelineLayout.Get() != newPipelineLayout)
        {

        }

        device->vkCmdBindPipeline(commandBuffer, newPipeline->bindPoint, newPipeline->handle);

        currentPipeline = newPipeline;
        currentPipelineLayout = newPipelineLayout;
    }

    void VulkanCommandContext::SetPushConstants(const void* data, uint32_t size, uint32_t offset)
    {
        ALIMER_ASSERT_MSG(currentPipelineLayout.Get() != nullptr, "No PipelineLayout bound");

#if defined(_DEBUG)
        if (size > device->properties2.properties.limits.maxPushConstantsSize)
        {
            LOGF("Push constant limit of {} exceeded (pushing {} bytes)", device->properties2.properties.limits.maxPushConstantsSize, size);
            return;
        }
#endif

        const VkPushConstantRange& range = currentPipelineLayout->pushConstantRange;
        device->vkCmdPushConstants(commandBuffer, currentPipelineLayout->handle, range.stageFlags, offset, size, data);
    }

    void VulkanCommandContext::SetBindGroup(uint32_t groupIndex, RHIBindGroup* bindGroup)
    {
        ALIMER_VERIFY(bindGroup != nullptr);
        ALIMER_VERIFY(groupIndex < kMaxBindGroups);

        if (boundBindGroups[groupIndex].Get() != bindGroup)
        {
            bindGroupsDirty = true;
            boundBindGroups[groupIndex] = static_cast<VulkanBindGroup*>(bindGroup);
            descriptorSets[groupIndex] = boundBindGroups[groupIndex]->descriptorSet;
            numBoundBindGroups = Max(groupIndex + 1, numBoundBindGroups);
        }
    }

    void VulkanCommandContext::FlushBindGroups()
    {
        if (!currentPipelineLayout)
            return;

        ALIMER_ASSERT_MSG(currentPipelineLayout.Get() != nullptr, "No PipelineLayout bound");
        ALIMER_ASSERT_MSG(currentPipeline.Get() != nullptr, "No Pipeline bound");

        if (!bindGroupsDirty || !currentPipelineLayout->bindGroupLayoutCount)
            return;

        device->vkCmdBindDescriptorSets(
            commandBuffer,
            currentPipeline->bindPoint,
            currentPipelineLayout->handle,
            0u,
            currentPipelineLayout->bindGroupLayoutCount,
            descriptorSets,
            0, nullptr
        );
        bindGroupsDirty = false;
    }

    void VulkanCommandContext::PrepareDispatch()
    {
        FlushBindGroups();
    }

    void VulkanCommandContext::DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        PrepareDispatch();

        device->vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    void VulkanCommandContext::BeginQuery(const RHIQueryHeap* heap, uint32_t index)
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

    void VulkanCommandContext::EndQuery(const RHIQueryHeap* heap, uint32_t index)
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

    void VulkanCommandContext::ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset)
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

    void VulkanCommandContext::ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count)
    {
        auto vulkanHeap = static_cast<const VulkanQueryHeap*>(heap);
        device->vkCmdResetQueryPool(commandBuffer, vulkanHeap->handle, index, count);
    }

    RHITexture* VulkanCommandContext::AcquireSwapChainTexture(RHISwapChain* swapChain)
    {
        VulkanSwapChain* backendSwapChain = (VulkanSwapChain*)swapChain;
        const size_t swapChainAcquireSemaphoreIndex = backendSwapChain->acquireSemaphoreIndex;

        backendSwapChain->locker.lock();
        VkResult result = device->vkAcquireNextImageKHR(
            device->device,
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
                        device->destroyedSemaphores.emplace_back(x, device->frameCount);
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

    void VulkanCommandContext::BeginRenderPassCore(const RenderPassDesc& desc)
    {
        VkRect2D renderArea = {};
        renderArea.extent.width = device->properties2.properties.limits.maxFramebufferWidth;
        renderArea.extent.height = device->properties2.properties.limits.maxFramebufferHeight;
        uint32_t layerCount = device->properties2.properties.limits.maxFramebufferLayers;

        uint32_t colorAttachmentCount = 0;
        VkRenderingAttachmentInfo colorAttachments[kMaxColorAttachments] = {};
        VkRenderingAttachmentInfo depthAttachment = {};
        VkRenderingAttachmentInfo stencilAttachment = {};

        PixelFormat depthStencilFormat = desc.depthStencilAttachment != nullptr ? desc.depthStencilAttachment->view->GetFormat() : PixelFormat::Undefined;
        const bool hasDepthOrStencil = desc.depthStencilAttachment != nullptr;

        for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            ALIMER_VERIFY(desc.colorAttachments[i].view != nullptr);

            const RenderPassColorAttachment& attachment = desc.colorAttachments[i];
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
            TextureBarrier(view, TextureLayout::RenderTarget);
        }

        if (hasDepthOrStencil)
        {
            const RenderPassDepthStencilAttachment& attachment = *desc.depthStencilAttachment;

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
            TextureBarrier(view, attachment.depthReadOnly ? TextureLayout::DepthRead : TextureLayout::DepthWrite);
        }
        CommitBarriers();

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

        device->vkCmdBeginRendering(commandBuffer, &renderingInfo);

        // The viewport and scissor default to cover all of the attachments
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(renderArea.extent.height);
        viewport.width = static_cast<float>(renderArea.extent.width);
        viewport.height = -static_cast<float>(renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        device->vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissorRect{};
        scissorRect.offset.x = 0;
        scissorRect.offset.y = 0;
        scissorRect.extent.width = renderArea.extent.width;
        scissorRect.extent.height = renderArea.extent.height;
        device->vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
    }

    void VulkanCommandContext::EndRenderPassCore()
    {
        device->vkCmdEndRendering(commandBuffer);
    }

    void VulkanCommandContext::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        VkViewport vkViewport{};
        vkViewport.x = x;
        vkViewport.y = height - y;
        vkViewport.width = width;
        vkViewport.height = -height;
        vkViewport.minDepth = minDepth;
        vkViewport.maxDepth = maxDepth;
        device->vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
    }

#if TODO
    void VulkanCommandContext::SetViewport(const Viewport& viewport)
    {
        // Flip viewport to match DirectX coordinate system
        VkViewport vkViewport{};
        vkViewport.x = viewport.x;
        vkViewport.y = viewport.height - viewport.y;
        vkViewport.width = viewport.width;
        vkViewport.height = -viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        device->vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
    }

    void VulkanCommandContext::SetViewports(uint32_t count, const Viewport* viewports)
    {
        ALIMER_ASSERT(viewports != nullptr);
        ALIMER_ASSERT(count < device->limits.maxViewports);

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

        device->vkCmdSetViewport(commandBuffer, 0, count, vkViewports);
    }
#endif // TODO


    void VulkanCommandContext::SetScissorRect(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        VkRect2D vkScissorRect;
        vkScissorRect.offset.x = x;
        vkScissorRect.offset.y = y;
        vkScissorRect.extent.width = static_cast<uint32_t>(width);
        vkScissorRect.extent.height = static_cast<uint32_t>(height);
        device->vkCmdSetScissor(commandBuffer, 0, 1, &vkScissorRect);
    }

#if TODO
    void VulkanCommandContext::SetScissorRect(const Rect& rect)
    {
        device->vkCmdSetScissor(commandBuffer, 0, 1, (VkRect2D*)&rect);
    }

    void VulkanCommandContext::SetScissorRects(uint32_t count, const Rect* scissorRects)
    {
        ALIMER_ASSERT(scissorRects != nullptr);
        ALIMER_ASSERT(count < device->limits.maxViewports);

        device->vkCmdSetScissor(commandBuffer, 0, count, (const VkRect2D*)scissorRects);
    }
#endif // TODO


    void VulkanCommandContext::SetStencilReference(uint32_t reference)
    {
        device->vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FRONT_AND_BACK, reference);
    }

    void VulkanCommandContext::SetBlendColor(float red, float green, float blue, float alpha)
    {
        const float blendcolor[4] = { red, green, blue, alpha };
        device->vkCmdSetBlendConstants(commandBuffer, blendcolor);
    }

    //void VulkanCommandContext::SetBlendColor(const Color& color)
    //{
    //    device->vkCmdSetBlendConstants(commandBuffer, &color.r);
    //}

    void VulkanCommandContext::SetShadingRate(ShadingRate rate)
    {
        if (device->fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE
            && currentShadingRate != rate)
        {
            currentShadingRate = rate;

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

            if (device->fragmentShadingRateProperties.fragmentShadingRateNonTrivialCombinerOps == VK_TRUE)
            {
                if (device->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
                {
                    combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
                }
                if (device->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
                {
                    combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
                }
            }
            else
            {
                if (device->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
                {
                    combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
                }
                if (device->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
                {
                    combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
                }
            }

            device->vkCmdSetFragmentShadingRateKHR(
                commandBuffer,
                &fragmentSize,
                combiner
            );
        }
    }

    void VulkanCommandContext::SetDepthBounds(float minBounds, float maxBounds)
    {
        if (device->features2.features.depthBounds == VK_TRUE)
        {
            device->vkCmdSetDepthBounds(commandBuffer, minBounds, maxBounds);
        }
        else
        {
            LOGW("DepthBounds is not supported");
        }
    }

    void VulkanCommandContext::SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset)
    {
        auto backendBuffer = static_cast<const VulkanBuffer*>(buffer);

        device->vkCmdBindVertexBuffers(commandBuffer, slot, 1u, &backendBuffer->handle, &offset);
    }

    void VulkanCommandContext::SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets)
    {
        ALIMER_ASSERT(buffers != nullptr);
        ALIMER_ASSERT(count <= ALIMER_STATIC_ARRAY_SIZE(buffers));
        ALIMER_ASSERT(count <= device->properties2.properties.limits.maxVertexInputBindings);

        VkBuffer vkBuffers[kMaxVertexBuffers];
        for (uint32_t i = 0; i < count; ++i)
        {
            if (buffers[i] == nullptr)
            {
                vkBuffers[i] = device->nullBuffer;
            }
            else
            {
                vkBuffers[i] = static_cast<const VulkanBuffer*>(buffers[i])->handle;
            }
        }

        device->vkCmdBindVertexBuffers(commandBuffer, slot, count, vkBuffers, offsets);
    }

    void VulkanCommandContext::SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexType indexType)
    {
        auto vulkanBuffer = static_cast<const VulkanBuffer*>(buffer);
        const VkIndexType vkIndexType = (indexType == IndexType::UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

        device->vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->handle, offset, vkIndexType);
    }

    void VulkanCommandContext::PrepareDraw()
    {
        ALIMER_ASSERT(insideRenderPass);

        FlushBindGroups();
    }

    void VulkanCommandContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        PrepareDraw();

        device->vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandContext::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        PrepareDraw();

        device->vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void VulkanCommandContext::DrawIndirect(const RHIBuffer* buffer, uint64_t offset)
    {
        ALIMER_ASSERT(buffer);
        PrepareDraw();

        auto vulkanBuffer = static_cast<const VulkanBuffer*>(buffer);
        device->vkCmdDrawIndirect(commandBuffer, vulkanBuffer->handle, offset, 1, (uint32_t)sizeof(DrawIndirectCommand));
    }

    void VulkanCommandContext::DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
        ALIMER_ASSERT(indirectBuffer);
        PrepareDraw();

        auto backendBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        device->vkCmdDrawIndexedIndirect(commandBuffer, backendBuffer->handle, indirectBufferOffset, 1, (uint32_t)sizeof(DrawIndexedIndirectCommand));
    }

    void VulkanCommandContext::DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        PrepareDraw();

        device->vkCmdDrawMeshTasksEXT(commandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    void VulkanCommandContext::DispatchMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
        ALIMER_ASSERT(indirectBuffer);
        PrepareDraw();

        auto backendIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        device->vkCmdDrawMeshTasksIndirectEXT(commandBuffer,
            backendIndirectBuffer->handle,
            indirectBufferOffset,
            1,
            sizeof(DispatchIndirectCommand)
        );
    }

    void VulkanCommandContext::DispatchMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount)
    {
        ALIMER_ASSERT(indirectBuffer);
        ALIMER_ASSERT(countBuffer);

        auto backendIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer);
        auto vulkanCountBuffer = static_cast<const VulkanBuffer*>(countBuffer);

        PrepareDraw();
        device->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer,
            backendIndirectBuffer->handle, indirectBufferOffset,
            vulkanCountBuffer->handle, countBufferOffset,
            maxCount, sizeof(DispatchIndirectCommand));
    }

    void VulkanCommandContext::BeginPredication(const RHIBuffer* buffer, uint64_t offset, PredicationOperation operation)
    {
        if (device->conditionalRenderingFeatures.conditionalRendering == VK_TRUE)
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

    void VulkanCommandContext::EndPredication()
    {
        if (device->conditionalRenderingFeatures.conditionalRendering == VK_TRUE)
        {
            device->vkCmdEndConditionalRenderingEXT(commandBuffer);
        }
    }

    /* VulkanDevice */
    VulkanDevice::VulkanDevice(const std::string& appName, const RHIDeviceDesc& desc)
    {
        VkResult result = VK_SUCCESS;

        // Enumerate available layers and extensions
        {
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
                else if (strcmp(availableExtension.extensionName, "VK_KHR_xcb_surface") == 0)
                {
                    xcb_surface = true;
                }
                else if (strcmp(availableExtension.extensionName, "VK_KHR_xlib_surface") == 0)
                {
                    xlib_surface = true;
                }
                else if (strcmp(availableExtension.extensionName, "VK_KHR_wayland_surface") == 0)
                {
                    wayland_surface = true;
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
            if (xcb_surface)
            {
                instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
            }
            else
            {
                ALIMER_ASSERT(xlib_surface);
                instanceExtensions.push_back("VK_KHR_xlib_surface");
            }

            if (wayland_surface)
            {
                instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
            }
#endif

            if (desc.validationMode != ValidationMode::Disabled)
            {
                // Determine the optimal validation layers to enable that are necessary for useful debugging
                std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
                instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
            }

            bool validationFeatures = false;
            if (desc.validationMode == ValidationMode::GPU)
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
            appInfo.pApplicationName = appName.data();
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

            if (desc.validationMode != ValidationMode::Disabled && debugUtils)
            {
                debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugUtilsCreateInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                debugUtilsCreateInfo.messageType =
                    //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

                if (desc.validationMode == ValidationMode::Verbose)
                {
                    debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                    debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                }

                debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
                createInfo.pNext = &debugUtilsCreateInfo;
            }

            VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
            if (desc.validationMode == ValidationMode::GPU && validationFeatures)
            {
                static const VkValidationFeatureEnableEXT enable_features[2] = {
                    VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                    VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                };
                validationFeaturesInfo.enabledValidationFeatureCount = 2;
                validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
                PnextChainPushFront(&createInfo, &validationFeaturesInfo);
            }

            result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
                return;
            }

#define VULKAN_INSTANCE_FUNCTION(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn);
#include "RHI_Vulkan_Funcs.h"

            if (desc.validationMode != ValidationMode::Disabled && debugUtils)
            {
                result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
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
        }

        // Enumerate physical device and detect best one.
        {
            uint32_t physicalDeviceCount = 0;
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
            if (physicalDeviceCount == 0)
            {
                LOGE("Vulkan: Failed to find GPUs with Vulkan support");
                return;
            }

            std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
            VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

            for (const VkPhysicalDevice& candidatePhysicalDevice : physicalDevices)
            {
                // We require minimum 1.2
                VkPhysicalDeviceProperties2 physicalDeviceProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
                vkGetPhysicalDeviceProperties2(candidatePhysicalDevice, &physicalDeviceProperties);
                if (physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_2)
                {
                    continue;
                }

                PhysicalDeviceExtensions physicalDeviceExt = QueryPhysicalDeviceExtensions(candidatePhysicalDevice);
                if (!physicalDeviceExt.swapchain)
                {
                    continue;
                }

                QueueFamilyIndices queueFamilyIndices = QueryQueueFamilies(candidatePhysicalDevice, physicalDeviceExtensions.video.queue);
                if (!queueFamilyIndices.IsComplete())
                {
                    continue;
                }


                bool priority = physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                if (desc.powerPreference == GPUPowerPreference::LowPower)
                {
                    priority = physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
                }

                if (priority || _physicalDevice == VK_NULL_HANDLE)
                {
                    _physicalDevice = candidatePhysicalDevice;
                    if (priority)
                    {
                        // If this is prioritized GPU type, look no further
                        break;
                    }
                }
            }

            if (_physicalDevice == VK_NULL_HANDLE)
            {
                LOGE("Vulkan: Failed to find a suitable GPU");
                return;
            }
        }

        // Create logical device now
        {
            VkPhysicalDeviceProperties2 physicalDeviceProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            vkGetPhysicalDeviceProperties2(_physicalDevice, &physicalDeviceProperties);

            physicalDeviceExtensions = QueryPhysicalDeviceExtensions(_physicalDevice);
            queueFamilyIndices = QueryQueueFamilies(_physicalDevice, physicalDeviceExtensions.video.queue);

            std::vector<const char*> enabledDeviceExtensions;
            enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

            // Features
            VkBaseOutStructure* featureChainCurrent{ nullptr };
            auto addToFeatureChain = [&featureChainCurrent](auto* next) {
                auto n = reinterpret_cast<VkBaseOutStructure*>(next);
                featureChainCurrent->pNext = n;
                featureChainCurrent = n;
                };

            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            featureChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&features2);

            features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
            addToFeatureChain(&features11);

            features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            addToFeatureChain(&features12);

            if (physicalDeviceProperties.properties.apiVersion >= VK_API_VERSION_1_3)
            {
                features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                addToFeatureChain(&features13);
            }

            if (physicalDeviceProperties.properties.apiVersion >= VK_API_VERSION_1_4)
            {
                features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
                addToFeatureChain(&features14);
            }

            // Extensions
            depthClipEnableFeatures = {};
            rayTracingPipelineFeatures = {};
            rayQueryFeatures = {};
            fragmentShadingRateFeatures = {};
            meshShaderFeatures = {};
            conditionalRenderingFeatures = {};

            // Properties
            VkBaseOutStructure* propertiesChainCurrent{ nullptr };
            auto addToPropertiesChain = [&propertiesChainCurrent](auto* next) {
                auto n = reinterpret_cast<VkBaseOutStructure*>(next);
                propertiesChainCurrent->pNext = n;
                propertiesChainCurrent = n;
                };

            properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            propertiesChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&properties2);

            properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
            addToPropertiesChain(&properties11);

            properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
            addToPropertiesChain(&properties12);

            if (physicalDeviceProperties.properties.apiVersion >= VK_API_VERSION_1_3)
            {
                properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
                addToPropertiesChain(&properties13);
            }

            if (physicalDeviceProperties.properties.apiVersion >= VK_API_VERSION_1_4)
            {
                properties14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;
                addToPropertiesChain(&properties14);
            }

            samplerFilterMinmaxProperties = {};
            depthStencilResolveProperties = {};
            conservativeRasterizationProps = {};
            accelerationStructureProperties = {};
            rayTracingPipelineProperties = {};
            fragmentShadingRateProperties = {};
            meshShaderProperties = {};

            samplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
            addToPropertiesChain(&samplerFilterMinmaxProperties);

            depthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
            addToPropertiesChain(&depthStencilResolveProperties);

            // Core in 1.3
            if (physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_3)
            {
                maintenance4Features = {};
                maintenance4Properties = {};
                dynamicRenderingFeatures = {};
                synchronization2Features = {};
                extendedDynamicStateFeatures = {};
                extendedDynamicState2Features = {};
                pipelineCreationCacheControlFeatures = {};
                astcHdrFeatures = {};

                if (physicalDeviceExtensions.maintenance4)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

                    maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
                    maintenance4Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;

                    addToFeatureChain(&maintenance4Features);
                    addToPropertiesChain(&maintenance4Properties);
                }

                if (physicalDeviceExtensions.dynamicRendering)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

                    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                    addToFeatureChain(&dynamicRenderingFeatures);
                }

                if (physicalDeviceExtensions.synchronization2)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

                    synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                    addToFeatureChain(&synchronization2Features);
                }

                if (physicalDeviceExtensions.extendedDynamicState)
                {
                    enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

                    extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
                    addToFeatureChain(&extendedDynamicStateFeatures);
                }

                if (physicalDeviceExtensions.extendedDynamicState2)
                {
                    enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

                    extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
                    addToFeatureChain(&extendedDynamicState2Features);
                }

                if (physicalDeviceExtensions.pipelineCreationCacheControl)
                {
                    enabledDeviceExtensions.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);

                    pipelineCreationCacheControlFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
                    addToFeatureChain(&pipelineCreationCacheControlFeatures);
                }

                if (physicalDeviceExtensions.textureCompressionAstcHdr)
                {
                    enabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

                    astcHdrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
                    addToFeatureChain(&astcHdrFeatures);
                }
            }
            else
            {
                // Core in 1.4
                if (physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_4)
                {
                    if (physicalDeviceExtensions.maintenance5)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

                        maintenance5Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
                        addToFeatureChain(&maintenance5Features);
                    }

                    if (physicalDeviceExtensions.maintenance6)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

                        maintenance6Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES;
                        maintenance6Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES;

                        addToFeatureChain(&maintenance6Features);
                        addToPropertiesChain(&maintenance6Properties);
                    }

                    if (physicalDeviceExtensions.pushDescriptor)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

                        pushDescriptorProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES;
                        addToPropertiesChain(&pushDescriptorProps);
                    }
                }
            }

            if (physicalDeviceExtensions.memoryBudget)
            {
                enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.AMD_device_coherent_memory)
            {
                enabledDeviceExtensions.push_back(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.EXT_memory_priority)
            {
                enabledDeviceExtensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.deferredHostOperations)
            {
                enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.portabilitySubset)
            {
                enabledDeviceExtensions.push_back("VK_KHR_portability_subset");
            }

            if (physicalDeviceExtensions.depthClipEnable)
            {
                enabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

                depthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
                addToFeatureChain(&depthClipEnableFeatures);
            }

            if (physicalDeviceExtensions.shaderViewportIndexLayer)
            {
                enabledDeviceExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
            }

            if (physicalDeviceExtensions.externalMemory)
            {
#if defined(_WIN32)
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
#else
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
#endif
            }

            if (physicalDeviceExtensions.externalSemaphore)
            {
#if defined(_WIN32)
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
#else
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
#endif
            }

            if (physicalDeviceExtensions.externalFence)
            {
#if defined(_WIN32)
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else
                enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
#endif
            }

            if (physicalDeviceExtensions.conservativeRasterization)
            {
                conservativeRasterizationProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
                addToPropertiesChain(&conservativeRasterizationProps);
            }

            if (physicalDeviceExtensions.accelerationStructure)
            {
                ALIMER_ASSERT(physicalDeviceExtensions.deferredHostOperations);

                // Required by VK_KHR_acceleration_structure
                enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
                enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

                accelerationStructureFeatures = {};
                accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                addToFeatureChain(&accelerationStructureFeatures);

                accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
                addToPropertiesChain(&accelerationStructureProperties);

                if (physicalDeviceExtensions.raytracingPipeline)
                {
                    // Required by VK_KHR_pipeline_library
                    enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                    enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

                    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                    addToFeatureChain(&rayTracingPipelineFeatures);

                    rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                    addToPropertiesChain(&rayTracingPipelineProperties);
                }

                if (physicalDeviceExtensions.rayQuery)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

                    rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                    addToFeatureChain(&rayQueryFeatures);
                }
            }

            if (physicalDeviceExtensions.fragmentShadingRate)
            {
                enabledDeviceExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

                fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
                addToFeatureChain(&fragmentShadingRateFeatures);

                fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
                addToPropertiesChain(&fragmentShadingRateProperties);
            }

            if (physicalDeviceExtensions.meshShader)
            {
                enabledDeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

                meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
                addToFeatureChain(&meshShaderFeatures);

                meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
                addToPropertiesChain(&meshShaderProperties);
            }

            if (physicalDeviceExtensions.conditionalRendering)
            {
                enabledDeviceExtensions.push_back(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

                conditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
                addToFeatureChain(&conditionalRenderingFeatures);
            }

            if (physicalDeviceExtensions.video.queue)
            {
                enabledDeviceExtensions.push_back(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

                if (physicalDeviceExtensions.video.decode_queue)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

                    if (physicalDeviceExtensions.video.decode_h264)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.video.decode_h265)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
                    }
                }

#if defined(RHI_VIDEO_ENCODE)
                if (physicalDeviceExtensions.video.encode_queue)
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

            vkGetPhysicalDeviceFeatures2(_physicalDevice, &features2);
            vkGetPhysicalDeviceProperties2(_physicalDevice, &properties2);

            memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
            vkGetPhysicalDeviceMemoryProperties2(_physicalDevice, &memoryProperties2);

            if (!features2.features.textureCompressionBC &&
                !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
            {
                LOGE("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
                return;
            }

            ALIMER_VERIFY(features2.features.robustBufferAccess == VK_TRUE);
            ALIMER_VERIFY(features2.features.fullDrawIndexUint32 == VK_TRUE);
            ALIMER_VERIFY(features2.features.depthClamp == VK_TRUE);
            ALIMER_VERIFY(features2.features.depthBiasClamp == VK_TRUE);
            ALIMER_VERIFY(features2.features.fragmentStoresAndAtomics == VK_TRUE);
            ALIMER_VERIFY(features2.features.imageCubeArray == VK_TRUE);
            ALIMER_VERIFY(features2.features.independentBlend == VK_TRUE);
            ALIMER_VERIFY(features2.features.sampleRateShading == VK_TRUE);
            ALIMER_VERIFY(features2.features.shaderClipDistance == VK_TRUE);
            ALIMER_VERIFY(features2.features.occlusionQueryPrecise == VK_TRUE);

            // Bindless (https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs)
            ALIMER_VERIFY(features12.descriptorIndexing == VK_TRUE);
            ALIMER_VERIFY(features12.runtimeDescriptorArray == VK_TRUE);
            ALIMER_VERIFY(features12.descriptorBindingPartiallyBound == VK_TRUE);
            ALIMER_VERIFY(features12.descriptorBindingVariableDescriptorCount == VK_TRUE);
            ALIMER_VERIFY(features12.shaderSampledImageArrayNonUniformIndexing == VK_TRUE);

            const bool synchronization2 = features13.synchronization2 == VK_TRUE || synchronization2Features.synchronization2 == VK_TRUE;
            const bool dynamicRendering = features13.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE;

            ALIMER_VERIFY(synchronization2 == true);
            ALIMER_VERIFY(dynamicRendering == true);
            ALIMER_VERIFY(properties2.properties.limits.maxPushConstantsSize >= kMaxPushConstantsSize);

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
            result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &device);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create device");
                return;
            }

#define VULKAN_DEVICE_FUNCTION(func) func = (PFN_##func) vkGetDeviceProcAddr(device, #func);
#include "RHI_Vulkan_Funcs.h"

            // VK_KHR_dynamic_rendering
            if (features13.dynamicRendering == VK_FALSE &&
                dynamicRenderingFeatures.dynamicRendering == VK_TRUE)
            {
                vkCmdBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
                vkCmdEndRendering = (PFN_vkCmdEndRendering)vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");
            }

            // VK_KHR_synchronization2
            if (features13.synchronization2 == VK_FALSE &&
                synchronization2Features.synchronization2 == VK_TRUE)
            {
                vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR");
                vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(device, "vkCmdWriteTimestamp2KHR");
                vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device, "vkQueueSubmit2KHR");
            }

            // VK_KHR_push_descriptor
            if (features14.pushDescriptor == VK_FALSE &&
                physicalDeviceExtensions.pushDescriptor == true)
            {
                vkCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
            }

            // Queues
            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            for (uint8_t i = 0; i < ecast(QueueType::Count); i++)
            {
                if (queueFamilyIndices.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
                {
                    VkDeviceQueueInfo2 queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
                    queueInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[i];
                    queueInfo.queueIndex = queueFamilyIndices.queueIndices[i];
                    vkGetDeviceQueue2(device, &queueInfo, &queues[i].queue);

                    queueFamilyIndices.counts[i] = queueFamilyIndices.queueOffsets[queueFamilyIndices.familyIndices[i]];

                    for (uint32_t frameIndex = 0; frameIndex < kNumFramesInFlight; ++frameIndex)
                    {
                        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &queues[i].frameFences[frameIndex]));
                    }
                }
                else
                {
                    queues[i].queue = VK_NULL_HANDLE;
                }
            }

#ifdef _DEBUG
            LOGI("Enabled {} Device Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
            }
#endif
        }

        // Create memory allocator
        {
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = _physicalDevice;
            allocatorInfo.device = device;
            allocatorInfo.instance = instance;
            allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

            // Core in 1.1
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;

            if (physicalDeviceExtensions.memoryBudget)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
            }

            if (physicalDeviceExtensions.AMD_device_coherent_memory)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
            }

            if (features12.bufferDeviceAddress == VK_TRUE)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            if (physicalDeviceExtensions.EXT_memory_priority)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
            }

            // Core in 1.3
            if (properties2.properties.apiVersion >= VK_API_VERSION_1_3
                || physicalDeviceExtensions.maintenance4)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
            }

            if (features14.maintenance5 == VK_TRUE
                || maintenance5Features.maintenance5 == VK_TRUE)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
            }

            if (physicalDeviceExtensions.externalMemory)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
            }

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
            static VmaVulkanFunctions vulkanFunctions = {};
            vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
            vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
            allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

            result = vmaCreateAllocator(&allocatorInfo, &allocator);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create allocator");
            }

            if (physicalDeviceExtensions.externalMemory)
            {
                std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
#if defined(_WIN32)
                externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
                externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif

                allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
                result = vmaCreateAllocator(&allocatorInfo, &externalAllocator);
                if (result != VK_SUCCESS)
                {
                    VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
                }
            }

            copyAllocator.Init(this);

            // Dynamic PSO states
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
            psoDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
            if (features2.features.depthBounds == VK_TRUE)
            {
                psoDynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
            }
            if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            {
                psoDynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
            }
            //psoDynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);

            dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateInfo.dynamicStateCount = (uint32_t)psoDynamicStates.size();
            dynamicStateInfo.pDynamicStates = psoDynamicStates.data();
        }

        // Init caps
        {
            adapterProperties.vendorID = properties2.properties.vendorID;
            adapterProperties.deviceID = properties2.properties.deviceID;
            adapterProperties.deviceName = properties2.properties.deviceName;
            adapterProperties.driverDescription = properties12.driverName;
            if (properties12.driverInfo[0] != '\0')
            {
                adapterProperties.driverDescription += std::string(": ") + properties12.driverInfo;
            }

            switch (properties2.properties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    adapterProperties.adapterType = AdapterType::IntegratedGpu;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    adapterProperties.adapterType = AdapterType::DiscreteGpu;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    adapterProperties.adapterType = AdapterType::Cpu;
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    adapterProperties.adapterType = AdapterType::VirtualGpu;
                    break;

                default:
                    adapterProperties.adapterType = AdapterType::Other;
                    break;
            }

            static_assert(kUUIDSize == sizeof(properties11.deviceUUID));
            memcpy(adapterProperties.uuid, properties11.deviceUUID, kUUIDSize);

            if (properties11.deviceLUIDValid)
            {
                static_assert(kLUIDSize == sizeof(properties11.deviceLUID));
                memcpy(adapterProperties.luid, properties11.deviceLUID, kLUIDSize);
            }

            // Go through the memory types to figure out the amount of VRAM on this physical device.
            for (uint32_t heapIndex = 0; heapIndex < memoryProperties2.memoryProperties.memoryHeapCount; ++heapIndex)
            {
                VkMemoryHeap const& heap = memoryProperties2.memoryProperties.memoryHeaps[heapIndex];
                if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    adapterProperties.videoMemorySize += heap.size;
                }
                else
                {
                    adapterProperties.systemMemorySize += heap.size;
                }
            }

            timestampFrequency = uint64_t(1.0 / double(properties2.properties.limits.timestampPeriod) * 1000 * 1000 * 1000);

            supportsS8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);
            supportsD24S8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
            supportsD32S8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);

            // Limits
            limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
            limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
            limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
            limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
            limits.maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
            //limits.maxBindGroups = properties2.properties.limits.maxBoundDescriptorSets;

            uploadBufferTextureRowAlignment = 1;
            uploadBufferTextureSliceAlignment = 1;
            limits.maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
            limits.maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
            limits.minConstantBufferOffsetAlignment = properties2.properties.limits.minUniformBufferOffsetAlignment;
            limits.minStorageBufferOffsetAlignment = properties2.properties.limits.minStorageBufferOffsetAlignment;

            [[maybe_unused]] const uint32_t maxPushDescriptors = pushDescriptorProps.maxPushDescriptors;
            limits.maxBufferSize = std::numeric_limits<uint64_t>::max();

            limits.maxVertexBuffers = Min(properties2.properties.limits.maxVertexInputBindings, kMaxVertexBuffers);
            limits.maxVertexAttributes = Min(properties2.properties.limits.maxVertexInputAttributes, kMaxVertexAttributes);
            limits.maxVertexBufferArrayStride = Min(properties2.properties.limits.maxVertexInputBindingStride, properties2.properties.limits.maxVertexInputAttributeOffset + 1);

            limits.maxColorAttachments = properties2.properties.limits.maxColorAttachments;
            limits.maxViewports = Min(properties2.properties.limits.maxViewports, kMaxViewportsAndScissors);
            //limits.samplerMinLodBias = -properties2.properties.limits.maxSamplerLodBias;
            //limits.samplerMaxLodBias = properties2.properties.limits.maxSamplerLodBias;
            limits.samplerMaxAnisotropy = (uint16_t)properties2.properties.limits.maxSamplerAnisotropy;

            limits.maxComputeWorkgroupStorageSize = properties2.properties.limits.maxComputeSharedMemorySize;
            limits.maxComputeInvocationsPerWorkgroup = properties2.properties.limits.maxComputeWorkGroupInvocations;

            limits.maxComputeWorkgroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0];
            limits.maxComputeWorkgroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1];
            limits.maxComputeWorkgroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2];

            limits.maxComputeWorkgroupsPerDimension = std::min({
                properties2.properties.limits.maxComputeWorkGroupCount[0],
                properties2.properties.limits.maxComputeWorkGroupCount[1],
                properties2.properties.limits.maxComputeWorkGroupCount[2],
                });

            // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage
            limits.highestShaderModel = ShaderModel::Model_6_0;
            if (features11.multiview)
            {
                limits.highestShaderModel = ShaderModel::Model_6_1;
            }
            if (features12.shaderFloat16 || features2.features.shaderInt16)
            {
                limits.highestShaderModel = ShaderModel::Model_6_2;
            }
            if (physicalDeviceExtensions.accelerationStructure
                && accelerationStructureFeatures.accelerationStructure == VK_TRUE)
            {
                limits.highestShaderModel = ShaderModel::Model_6_3;
            }
            //if (m_Desc.isMeshShaderSupported || m_Desc.rayTracingTier >= 2)
            //    m_Desc.shaderModel = 65;
            //if (m_Desc.isShaderAtomicsI64Supported)
            //    m_Desc.shaderModel = 66;
            //if (features.features.shaderStorageImageMultisample)
            //    adapterProperties.limits.highestShaderModel = 67;

            limits.conservativeRasterizationTier = ConservativeRasterizationTier::NotSupported;
            if (physicalDeviceExtensions.conservativeRasterization)
            {
                limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier1;

                if (conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
                    limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier2;
                if (conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
                    limits.conservativeRasterizationTier = ConservativeRasterizationTier::Tier3;
            }

            limits.variableShadingRateTier = VariableRateShadingTier::NotSupported;
            if (physicalDeviceExtensions.fragmentShadingRate)
            {
                if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
                {
                    limits.variableShadingRateTier = VariableRateShadingTier::Tier1;
                }

                if (fragmentShadingRateFeatures.primitiveFragmentShadingRate && fragmentShadingRateFeatures.attachmentFragmentShadingRate)
                {
                    limits.highestShaderModel = ShaderModel::Model_6_4;
                    limits.variableShadingRateTier = VariableRateShadingTier::Tier2;
                }

                const auto& tileExtent = fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize;
                limits.variableShadingRateImageTileSize = std::max(tileExtent.width, tileExtent.height);
                limits.isAdditionalVariableShadingRatesSupported = fragmentShadingRateProperties.maxFragmentSize.height > 2 || fragmentShadingRateProperties.maxFragmentSize.width > 2;
            }

            // Ray tracing
            limits.rayTracingTier = RayTracingTier::NotSupported;
            if (features12.bufferDeviceAddress == VK_TRUE
                && accelerationStructureFeatures.accelerationStructure == VK_TRUE
                && rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE)
            {
                limits.rayTracingTier = RayTracingTier::Tier1;

                if (rayQueryFeatures.rayQuery == VK_TRUE)
                {
                    limits.rayTracingTier = RayTracingTier::Tier2;
                }

                limits.rayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
                limits.rayTracingShaderTableAlignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
                limits.rayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
                limits.rayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
                limits.rayTracingMaxGeometryCount = (uint32_t)accelerationStructureProperties.maxGeometryCount;
                limits.rayTracingScratchAlignment = accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
            }

            // Mesh shader
            limits.meshShaderTier = MeshShaderTier::NotSupported;
            if (meshShaderFeatures.meshShader == VK_TRUE
                && meshShaderFeatures.taskShader == VK_TRUE)
            {
                limits.meshShaderTier = MeshShaderTier::Tier1;
            }
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
            VK_CHECK(vkCreateBufferView(device, &bufferViewInfo, nullptr, &nullBufferView));

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
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            imageViewInfo.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView3D));
        }

        // Allocate at least one descriptor pool.
        descriptorSetPools.emplace_back(CreateDescriptorSetPool());

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
                vkDestroyFence(device, queues[i].frameFences[frameIndex], nullptr);
            }
        }

        copyAllocator.Shutdown();

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

        commandBuffers.clear();
        cmdBuffersCount = 0;

        // Destory pending objects.
        ProcessDeletionQueue(true);
        frameCount = 0;

        // Release caches
        {
            // DescriptorSetLayouts
            for (auto& it : descriptorSetLayoutCache)
            {
                vkDestroyDescriptorSetLayout(device, it.second, nullptr);
            }
            descriptorSetLayoutCache.clear();

            // PipelineLayouts
            for (auto& it : pipelineLayoutCache)
            {
                vkDestroyPipelineLayout(device, it.second, nullptr);
            }
            pipelineLayoutCache.clear();

            // Samplers
            for (auto& it : samplerCache)
            {
                vkDestroySampler(device, it.second, nullptr);
            }
            samplerCache.clear();
            staticSamplers.clear();

            // Destroy Descriptor Pools
            for (VkDescriptorPool descriptorPool : descriptorSetPools)
            {
                vkDestroyDescriptorPool(device, descriptorPool, nullptr);
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
            vkDestroyPipelineCache(device, pipelineCache, nullptr);
            pipelineCache = VK_NULL_HANDLE;
        }

        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

        if (debugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
            debugUtilsMessenger = VK_NULL_HANDLE;
        }

        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }
    }

    bool VulkanDevice::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(device);
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
                VulkanCommandContext& commandBuffer = *commandBuffers[cmd].get();
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
                queues[i].Submit(this, queues[i].frameFences[frameIndex]);
            }
        }

        // Begin new frame
        frameCount++;
        frameIndex = frameCount % kNumFramesInFlight;

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (frameCount >= kNumFramesInFlight)
        {
            for (uint8_t i = 0; i < ecast(QueueType::Count); ++i)
            {
                if (queues[i].queue == VK_NULL_HANDLE)
                    continue;

                VK_CHECK(vkWaitForFences(device, 1, &queues[i].frameFences[frameIndex], true, 0xFFFFFFFFFFFFFFFF));
                VK_CHECK(vkResetFences(device, 1, &queues[i].frameFences[frameIndex]));
            }
        }

        ProcessDeletionQueue(false);
        return frameCount;
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
            createInfo.size = VmaAlignUp(desc.size, properties2.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::ShaderRead))
        {
            // Read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::ShaderWrite))
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
            && limits.rayTracingTier != RayTracingTier::NotSupported)
        {
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
            needBufferDeviceAddress = true;
        }

        // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
        if (features12.bufferDeviceAddress == VK_TRUE && needBufferDeviceAddress)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        uint32_t sharingIndices[3];
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
        if (physicalDeviceExtensions.maintenance5)
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
            buffer->deviceAddress = vkGetBufferDeviceAddress(device, &info);
        }

        // TODO
        buffer->currentState = BufferStates::Undefined;

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

                if (CheckBitsAny(desc.usage, BufferUsage::ShaderWrite))
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

    RHITextureRef VulkanDevice::CreateTextureCore(const TextureDesc& desc, const TextureData* initialData)
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

        switch (desc.type)
        {
            case TextureType::Texture1D:
                createInfo.imageType = VK_IMAGE_TYPE_1D;
                createInfo.arrayLayers = desc.depthOrArrayLayers;
                break;

            case TextureType::Texture2D:
                createInfo.imageType = VK_IMAGE_TYPE_2D;
                createInfo.extent.height = desc.height;
                createInfo.arrayLayers = desc.depthOrArrayLayers;
                break;

            case TextureType::Texture3D:
                createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
                createInfo.imageType = VK_IMAGE_TYPE_3D;
                createInfo.extent.height = desc.height;
                createInfo.extent.depth = desc.depthOrArrayLayers;
                createInfo.arrayLayers = 1u;
                break;

            case TextureType::TextureCube:
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

        uint32_t sharingIndices[3];
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
            VK_CHECK(vkGetMemoryWin32HandleKHR(device, &getWin32HandleInfoKHR, &texture->sharedHandle));
#elif defined(__linux__)
            VkMemoryGetFdInfoKHR memoryGetFdInfoKHR = {};
            memoryGetFdInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
            memoryGetFdInfoKHR.pNext = nullptr;
            memoryGetFdInfoKHR.memory = allocationInfo.deviceMemory;
            memoryGetFdInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
            VK_CHECK(vkGetMemoryFdKHR(device, &memoryGetFdInfoKHR, &texture->sharedHandle));
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

    RHITextureRef VulkanDevice::CreateTextureFromNativeHandleCore(RHINativeHandle handle, const TextureDesc& desc)
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

    VkDescriptorSetLayout VulkanDevice::GetOrCreateDescriptorSetLayout(const VulkanDescriptorSetLayout& setLayout)
    {
        size_t hash = 0;
        size_t i = 0;
        for (auto& x : setLayout.bindings)
        {
            HashCombine(hash, x.binding);
            HashCombine(hash, x.descriptorCount);
            HashCombine(hash, x.descriptorType);
            HashCombine(hash, x.stageFlags);
            // Does it makes sense?
            HashCombine(hash, setLayout.imageViewTypes[i++]);
        }

        auto it = descriptorSetLayoutCache.find(hash);
        if (it == descriptorSetLayoutCache.end())
        {
            VkDescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            createInfo.bindingCount = (uint32_t)setLayout.bindings.size();
            createInfo.pBindings = setLayout.bindings.data();

            // Bindless
            const bool isBindless = false;
            VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags = {};
            if (isBindless)
            {
                //    setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                //    setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(vkBindingFlags.size());
                //    setLayoutBindingFlags.pBindingFlags = vkBindingFlags.data();

                createInfo.pNext = &setLayoutBindingFlags;
                createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            }

            VkDescriptorSetLayout newDescriptorSetLayout;
            const VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &newDescriptorSetLayout);
            if (result != VK_SUCCESS)
            {
                LOGE("Vulkan: Failed to create DescriptorSetLayout, error: {}", VkResultToString(result));
                return VK_NULL_HANDLE;
            }

            descriptorSetLayoutCache[hash] = newDescriptorSetLayout;
            return newDescriptorSetLayout;
        }

        return it->second;
    }

    VkPipelineLayout VulkanDevice::GetOrCreatePipelineLayout(const VulkanPipelineLayoutReflection& reflection)
    {
        size_t hash = 0;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        for (auto& pair : reflection.layoutBindings)
        {
            descriptorSetLayouts.push_back(GetOrCreateDescriptorSetLayout(pair.second));
            HashCombine(hash, descriptorSetLayouts.back());
        }
        for (auto& x : reflection.bindlessBindings)
        {
            HashCombine(hash, x.used);
            HashCombine(hash, x.binding.binding);
            HashCombine(hash, x.binding.descriptorCount);
            HashCombine(hash, x.binding.descriptorType);
            HashCombine(hash, x.binding.stageFlags);
        }
        HashCombine(hash, reflection.pushConstantRange.stageFlags);
        HashCombine(hash, reflection.pushConstantRange.offset);
        HashCombine(hash, reflection.pushConstantRange.size);

        auto it = pipelineLayoutCache.find(hash);
        if (it == pipelineLayoutCache.end())
        {
            VkPipelineLayoutCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            createInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
            createInfo.pSetLayouts = descriptorSetLayouts.data();
            if (reflection.pushConstantRange.size > 0)
            {
                createInfo.pushConstantRangeCount = 1u;
                createInfo.pPushConstantRanges = &reflection.pushConstantRange;
            }

            VkPipelineLayout newPipelineLayout;
            const VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &newPipelineLayout);
            if (result != VK_SUCCESS)
            {
                LOGE("Vulkan: Failed to create PipelineLayout, error: {}", VkResultToString(result));
                return VK_NULL_HANDLE;
            }

            pipelineLayoutCache[hash] = newPipelineLayout;
            return newPipelineLayout;
        }

        return it->second;
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
            createInfo.addressModeU = ToVk(desc->addressModeU, features12.samplerMirrorClampToEdge);
            createInfo.addressModeV = ToVk(desc->addressModeU, features12.samplerMirrorClampToEdge);
            createInfo.addressModeW = ToVk(desc->addressModeU, features12.samplerMirrorClampToEdge);
            createInfo.mipLodBias = 0.0f;

            uint16_t maxAnisotropy = desc->maxAnisotropy;
            if (features2.features.samplerAnisotropy == VK_TRUE && maxAnisotropy > 1)
            {
                createInfo.anisotropyEnable = VK_TRUE;
                createInfo.maxAnisotropy = std::clamp(maxAnisotropy * 1.f, 1.f, properties2.properties.limits.maxSamplerAnisotropy);
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
            if (features12.samplerFilterMinmax == VK_TRUE &&
                (desc->reductionType == SamplerReductionType::Minimum || desc->reductionType == SamplerReductionType::Maximum))
            {
                samplerReductionModeInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
                samplerReductionModeInfo.reductionMode = ToVk(desc->reductionType);
                createInfo.pNext = &samplerReductionModeInfo;
            }

            VkSampler newSampler;
            VkResult result = vkCreateSampler(device, &createInfo, nullptr, &newSampler);
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
        VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
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
        SpvReflectResult reflectResult = spvReflectCreateShaderModule(desc.bytecodeSize, desc.bytecode, &reflectModule);
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

        uint32_t bindingCount = 0;
        reflectResult = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindingCount, nullptr);
        ALIMER_ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<SpvReflectDescriptorBinding*> descriptorBindings(bindingCount);
        reflectResult = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindingCount, descriptorBindings.data());
        ALIMER_ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS);

#if TODO_REFL
        for (auto& descriptorBinding : descriptorBindings)
        {
            const bool bindless = descriptorBinding->set > START_BINDLESS_SPACE;

            // There can be padding between normal/bindless spaces because sets need to be bound contiguously
            if (bindless)
            {
                reflection->bindlessBindings.resize(std::max(reflection->bindlessBindings.size(), (size_t)descriptorBinding->set));
                reflection->bindlessBindings[descriptorBinding->set - 1].used = true;
            }

            auto& descriptor = bindless ? reflection->bindlessBindings[descriptorBinding->set].binding : reflection->layoutBindings[descriptorBinding->set].bindings.emplace_back();
            descriptor.stageFlags = module->stage;
            descriptor.binding = descriptorBinding->binding;
            descriptor.descriptorCount = descriptorBinding->count;
            descriptor.descriptorType = (VkDescriptorType)descriptorBinding->descriptor_type;

            if (bindless)
                continue;

            auto& imageViewType = reflection->layoutBindings[descriptorBinding->set].imageViewTypes.emplace_back();
            imageViewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

            // Immutable samples
            if (descriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER
                && descriptorBinding->binding >= VULKAN_BINDING_SHIFT_S + kImmutableSamplerSlotBegin)
            {
                descriptor.pImmutableSamplers = immutableSamplers.data() + descriptorBinding->binding - VULKAN_BINDING_SHIFT_S - kImmutableSamplerSlotBegin;
                continue;
            }

            if (descriptor.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                // For now, always replace VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER with VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                // It would be quite messy to track which buffer is dynamic and which is not in the binding code, consider multiple pipeline bind points too
                // But maybe the dynamic uniform buffer is not always best because it occupies more registers (like DX12 root descriptor)?
                descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                for (uint32_t i = 0; i < descriptor.descriptorCount; ++i)
                {
                    reflection->uniformBufferSizes[descriptor.binding + i] = descriptorBinding->block.size;
                    reflection->uniformBufferDynamicSlots.push_back(descriptor.binding + i);
                }

                continue;
            }

            switch (descriptorBinding->descriptor_type)
            {
                default:
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    switch (descriptorBinding->image.dim)
                    {
                        default:
                        case SpvDim1D:
                            if (descriptorBinding->image.arrayed == 0)
                            {
                                imageViewType = VK_IMAGE_VIEW_TYPE_1D;
                            }
                            else
                            {
                                imageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                            }
                            break;
                        case SpvDim2D:
                            if (descriptorBinding->image.arrayed == 0)
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
                            if (descriptorBinding->image.arrayed == 0)
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
#endif // TODO_REFL

        //
        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = spvReflectGetCodeSize(&reflectModule);
        moduleInfo.pCode = spvReflectGetCode(&reflectModule);

        VkResult result = vkCreateShaderModule(device, &moduleInfo, nullptr, &module->stageInfo.module);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create a shader module");
            return nullptr;
        }

        spvReflectDestroyShaderModule(&reflectModule);
        return module;
    }

    RHIBindGroupLayoutRef VulkanDevice::CreateBindGroupLayoutCore(const BindGroupLayoutDesc& desc)
    {
        const size_t bindingLayoutCount = desc.entryCount;

        SharedPtr<VulkanBindGroupLayout> layout(new VulkanBindGroupLayout());
        layout->device = this;
        layout->layoutBindings.reserve(bindingLayoutCount);
        layout->layoutBindingsOriginal.reserve(bindingLayoutCount);

        std::vector<VkDescriptorBindingFlags> vkBindingFlags;
        vkBindingFlags.reserve(bindingLayoutCount);

        constexpr VkDescriptorBindingFlags bindlessBindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
            | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
            | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            //| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
            ;

        for (size_t i = 0; i < bindingLayoutCount; ++i)
        {
            const BindGroupLayoutEntry& entry = desc.entries[i];

            VkDescriptorSetLayoutBinding layoutBinding = {};
            uint32_t registerOffset = 0;

            if (entry.sampler.staticSampler != nullptr)
            {
                registerOffset = VULKAN_BINDING_SHIFT_S;
                VkSampler sampler = GetOrCreateVulkanSampler(entry.sampler.staticSampler);

                layoutBinding.binding = entry.binding + registerOffset;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                layoutBinding.descriptorCount = 1;
                layoutBinding.stageFlags = ToVkShaderStageFlags(entry.visibility);
                layoutBinding.pImmutableSamplers = &sampler;
                layout->layoutBindings.push_back(layoutBinding);
                layout->layoutBindingsOriginal.push_back(entry.binding);
                vkBindingFlags.push_back(bindlessBindingFlags);
                continue;
            }

            DescriptorType descriptorType = GetDescriptorType(entry);
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            switch (descriptorType)
            {
                case DescriptorType::Buffer:
                    switch (entry.buffer.type)
                    {
                        case BufferBindingType::Constant:
                            registerOffset = VULKAN_BINDING_SHIFT_B;
                            if (entry.buffer.hasDynamicOffset)
                            {
                                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                            }
                            else
                            {
                                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                            }
                            break;

                        case BufferBindingType::Storage:
                        case BufferBindingType::ReadOnlyStorage:
                            registerOffset = (entry.buffer.type == BufferBindingType::ReadOnlyStorage) ? VULKAN_BINDING_SHIFT_T : VULKAN_BINDING_SHIFT_U;
                            // UniformTexelBuffer, StorageTexelBuffer ?
                            if (entry.buffer.hasDynamicOffset)
                            {
                                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                            }
                            else
                            {
                                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            }
                            break;
                    }
                    break;


                case DescriptorType::Sampler:
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                    registerOffset = VULKAN_BINDING_SHIFT_S;
                    break;

                case DescriptorType::Texture:
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    registerOffset = VULKAN_BINDING_SHIFT_T;
                    break;

                case DescriptorType::StorageTexture:
                    registerOffset = VULKAN_BINDING_SHIFT_U;
                    switch (entry.storageTexture.access)
                    {
                        case StorageTextureAccess::WriteOnly:
                        case StorageTextureAccess::ReadWrite:
                            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            break;
                        case StorageTextureAccess::ReadOnly:
                            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            break;
                        case StorageTextureAccess::Undefined:
                            ALIMER_UNREACHABLE();
                    }

                default:
                    break;
            }
            layoutBinding.binding = registerOffset + entry.binding;
            layoutBinding.descriptorCount = entry.count;
            layoutBinding.stageFlags = ToVkShaderStageFlags(entry.visibility);

            layout->layoutBindings.push_back(layoutBinding);
            layout->layoutBindingsOriginal.push_back(entry.binding);

            vkBindingFlags.push_back(bindlessBindingFlags);
        }

        // Create entries for static samples
        for (size_t samplerIndex = 0; samplerIndex < vkStaticSamplers.size(); samplerIndex++)
        {
            VkSampler sampler = vkStaticSamplers[samplerIndex];

            uint32_t binding = kImmutableSamplerSlotBegin + static_cast<uint32_t>(samplerIndex);

            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding = binding + VULKAN_BINDING_SHIFT_S;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
            layoutBinding.pImmutableSamplers = &sampler;

            layout->layoutBindings.push_back(layoutBinding);
            layout->layoutBindingsOriginal.push_back(binding);
        }

        //layout->isBindless = true;
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = (uint32_t)layout->layoutBindings.size();
        createInfo.pBindings = layout->layoutBindings.data();

        // Bindless
        VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags = {};
        if (layout->isBindless)
        {
            setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(vkBindingFlags.size());
            setLayoutBindingFlags.pBindingFlags = vkBindingFlags.data();

            createInfo.pNext = &setLayoutBindingFlags;
            createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }

        VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout->handle);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64_t>(layout->handle), desc.label);

        return layout;
    }

    RHIPipelineLayoutRef VulkanDevice::CreatePipelineLayoutCore(const PipelineLayoutDesc& desc)
    {
        SharedPtr<VulkanPipelineLayout> layout(new VulkanPipelineLayout());
        layout->device = this;
        layout->bindGroupLayoutCount = (uint32_t)desc.bindGroupLayoutCount;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(desc.bindGroupLayoutCount);
        for (uint32_t i = 0; i < desc.bindGroupLayoutCount; i++)
        {
            descriptorSetLayouts[i] = StaticCast<VulkanBindGroupLayout>(desc.bindGroupLayouts[i])->handle;
        }

        // Push constants
        if (desc.pushConstantRange != nullptr)
        {
            uint32_t offset = 0;

            VkPushConstantRange& range = layout->pushConstantRange;
            range = {};
            range.stageFlags = ToVkShaderStageFlags(desc.pushConstantRange->visibility);
            range.offset = offset;
            range.size = desc.pushConstantRange->size;
            offset += range.size;
        }

        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
        createInfo.pSetLayouts = descriptorSetLayouts.data();
        if (layout->pushConstantRange.size)
        {
            createInfo.pushConstantRangeCount = 1u;
            createInfo.pPushConstantRanges = &layout->pushConstantRange;
        }

        const VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &layout->handle);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        if (desc.label)
        {
            layout->SetLabel(desc.label);
        }

        return layout;
    }

    RHIBindGroupRef VulkanDevice::CreateBindGroupCore(RHIBindGroupLayout* layout, const BindGroupDesc& desc)
    {
        VulkanBindGroupLayout* vulkanLayout = static_cast<VulkanBindGroupLayout*>(layout);

        auto AllocateDescriptorSet = [&](VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout setLayout, VkDescriptorSet& descriptorSet, uint32_t maxVariableDescriptorCounts)
            {
                // For variable length descriptor arrays, this specify the maximum count we expect them to be.
                // Note that this value will apply to all bindings defined as variable arrays in the BindGroupLayout
                // used to allocate this BindGroup
                VkDescriptorSetVariableDescriptorCountAllocateInfo variableLengthInfo{};
                variableLengthInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableLengthInfo.descriptorSetCount = 1;
                variableLengthInfo.pDescriptorCounts = &maxVariableDescriptorCounts;

                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &setLayout;
                allocInfo.pNext = &variableLengthInfo;

                return vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
            };

        // Have we create a DescriptorSet pool already?
        if (descriptorSetPools.empty())
            descriptorSetPools.emplace_back(CreateDescriptorSetPool());

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        //  Create DescriptorSet
        const uint32_t maxVariableArrayLength = 0;
        VkResult result = AllocateDescriptorSet(device, descriptorSetPools.back(), vulkanLayout->handle, descriptorSet, maxVariableArrayLength);
        // If we have run out of pool memory
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
        {
            // We need to allocate a new DescriptorPool and retry
            descriptorSetPools.emplace_back(CreateDescriptorSetPool());
            result = AllocateDescriptorSet(device, descriptorSetPools.back(), vulkanLayout->handle, descriptorSet, maxVariableArrayLength);
        }
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        if (desc.label)
        {
            SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(descriptorSet), desc.label);
        }

        SharedPtr<VulkanBindGroup> vulkanBindGroup(new VulkanBindGroup());
        vulkanBindGroup->device = this;
        vulkanBindGroup->bindGroupLayout = vulkanLayout;
        vulkanBindGroup->descriptorPool = descriptorSetPools.back();
        vulkanBindGroup->descriptorSet = descriptorSet;
        vulkanBindGroup->Update(desc.entryCount, desc.entries);

        return vulkanBindGroup;
    }

    RHIPipelineRef VulkanDevice::CreateRenderPipelineCore(const RenderPipelineDesc& desc)
    {
        SharedPtr<VulkanPipeline> pipeline(new VulkanPipeline());
        pipeline->device = this;
        pipeline->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        pipeline->layout = static_cast<VulkanPipelineLayout*>(desc.layout);

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

        // sort because dynamic offsets array is tightly packed to match slot numbers:
        std::sort(pipeline->reflection.uniformBufferDynamicSlots.begin(), pipeline->reflection.uniformBufferDynamicSlots.end());

        // VertexInputState (need always be specified when using VertexShader)
        VkPipelineVertexInputStateCreateInfo vertexInputState{};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        std::vector<VkVertexInputBindingDescription> vertexBindings;
        std::vector<VkVertexInputAttributeDescription> vertexAttributes;
        if (desc.vertexInput)
        {
            const VertexInputDesc* vertexInput = desc.vertexInput;

            vertexBindings.resize(vertexInput->bufferCount);
            vertexAttributes.resize(vertexInput->attributeCount);

            for (uint32_t bufferIndex = 0; bufferIndex < vertexInput->bufferCount; ++bufferIndex)
            {
                const VertexBufferLayout& layout = vertexInput->buffers[bufferIndex];
                vertexBindings[bufferIndex].binding = bufferIndex;
                vertexBindings[bufferIndex].stride = layout.stride;
                vertexBindings[bufferIndex].inputRate = ToVk(layout.stepMode);

                // Compute stride from attributes
                if (vertexBindings[bufferIndex].stride == 0)
                {
                    for (uint32_t attributeIndex = 0; attributeIndex < vertexInput->attributeCount; ++attributeIndex)
                    {
                        const VertexAttribute& attribute = vertexInput->attributes[attributeIndex];

                        const VertexFormatInfo& vertexFormatInfo = GetVertexFormatInfo(attribute.format);
                        vertexBindings[bufferIndex].stride += vertexFormatInfo.byteSize;
                    }
                }
            }

            for (uint32_t attributeIndex = 0; attributeIndex < vertexInput->attributeCount; ++attributeIndex)
            {
                const VertexAttribute& attribute = vertexInput->attributes[attributeIndex];

                vertexAttributes[attributeIndex].location = attribute.location;
                vertexAttributes[attributeIndex].binding = attribute.bufferIndex;
                vertexAttributes[attributeIndex].format = ToVkVertexFormat(attribute.format);
                vertexAttributes[attributeIndex].offset = attribute.offset;
            }

            vertexInputState.vertexBindingDescriptionCount = vertexInput->bufferCount;
            vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
            vertexInputState.vertexAttributeDescriptionCount = vertexInput->attributeCount;
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

        // TessellationState
        VkPipelineTessellationStateCreateInfo tessellationState{};
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        if (inputAssemblyState.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
        {
            tessellationState.patchControlPoints = desc.patchControlPoints;
        }
        else
        {
            tessellationState.patchControlPoints = 0;
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
        if (depthClipEnableFeatures.depthClipEnable == VK_TRUE)
        {
            depthClipStateInfo.depthClipEnable = (desc.rasterizerState.depthClipMode == DepthClipMode::Clip) ? VK_TRUE : VK_FALSE;

            rasterizationState.depthClampEnable = VK_TRUE;
            rasterizationState.pNext = &depthClipStateInfo;

            VK_APPEND_EXT(depthClipStateInfo);
        }

        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = ToVk(desc.rasterizerState.fillMode, features2.features.fillModeNonSolid);
        rasterizationState.cullMode = ToVk(desc.rasterizerState.cullMode);
        rasterizationState.frontFace = ToVk(desc.rasterizerState.frontFace);
        // Can be managed by command buffer
        rasterizationState.depthBiasEnable = desc.rasterizerState.depthBias != 0.0f || desc.rasterizerState.depthBiasSlopeScale != 0.0f;
        rasterizationState.depthBiasConstantFactor = desc.rasterizerState.depthBias;
        rasterizationState.depthBiasClamp = desc.rasterizerState.depthBiasClamp;
        rasterizationState.depthBiasSlopeFactor = desc.rasterizerState.depthBiasSlopeScale;
        rasterizationState.lineWidth = 1.0f;

        VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeState = {};
        if (physicalDeviceExtensions.conservativeRasterization &&
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
        if (features2.features.depthBounds == VK_TRUE)
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
        createInfo.pTessellationState = (inputAssemblyState.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) ? &tessellationState : nullptr;
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
        if (pipeline->layout)
        {
            createInfo.layout = pipeline->layout->handle;
        }
        else
        {
            VkPipelineLayout pipelineLayout = GetOrCreatePipelineLayout(pipeline->reflection);
            createInfo.layout = pipelineLayout;
        }
        createInfo.renderPass = VK_NULL_HANDLE;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, nullptr, &pipeline->handle);

        if (result != VK_SUCCESS)
        {
            LOGE("Vulkan: Failed to create graphics pipeline");
            return nullptr;
        }

        if (desc.label)
        {
            pipeline->SetLabel(desc.label);
        }

        return pipeline;
    }

    RHIPipelineRef VulkanDevice::CreateComputePipelineCore(const ComputePipelineDesc& desc)
    {
        SharedPtr<VulkanPipeline> pipeline(new VulkanPipeline());
        pipeline->device = this;
        pipeline->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        pipeline->type = RHIPipeline::Type::Compute;
        pipeline->layout = static_cast<VulkanPipelineLayout*>(desc.layout);

        VkPipelineShaderStageCreateInfo stage = StaticCast<VulkanShaderModule>(desc.shader)->stageInfo;
        ALIMER_ASSERT(stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

        VkComputePipelineCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.layout = pipeline->layout->handle;
        VkResult result = vkCreateComputePipelines(device, pipelineCache, 1, &createInfo, nullptr, &pipeline->handle);

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

    RHIPipelineRef VulkanDevice::CreateRayTracingPipelineCore(const RayTracingPipelineDesc& desc)
    {
        SharedPtr<VulkanPipeline> pipeline(new VulkanPipeline());
        pipeline->device = this;
        pipeline->bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        pipeline->type = RHIPipeline::Type::RayTracing;
        pipeline->layout = static_cast<VulkanPipelineLayout*>(desc.layout);

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
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::InputAssemblyVertices))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::InputAssemblyPrimitives))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::VSInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::GSInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::GSPrimitives))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::CInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::CPrimitives))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::FragmentShaderInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::HSInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::DSInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
            }
            if (CheckBitsAll(desc.pipelineStatisticsMask, PipelineStatisticsFlags::ComputeShaderInvocations))
            {
                createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
            }
        }

        VkQueryPool handle = VK_NULL_HANDLE;
        VkResult result = vkCreateQueryPool(device, &createInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        SharedPtr<VulkanQueryHeap> resource(new VulkanQueryHeap());
        resource->device = this;
        resource->desc = desc;
        resource->handle = handle;
        resource->queryResultSize = GetQueryResultSize(desc.type);

        if (desc.label)
        {
            resource->SetLabel(desc.label);
        }

        return resource;
    }

    RHISwapChainRef VulkanDevice::CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc)
    {
        SharedPtr<VulkanSwapChain> swapChain(new VulkanSwapChain(surface));
        swapChain->device = this;

        // Create surface first
        VkResult result = VK_SUCCESS;

        switch (surface->GetType())
        {
#if defined(_WIN32)
            case RHISurface::Type::WindowsHWND:
            {
                //PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
                if (!vkCreateWin32SurfaceKHR)
                {
                    LOGE("{} extension is not enabled in the Vulkan instance.", VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                    return nullptr;
                }

                VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
                surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
                surfaceCreateInfo.hwnd = static_cast<HWND>(surface->GetHWND());

                result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            }
            break;
#endif

#if defined(__ANDROID__)
            case RHISurface::Type::AndroidWindow:
            {
                VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
                surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
                surfaceCreateInfo.window = surface->GetAndroidNativeWindow();

                result = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            }
            break;
#endif

#if defined(__APPLE__)
            case RHISurface::Type::MetalLayer:
            {
                VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = {};
                surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
                surfaceCreateInfo.pLayer = surface->GetMetalLayer();

                result = vkCreateMetalSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            }
            break;
#endif

#if ALIMER_PLATFORM_LINUX
            case RHISurface::Type::XlibWindow:
            {
                //if (xcb_surface)
                //{
                //    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
                //    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
                //    surfaceCreateInfo.connection = xlibXcb.GetXCBConnection(static_cast<Display*>(surface->GetXDisplay()));
                //    surfaceCreateInfo.window = (xcb_window_t)window;
                //
                //    result = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
                //}
                //else
                //{
                //    VkXlibSurfaceCreateInfoKHR  surfaceCreateInfo = {};
                //    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
                //    surfaceCreateInfo.dpy = static_cast<Display*>(surface->GetXDisplay());
                //    surfaceCreateInfo.window = surface->GetXWindow());
                //
                //    result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
                //}
            }
            break;

            case RHISurface::Type::WaylandSurface:
            {
                VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
                surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
                surfaceCreateInfo.display = static_cast<struct wl_display*>(surface->GetWaylandDisplay());
                surfaceCreateInfo.surface = static_cast<struct wl_surface*>(surface->GetWaylandSurface());

                result = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            }
            break;
#endif

            default:
                ALIMER_UNREACHABLE();
                return nullptr;
        }

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create surface");
            return nullptr;
        }

        VkBool32 presentSupport = false;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            _physicalDevice,
            queueFamilyIndices.familyIndices[ecast(QueueType::Graphics)],
            swapChain->vkSurface,
            &presentSupport);

        // Present family not found, we cannot create SwapChain
        if (result != VK_SUCCESS || presentSupport == VK_FALSE)
        {
            return nullptr;
        }

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
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, swapChain->vkSurface, &caps));

        uint32_t formatCount;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, swapChain->vkSurface, &formatCount, nullptr));

        std::vector<VkSurfaceFormatKHR> swapchainFormats(formatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, swapChain->vkSurface, &formatCount, swapchainFormats.data()));

        uint32_t presentModeCount;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, swapChain->vkSurface, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> swapchainPresentModes(presentModeCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, swapChain->vkSurface, &presentModeCount, swapchainPresentModes.data()));

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
        createInfo.surface = swapChain->vkSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = { swapChain->extent.width, swapChain->extent.height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkFormatProperties2 formatProps2 = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(_physicalDevice, createInfo.imageFormat, &formatProps2);

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

        VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain->handle));

        if (createInfo.oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, createInfo.oldSwapchain, nullptr);
        }

        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain->handle, &imageCount, nullptr));
        std::vector<VkImage> swapchainImages(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain->handle, &imageCount, swapchainImages.data()));

        swapChain->imageIndex = 0;
        swapChain->backbufferTextures.resize(imageCount);
        swapChain->colorFormat = FromVkFormat(createInfo.imageFormat);
        swapChain->extent = createInfo.imageExtent;

        TextureDesc textureDesc{};
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
            destroyedSemaphores.push_back(std::make_pair(x, frameCount));
        }
        for (auto& x : swapChain->releaseSemaphores)
        {
            destroyedSemaphores.push_back(std::make_pair(x, frameCount));
        }
        swapChain->acquireSemaphores.clear();
        swapChain->releaseSemaphores.clear();
        destroyMutex.unlock();

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (size_t i = 0; i < imageCount; ++i)
        {
            VK_CHECK(
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &swapChain->acquireSemaphores.emplace_back())
            );
        }

        for (size_t i = 0; i < imageCount; ++i)
        {
            VK_CHECK(
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &swapChain->releaseSemaphores.emplace_back())
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

    GraphicsContext* VulkanDevice::BeginGraphicsContext(const std::string& label)
    {
        return BeginCommandContext(QueueType::Graphics, label);
    }

    ComputeContext* VulkanDevice::BeginComputeContext(const std::string& label)
    {
        return BeginCommandContext(QueueType::Compute, label);
    }

    GraphicsContext* VulkanDevice::BeginCommandContext(QueueType queue, const std::string& label)
    {
        cmdBuffersLocker.lock();
        uint32_t index = cmdBuffersCount++;
        if (index >= commandBuffers.size())
        {
            commandBuffers.push_back(std::make_unique<VulkanCommandContext>(this, queue, index));
        }
        cmdBuffersLocker.unlock();

        commandBuffers[index]->Begin(frameIndex, label);

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

    QueueFamilyIndices VulkanDevice::QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue)
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
            indices.familyIndices[ecast(QueueType::Graphics)],
            indices.queueIndices[ecast(QueueType::Graphics)], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
        {
            LOGE("Vulkan: Could not find suitable graphics queue.");
            return indices;
        }

        // XXX: This assumes timestamp valid bits is the same for all queue types.
        indices.timestampValidBits = queueFamilies[indices.familyIndices[ecast(QueueType::Graphics)]].queueFamilyProperties.timestampValidBits;

        // Prefer another graphics queue since we can do async graphics that way.
        // The compute queue is to be treated as high priority since we also do async graphics on it.
        if (!FindVacantQueue(indices.familyIndices[ecast(QueueType::Compute)], indices.queueIndices[ecast(QueueType::Compute)], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
            !FindVacantQueue(indices.familyIndices[ecast(QueueType::Compute)], indices.queueIndices[ecast(QueueType::Compute)], VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
        {
            // Fallback to the graphics queue if we must.
            indices.familyIndices[ecast(QueueType::Compute)] = indices.familyIndices[ecast(QueueType::Graphics)];
            indices.queueIndices[ecast(QueueType::Compute)] = indices.queueIndices[ecast(QueueType::Graphics)];
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(indices.familyIndices[ecast(QueueType::Copy)], indices.queueIndices[ecast(QueueType::Copy)], VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
            !FindVacantQueue(indices.familyIndices[ecast(QueueType::Copy)], indices.queueIndices[ecast(QueueType::Copy)], VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
        {
            indices.familyIndices[ecast(QueueType::Copy)] = indices.familyIndices[ecast(QueueType::Compute)];
            indices.queueIndices[ecast(QueueType::Copy)] = indices.queueIndices[ecast(QueueType::Compute)];
        }

        if (supportsVideoQueue)
        {
            if (!FindVacantQueue(indices.familyIndices[ecast(QueueType::VideoDecode)],
                indices.queueIndices[ecast(QueueType::VideoDecode)],
                VK_QUEUE_VIDEO_DECODE_BIT_KHR, 0, 0.5f))
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

    bool VulkanDevice::GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
    {
#if defined(_WIN32)
        //PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
        if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
        {
            LOGE("{} extension is not enabled in the Vulkan instance.", VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            return false;
        }

        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex) == VK_TRUE;
#elif defined(__ANDROID__)
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

    void VulkanDevice::FillBufferSharingIndices(VkBufferCreateInfo& info, uint32_t* sharingIndices)
    {
        for (auto& i : queueFamilyIndices.familyIndices)
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
        for (auto& i : queueFamilyIndices.familyIndices)
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

    void VulkanDevice::SetObjectName(VkObjectType type, uint64_t handle, const char* label)
    {
        if (!debugUtils)
            return;

        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = type;
        nameInfo.objectHandle = handle;
        nameInfo.pObjectName = label;
        vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
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

        const VkResult result = vkGetPhysicalDeviceImageFormatProperties2(_physicalDevice, &info, properties2);
        return result == VK_SUCCESS;
    }

    void VulkanDevice::ProcessDeletionQueue(bool force)
    {
        const auto Destroy = [&](auto&& queue, auto&& handler) {
            while (!queue.empty()) {
                if (force || (queue.front().second + kNumFramesInFlight < frameCount))
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
        Destroy(destroyedImageViews, [&](auto& item) { vkDestroyImageView(device, item, nullptr); });
        Destroy(destroyedBuffers, [&](auto& item) { vmaDestroyBuffer(allocator, item.first, item.second); });
        Destroy(destroyedBufferViews, [&](auto& item) { vkDestroyBufferView(device, item, nullptr); });
        Destroy(destroyedShaderModules, [&](auto& item) { vkDestroyShaderModule(device, item, nullptr); });
        Destroy(destroyedDescriptorSetLayouts, [&](auto& item) { vkDestroyDescriptorSetLayout(device, item, nullptr); });
        Destroy(destroyedPipelineLayouts, [&](auto& item) { vkDestroyPipelineLayout(device, item, nullptr); });
        Destroy(destroyedPipelines, [&](auto& item) { vkDestroyPipeline(device, item, nullptr); });
        Destroy(destroyedQueryPools, [&](auto& item) { vkDestroyQueryPool(device, item, nullptr); });
        Destroy(destroyedSemaphores, [&](auto& item) {vkDestroySemaphore(device, item, nullptr); });
        Destroy(destroyedSwapchains, [&](auto& item) { vkDestroySwapchainKHR(device, item, nullptr); });
        Destroy(destroyedSurfaces, [&](auto& item) { vkDestroySurfaceKHR(instance, item, nullptr); });
        Destroy(destroyedDescriptorSets, [&](auto& item) { vkFreeDescriptorSets(device, item.first, 1u, &item.second); });
        destroyMutex.unlock();
    }

    bool VulkanDevice::QueryFeatureSupport(RHIFeature feature)
    {
        switch (feature)  // NOLINT(clang-diagnostic-switch-enum)
        {
            case RHIFeature::TimestampQuery:
                return properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

            case RHIFeature::PipelineStatisticsQuery:
                return features2.features.pipelineStatisticsQuery == VK_TRUE;

            case RHIFeature::TextureCompressionBC:
                return features2.features.textureCompressionBC == VK_TRUE;

            case RHIFeature::TextureCompressionETC2:
                return features2.features.textureCompressionETC2 == VK_TRUE;

            case RHIFeature::TextureCompressionASTC_HDR:
                return features13.textureCompressionASTC_HDR == VK_TRUE || astcHdrFeatures.textureCompressionASTC_HDR == VK_TRUE;

            case RHIFeature::TextureCompressionASTC:
                return features2.features.textureCompressionASTC_LDR == VK_TRUE;

            case RHIFeature::IndirectFirstInstance:
                return features2.features.drawIndirectFirstInstance == VK_TRUE;

            case RHIFeature::ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case RHIFeature::GPUUploadHeapSupported:
                // https://github.com/KhronosGroup/Vulkan-Docs/issues/2096
                // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                return true;

            case RHIFeature::CopyQueueTimestampQuery:
                return properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

            case RHIFeature::TessellationShader:
                return features2.features.tessellationShader == VK_TRUE;

            case RHIFeature::DepthBoundsTest:
                return features2.features.depthBounds == VK_TRUE;

            case RHIFeature::SamplerMirrorOnce:
                return features12.samplerMirrorClampToEdge == VK_TRUE;

            case RHIFeature::SamplerBorder:
                return true;

            case RHIFeature::SamplerMinMax:
                return features12.samplerFilterMinmax == VK_TRUE;

            case RHIFeature::Bindless:
                // https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs
                return features12.descriptorIndexing == VK_TRUE
                    && features12.runtimeDescriptorArray == VK_TRUE
                    && features12.descriptorBindingPartiallyBound == VK_TRUE
                    && features12.descriptorBindingVariableDescriptorCount == VK_TRUE
                    && features12.shaderSampledImageArrayNonUniformIndexing == VK_TRUE
                    ;

            case RHIFeature::DepthResolveMinMax:
                return
                    (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                    (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MAX_BIT);

            case RHIFeature::StencilResolveMinMax:
                return
                    (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                    (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MAX_BIT);

            case RHIFeature::ShaderOutputViewportIndex:
                return features12.shaderOutputLayer == VK_TRUE && features12.shaderOutputViewportIndex == VK_TRUE;

            case RHIFeature::CacheCoherentUMA:
                if (memoryProperties2.memoryProperties.memoryHeapCount == 1 &&
                    memoryProperties2.memoryProperties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    return true;
                }

                return false;

            case RHIFeature::Predication:
                return conditionalRenderingFeatures.conditionalRendering == VK_TRUE;

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
        vkGetPhysicalDeviceFormatProperties2(_physicalDevice, vkFormat, &props);

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

    bool VulkanDevice::QueryVertexFormatSupport(VertexFormat format)
    {
        const VkFormat vkFormat = ToVkVertexFormat(format);
        if (vkFormat == VK_FORMAT_UNDEFINED)
            return false;

        VkFormatProperties2 props = {};
        props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
        vkGetPhysicalDeviceFormatProperties2(_physicalDevice, vkFormat, &props);

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
                return RHINativeHandle(instance);
            case RHINativeHandleType::VK_PhysicalDevice:
                return RHINativeHandle(_physicalDevice);
            case RHINativeHandleType::VK_Device:
                return RHINativeHandle(device);
            default:
                return nullptr;
        }
    }

    VkFormat VulkanDevice::ToVkFormat(PixelFormat format)
    {
        if (format == PixelFormat::Stencil8 && !supportsS8)
        {
            return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        if (format == PixelFormat::Depth24UnormStencil8 && !supportsD24S8)
        {
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        // https://github.com/doitsujin/dxvk/blob/master/src/dxgi/dxgi_format.cpp
        VkFormat vkFormat = static_cast<VkFormat>(Alimer::ToVkFormat(format));
        return vkFormat;
    }

    bool VulkanDevice::IsDepthStencilFormatSupported(VkFormat format) const
    {
        ALIMER_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

        VkFormatProperties2 properties2 = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(_physicalDevice, format, &properties2);
        return properties2.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    void VulkanQueue::Submit(VulkanDevice* device, VkFence fence)
    {
        if (queue == VK_NULL_HANDLE)
            return;

        std::scoped_lock lock(locker);

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
            device->vkDestroyCommandPool(device->device, context.transferCommandPool, nullptr);
            device->vkDestroyCommandPool(device->device, context.transitionCommandPool, nullptr);
            device->vkDestroySemaphore(device->device, context.semaphores[0], nullptr);
            device->vkDestroySemaphore(device->device, context.semaphores[1], nullptr);
            device->vkDestroySemaphore(device->device, context.semaphores[2], nullptr);
            device->vkDestroyFence(device->device, context.fence, nullptr);

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
                if (device->vkGetFenceStatus(device->device, freeList[i].fence) == VK_SUCCESS)
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
            poolCreateInfo.queueFamilyIndex = device->queueFamilyIndices.familyIndices[ecast(QueueType::Copy)];
            VK_CHECK(device->vkCreateCommandPool(device->device, &poolCreateInfo, nullptr, &context.transferCommandPool));

            poolCreateInfo.queueFamilyIndex = device->queueFamilyIndices.familyIndices[ecast(QueueType::Graphics)];
            VK_CHECK(device->vkCreateCommandPool(device->device, &poolCreateInfo, nullptr, &context.transitionCommandPool));

            VkCommandBufferAllocateInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferInfo.commandPool = context.transferCommandPool;
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferInfo.commandBufferCount = 1u;
            VK_CHECK(device->vkAllocateCommandBuffers(device->device, &commandBufferInfo, &context.transferCommandBuffer));

            commandBufferInfo.commandPool = context.transitionCommandPool;
            VK_CHECK(device->vkAllocateCommandBuffers(device->device, &commandBufferInfo, &context.transitionCommandBuffer));

            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            VK_CHECK(device->vkCreateFence(device->device, &fenceInfo, nullptr, &context.fence));

            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VK_CHECK(device->vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &context.semaphores[0]));
            VK_CHECK(device->vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &context.semaphores[1]));
            VK_CHECK(device->vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &context.semaphores[2]));

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
        VK_CHECK(device->vkResetCommandPool(device->device, context.transferCommandPool, 0));
        VK_CHECK(device->vkResetCommandPool(device->device, context.transitionCommandPool, 0));

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        VK_CHECK(device->vkBeginCommandBuffer(context.transferCommandBuffer, &beginInfo));
        VK_CHECK(device->vkBeginCommandBuffer(context.transitionCommandBuffer, &beginInfo));
        VK_CHECK(device->vkResetFences(device->device, 1, &context.fence));

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

        factory->vkGetPhysicalDeviceFeatures2(handle, &features2);
        factory->vkGetPhysicalDeviceProperties2(handle, &properties2);

        synchronization2 = features13.synchronization2 == VK_TRUE || synchronization2Features.synchronization2 == VK_TRUE;
        dynamicRendering = features13.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE;


        memoryProperties2 = {};
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        factory->vkGetPhysicalDeviceMemoryProperties2(handle, &memoryProperties2);

        switch (properties2.properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                _type = AdapterType::IntegratedGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                _type = AdapterType::DiscreteGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                _type = AdapterType::VirtualGpu;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                _type = AdapterType::Cpu;
                break;
            default:
                _type = AdapterType::Other;
                break;
        }

        driverDescription = properties12.driverName;
        if (properties12.driverInfo[0] != '\0')
        {
            driverDescription += std::string(": ") + properties12.driverInfo;
        }

        // The environment can request to various options for depth-stencil formats that could be
        // unavailable. Override the decision if it is not applicable.
        supportsDepth32Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
        supportsDepth24Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
        supportsStencil8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

        // Init limits
        _limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
        _limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
        _limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
        _limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
        _limits.maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
        _limits.maxBindGroups = properties2.properties.limits.maxBoundDescriptorSets;
        _limits.maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
        _limits.maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
        _limits.minConstantBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minUniformBufferOffsetAlignment;
        _limits.minStorageBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minStorageBufferOffsetAlignment;
        //_limits.maxPushConstantsSize = properties2.properties.limits.maxPushConstantsSize;
        [[maybe_unused]] const uint32_t maxPushDescriptors = pushDescriptorProps.maxPushDescriptors;
        _limits.maxBufferSize = properties13.maxBufferSize;
        _limits.maxColorAttachments = properties2.properties.limits.maxColorAttachments;
        _limits.maxViewports = properties2.properties.limits.maxViewports;
        //_limits.viewportBoundsMin = properties2.properties.limits.viewportBoundsRange[0];
        //_limits.viewportBoundsMax = properties2.properties.limits.viewportBoundsRange[1];

        /* Compute */
        _limits.maxComputeWorkgroupStorageSize = properties2.properties.limits.maxComputeSharedMemorySize;
        _limits.maxComputeInvocationsPerWorkgroup = properties2.properties.limits.maxComputeWorkGroupInvocations;

        _limits.maxComputeWorkgroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0];
        _limits.maxComputeWorkgroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1];
        _limits.maxComputeWorkgroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2];

        _limits.maxComputeWorkgroupsPerDimension = std::min({
            properties2.properties.limits.maxComputeWorkGroupCount[0],
            properties2.properties.limits.maxComputeWorkGroupCount[1],
            properties2.properties.limits.maxComputeWorkGroupCount[2],
            }
            );

#if TODO_VULKAN
        // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage
        _limits.shaderModel = GPUShaderModel_6_0;
        if (features11.multiview)
            limits.shaderModel = GPUShaderModel_6_1;
        if (features12.shaderFloat16 || features2.features.shaderInt16)
            limits.shaderModel = GPUShaderModel_6_2;
        if (extensions.accelerationStructure)
            limits.shaderModel = GPUShaderModel_6_3;
        if (limits.variableShadingRateTier >= GPUVariableRateShadingTier_2)
            limits.shaderModel = GPUShaderModel_6_4;
        //if (m_Desc.isMeshShaderSupported || m_Desc.rayTracingTier >= 2)
        //    m_Desc.shaderModel = 65;
        //if (m_Desc.isShaderAtomicsI64Supported)
        //    m_Desc.shaderModel = 66;
        //if (features.features.shaderStorageImageMultisample)
        //    m_Desc.shaderModel = 67;

        limits.conservativeRasterizationTier = GPUConservativeRasterizationTier_NotSupported;
        if (extensions.conservativeRasterization)
        {
            limits.conservativeRasterizationTier = GPUConservativeRasterizationTier_1;

            if (conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
                limits.conservativeRasterizationTier = GPUConservativeRasterizationTier_2;
            if (conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
                limits.conservativeRasterizationTier = GPUConservativeRasterizationTier_3;
        }

        limits.variableShadingRateTier = GPUVariableRateShadingTier_NotSupported;
        if (extensions.fragmentShadingRate)
        {
            if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            {
                limits.variableShadingRateTier = GPUVariableRateShadingTier_1;
            }

            if (fragmentShadingRateFeatures.primitiveFragmentShadingRate && fragmentShadingRateFeatures.attachmentFragmentShadingRate)
            {
                limits.variableShadingRateTier = GPUVariableRateShadingTier_2;
            }

            const auto& tileExtent = fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize;
            limits.variableShadingRateImageTileSize = std::max(tileExtent.width, tileExtent.height);
            limits.isAdditionalVariableShadingRatesSupported = fragmentShadingRateProperties.maxFragmentSize.height > 2 || fragmentShadingRateProperties.maxFragmentSize.width > 2;
        }

        // Ray tracing
        limits.rayTracingTier = GPURayTracingTier_NotSupported;
        if (features12.bufferDeviceAddress == VK_TRUE
            && accelerationStructureFeatures.accelerationStructure == VK_TRUE
            && rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE)
        {
            limits.rayTracingTier = GPURayTracingTier_1;

            if (rayQueryFeatures.rayQuery == VK_TRUE)
            {
                limits.rayTracingTier = GPURayTracingTier_2;
            }

            //if (OpacityMicromapFeatures.micromap)
            //    m_Desc.tiers.rayTracing++;

            limits.rayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            limits.rayTracingShaderTableAlignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
            limits.rayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
            limits.rayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
            limits.rayTracingMaxGeometryCount = (uint32_t)accelerationStructureProperties.maxGeometryCount;
            limits.rayTracingScratchAlignment = accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
        }

        // Mesh shader
        limits.meshShaderTier = GPUMeshShaderTier_NotSupported;
        if (meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE)
        {
            limits.meshShaderTier = GPUMeshShaderTier_1;
        }
#endif // TODO_VULKAN

        return true;
    }

    RHIDeviceRef VulkanRHIAdapter::CreateDevice(const RHIDeviceDesc& desc)
    {
        // TODO
        return nullptr;
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
        HMODULE module = LoadLibraryA("vulkan-1.dll");
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
            else if (strcmp(availableExtension.extensionName, "VK_KHR_xcb_surface") == 0)
            {
                xcb_surface = true;
            }
            else if (strcmp(availableExtension.extensionName, "VK_KHR_xlib_surface") == 0)
            {
                xlib_surface = true;
            }
            else if (strcmp(availableExtension.extensionName, "VK_KHR_wayland_surface") == 0)
            {
                wayland_surface = true;
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
        if (xcb_surface)
        {
            instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        }
        else
        {
            ALIMER_ASSERT(xlib_surface);
            instanceExtensions.push_back("VK_KHR_xlib_surface");
        }

        if (wayland_surface)
        {
            instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        }
#endif

        if (desc.validationMode != ValidationMode::Disabled)
        {
            // Determine the optimal validation layers to enable that are necessary for useful debugging
            std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

        bool validationFeatures = false;
        if (desc.validationMode == ValidationMode::GPU)
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

        if (desc.validationMode != ValidationMode::Disabled && debugUtils)
        {
            debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType =
                //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            if (desc.validationMode == ValidationMode::Verbose)
            {
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            }

            debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
            createInfo.pNext = &debugUtilsCreateInfo;
        }

        VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
        if (desc.validationMode == ValidationMode::GPU && validationFeatures)
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

        if (desc.validationMode != ValidationMode::Disabled && debugUtils)
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
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME) == 0)
            {
                extensions.unifiedImageLayouts = true;
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
#if defined(_WIN32)
        //PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
        if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
        {
            LOGE("{} extension is not enabled in the Vulkan instance.", VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            return false;
        }

        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex) == VK_TRUE;
#elif defined(__ANDROID__)
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
