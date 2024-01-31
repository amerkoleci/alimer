// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanAllocator : IDisposable
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPhysicalDeviceMemoryProperties _memoryProperties;

    public VulkanAllocator(VulkanGraphicsDevice device)
    {
        _device = device;
        vkGetPhysicalDeviceMemoryProperties(device.PhysicalDevice, out _memoryProperties);
    }

    public void Dispose()
    {
    }

    public bool TryFindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties, out uint typeIndex)
    {
        typeIndex = 0;

        for (int i = 0; i < _memoryProperties.memoryTypeCount; i++)
        {
            if (((typeFilter & (1 << i)) != 0)
                && (_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                typeIndex = (uint)i;
                return true;
            }
        }

        return false;
    }


    public VkResult AllocateMemory(
        out VkDeviceMemory deviceMemory,
        VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
        VkImage dedicatedImage, VkBuffer dedicatedBuffer,
        bool enableDeviceAddress = false, bool enableExportMemory = false)
    {
        // Find a memory space that satisfies the requirements
        if (!TryFindMemoryType(memRequirements.memoryTypeBits, properties, out uint memoryTypeIndex))
        {
            // This is incorrect; need better error reporting
            deviceMemory = VkDeviceMemory.Null;
            return VkResult.ErrorOutOfDeviceMemory;
        }

        // allocate memory
        VkMemoryAllocateFlagsInfo allocFlags = new();
        if (enableDeviceAddress)
            allocFlags.flags |= VkMemoryAllocateFlags.DeviceAddress;

        void* pNext = &allocFlags;

        // Dedicated memory
        VkMemoryDedicatedAllocateInfo dedicatedAllocation = new()
        {
            pNext = pNext,
            image = dedicatedImage,
            buffer = dedicatedBuffer
        };

        if (dedicatedImage.IsNotNull || dedicatedBuffer.IsNotNull)
        {
            // Append the VkMemoryDedicatedAllocateInfo structure to the chain
            pNext = &dedicatedAllocation;
        }

        VkExternalMemoryHandleTypeFlags handleType;
        if (OperatingSystem.IsWindows())
        {
            handleType = VkExternalMemoryHandleTypeFlags.OpaqueWin32;
        }
        else
        {
            handleType = VkExternalMemoryHandleTypeFlags.OpaqueFD;
        }

        VkExportMemoryAllocateInfo exportInfo = new()
        {
            pNext = pNext,
            handleTypes = handleType
        };

        if (enableExportMemory)
        {
            // Append the VkExportMemoryAllocateInfo structure to the chain
            pNext = &exportInfo;
        }

        VkMemoryAllocateInfo allocInfo = new();
        allocInfo.pNext = pNext;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        return vkAllocateMemory(_device.Handle, &allocInfo, null, out deviceMemory);
    }

    public VkDeviceMemory AllocateTextureMemory(VkImage image, bool shared, out ulong size)
    {
        // grab the image memory requirements
        vkGetImageMemoryRequirements(_device.Handle, image, out VkMemoryRequirements memRequirements);

        // allocate memory
        VkMemoryPropertyFlags memProperties = VkMemoryPropertyFlags.DeviceLocal;
        bool enableDeviceAddress = false;
        bool enableMemoryExport = shared;
        VkResult res = AllocateMemory(out VkDeviceMemory deviceMemory, memRequirements, memProperties, image, VkBuffer.Null, enableDeviceAddress, enableMemoryExport);
        res.CheckResult();

        vkBindImageMemory(_device.Handle, image, deviceMemory, 0);
        size = memRequirements.size;
        return deviceMemory;
    }

    public VkDeviceMemory AllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags memProperties, out ulong size, bool shared, bool enableDeviceAddress = false)
    {
        // grab the image memory requirements
        vkGetBufferMemoryRequirements(_device.Handle, buffer, out VkMemoryRequirements memRequirements);

        // allocate memory
        bool enableMemoryExport = shared;
        VkResult res = AllocateMemory(out VkDeviceMemory deviceMemory, memRequirements, memProperties, VkImage.Null, buffer, enableDeviceAddress, enableMemoryExport);
        res.CheckResult();

        vkBindBufferMemory(_device.Handle, buffer, deviceMemory, 0);
        size = memRequirements.size;
        return deviceMemory;
    }

    public void FreeMemory(VkDeviceMemory deviceMemory)
    {
        vkFreeMemory(_device.Handle, deviceMemory, null);
    }

    public void FreeTextureMemory(VkDeviceMemory deviceMemory)
    {
        FreeMemory(deviceMemory);
    }

    public void FreeBufferMemory(VkDeviceMemory deviceMemory)
    {
        FreeMemory(deviceMemory);
    }
}
