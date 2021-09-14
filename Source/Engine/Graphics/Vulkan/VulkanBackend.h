// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

namespace Alimer
{
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

    [[nodiscard]] constexpr VkFormat ToVulkan(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
        case PixelFormat::R8Unorm:        return VK_FORMAT_R8_UNORM;
        case PixelFormat::R8Snorm:        return VK_FORMAT_R8_SNORM;
        case PixelFormat::R8Uint:         return VK_FORMAT_R8_UINT;
        case PixelFormat::R8Sint:         return VK_FORMAT_R8_SINT;
            // 16-bit formats
        case PixelFormat::R16Unorm:       return VK_FORMAT_R16_UNORM;
        case PixelFormat::R16Snorm:       return VK_FORMAT_R16_SNORM;
        case PixelFormat::R16Uint:        return VK_FORMAT_R16_UINT;
        case PixelFormat::R16Sint:        return VK_FORMAT_R16_SINT;
        case PixelFormat::R16Float:       return VK_FORMAT_R16_SFLOAT;
        case PixelFormat::RG8Unorm:       return VK_FORMAT_R8G8_UNORM;
        case PixelFormat::RG8Snorm:       return VK_FORMAT_R8G8_SNORM;
        case PixelFormat::RG8Uint:        return VK_FORMAT_R8G8_UINT;
        case PixelFormat::RG8Sint:        return VK_FORMAT_R8G8_SINT;
            // Packed 16-Bit Pixel Formats
        case PixelFormat::BGRA4Unorm:     return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case PixelFormat::B5G6R5Unorm:    return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case PixelFormat::B5G5R5A1Unorm:  return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            // 32-bit formats
        case PixelFormat::R32Uint:        return VK_FORMAT_R32_UINT;
        case PixelFormat::R32Sint:        return VK_FORMAT_R32_SINT;
        case PixelFormat::R32Float:       return VK_FORMAT_R32_SFLOAT;
        case PixelFormat::RG16Unorm:      return VK_FORMAT_R16G16_UNORM;
        case PixelFormat::RG16Snorm:      return VK_FORMAT_R16G16_SNORM;
        case PixelFormat::RG16Uint:       return VK_FORMAT_R16G16_UINT;
        case PixelFormat::RG16Sint:       return VK_FORMAT_R16G16_SINT;
        case PixelFormat::RG16Float:      return VK_FORMAT_R16G16_SFLOAT;
        case PixelFormat::RGBA8Unorm:     return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::RGBA8UnormSrgb: return VK_FORMAT_R8G8B8A8_SRGB;
        case PixelFormat::RGBA8Snorm:     return VK_FORMAT_R8G8B8A8_SNORM;
        case PixelFormat::RGBA8Uint:      return VK_FORMAT_R8G8B8A8_UINT;
        case PixelFormat::RGBA8Sint:      return VK_FORMAT_R8G8B8A8_SINT;
        case PixelFormat::BGRA8Unorm:     return VK_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::BGRA8UnormSrgb: return VK_FORMAT_B8G8R8A8_SRGB;
            // Packed 32-Bit formats
        case PixelFormat::RGB10A2Unorm:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case PixelFormat::RG11B10Float:   return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PixelFormat::RGB9E5Float:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            // 64-Bit formats
        case PixelFormat::RG32Uint:       return VK_FORMAT_R32G32_UINT;
        case PixelFormat::RG32Sint:       return VK_FORMAT_R32G32_SINT;
        case PixelFormat::RG32Float:      return VK_FORMAT_R32G32_SFLOAT;
        case PixelFormat::RGBA16Unorm:    return VK_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat::RGBA16Snorm:    return VK_FORMAT_R16G16B16A16_SNORM;
        case PixelFormat::RGBA16Uint:     return VK_FORMAT_R16G16B16A16_UINT;
        case PixelFormat::RGBA16Sint:     return VK_FORMAT_R16G16B16A16_SINT;
        case PixelFormat::RGBA16Float:    return VK_FORMAT_R16G16B16A16_SFLOAT;
            // 128-Bit formats
        case PixelFormat::RGBA32Uint:     return VK_FORMAT_R32G32B32A32_UINT;
        case PixelFormat::RGBA32Sint:     return VK_FORMAT_R32G32B32A32_SINT;
        case PixelFormat::RGBA32Float:    return VK_FORMAT_R32G32B32A32_SFLOAT;
            // Depth-stencil formats
        case PixelFormat::Depth16Unorm:   return VK_FORMAT_D16_UNORM;
        case PixelFormat::Depth32Float:   return VK_FORMAT_D32_SFLOAT;
        case PixelFormat::Depth24UnormStencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat::Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            // Compressed BC formats
        case PixelFormat::BC1RGBAUnorm:       return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PixelFormat::BC1RGBAUnormSrgb:   return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case PixelFormat::BC2RGBAUnorm:       return VK_FORMAT_BC2_UNORM_BLOCK;
        case PixelFormat::BC2RGBAUnormSrgb:   return VK_FORMAT_BC2_SRGB_BLOCK;
        case PixelFormat::BC3RGBAUnorm:       return VK_FORMAT_BC3_UNORM_BLOCK;
        case PixelFormat::BC3RGBAUnormSrgb:   return VK_FORMAT_BC3_SRGB_BLOCK;
        case PixelFormat::BC4RUnorm:          return VK_FORMAT_BC4_UNORM_BLOCK;
        case PixelFormat::BC4RSnorm:          return VK_FORMAT_BC4_SNORM_BLOCK;
        case PixelFormat::BC5RGUnorm:         return VK_FORMAT_BC5_UNORM_BLOCK;
        case PixelFormat::BC5RGSnorm:         return VK_FORMAT_BC5_SNORM_BLOCK;
        case PixelFormat::BC6HRGBFloat:       return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case PixelFormat::BC6HRGBUFloat:      return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PixelFormat::BC7RGBAUnorm:       return VK_FORMAT_BC7_UNORM_BLOCK;
        case PixelFormat::BC7RGBAUnormSrgb:   return VK_FORMAT_BC7_SRGB_BLOCK;
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
        case PixelFormat::Undefined:
            return VK_FORMAT_UNDEFINED;
        }
    }
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
