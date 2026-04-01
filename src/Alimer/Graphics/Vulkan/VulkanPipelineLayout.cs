// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Utilities.MemoryUtilities;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipelineLayout : PipelineLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipelineLayout _handle = VkPipelineLayout.Null;

    public VulkanPipelineLayout(VulkanGraphicsDevice device, in PipelineLayoutDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;

        int setLayoutCount = descriptor.BindGroupLayouts.Length;
        setLayoutCount += device.BindlessManager.DescriptorSetCount;
        VkDescriptorSetLayout* pSetLayouts = stackalloc VkDescriptorSetLayout[setLayoutCount];

        for (int i = 0; i < descriptor.BindGroupLayouts.Length; i++)
        {
            pSetLayouts[i] = ((VulkanBindGroupLayout)descriptor.BindGroupLayouts[i]).Handle;
        }

        BindlessLayoutFirstIndex = descriptor.BindGroupLayouts.Length;
        int startIndex = BindlessLayoutFirstIndex;
        pSetLayouts[startIndex++] = device.BindlessManager.Samplers.DescriptorSetLayout;
        pSetLayouts[startIndex++] = device.BindlessManager.SampledImages.DescriptorSetLayout;
        pSetLayouts[startIndex++] = device.BindlessManager.StorageImages.DescriptorSetLayout;
        pSetLayouts[startIndex++] = device.BindlessManager.StorageBuffers.DescriptorSetLayout;

        VkPushConstantRange pushConstantRange = new()
        {
            stageFlags = VK_SHADER_STAGE_ALL,
            offset = 0u,
            size = PushConstantsSize,
        };

        VkPipelineLayoutCreateInfo createInfo = new()
        {
            setLayoutCount = (uint)setLayoutCount,
            pSetLayouts = pSetLayouts,
            pushConstantRangeCount = 1u,
            pPushConstantRanges = &pushConstantRange
        };

        VkResult result = _device.DeviceApi.vkCreatePipelineLayout(in createInfo, out _handle);
        if (result != VK_SUCCESS)
        {
            Log.Error($"Vulkan: Failed to create {nameof(PipelineLayout)}.");
            return;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkPipelineLayout Handle => _handle;

    public int BindlessLayoutFirstIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.PipelineLayout, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyPipelineLayout(_handle);
    }
}
