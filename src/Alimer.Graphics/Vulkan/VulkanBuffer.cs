// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.Vma;
using static Alimer.Numerics.MathUtilities;
using System.Runtime.CompilerServices;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBuffer : GraphicsBuffer
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkBuffer _handle = VkBuffer.Null;
    private readonly VmaAllocation _allocation = VmaAllocation.Null;
    public readonly void* pMappedData;
    private readonly ulong _mappedSize;
    private ulong _gpuAddress;

    public VulkanBuffer(VulkanGraphicsDevice device, in BufferDescription description, void* initialData)
        : base(description)
    {
        _device = device;

        VkBufferCreateInfo createInfo = new()
        {
            flags = 0,
            size = description.Size,
            usage = VkBufferUsageFlags.None
        };

        if ((description.Usage & BufferUsage.Vertex) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.VertexBuffer;
        }

        if ((description.Usage & BufferUsage.Index) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.IndexBuffer;
        }

        if ((description.Usage & BufferUsage.Constant) != 0)
        {
            createInfo.size = AlignUp(description.Size, device.PhysicalDeviceProperties.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VkBufferUsageFlags.UniformBuffer;
        }

        if ((description.Usage & BufferUsage.ShaderRead) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.StorageBuffer; // read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VkBufferUsageFlags.UniformTexelBuffer;
        }
        if ((description.Usage & BufferUsage.ShaderWrite) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.StorageBuffer;
            createInfo.usage |= VkBufferUsageFlags.StorageTexelBuffer;
        }
        if ((description.Usage & BufferUsage.Indirect) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.IndirectBuffer;
        }

        if ((description.Usage & BufferUsage.Predication) != 0 &&
            device.QueryFeatureSupport(Feature.Predication))
        {
            createInfo.usage |= VkBufferUsageFlags.ConditionalRenderingEXT;
        }

        if ((description.Usage & BufferUsage.RayTracing) != 0 &&
            device.QueryFeatureSupport(Feature.RayTracing))
        {
            createInfo.usage |= VkBufferUsageFlags.AccelerationStructureStorageKHR;
            createInfo.usage |= VkBufferUsageFlags.AccelerationStructureBuildInputReadOnlyKHR;
            createInfo.usage |= VkBufferUsageFlags.ShaderBindingTableKHR;
        }

        // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
        if (device.PhysicalDeviceFeatures1_2.bufferDeviceAddress == true)
        {
            createInfo.usage |= VkBufferUsageFlags.ShaderDeviceAddress;
        }

        uint* sharingIndices = stackalloc uint[(int)QueueType.Count];
        device.FillBufferSharingIndices(ref createInfo, sharingIndices);

        // TODO: Add sparse buffer support

        VmaAllocationCreateInfo memoryInfo = new()
        {
            usage = VmaMemoryUsage.Auto
        };
        if (description.CpuAccess == CpuAccessMode.Read)
        {
            createInfo.usage |= VkBufferUsageFlags.TransferDst;
            memoryInfo.flags = VmaAllocationCreateFlags.HostAccessRandom | VmaAllocationCreateFlags.Mapped;
        }
        else if (description.CpuAccess == CpuAccessMode.Write)
        {
            createInfo.usage |= VkBufferUsageFlags.TransferSrc;
            memoryInfo.flags = VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.Mapped;
        }
        else
        {
            createInfo.usage |= VkBufferUsageFlags.TransferSrc | VkBufferUsageFlags.TransferDst;
        }

        VmaAllocationInfo allocationInfo = default;
        VkResult result = vmaCreateBuffer(device.MemoryAllocator,
            &createInfo,
            &memoryInfo,
            out _handle,
            out _allocation,
            &allocationInfo);

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create buffer.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        if ((memoryInfo.flags & VmaAllocationCreateFlags.Mapped) != 0)
        {
            pMappedData = allocationInfo.pMappedData;
            _mappedSize = allocationInfo.size;
        }

        if ((createInfo.usage & VkBufferUsageFlags.ShaderDeviceAddress) != 0)
        {
            VkBufferDeviceAddressInfo info = new()
            {
                buffer = _handle
            };
            _gpuAddress = vkGetBufferDeviceAddress(device.Handle, &info);
        }

        // Issue data copy on request
        if (initialData != null)
        {
            VulkanUploadContext context = default;
            void* mappedData = null;
            if (description.CpuAccess == CpuAccessMode.Write)
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

                vkCmdCopyBuffer(
                    context.TransferCommandBuffer,
                    context.UploadBuffer!.Handle,
                    _handle,
                    1,
                    &copyRegion
                );

                if (device.PhysicalDeviceFeatures1_3.synchronization2)
                {
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

                    vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);
                }
                else
                {
                    VkBufferMemoryBarrier barrier = new()
                    {
                        srcAccessMask = VkAccessFlags.TransferWrite,
                        dstAccessMask = VkAccessFlags.MemoryRead | VkAccessFlags.MemoryWrite,
                        srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        buffer = _handle,
                        offset = 0,
                        size = VK_WHOLE_SIZE
                    };

                    vkCmdPipelineBarrier(context.TransitionCommandBuffer,
                        VkPipelineStageFlags.Transfer,
                        VkPipelineStageFlags.AllCommands,
                        0,
                        0, null,
                        1, &barrier,
                        0, null
                    );
                }

                device.Submit(in context);
            }
        }
    }

    public VulkanBuffer(VulkanGraphicsDevice device, VkBuffer existingHandle, in BufferDescription descriptor)
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

    public VkBuffer Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBuffer" /> class.
    /// </summary>
    ~VulkanBuffer() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.Buffer, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        VmaAllocator memoryAllocator = _device.MemoryAllocator;

        if (!_allocation.IsNull)
        {
            vmaDestroyBuffer(memoryAllocator, _handle, _allocation);
        }
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
