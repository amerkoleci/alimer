// Copyright © Amer Koleci and Contributors.
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
        int descriptorWriteCount = _layout.LayoutBindingCount;
        VkWriteDescriptorSet* descriptorWrites = stackalloc VkWriteDescriptorSet[descriptorWriteCount];
        VkDescriptorBufferInfo* bufferInfos = stackalloc VkDescriptorBufferInfo[descriptorWriteCount];
        VkDescriptorImageInfo* imageInfos = stackalloc VkDescriptorImageInfo[descriptorWriteCount];
        VkWriteDescriptorSetAccelerationStructureKHR* accelStructInfos = stackalloc VkWriteDescriptorSetAccelerationStructureKHR[descriptorWriteCount];

        for (uint i = 0; i < descriptorWriteCount; i++)
        {
            ref VkDescriptorSetLayoutBinding layoutBinding = ref _layout.GetLayoutBinding(i);
            VkDescriptorType descriptorType = layoutBinding.descriptorType;

            if (i > description.Entries.Length - 1)
            {
                // Create a null SRV, UAV, or CBV
                descriptorWrites[i] = new()
                {
                    dstSet = _handle,
                    dstBinding = layoutBinding.binding,
                    descriptorCount = 1,
                    descriptorType = descriptorType
                };

                switch (descriptorType)
                {
                    case VkDescriptorType.Sampler:
                        imageInfos[i].sampler = device.NullSampler;
                        descriptorWrites[i].pImageInfo = &imageInfos[i];
                        break;
                }

                continue;
            }

            ref readonly BindGroupEntry entry = ref description.Entries[i];


            descriptorWrites[i] = new()
            {
                dstSet = _handle,
                dstBinding = layoutBinding.binding,
                descriptorCount = 1,
                descriptorType = descriptorType
            };

            switch (descriptorType)
            {
                case VkDescriptorType.Sampler:
                    VulkanSampler backendSampler = ((VulkanSampler)entry.Sampler);
                    imageInfos[i].sampler = backendSampler.Handle;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
                    break;

                case VkDescriptorType.UniformBuffer:
                case VkDescriptorType.UniformBufferDynamic:
                case VkDescriptorType.StorageBuffer:
                case VkDescriptorType.StorageBufferDynamic:
                    VulkanBuffer vulkanBuffer = ((VulkanBuffer)entry.Buffer);
                    bufferInfos[i].buffer = vulkanBuffer.Handle;
                    bufferInfos[i].offset = entry.Offset;
                    bufferInfos[i].range = entry.Size;
                    descriptorWrites[i].pBufferInfo = &bufferInfos[i];
                    break;

                case VkDescriptorType.SampledImage:
                    VulkanTexture backendBuffer = ((VulkanTexture)entry.Texture);
                    imageInfos[i].sampler = VkSampler.Null;
                    imageInfos[i].imageView = backendBuffer.GetView(0, 0);
                    imageInfos[i].imageLayout = VkImageLayout.ShaderReadOnlyOptimal;
                    descriptorWrites[i].pImageInfo = &imageInfos[i];
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
        _device.SetObjectName(VkObjectType.DescriptorSet, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyDescriptorPool(_device.Handle, _descriptorPool);
    }
}
