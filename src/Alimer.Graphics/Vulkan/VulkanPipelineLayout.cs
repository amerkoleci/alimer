// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipelineLayout : PipelineLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipelineLayout _handle = VkPipelineLayout.Null;

    public VulkanPipelineLayout(VulkanGraphicsDevice device, in PipelineLayoutDescription description)
        : base(description)
    {
        _device = device;
        VkPipelineLayoutCreateInfo createInfo = new();
        //createInfo.stage = stage;
        //createInfo.layout = pipeline->layout->handle;

        VkResult result = vkCreatePipelineLayout(device.Handle, &createInfo, null, out _handle);
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

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkPipelineLayout Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanPipelineLayout" /> class.
    /// </summary>
    ~VulkanPipelineLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.PipelineLayout, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyPipelineLayout(_device.Handle, _handle);
    }
}
