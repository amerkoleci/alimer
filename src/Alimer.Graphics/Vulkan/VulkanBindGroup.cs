// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroup : BindGroup
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VulkanBindGroupLayout _layout;
    private readonly VkDescriptorPool _descriptorPool = VkDescriptorPool.Null;
    private readonly VkDescriptorSet _handle = VkDescriptorSet.Null;

    public VulkanBindGroup(VulkanGraphicsDevice device, BindGroupLayout layout, in BindGroupDescription description)
        : base(description)
    {
        _device = device;
        _layout = (VulkanBindGroupLayout)layout;

        ReadOnlySpan<VkDescriptorPoolSize> poolSizes = _layout.PoolSizes;
        uint maxSets = 1u;

        VkResult result = vkCreateDescriptorPool(device.Handle, poolSizes, maxSets, out _descriptorPool);
        if (result != VkResult.Success)
        {
            Log.Error($"Vulkan: Failed to create {nameof(BindGroup)}.");
            return;
        }

        VkDescriptorSetLayout descriptorSetLayout = _layout.Handle;

        VkDescriptorSetAllocateInfo allocInfo = new()
        {
            descriptorPool = _descriptorPool,
            descriptorSetCount = 1,
            pSetLayouts = &descriptorSetLayout
        };

        VkDescriptorSet descriptorSet;
        result = vkAllocateDescriptorSets(device.Handle, &allocInfo, &descriptorSet);
        if (result != VkResult.Success)
        {
            Log.Error($"Vulkan: Failed to allocate DescriptorSet.");
            return;
        }
        _handle = descriptorSet;

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        // TODO: Handle null descriptors to avoid vulkan warnings
        int descriptorWriteCount = 0;
        VkWriteDescriptorSet* descriptorWrites = stackalloc VkWriteDescriptorSet[_layout.LayoutBindingCount];
        VkDescriptorBufferInfo* bufferInfos = stackalloc VkDescriptorBufferInfo[_layout.LayoutBindingCount];
        VkDescriptorImageInfo* imageInfos = stackalloc VkDescriptorImageInfo[_layout.LayoutBindingCount];
        //VkWriteDescriptorSetAccelerationStructureKHR* accelStructInfos = stackalloc VkWriteDescriptorSetAccelerationStructureKHR[descriptorWriteCount];

        for (uint i = 0; i < _layout.LayoutBindingCount; i++)
        {
            ref VkDescriptorSetLayoutBinding layoutBinding = ref _layout.GetLayoutBinding(i);
            if (layoutBinding.pImmutableSamplers != null)
                continue;

            VkDescriptorType descriptorType = layoutBinding.descriptorType;

            VulkanBuffer? backendBuffer = default;
            VulkanTexture? backendTexture = default;
            VulkanSampler? backendSampler = default;

            BindGroupEntry? foundEntry = default;
            foreach (BindGroupEntry entry in description.Entries)
            {
                uint registerOffset = device.GetRegisterOffset(layoutBinding.descriptorType);
                uint originalBinding = layoutBinding.binding - registerOffset;

                if (entry.Binding != originalBinding)
                    continue;

                switch (layoutBinding.descriptorType)
                {
                    case VkDescriptorType.Sampler:
                        backendSampler = entry.Sampler != null ? (VulkanSampler)entry.Sampler : default;
                        if (backendSampler == null)
                            continue;
                        break;

                    case VkDescriptorType.SampledImage:
                    case VkDescriptorType.StorageImage:
                        backendTexture = entry.Texture != null ? (VulkanTexture)entry.Texture : default;
                        if (backendTexture == null)
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

                    //case VkDescriptorType.AccelerationStructureKHR:
                    //    return shaderResource;

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
                case VkDescriptorType.Sampler:
                    imageInfos[i].sampler = foundEntry.HasValue ? backendSampler!.Handle : device.NullSampler;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VkDescriptorType.SampledImage:
                    imageInfos[i].sampler = VkSampler.Null;
                    if (foundEntry.HasValue)
                    {
                        imageInfos[i].imageView = backendTexture!.GetView(0, 0);
                        imageInfos[i].imageLayout = VkImageLayout.ShaderReadOnlyOptimal;
                    }
                    else
                    {
                        imageInfos[i].imageView = device.NullImage2DView;
                        imageInfos[i].imageLayout = VkImageLayout.General;
                    }
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VkDescriptorType.StorageImage:
                    imageInfos[i].sampler = VkSampler.Null;
                    imageInfos[i].imageView = backendTexture!.GetView(0, 0);
                    imageInfos[i].imageLayout = VkImageLayout.General;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VkDescriptorType.UniformBuffer:
                case VkDescriptorType.StorageBuffer:
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
                        bufferInfos[i].buffer = device.NullBuffer;
                        bufferInfos[i].range = VK_WHOLE_SIZE;
                    }

                    descriptorWrites[i].pBufferInfo = &bufferInfos[i];
                    break;

                default:
                    throw new GraphicsException($"Vulkan: DescriptorType '{descriptorType}' not handled");
            }
        }

        vkUpdateDescriptorSets(
            device.Handle,
            (uint)descriptorWriteCount,
            descriptorWrites,
            0,
            null
        );
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
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.DescriptorSet, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyDescriptorPool(_device.Handle, _descriptorPool);
    }
}
