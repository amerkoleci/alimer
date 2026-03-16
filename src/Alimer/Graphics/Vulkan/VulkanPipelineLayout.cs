// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Utilities.MemoryUtilities;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipelineLayout : PipelineLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipelineLayout _handle = VkPipelineLayout.Null;
    private readonly VkPushConstantRange* _pushConstantRanges;

    public VulkanPipelineLayout(VulkanGraphicsDevice device, in PipelineLayoutDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;

        int setLayoutCount = descriptor.BindGroupLayouts.Length;

        if (device.Bindless)
        {
            setLayoutCount += 1;
        }
        VkDescriptorSetLayout* pSetLayouts = stackalloc VkDescriptorSetLayout[setLayoutCount];

        for (int i = 0; i < descriptor.BindGroupLayouts.Length; i++)
        {
            pSetLayouts[i] = ((VulkanBindGroupLayout)descriptor.BindGroupLayouts[i]).Handle;
        }

        if (device.Bindless)
        {
            BindlessLayoutFirstIndex = descriptor.BindGroupLayouts.Length;
            int startIndex = BindlessLayoutFirstIndex;
            pSetLayouts[startIndex++] = device.BindlessDescriptorSet.Samplers.DescriptorSetLayout;
        }

        int pushConstantRangeCount = descriptor.PushConstantRanges.Length;
        _pushConstantRanges = AllocateArray<VkPushConstantRange>((nuint)pushConstantRangeCount);
        {
            uint offset = 0;
            for (int i = 0; i < pushConstantRangeCount; i++)
            {
                _pushConstantRanges[i] = new VkPushConstantRange()
                {
                    stageFlags = VK_SHADER_STAGE_ALL,
                    offset = offset,
                    size = descriptor.PushConstantRanges[i].Size,
                };

                offset += _pushConstantRanges[i].size;
            }
        }

        VkPipelineLayoutCreateInfo createInfo = new()
        {
            setLayoutCount = (uint)setLayoutCount,
            pSetLayouts = pSetLayouts,
            pushConstantRangeCount = (uint)pushConstantRangeCount,
            pPushConstantRanges = _pushConstantRanges
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

    public ref VkPushConstantRange GetPushConstantRange(uint index) => ref _pushConstantRanges[index];
    public int BindlessLayoutFirstIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.PipelineLayout, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_pushConstantRanges);
        _device.DeviceApi.vkDestroyPipelineLayout(_handle);
    }
}
