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

    class VulkanDevice final : public IDevice
    {
    public:
        VulkanDevice(ValidationMode validationMode);
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
