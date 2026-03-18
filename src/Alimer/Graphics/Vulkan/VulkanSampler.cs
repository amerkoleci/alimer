// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanSampler : Sampler
{
    private readonly VulkanGraphicsDevice _device;
    private readonly int _bindlessIndex = InvalidBindlessIndex;

    public VulkanSampler(VulkanGraphicsDevice device, in SamplerDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        Handle = device.GetOrCreateVulkanSampler(in descriptor);

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        if (device.Bindless)
        {
            _bindlessIndex = device.BindlessDescriptorSet!.Samplers.Allocate();

            if (_bindlessIndex != InvalidBindlessIndex)
            {
                VkDescriptorImageInfo imageInfo = new()
                {
                    sampler = Handle
                };

                VkWriteDescriptorSet write = new()
                {
                    dstSet = device.BindlessDescriptorSet!.Samplers.DescriptorSet,
                    dstBinding = 0,
                    dstArrayElement = (uint)_bindlessIndex,
                    descriptorCount = 1,
                    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    pImageInfo = &imageInfo
                };

                device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);
            }
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkSampler Handle { get; }
    public override int BindlessIndex => _bindlessIndex;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.Sampler, Handle, newLabel);
    }
}
