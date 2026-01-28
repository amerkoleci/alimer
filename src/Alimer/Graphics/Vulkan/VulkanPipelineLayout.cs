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

    public VulkanPipelineLayout(VulkanGraphicsDevice device, in PipelineLayoutDescriptor description)
        : base(description)
    {
        _device = device;

        int setLayoutCount = description.BindGroupLayouts.Length;
        VkDescriptorSetLayout* pSetLayouts = stackalloc VkDescriptorSetLayout[setLayoutCount];

        for (int i = 0; i < setLayoutCount; i++)
        {
            pSetLayouts[i] = ((VulkanBindGroupLayout)description.BindGroupLayouts[i]).Handle;
        }

        int pushConstantRangeCount = description.PushConstantRanges.Length;
        _pushConstantRanges = AllocateArray<VkPushConstantRange>((nuint)pushConstantRangeCount);
        {
            uint offset = 0;
            for (int i = 0; i < pushConstantRangeCount; i++)
            {
                _pushConstantRanges[i] = new VkPushConstantRange()
                {
                    stageFlags = VkShaderStageFlags.All,
                    offset = offset,
                    size = description.PushConstantRanges[i].Size,
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

        VkResult result = _device.DeviceApi.vkCreatePipelineLayout(device.Handle, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error($"Vulkan: Failed to create {nameof(PipelineLayout)}.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanPipelineLayout" /> class.
    /// </summary>
    ~VulkanPipelineLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkPipelineLayout Handle => _handle;

    public ref VkPushConstantRange GetPushConstantRange(uint index) => ref _pushConstantRanges[index];

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.PipelineLayout, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_pushConstantRanges);
        _device.DeviceApi.vkDestroyPipelineLayout(_device.Handle, _handle);
    }
}
