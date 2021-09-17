// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphics.h"

namespace Alimer
{
    VulkanBuffer::VulkanBuffer(VulkanGraphics& device_, const BufferCreateInfo* info, const void* initialData)
        : Buffer(info)
        , device(device_)
    {
        if (info->handle)
        {
            handle = (VkBuffer)info->handle;
            return;
        }

        VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        createInfo.size = size;
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

        if (device.features_1_2.bufferDeviceAddress == VK_TRUE)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.flags = 0;
        memoryInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;

        switch (info->heapType)
        {
        case HeapType::Upload:
            memoryInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            memoryInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            memoryInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            memoryInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        case HeapType::Readback:
            memoryInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            memoryInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
            createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;

        case HeapType::Default:
        default:
            memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            memoryInfo.requiredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        }

        uint32_t sharingIndices[3];
        if (device.GetGraphicsQueueFamily() != device.GetComputeQueueFamily()
            || device.GetGraphicsQueueFamily() != device.GetCopyQueueFamily())
        {
            // For buffers, always just use CONCURRENT access modes,
            // so we don't have to deal with acquire/release barriers in async compute.
            createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

            sharingIndices[createInfo.queueFamilyIndexCount++] = device.GetGraphicsQueueFamily();

            if (device.GetGraphicsQueueFamily() != device.GetComputeQueueFamily())
            {
                sharingIndices[createInfo.queueFamilyIndexCount++] = device.GetComputeQueueFamily();
            }

            if (device.GetGraphicsQueueFamily() != device.GetCopyQueueFamily()
                && device.GetComputeQueueFamily() != device.GetCopyQueueFamily())
            {
                sharingIndices[createInfo.queueFamilyIndexCount++] = device.GetCopyQueueFamily();
            }

            createInfo.pQueueFamilyIndices = sharingIndices;
        }

        VmaAllocationInfo allocationInfo{};
        VkResult result = vmaCreateBuffer(device.GetAllocator(),
            &createInfo, &memoryInfo,
            &handle, &allocation, &allocationInfo);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create buffer.");
            return;
        }

        if (info->label != nullptr)
        {
            SetName(info->label);
        }

        allocatedSize = allocationInfo.size;

        if (createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            info.buffer = handle;
            deviceAddress = vkGetBufferDeviceAddress(device.GetHandle(), &info);
        }

        if (memoryInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
        {
            mappedData = static_cast<uint8_t*>(allocationInfo.pMappedData);
        }

        if (initialData != nullptr)
        {
            VulkanUploadContext context = device.UploadBegin(info->size);
            memcpy(context.data, initialData, info->size);

            VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = handle;
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
                handle,
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

            device.UploadEnd(context);
        }
        else if (info->heapType == HeapType::Default)
        {
            // zero-initialize:
            //initLocker.lock();
            //vkCmdFillBuffer(
            //    GetFrameResources().initCommandBuffer,
            //    internal_state->resource,
            //    0,
            //    VK_WHOLE_SIZE,
            //    0
            //);
            //submit_inits = true;
            //initLocker.unlock();
        }

        OnCreated();
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
            device.DeferDestroy(handle, allocation);
        }

        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        OnDestroyed();
    }

    void VulkanBuffer::ApiSetName()
    {
        device.SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)handle, name);
    }
}
