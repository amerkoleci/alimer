// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"

#if defined(ALIMER_RHI_VULKAN)
#include "RHI_Vulkan.h"
#endif

namespace RHI
{
    DeviceHandle CreateDevice(GraphicsAPI api, ValidationMode validationMode)
    {
#if defined(ALIMER_RHI_VULKAN)
        return DeviceHandle::Create(new VulkanDevice(validationMode));
#endif
    }
}
