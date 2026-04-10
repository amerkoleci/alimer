// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Vulkan.Vma;
using static Alimer.Graphics.Vulkan.VmaAllocationCreateFlags;
using static Alimer.Graphics.Vulkan.VmaMemoryUsage;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBuffer : GPUBuffer
{
    private VkBuffer _handle = VkBuffer.Null;
    private VmaAllocation _allocation = default;

    public readonly void* pMappedData;
    private readonly ulong _mappedSize;

    public VulkanBuffer(VulkanGraphicsDevice device, in GPUBufferDescriptor descriptor, void* initialData)
        : base(descriptor)
    {
        VkDevice = device;

        bool needBufferDeviceAddress = false;

        VkBufferCreateInfo createInfo = new()
        {
            flags = 0,
            size = descriptor.Size,
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
        };

        if ((descriptor.Usage & GPUBufferUsage.Vertex) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.Index) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.Constant) != 0)
        {
            createInfo.size = MathUtilities.AlignUp(descriptor.Size, device.VkAdapter.Properties2.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.ShaderRead) != 0)
        {
            // Read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.ShaderWrite) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.Indirect) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }

        if ((descriptor.Usage & GPUBufferUsage.Predication) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
        }

        if ((descriptor.Usage & GPUBufferUsage.RayTracing) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        }

        // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
        if (needBufferDeviceAddress
            && device.VkAdapter.Features12.bufferDeviceAddress)
        {
            createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        uint* sharingIndices = stackalloc uint[(int)CommandQueueType.Count];
        device.FillBufferSharingIndices(ref createInfo, sharingIndices);

        // TODO: Add sparse buffer support
        VmaAllocationCreateFlags allocationCreateFlags = 0;

        if (descriptor.MemoryType == MemoryType.Readback)
        {
            createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocationCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        else if (descriptor.MemoryType == MemoryType.Upload)
        {
            createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocationCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VmaAllocationCreateInfo memoryInfo = new()
        {
            flags = allocationCreateFlags,
            usage = VMA_MEMORY_USAGE_AUTO,
        };
        VmaAllocationInfo allocationInfo;
        VkResult result = vmaCreateBuffer(device.Allocator, &createInfo, &memoryInfo, out _handle, out _allocation, &allocationInfo);

        if (result != VK_SUCCESS)
        {
            Log.Error("Vulkan: Failed to create buffer.");
            return;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        if ((memoryInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0)
        {
            pMappedData = allocationInfo.pMappedData;
            _mappedSize = allocationInfo.size;
        }

        if ((createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0)
        {
            VkBufferDeviceAddressInfo info = new()
            {
                buffer = _handle
            };
            GpuAddress = VkDevice.DeviceApi.vkGetBufferDeviceAddress(&info);
        }

        // Issue data copy on request
        if (initialData != null)
        {
            VulkanUploadContext context = default;
            void* mappedData;
            if (descriptor.MemoryType == MemoryType.Upload)
            {
                mappedData = this.pMappedData;
            }
            else
            {
                context = device.Allocate(createInfo.size);
                mappedData = context.UploadBuffer.pMappedData;
            }

            Unsafe.CopyBlockUnaligned(mappedData, initialData, (uint)descriptor.Size);
            //std::memcpy(mappedData, initialData, desc.size);

            if (context.IsValid)
            {
                VkBufferCopy copyRegion = new()
                {
                    size = descriptor.Size,
                    srcOffset = 0,
                    dstOffset = 0
                };

                VkDevice.DeviceApi.vkCmdCopyBuffer(
                    context.TransferCommandBuffer,
                    context.UploadBuffer!.Handle,
                    _handle,
                    1,
                    &copyRegion
                );

                VkBufferMemoryBarrier2 barrier = new()
                {
                    buffer = _handle,
                    srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                    srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                    size = VK_WHOLE_SIZE,
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
                };

                if ((descriptor.Usage & GPUBufferUsage.Vertex) != 0)
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                }

                if ((descriptor.Usage & GPUBufferUsage.Index) != 0)
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
                }

                if ((descriptor.Usage & GPUBufferUsage.Constant) != 0)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_UNIFORM_READ_BIT;
                }

                if ((descriptor.Usage & GPUBufferUsage.ShaderRead) != 0)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                }

                if ((descriptor.Usage & GPUBufferUsage.ShaderReadWrite) != 0)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_WRITE_BIT;
                }

                if ((descriptor.Usage & GPUBufferUsage.Indirect) != 0)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                }
                if ((descriptor.Usage & GPUBufferUsage.RayTracing) != 0)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                }
                //if ((description.Usage & BufferUsage.VideoDecode) != 0)
                //{
                //    barrier.dstAccessMask |= VkAccessFlags2.VideoDecodeReadKHR;
                //}

                VkDependencyInfo dependencyInfo = new()
                {
                    bufferMemoryBarrierCount = 1,
                    pBufferMemoryBarriers = &barrier
                };

                VkDevice.DeviceApi.vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);

                device.Submit(ref context);
            }
        }
    }

    public VulkanBuffer(VulkanGraphicsDevice device, VkBuffer existingHandle, in GPUBufferDescriptor descriptor)
        : base(descriptor)
    {
        VkDevice = device;
        _handle = existingHandle;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public VulkanGraphicsDevice VkDevice { get; }

    /// <inheritdoc />
    public override GPUDevice Device => VkDevice;

    /// <inheritdoc />
    public override GPUAddress GpuAddress { get; }

    public VkBuffer Handle => _handle;

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        VkDevice.SetObjectName(VK_OBJECT_TYPE_BUFFER, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (_allocation.IsNotNull)
        {
            vmaDestroyBuffer(VkDevice.Allocator, _handle, _allocation);
            _allocation = VmaAllocation.Null;
        }
        else
        {
            VkDevice.DeviceApi.vkDestroyBuffer(_handle, null);
        }

        _handle = VkBuffer.Null;
    }

    protected override GPUBufferView CreateViewCore(in GPUBufferViewDescriptor descriptor) => new VulkanBufferView(this, descriptor);

    internal override void* GetMappedData() => pMappedData;

    /// <inheitdoc />
    protected override void SetDataUnsafe(void* sourcePtr, uint offsetInBytes, uint sizeInBytes)
    {
        NativeMemory.Copy(sourcePtr, (byte*)pMappedData + offsetInBytes, sizeInBytes);
    }

    /// <inheitdoc />
    protected override void GetDataUnsafe(void* destinationPtr, uint offsetInBytes, uint sizeInBytes)
    {
        NativeMemory.Copy((byte*)pMappedData + offsetInBytes, destinationPtr, sizeInBytes);
    }
}
