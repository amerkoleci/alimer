// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipeline : Pipeline
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipeline _handle = VkPipeline.Null;

    public VulkanPipeline(VulkanGraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        VkGraphicsPipelineCreateInfo createInfo = new();
        //createInfo.stage = stage;
        //createInfo.layout = pipeline->layout->handle;

        VkPipeline pipeline;
        VkResult result = vkCreateGraphicsPipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create Render Pipeline.");
            return;
        }

        _handle = pipeline;

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    public VulkanPipeline(VulkanGraphicsDevice device, in ComputePipelineDescription description)
        : base(PipelineType.Compute, description.Label)
    {
        _device = device;
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

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkPipeline Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanPipeline" /> class.
    /// </summary>
    ~VulkanPipeline() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.Pipeline, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyPipeline(_device.Handle, _handle);
    }
}
