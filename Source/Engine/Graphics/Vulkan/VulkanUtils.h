// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Log.h"
#include "Graphics/GraphicsDefs.h"
#define NOMINMAX
#include "volk.h"
#include "vk_mem_alloc.h"
#include <array>
#include <deque>
#include <set>

// Default fence timeout in nanoseconds
#define VK_DEFAULT_FENCE_TIMEOUT 100000000000

namespace Alimer
{
	class VulkanGraphics;
	class VulkanBuffer;
	class VulkanTexture;
	class VulkanTextureView;
	class VulkanSwapChain;
	class VulkanShader;
	class VulkanDescriptorSetLayout;
	class VulkanPipelineLayout;
	class VulkanPipeline;
	class VulkanCommandBuffer;

	static constexpr uint32_t kVulkanBindingShift_CBV = 0;
	static constexpr uint32_t kVulkanBindingShift_SRV = 1000;
	static constexpr uint32_t kVulkanBindingShift_UAV = 2000;
	static constexpr uint32_t kVulkanBindingShift_Sampler = 3000;

	constexpr const char* ToString(VkResult result)
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

    [[nodiscard]] constexpr VkFormat ToVulkanFormat(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
        case PixelFormat::R8Unorm:  return VK_FORMAT_R8_UNORM;
        case PixelFormat::R8Snorm:  return VK_FORMAT_R8_SNORM;
        case PixelFormat::R8Uint:   return VK_FORMAT_R8_UINT;
        case PixelFormat::R8Sint:   return VK_FORMAT_R8_SINT;
            // 16-bit formats
        case PixelFormat::R16Unorm:     return VK_FORMAT_R16_UNORM;
        case PixelFormat::R16Snorm:     return VK_FORMAT_R16_SNORM;
        case PixelFormat::R16Uint:      return VK_FORMAT_R16_UINT;
        case PixelFormat::R16Sint:      return VK_FORMAT_R16_SINT;
        case PixelFormat::R16Float:     return VK_FORMAT_R16_SFLOAT;
        case PixelFormat::RG8Unorm:     return VK_FORMAT_R8G8_UNORM;
        case PixelFormat::RG8Snorm:     return VK_FORMAT_R8G8_SNORM;
        case PixelFormat::RG8Uint:      return VK_FORMAT_R8G8_UINT;
        case PixelFormat::RG8Sint:      return VK_FORMAT_R8G8_SINT;
            // 32-bit formats
        case PixelFormat::R32Uint:          return VK_FORMAT_R32_UINT;
        case PixelFormat::R32Sint:          return VK_FORMAT_R32_SINT;
        case PixelFormat::R32Float:         return VK_FORMAT_R32_SFLOAT;
        case PixelFormat::RG16Unorm:        return VK_FORMAT_R16G16_UNORM;
        case PixelFormat::RG16Snorm:        return VK_FORMAT_R16G16_SNORM;
        case PixelFormat::RG16Uint:         return VK_FORMAT_R16G16_UINT;
        case PixelFormat::RG16Sint:         return VK_FORMAT_R16G16_SINT;
        case PixelFormat::RG16Float:        return VK_FORMAT_R16G16_SFLOAT;
        case PixelFormat::RGBA8Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::RGBA8UnormSrgb:   return VK_FORMAT_R8G8B8A8_SRGB;
        case PixelFormat::RGBA8Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
        case PixelFormat::RGBA8Uint:        return VK_FORMAT_R8G8B8A8_UINT;
        case PixelFormat::RGBA8Sint:        return VK_FORMAT_R8G8B8A8_SINT;
        case PixelFormat::BGRA8Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::BGRA8UnormSrgb:   return VK_FORMAT_B8G8R8A8_SRGB;
            // Packed 32-Bit formats
        case PixelFormat::RGB10A2Unorm:     return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case PixelFormat::RG11B10Float:     return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PixelFormat::RGB9E5Float:      return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            // 64-Bit formats
        case PixelFormat::RG32Uint:         return VK_FORMAT_R32G32_UINT;
        case PixelFormat::RG32Sint:         return VK_FORMAT_R32G32_SINT;
        case PixelFormat::RG32Float:        return VK_FORMAT_R32G32_SFLOAT;
        case PixelFormat::RGBA16Unorm:      return VK_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat::RGBA16Snorm:      return VK_FORMAT_R16G16B16A16_SNORM;
        case PixelFormat::RGBA16Uint:       return VK_FORMAT_R16G16B16A16_UINT;
        case PixelFormat::RGBA16Sint:       return VK_FORMAT_R16G16B16A16_SINT;
        case PixelFormat::RGBA16Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;
            // 128-Bit formats
        case PixelFormat::RGBA32Uint:       return VK_FORMAT_R32G32B32A32_UINT;
        case PixelFormat::RGBA32Sint:       return VK_FORMAT_R32G32B32A32_SINT;
        case PixelFormat::RGBA32Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;
            // Depth-stencil formats
        case PixelFormat::Depth16Unorm:     return VK_FORMAT_D16_UNORM;
        case PixelFormat::Depth32Float:     return VK_FORMAT_D32_SFLOAT;
        case PixelFormat::Depth24UnormStencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat::Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            // Compressed BC formats
        case PixelFormat::BC1RGBAUnorm:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PixelFormat::BC1RGBAUnormSrgb:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case PixelFormat::BC2RGBAUnorm:         return VK_FORMAT_BC2_UNORM_BLOCK;
        case PixelFormat::BC2RGBAUnormSrgb:     return VK_FORMAT_BC2_SRGB_BLOCK;
        case PixelFormat::BC3RGBAUnorm:         return VK_FORMAT_BC3_UNORM_BLOCK;
        case PixelFormat::BC3RGBAUnormSrgb:     return VK_FORMAT_BC3_SRGB_BLOCK;
        case PixelFormat::BC4RSnorm:            return VK_FORMAT_BC4_SNORM_BLOCK;
        case PixelFormat::BC4RUnorm:            return VK_FORMAT_BC4_UNORM_BLOCK;
        case PixelFormat::BC5RGSnorm:           return VK_FORMAT_BC5_SNORM_BLOCK;
        case PixelFormat::BC5RGUnorm:           return VK_FORMAT_BC5_UNORM_BLOCK;
        case PixelFormat::BC6HRGBFloat:         return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case PixelFormat::BC6HRGBUFloat:        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PixelFormat::BC7RGBAUnorm:         return VK_FORMAT_BC7_UNORM_BLOCK;
        case PixelFormat::BC7RGBAUnormSrgb:     return VK_FORMAT_BC7_SRGB_BLOCK;
            // EAC/ETC compressed formats
        case PixelFormat::ETC2RGB8Unorm:            return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PixelFormat::ETC2RGB8UnormSrgb:        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case PixelFormat::ETC2RGB8A1Unorm:          return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case PixelFormat::ETC2RGB8A1UnormSrgb:      return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case PixelFormat::ETC2RGBA8Unorm:           return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case PixelFormat::ETC2RGBA8UnormSrgb:       return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case PixelFormat::EACR11Unorm:              return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case PixelFormat::EACR11Snorm:              return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case PixelFormat::EACRG11Unorm:             return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case PixelFormat::EACRG11Snorm:             return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
            // ASTC compressed formats
        case PixelFormat::ASTC4x4Unorm:
            return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case PixelFormat::ASTC4x4UnormSrgb:
            return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case PixelFormat::ASTC5x4Unorm:
            return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case PixelFormat::ASTC5x4UnormSrgb:
            return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case PixelFormat::ASTC5x5Unorm:
            return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case PixelFormat::ASTC5x5UnormSrgb:
            return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case PixelFormat::ASTC6x5Unorm:
            return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case PixelFormat::ASTC6x5UnormSrgb:
            return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case PixelFormat::ASTC6x6Unorm:
            return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case PixelFormat::ASTC6x6UnormSrgb:
            return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case PixelFormat::ASTC8x5Unorm:
            return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case PixelFormat::ASTC8x5UnormSrgb:
            return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case PixelFormat::ASTC8x6Unorm:
            return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case PixelFormat::ASTC8x6UnormSrgb:
            return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case PixelFormat::ASTC8x8Unorm:
            return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case PixelFormat::ASTC8x8UnormSrgb:
            return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case PixelFormat::ASTC10x5Unorm:
            return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case PixelFormat::ASTC10x5UnormSrgb:
            return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case PixelFormat::ASTC10x6Unorm:
            return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case PixelFormat::ASTC10x6UnormSrgb:
            return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case PixelFormat::ASTC10x8Unorm:
            return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case PixelFormat::ASTC10x8UnormSrgb:
            return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case PixelFormat::ASTC10x10Unorm:
            return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case PixelFormat::ASTC10x10UnormSrgb:
            return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case PixelFormat::ASTC12x10Unorm:
            return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case PixelFormat::ASTC12x10UnormSrgb:
            return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case PixelFormat::ASTC12x12Unorm:
            return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case PixelFormat::ASTC12x12UnormSrgb:
            return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        default:
            ALIMER_UNREACHABLE();
            return VK_FORMAT_UNDEFINED;
        }
    }

    [[nodiscard]] constexpr PixelFormat FromVulkanFormat(VkFormat format)
    {
        switch (format)
        {
            // 8-bit formats
        case VK_FORMAT_R8_UNORM:	return PixelFormat::R8Unorm;
        case VK_FORMAT_R8_SNORM:	return PixelFormat::R8Snorm;
        case VK_FORMAT_R8_UINT:		return PixelFormat::R8Uint;
        case VK_FORMAT_R8_SINT:		return PixelFormat::R8Sint;
            // 16-bit formats
        case VK_FORMAT_R16_UNORM:	return PixelFormat::R16Unorm;
        case VK_FORMAT_R16_SNORM:	return PixelFormat::R16Snorm;
        case VK_FORMAT_R16_UINT:	return PixelFormat::R16Uint;
        case VK_FORMAT_R16_SINT:	return PixelFormat::R16Sint;
        case VK_FORMAT_R16_SFLOAT:	return PixelFormat::R16Float;
        case VK_FORMAT_R8G8_UNORM:	return PixelFormat::RG8Unorm;
        case VK_FORMAT_R8G8_SNORM:	return PixelFormat::RG8Snorm;
        case VK_FORMAT_R8G8_UINT:	return PixelFormat::RG8Uint;
        case VK_FORMAT_R8G8_SINT:	return PixelFormat::RG8Sint;
            // 32-bit formats
        case VK_FORMAT_R32_UINT:        return PixelFormat::R32Uint;
        case VK_FORMAT_R32_SINT:        return PixelFormat::R32Sint;
        case VK_FORMAT_R32_SFLOAT:      return PixelFormat::R32Float;
        case VK_FORMAT_R16G16_UNORM:	return PixelFormat::RG16Unorm;
        case VK_FORMAT_R16G16_SNORM:	return PixelFormat::RG16Snorm;
        case VK_FORMAT_R16G16_UINT:		return PixelFormat::RG16Uint;
        case VK_FORMAT_R16G16_SINT:		return PixelFormat::RG16Sint;
        case VK_FORMAT_R16G16_SFLOAT:	return PixelFormat::RG16Float;
        case VK_FORMAT_R8G8B8A8_UNORM:	return PixelFormat::RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SRGB:	return PixelFormat::RGBA8UnormSrgb;
        case VK_FORMAT_R8G8B8A8_SNORM:	return PixelFormat::RGBA8Snorm;
        case VK_FORMAT_R8G8B8A8_UINT:	return PixelFormat::RGBA8Uint;
        case VK_FORMAT_R8G8B8A8_SINT:	return PixelFormat::RGBA8Sint;
        case VK_FORMAT_B8G8R8A8_UNORM:	return PixelFormat::BGRA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB:   return PixelFormat::BGRA8UnormSrgb;
            // Packed 32-Bit formats
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:	return PixelFormat::RGB10A2Unorm;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     return PixelFormat::RG11B10Float;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:      return PixelFormat::RGB9E5Float;
            // 64-Bit formats
        case VK_FORMAT_R32G32_UINT:			return PixelFormat::RG32Uint;
        case VK_FORMAT_R32G32_SINT:         return PixelFormat::RG32Sint;
        case VK_FORMAT_R32G32_SFLOAT:       return PixelFormat::RG32Float;
        case VK_FORMAT_R16G16B16A16_UNORM:  return PixelFormat::RGBA16Unorm;
        case VK_FORMAT_R16G16B16A16_SNORM:  return PixelFormat::RGBA16Snorm;
        case VK_FORMAT_R16G16B16A16_UINT:   return PixelFormat::RGBA16Uint;
        case VK_FORMAT_R16G16B16A16_SINT:   return PixelFormat::RGBA16Sint;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return PixelFormat::RGBA16Float;
            // 128-Bit formats
        case VK_FORMAT_R32G32B32A32_UINT:   return PixelFormat::RGBA32Uint;
        case VK_FORMAT_R32G32B32A32_SINT:   return PixelFormat::RGBA32Sint;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return PixelFormat::RGBA32Float;
            // Depth-stencil formats
        case VK_FORMAT_D16_UNORM:			return PixelFormat::Depth16Unorm;
        case VK_FORMAT_D32_SFLOAT:			return PixelFormat::Depth32Float;
        case VK_FORMAT_D24_UNORM_S8_UINT:	return PixelFormat::Depth24UnormStencil8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:	return PixelFormat::Depth32FloatStencil8;
            // Compressed BC formats
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:	return PixelFormat::BC1RGBAUnorm;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:		return PixelFormat::BC1RGBAUnormSrgb;
        case VK_FORMAT_BC2_UNORM_BLOCK:			return PixelFormat::BC2RGBAUnorm;
        case VK_FORMAT_BC2_SRGB_BLOCK:			return PixelFormat::BC2RGBAUnormSrgb;
        case VK_FORMAT_BC3_UNORM_BLOCK:			return PixelFormat::BC3RGBAUnorm;
        case VK_FORMAT_BC3_SRGB_BLOCK:			return PixelFormat::BC3RGBAUnormSrgb;
        case VK_FORMAT_BC4_SNORM_BLOCK:			return PixelFormat::BC4RSnorm;
        case VK_FORMAT_BC4_UNORM_BLOCK:			return PixelFormat::BC4RUnorm;
        case VK_FORMAT_BC5_SNORM_BLOCK:			return PixelFormat::BC5RGSnorm;
        case VK_FORMAT_BC5_UNORM_BLOCK:			return PixelFormat::BC5RGUnorm;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:		return PixelFormat::BC6HRGBFloat;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:		return PixelFormat::BC6HRGBUFloat;
        case VK_FORMAT_BC7_UNORM_BLOCK:			return PixelFormat::BC7RGBAUnorm;
        case VK_FORMAT_BC7_SRGB_BLOCK:			return PixelFormat::BC7RGBAUnormSrgb;

        default:
            ALIMER_UNREACHABLE();
            return PixelFormat::Undefined;
        }
    }

	[[nodiscard]] constexpr VkCompareOp ToVkCompareOp(CompareFunction function)
	{
		switch (function)
		{
		case CompareFunction::Never:
			return VK_COMPARE_OP_NEVER;
		case CompareFunction::Less:
			return VK_COMPARE_OP_LESS;
		case CompareFunction::Equal:
			return VK_COMPARE_OP_EQUAL;
		case CompareFunction::LessEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareFunction::Greater:
			return VK_COMPARE_OP_GREATER;
		case CompareFunction::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case CompareFunction::GreaterEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareFunction::Always:
			return VK_COMPARE_OP_ALWAYS;

		default:
			ALIMER_UNREACHABLE();
			return VK_COMPARE_OP_MAX_ENUM;
		}
	}

    [[nodiscard]] constexpr VkSampleCountFlagBits VulkanSampleCount(SampleCount count)
    {
        switch (count)
        {
            case SampleCount::Count1:
                return VK_SAMPLE_COUNT_1_BIT;
            case SampleCount::Count2:
                return VK_SAMPLE_COUNT_2_BIT;
            case SampleCount::Count4:
                return VK_SAMPLE_COUNT_4_BIT;
            case SampleCount::Count8:
                return VK_SAMPLE_COUNT_8_BIT;
            case SampleCount::Count16:
                return VK_SAMPLE_COUNT_16_BIT;
            case SampleCount::Count32:
                return VK_SAMPLE_COUNT_32_BIT;

            default:
                ALIMER_UNREACHABLE();
                return VK_SAMPLE_COUNT_1_BIT;
        }
    }

	[[nodiscard]] constexpr VkAttachmentLoadOp ToVulkan(LoadAction action)
	{
		switch (action) {
		case LoadAction::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case LoadAction::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case LoadAction::Discard:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default:
			ALIMER_UNREACHABLE();
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}
	}

	[[nodiscard]] constexpr VkAttachmentStoreOp ToVulkan(StoreAction action)
	{
		switch (action) {
		case StoreAction::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case StoreAction::Discard:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		default:
			ALIMER_UNREACHABLE();
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
		}
	}

	[[nodiscard]] inline VkShaderStageFlags ToVulkan(ShaderStages stage)
	{
		switch (stage) {
		case ShaderStages::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStages::Pixel:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStages::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			ALIMER_UNREACHABLE();
			return 0;
		}
	}

	[[nodiscard]] inline VkShaderStageFlags ToVulkanStageFlags(ShaderStages stage)
	{
		if (stage == ShaderStages::All)
		{
			return VK_SHADER_STAGE_ALL;
		}

		VkShaderStageFlags vkStage = 0u;
		
		if (CheckBitsAny(stage, ShaderStages::Vertex))
		{
			vkStage |= VK_SHADER_STAGE_VERTEX_BIT;
		}
		if (CheckBitsAny(stage, ShaderStages::Pixel))
		{
			vkStage |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		if (CheckBitsAny(stage, ShaderStages::Compute))
		{
			vkStage |= VK_SHADER_STAGE_COMPUTE_BIT;
		}

		return vkStage;
	}

	constexpr VkImageAspectFlags GetVkImageAspectFlags(PixelFormat format, bool ignoreStencil = false)
	{
		VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT;
		if (IsDepthStencilFormat(format))
		{
			flags = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (ignoreStencil == false && IsStencilFormat(format))
			{
				flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		return flags;
	}

	[[nodiscard]] constexpr bool IsDynamicBuffer(VkDescriptorType type)
	{
		return
			type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
			type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	}

	[[nodiscard]] constexpr bool IsBuffer(VkDescriptorType type)
	{
		return
			type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
			type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
			IsDynamicBuffer(type);
	}

	enum class VulkanBufferState : uint32_t
	{
		Undefined = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Uniform = 1 << 2,
		ShaderRead = 1 << 3,
		ShaderWrite = 1 << 4,
		IndirectArgument = 1 << 5,
		CopyDest = 1 << 6,
		CopySource = 1 << 7,
		AccelerationStructureRead = 1 << 8,
		AccelerationStructureWrite = 1 << 9,
	};
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(VulkanBufferState);

	struct VulkanAttachmentDescription
	{
		PixelFormat format;
		LoadAction loadAction;
		StoreAction storeAction;
	};

	struct VulkanRenderPassKey
	{
        uint32_t colorAttachmentCount = 0;
		VulkanAttachmentDescription colorAttachments[kMaxColorAttachments] = {};
		VulkanAttachmentDescription depthStencilAttachment = {};
		SampleCount sampleCount = SampleCount::Count1;

		size_t GetHash() const
		{
			if (hash == 0)
			{
				HashCombine(hash, colorAttachmentCount, (uint32_t)sampleCount);
				HashCombine(hash, depthStencilAttachment.format, depthStencilAttachment.loadAction, depthStencilAttachment.storeAction);

				for (uint32_t i = 0; i < colorAttachmentCount; ++i)
				{
					HashCombine(hash, colorAttachments[i].format, colorAttachments[i].loadAction, colorAttachments[i].storeAction);
				}
			}

			return hash;
		}

	private:
		mutable size_t hash = 0;
	};

	struct VulkanFboKey
	{
		VkRenderPass renderPass;
		uint32_t attachmentCount = 0;
		VkImageView attachments[kMaxColorAttachments + 1] = {};
		uint32_t width;
		uint32_t height;
		uint32_t layers;
	};
}

/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			LOGE("Detected Vulkan error: {}", Alimer::ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) LOGE("Vulkan: {}, error: {}", message, Alimer::ToString(result));
