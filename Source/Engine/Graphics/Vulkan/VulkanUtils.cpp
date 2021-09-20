// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanUtils.h"
#include "VulkanGraphics.h"

namespace Alimer
{
    VulkanBuffer::VulkanBuffer(const BufferCreateInfo* info)
        : Buffer(info)
    {
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Destroy();
    }

    void VulkanBuffer::Destroy()
    {
        if (handle != VK_NULL_HANDLE
            && allocation != VK_NULL_HANDLE)
        {
            device->DeferDestroy(handle, allocation);
        }

        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        OnDestroyed();
    }

    VulkanSampler::~VulkanSampler()
    {
        device->DeferDestroy(handle, bindlessIndex);
        handle = VK_NULL_HANDLE;
    }

    VulkanShader::VulkanShader(ShaderStages stage, const std::string& entryPoint)
        : Shader(stage, entryPoint)
    {
    }

    VulkanShader::~VulkanShader()
    {
        Destroy();
    }

    void VulkanShader::Destroy()
    {
        if (handle != VK_NULL_HANDLE)
        {
            device->DeferDestroy(handle);
            handle = VK_NULL_HANDLE;
        }
    }
}

