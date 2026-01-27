// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroup : BindGroup
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VulkanBindGroupLayout _layout;
    private readonly VkDescriptorSet _handle = VkDescriptorSet.Null;

    public VulkanBindGroup(VulkanGraphicsDevice device, BindGroupLayout layout, in BindGroupDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        _layout = (VulkanBindGroupLayout)layout;

        // Allocate DescriptorSet from the bind group pool
        uint maxVariableArrayLength = 0;
        VkDescriptorSet descriptorSet = VkDescriptorSet.Null;
        VkResult result = _device.AllocateDescriptorSet(_layout.Handle, &descriptorSet, maxVariableArrayLength);

        // If we have run out of pool memory and rely on internalPools, create a new internal pool and retry
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY
            || result == VK_ERROR_FRAGMENTED_POOL)
        {
            device.AllocateDescriptorPool();
            result = _device.AllocateDescriptorSet(_layout.Handle, &descriptorSet, maxVariableArrayLength);
            result.CheckResult();
        }

        _handle = descriptorSet;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        Update(descriptor.Entries);
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBindGroup" /> class.
    /// </summary>
    ~VulkanBindGroup() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override BindGroupLayout Layout => _layout;

    public VkDescriptorSet Handle => _handle;

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }

    /// <inheitdoc />
    public override void Update(ReadOnlySpan<BindGroupEntry> entries)
    {
        // TODO: Handle null descriptors to avoid vulkan warnings
        int descriptorWriteCount = 0;
        VkWriteDescriptorSet* descriptorWrites = stackalloc VkWriteDescriptorSet[_layout.LayoutBindingCount];
        VkDescriptorBufferInfo* bufferInfos = stackalloc VkDescriptorBufferInfo[_layout.LayoutBindingCount];
        VkDescriptorImageInfo* imageInfos = stackalloc VkDescriptorImageInfo[_layout.LayoutBindingCount];
        VkWriteDescriptorSetAccelerationStructureKHR* accelStructInfos = stackalloc VkWriteDescriptorSetAccelerationStructureKHR[descriptorWriteCount];

        for (uint i = 0; i < _layout.LayoutBindingCount; i++)
        {
            ref VkDescriptorSetLayoutBinding layoutBinding = ref _layout.GetLayoutBinding(i);
            if (layoutBinding.pImmutableSamplers != null)
                continue;

            VkDescriptorType descriptorType = layoutBinding.descriptorType;

            VulkanBuffer? backendBuffer = default;
            VulkanTextureView? backendTextureView = default;
            VulkanSampler? backendSampler = default;
            VulkanAccelerationStructure? backendAccelerationStructure = default;

            BindGroupEntry? foundEntry = default;
            foreach (BindGroupEntry entry in entries)
            {
                uint registerOffset = _device.GetRegisterOffset(layoutBinding.descriptorType);
                uint originalBinding = layoutBinding.binding - registerOffset;

                if (entry.Binding != originalBinding)
                    continue;

                switch (layoutBinding.descriptorType)
                {
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        backendSampler = entry.Sampler != null ? (VulkanSampler)entry.Sampler : default;
                        if (backendSampler == null)
                            continue;
                        break;

                    case VkDescriptorType.SampledImage:
                    case VkDescriptorType.StorageImage:
                        backendTextureView = entry.TextureView != null ? (VulkanTextureView)entry.TextureView : default;
                        if (backendTextureView == null)
                            continue;
                        break;

                    case VkDescriptorType.UniformTexelBuffer:
                    case VkDescriptorType.StorageTexelBuffer:
                    case VkDescriptorType.StorageBuffer:
                    case VkDescriptorType.StorageBufferDynamic:
                        backendBuffer = entry.Buffer != null ? (VulkanBuffer)entry.Buffer : default;
                        if (backendBuffer == null)
                            continue;
                        break;

                    case VkDescriptorType.UniformBuffer:
                    case VkDescriptorType.UniformBufferDynamic:
                        backendBuffer = entry.Buffer != null ? (VulkanBuffer)entry.Buffer : default;
                        if (backendBuffer == null)
                            continue;
                        break;

                    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        backendAccelerationStructure = entry.AccelerationStructure != null ? (VulkanAccelerationStructure)entry.AccelerationStructure : default;
                        if (backendAccelerationStructure == null)
                            continue;
                        break;

                    default:
                        break;
                }

                foundEntry = entry;
                break;
            }

            descriptorWrites[descriptorWriteCount++] = new()
            {
                dstSet = _handle,
                dstBinding = layoutBinding.binding,
                dstArrayElement = 0,
                descriptorCount = layoutBinding.descriptorCount,
                descriptorType = descriptorType
            };

            switch (descriptorType)
            {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    imageInfos[i].sampler = foundEntry.HasValue ? backendSampler!.Handle : _device.NullSampler;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    imageInfos[i].sampler = VkSampler.Null;
                    if (foundEntry.HasValue)
                    {
                        imageInfos[i].imageView = backendTextureView!.Handle;
                        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                    else
                    {
                        imageInfos[i].imageView = _device.NullImage2DView;
                        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    imageInfos[i].sampler = VkSampler.Null;
                    imageInfos[i].imageView = backendTextureView!.Handle;
                    imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VkDescriptorType.UniformBufferDynamic:
                case VkDescriptorType.StorageBufferDynamic:
                    if (backendBuffer != null)
                    {
                        bufferInfos[i].buffer = backendBuffer!.Handle;
                        bufferInfos[i].offset = foundEntry!.Value.Offset;
                        bufferInfos[i].range = foundEntry!.Value.Size;
                    }
                    else
                    {
                        bufferInfos[i].buffer = _device.NullBuffer;
                        bufferInfos[i].range = VK_WHOLE_SIZE;
                    }

                    descriptorWrites[i].pBufferInfo = &bufferInfos[i];
                    break;

                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                    VkAccelerationStructureKHR accelerationStructure = backendAccelerationStructure!.Handle;
                    accelStructInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                    accelStructInfos[i].accelerationStructureCount = 1u;
                    accelStructInfos[i].pAccelerationStructures = &accelerationStructure;

                    descriptorWrites[i].pNext = &accelStructInfos[i];
                    break;

                default:
                    throw new GraphicsException($"Vulkan: DescriptorType '{descriptorType}' not handled");
            }
        }

        _device.DeviceApi.vkUpdateDescriptorSets(
            _device.Handle,
            (uint)descriptorWriteCount,
            descriptorWrites,
            0,
            null
        );
    }
}
