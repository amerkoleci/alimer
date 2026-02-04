// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using System.Diagnostics;
using static Alimer.Utilities.MemoryUtilities;
using static Alimer.Utilities.MarshalUtilities;
using Alimer.Utilities;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanComputePipeline : ComputePipeline
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipeline _handle = VkPipeline.Null;

    public VulkanComputePipeline(VulkanGraphicsDevice device, in ComputePipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        VkLayout = (VulkanPipelineLayout)descriptor.Layout;

        VkPipelineShaderStageCreateInfo stage = new()
        {
            stage = VK_SHADER_STAGE_COMPUTE_BIT,
            module = ((VulkanShaderModule)descriptor.ComputeShader).Handle,
            pName = (byte*)descriptor.ComputeShader.EntryPoint
        };

        VkComputePipelineCreateInfo createInfo = new()
        {
            stage = stage,
            layout = VkLayout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        VkResult result = _device.DeviceApi.vkCreateComputePipelines(
            device.PipelineCache,
            1, &createInfo,
            null,
            &pipeline
            );

        if (result != VK_SUCCESS)
        {
            Log.Error("Vulkan: Failed to create Compute Pipeline.");
            return;
        }

        _handle = pipeline;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override PipelineLayout Layout => VkLayout;

    public VkPipeline Handle => _handle;
    public VulkanPipelineLayout VkLayout { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanRenderPipeline" /> class.
    /// </summary>
    ~VulkanComputePipeline() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.Pipeline, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyPipeline( _handle);
    }
}
