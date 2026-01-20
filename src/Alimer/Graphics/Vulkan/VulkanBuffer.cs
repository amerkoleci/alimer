// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Runtime.CompilerServices;
using static Alimer.Graphics.Vulkan.Vma;
using static Alimer.Graphics.Vulkan.VmaMemoryUsage;
using static Alimer.Graphics.Vulkan.VmaAllocationCreateFlags;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBuffer : GraphicsBuffer
{
    private readonly VulkanGraphicsDevice _device;
    private VkBuffer _handle = VkBuffer.Null;
    private VmaAllocation _allocation = default;

    public readonly void* pMappedData;
    private readonly ulong _mappedSize;

    public VulkanBuffer(VulkanGraphicsDevice device, in BufferDescriptor description, void* initialData)
        : base(description)
    {
        _device = device;

        bool needBufferDeviceAddress = false;

        VkBufferCreateInfo createInfo = new()
        {
            flags = 0,
            size = description.Size,
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
        };

        if ((description.Usage & BufferUsage.Vertex) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if ((description.Usage & BufferUsage.Index) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if ((description.Usage & BufferUsage.Constant) != 0)
        {
            createInfo.size = MathUtilities.AlignUp(description.Size, device.VkAdapter.Properties2.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if ((description.Usage & BufferUsage.ShaderRead) != 0)
        {
            // Read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        }
        if ((description.Usage & BufferUsage.ShaderWrite) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        }
        if ((description.Usage & BufferUsage.Indirect) != 0)
        {
            needBufferDeviceAddress = true;
            createInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }

        if ((description.Usage & BufferUsage.Predication) != 0)
        {
            createInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
        }

        if ((description.Usage & BufferUsage.RayTracing) != 0)
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

        if (description.MemoryType == MemoryType.Readback)
        {
            createInfo.usage |= VkBufferUsageFlags.TransferDst;
            allocationCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        else if (description.MemoryType == MemoryType.Upload)
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

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create buffer.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
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
            GpuAddress = _device.DeviceApi.vkGetBufferDeviceAddress(device.Handle, &info);
        }

        // Issue data copy on request
        if (initialData != null)
        {
            VulkanUploadContext context = default;
            void* mappedData = null;
            if (description.MemoryType == MemoryType.Upload)
            {
                mappedData = this.pMappedData;
            }
            else
            {
                context = device.Allocate(createInfo.size);
                mappedData = context.UploadBuffer.pMappedData;
            }

            Unsafe.CopyBlockUnaligned(mappedData, initialData, (uint)description.Size);
            //std::memcpy(mappedData, initialData, desc.size);

            if (context.IsValid)
            {
                VkBufferCopy copyRegion = new()
                {
                    size = description.Size,
                    srcOffset = 0,
                    dstOffset = 0
                };

                _device.DeviceApi.vkCmdCopyBuffer(
                    context.TransferCommandBuffer,
                    context.UploadBuffer!.Handle,
                    _handle,
                    1,
                    &copyRegion
                );

                VkBufferMemoryBarrier2 barrier = new()
                {
                    buffer = _handle,
                    srcStageMask = VkPipelineStageFlags2.Transfer,
                    dstStageMask = VkPipelineStageFlags2.AllCommands,
                    srcAccessMask = VkAccessFlags2.TransferWrite,
                    dstAccessMask = VkAccessFlags2.MemoryRead | VkAccessFlags2.MemoryWrite,
                    size = VK_WHOLE_SIZE,
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
                };

                if ((description.Usage & BufferUsage.Vertex) != 0)
                {
                    barrier.dstStageMask |= VkPipelineStageFlags2.VertexAttributeInput;
                    barrier.dstAccessMask |= VkAccessFlags2.VertexAttributeRead;
                }

                if ((description.Usage & BufferUsage.Index) != 0)
                {
                    barrier.dstStageMask |= VkPipelineStageFlags2.IndexInput;
                    barrier.dstAccessMask |= VkAccessFlags2.IndexRead;
                }

                if ((description.Usage & BufferUsage.Constant) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.UniformRead;
                }

                if ((description.Usage & BufferUsage.ShaderRead) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderRead;
                }
                if ((description.Usage & BufferUsage.ShaderWrite) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderRead;
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderWrite;
                }
                if ((description.Usage & BufferUsage.Indirect) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.IndirectCommandRead;
                }
                if ((description.Usage & BufferUsage.RayTracing) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.AccelerationStructureReadKHR;
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

                _device.DeviceApi.vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);

                device.Submit(in context);
            }
        }
    }

    public VulkanBuffer(VulkanGraphicsDevice device, VkBuffer existingHandle, in BufferDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        _handle = existingHandle;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override GPUAddress GpuAddress { get; }

    public VkBuffer Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBuffer" /> class.
    /// </summary>
    ~VulkanBuffer() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.Buffer, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (_allocation.IsNotNull)
        {
            vmaDestroyBuffer(_device.Allocator, _handle, _allocation);
            _allocation = VmaAllocation.Null;
        }
        else
        {
            _device.DeviceApi.vkDestroyBuffer(_device.Handle, _handle, null);
        }

        _handle = VkBuffer.Null;
    }

    /// <inheitdoc />
    protected override void SetDataUnsafe(void* dataPtr, int offsetInBytes)
    {
        Unsafe.CopyBlockUnaligned((byte*)pMappedData + offsetInBytes, dataPtr, (uint)Size);
    }

    /// <inheitdoc />
    protected override void GetDataUnsafe(void* destPtr, int offsetInBytes)
    {
        Unsafe.CopyBlockUnaligned(destPtr, (byte*)pMappedData + offsetInBytes, (uint)Size);
    }
}
