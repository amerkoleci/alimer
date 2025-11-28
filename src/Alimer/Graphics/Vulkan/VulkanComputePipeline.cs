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

        Utf8String entryPoint = new(VkStringInterop.ConvertToUnmanaged(descriptor.ComputeShader.EntryPoint));

        VkPipelineShaderStageCreateInfo stage = new()
        {
            stage = VkShaderStageFlags.Compute,
            pName = entryPoint
        };

        VkResult result = _device.DeviceApi.vkCreateShaderModule(device.Handle, descriptor.ComputeShader.ByteCode, null, out stage.module);
        if (result != VkResult.Success)
        {
            Log.Error("Failed to create a pipeline shader module");
            return;
        }

        VkComputePipelineCreateInfo createInfo = new()
        {
            stage = stage,
            layout = VkLayout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        result = _device.DeviceApi.vkCreateComputePipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        // Delete shader module.
        _device.DeviceApi.vkDestroyShaderModule(device.Handle, stage.module);

        if (result != VkResult.Success)
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
    ~VulkanComputePipeline () => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.Pipeline, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyPipeline(_device.Handle, _handle);
    }
}
