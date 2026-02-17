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

    public VulkanBindGroup(VulkanBindGroupLayout layout, in BindGroupDescriptor descriptor)
        : base(descriptor)
    {
        _device = layout.VkDevice;
        _layout = layout;

        // Allocate DescriptorSet from the bind group pool
        uint maxVariableArrayLength = 0;
        VkDescriptorSet descriptorSet = VkDescriptorSet.Null;
        VkResult result = _device.AllocateDescriptorSet(_layout.Handle, &descriptorSet, maxVariableArrayLength);

        // If we have run out of pool memory and rely on internalPools, create a new internal pool and retry
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY
            || result == VK_ERROR_FRAGMENTED_POOL)
        {
            _device.AllocateDescriptorPool();
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
    public override void Update(Span<BindGroupEntry> entries)
    {
        // TODO: Handle null descriptors to avoid vulkan warnings
        // TODO: Handle total layout binding count (entries + array size)
        VkWriteDescriptorSet* descriptorWrites = stackalloc VkWriteDescriptorSet[_layout.LayoutBindingCount];
        VkDescriptorBufferInfo* bufferInfos = stackalloc VkDescriptorBufferInfo[_layout.LayoutBindingCount];
        VkDescriptorImageInfo* imageInfos = stackalloc VkDescriptorImageInfo[_layout.LayoutBindingCount];
        VkWriteDescriptorSetAccelerationStructureKHR* accelStructInfos = stackalloc VkWriteDescriptorSetAccelerationStructureKHR[_layout.LayoutBindingCount];

        int descriptorWriteCount = 0;
        for (uint i = 0; i < _layout.Entries.Length; i++)
        {
            ref BindGroupLayoutEntry layoutEntry = ref _layout.Entries[i];
            if (layoutEntry.StaticSampler.HasValue)
                continue;

            ref VkDescriptorSetLayoutBinding layoutBinding = ref _layout.GetLayoutBinding(i);
            VkDescriptorType descriptorType = layoutBinding.descriptorType;

            VulkanBuffer? backendBuffer = default;
            VulkanTextureView? backendTextureView = default;
            VulkanSampler? backendSampler = default;
            VulkanAccelerationStructure? backendAccelerationStructure = default;

            BindGroupEntry? foundEntry = default;
            foreach (BindGroupEntry entry in entries)
            {
                uint registerOffset = _device.GetRegisterOffset(layoutBinding.descriptorType, false);
                uint originalBinding = layoutBinding.binding - registerOffset;

                if (entry.Binding != originalBinding)
                {
                    // ReadOnly storage?
                    switch (layoutBinding.descriptorType)
                    {
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            registerOffset = _device.GetRegisterOffset(layoutBinding.descriptorType, true);
                            originalBinding = layoutBinding.binding - registerOffset;
                            if (entry.Binding != originalBinding)
                            {
                                continue;
                            }
                            break;

                        default:
                            continue;
                    }
                }

                switch (layoutBinding.descriptorType)
                {
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        if (entry.Resource is not Sampler)
                            continue;

                        backendSampler = (VulkanSampler)entry.Resource;
                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        if (entry.Resource is TextureView)
                        {
                            backendTextureView = (VulkanTextureView)entry.Resource;
                        }
                        else if (entry.Resource is Texture)
                        {
                            backendTextureView = (VulkanTextureView)((VulkanTexture)entry.Resource).DefaultView!;
                        }

                        if (backendTextureView == null)
                            continue;
                        break;


                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        if (entry.Resource is not GraphicsBuffer)
                            continue;

                        backendBuffer = (VulkanBuffer)entry.Resource;
                        break;

                    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        if (entry.Resource is not AccelerationStructure)
                            continue;

                        backendAccelerationStructure = (VulkanAccelerationStructure)entry.Resource;
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
                dstArrayElement = foundEntry.HasValue ? foundEntry.Value.ArrayElement : 0u,
                descriptorCount = layoutBinding.descriptorCount,
                descriptorType = descriptorType,
                pTexelBufferView = default,
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
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
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
            (uint)descriptorWriteCount,
            descriptorWrites,
            0,
            null
        );
    }
}
