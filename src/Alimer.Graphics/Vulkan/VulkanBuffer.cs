// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.Vma;
using Alimer.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBuffer : GraphicsBuffer
{
    private readonly VkBuffer _handle = VkBuffer.Null;
    private readonly VmaAllocation _allocation = VmaAllocation.Null;
    private readonly void* pMappedData;
    private ulong _gpuAddress;

    public VulkanBuffer(VulkanGraphicsDevice device, in BufferDescriptor descriptor, void* initialData)
        : base(device, descriptor)
    {
        VkBufferCreateInfo createInfo = new()
        {
            flags = 0,
            size = descriptor.Size,
            usage = VkBufferUsageFlags.None
        };

        bool needShaderDeviceAddress = false;
        if ((descriptor.Usage & BufferUsage.Vertex) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.VertexBuffer;
            needShaderDeviceAddress = true;
        }

        if ((descriptor.Usage & BufferUsage.Index) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.IndexBuffer;
            needShaderDeviceAddress = true;
        }

        if ((descriptor.Usage & BufferUsage.Constant) != 0)
        {
            createInfo.size = MathHelper.AlignUp(descriptor.Size, device.PhysicalDeviceProperties.properties.limits.minUniformBufferOffsetAlignment);
            createInfo.usage |= VkBufferUsageFlags.UniformBuffer;
        }

        if ((descriptor.Usage & BufferUsage.ShaderRead) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.StorageBuffer; // read only ByteAddressBuffer is also storage buffer
            createInfo.usage |= VkBufferUsageFlags.UniformTexelBuffer;
        }
        if ((descriptor.Usage & BufferUsage.ShaderWrite) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.StorageBuffer;
            createInfo.usage |= VkBufferUsageFlags.StorageTexelBuffer;
        }
        if ((descriptor.Usage & BufferUsage.Indirect) != 0)
        {
            createInfo.usage |= VkBufferUsageFlags.IndirectBuffer;
        }

        if ((descriptor.Usage & BufferUsage.Predication) != 0 &&
            device.QueryFeature(Feature.ConditionalRendering))
        {
            createInfo.usage |= VkBufferUsageFlags.ConditionalRenderingEXT;
        }

        if ((descriptor.Usage & BufferUsage.RayTracing) != 0 &&
            device.QueryFeature(Feature.RayTracing))
        {
            needShaderDeviceAddress = true;
            createInfo.usage |= VkBufferUsageFlags.AccelerationStructureStorageKHR;
            createInfo.usage |= VkBufferUsageFlags.AccelerationStructureBuildInputReadOnlyKHR;
            createInfo.usage |= VkBufferUsageFlags.ShaderBindingTableKHR;
        }

        // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT require bufferDeviceAddress enabled.
        if (device.PhysicalDeviceFeatures1_2.bufferDeviceAddress == true &&
            needShaderDeviceAddress)
        {
            createInfo.usage |= VkBufferUsageFlags.ShaderDeviceAddress;
        }

        uint* sharingIndices = stackalloc uint[3];
        device.FillBufferSharingIndices(ref createInfo, sharingIndices);

        // TODO: Add sparse buffer support

        VmaAllocationCreateInfo memoryInfo = new()
        {
            usage = VmaMemoryUsage.Auto
        };
        if (descriptor.CpuAccess == CpuAccessMode.Read)
        {
            createInfo.usage |= VkBufferUsageFlags.TransferDst;
            memoryInfo.flags = VmaAllocationCreateFlags.HostAccessRandom | VmaAllocationCreateFlags.Mapped;
        }
        else if (descriptor.CpuAccess == CpuAccessMode.Write)
        {
            createInfo.usage |= VkBufferUsageFlags.TransferSrc;
            memoryInfo.flags = VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.Mapped;
        }
        else
        {
            createInfo.usage |= VkBufferUsageFlags.TransferDst;
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

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        if ((memoryInfo.flags & VmaAllocationCreateFlags.Mapped) != 0)
        {
            pMappedData = allocationInfo.pMappedData;
            //pMappedData = allocation->GetMappedData();
        }

        if ((createInfo.usage & VkBufferUsageFlags.ShaderDeviceAddress) != 0)
        {
            VkBufferDeviceAddressInfo info = new()
            {
                sType = VkStructureType.BufferDeviceAddressInfo,
                buffer = _handle
            };
            _gpuAddress = vkGetBufferDeviceAddress(device.Handle, &info);
        }

        // Issue data copy on request
        if (initialData != null)
        {
            VulkanUploadContext context = default;
            void* pMappedData = null;
            if (descriptor.CpuAccess == CpuAccessMode.Write)
            {
                pMappedData = this.pMappedData;
            }
            else
            {
                context = device.Allocate(createInfo.size);
                pMappedData = context.UploadBuffer.pMappedData;
            }

            Unsafe.CopyBlockUnaligned(pMappedData, initialData, (uint)descriptor.Size);
            //std::memcpy(pMappedData, initialData, desc.size);

            if (context.IsValid)
            {
                VkBufferCopy copyRegion = new();
                copyRegion.size = descriptor.Size;
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;

                vkCmdCopyBuffer(
                    context.TransferCommandBuffer,
                    context.UploadBuffer.Handle,
                    _handle,
                    1,
                    &copyRegion
                );

                VkBufferMemoryBarrier2 barrier = new()
                {
                    sType = VkStructureType.BufferMemoryBarrier2,
                    buffer = _handle,
                    srcStageMask = VkPipelineStageFlags2.Transfer,
                    srcAccessMask = VkAccessFlags2.TransferWrite,
                    dstStageMask = VkPipelineStageFlags2.AllCommands,
                    dstAccessMask = VkAccessFlags2.MemoryRead | VkAccessFlags2.MemoryWrite,
                    size = VK_WHOLE_SIZE,
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
                };

                if ((descriptor.Usage & BufferUsage.Vertex) != 0)
                {
                    barrier.dstStageMask |= VkPipelineStageFlags2.VertexAttributeInput;
                    barrier.dstAccessMask |= VkAccessFlags2.VertexAttributeRead;
                }

                if ((descriptor.Usage & BufferUsage.Index) != 0)
                {
                    barrier.dstStageMask |= VkPipelineStageFlags2.IndexInput;
                    barrier.dstAccessMask |= VkAccessFlags2.IndexRead;
                }

                if ((descriptor.Usage & BufferUsage.Constant) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.UniformRead;
                }

                if ((descriptor.Usage & BufferUsage.ShaderRead) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderRead;
                }
                if ((descriptor.Usage & BufferUsage.ShaderWrite) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderRead;
                    barrier.dstAccessMask |= VkAccessFlags2.ShaderWrite;
                }
                if ((descriptor.Usage & BufferUsage.Indirect) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.IndirectCommandRead;
                }
                if ((descriptor.Usage & BufferUsage.RayTracing) != 0)
                {
                    barrier.dstAccessMask |= VkAccessFlags2.AccelerationStructureReadKHR;
                }
                //if ((descriptor.Usage & BufferUsage.VideoDecode) != 0)
                //{
                //    barrier.dstAccessMask |= VkAccessFlags2.VideoDecodeReadKHR;
                //}

                VkDependencyInfo dependencyInfo = new()
                {
                    sType = VkStructureType.DependencyInfo,
                    bufferMemoryBarrierCount = 1,
                    pBufferMemoryBarriers = &barrier
                };

                vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);

                device.Submit(in context);
            }
        }
    }

    public VulkanBuffer(VulkanGraphicsDevice device, VkBuffer existingHandle, in BufferDescriptor descriptor)
    : base(device, descriptor)
    {
        _handle = existingHandle;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public VkDevice VkDevice => ((VulkanGraphicsDevice)Device).Handle;

    public VkBuffer Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBuffer" /> class.
    /// </summary>
    ~VulkanBuffer() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        ((VulkanGraphicsDevice)Device).SetObjectName(VkObjectType.Buffer, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        VmaAllocator memoryAllocator = ((VulkanGraphicsDevice)Device).MemoryAllocator;

        if (!_allocation.IsNull)
        {
            vmaDestroyBuffer(memoryAllocator, _handle, _allocation);
        }
    }
}
