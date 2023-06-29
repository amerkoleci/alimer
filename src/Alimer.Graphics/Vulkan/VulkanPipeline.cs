// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipeline : Pipeline
{
    private readonly VkPipeline _handle = VkPipeline.Null;

    public VulkanPipeline(VulkanGraphicsDevice device, in ComputePipelineDescription description)
        : base(device, PipelineType.Compute, description.Label)
    {
        VkComputePipelineCreateInfo createInfo = new();
        //createInfo.stage = stage;
        //createInfo.layout = pipeline->layout->handle;

        VkPipeline pipeline;
        VkResult result = vkCreateComputePipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create Compute Pipeline.");
            return;
        }

        _handle = pipeline;
    }

    public VkDevice VkDevice => ((VulkanGraphicsDevice)Device).Handle;

    public VkPipeline Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanPipeline" /> class.
    /// </summary>
    ~VulkanPipeline() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        ((VulkanGraphicsDevice)Device).SetObjectName(VkObjectType.Pipeline, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyPipeline(VkDevice, _handle);
    }
}
