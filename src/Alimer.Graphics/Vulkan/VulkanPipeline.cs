// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipeline : Pipeline
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VulkanPipelineLayout _layout;
    private readonly VkPipeline _handle = VkPipeline.Null;

    public VulkanPipeline(VulkanGraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        _layout = (VulkanPipelineLayout)description.Layout;

        var shaderStageCount = description.ShaderStages.Length;
        var shaderStages = stackalloc VkPipelineShaderStageCreateInfo[shaderStageCount];
        var vkShaderNames = stackalloc sbyte*[shaderStageCount];

        for (var i = 0; i < shaderStageCount; i++)
        {
            ref var shaderDesc = ref description.ShaderStages[i];

            var entryPointName = shaderDesc.EntryPoint.GetUtf8Span();
            var entryPointNameLength = entryPointName.Length + 1;

            var pName = Interop.AllocateArray<sbyte>((uint)entryPointNameLength);
            var destination = new Span<sbyte>(pName, entryPointNameLength);

            entryPointName.CopyTo(destination);
            destination[entryPointName.Length] = 0x00;

            vkShaderNames[i] = pName;

            shaderStages[i] = new()
            {
                stage = shaderDesc.Stage.ToVk(),
                pName = pName,
            };

            var vkResult = vkCreateShaderModule(device.Handle, shaderDesc.ByteCode, null, out shaderStages[i].module);
            if (vkResult != VkResult.Success)
            {
                Log.Error("Failed to create a pipeline shader module");
                return;
            }
        }

        VkGraphicsPipelineCreateInfo createInfo = new()
        {
            stageCount = (uint)shaderStageCount,
            pStages = shaderStages,
            layout = _layout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        var result = vkCreateGraphicsPipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

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
        _layout = (VulkanPipelineLayout)description.Layout;

        var result = vkCreateShaderModule(device.Handle, description.ComputeShader.ByteCode, null, out VkShaderModule shaderModule);
        if (result != VkResult.Success)
        {
            Log.Error("Failed to create a pipeline shader module");
            return;
        }

        VkString entryPoint = new(description.ComputeShader.EntryPoint);

        var createInfo = new VkComputePipelineCreateInfo()
        {
            stage = new()
            {
                stage = VkShaderStageFlags.Compute,
                module = shaderModule,
                pName = entryPoint
            },
            layout = _layout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        result = vkCreateComputePipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

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
