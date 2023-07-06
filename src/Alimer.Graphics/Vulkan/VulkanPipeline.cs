// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

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

        // ShaderStages
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

        // VertexInputState
        VkVertexInputBindingDescription* vertexBindings = stackalloc VkVertexInputBindingDescription[MaxVertexBufferBindings];
        VkVertexInputAttributeDescription* vertexAttributes = stackalloc VkVertexInputAttributeDescription[MaxVertexAttributes];

        VkPipelineVertexInputStateCreateInfo vertexInputState = new();
        vertexInputState.pVertexBindingDescriptions = vertexBindings;
        vertexInputState.pVertexAttributeDescriptions = vertexAttributes;

        //for (var index = 0; index < MaxVertexBufferBindings; ++index)
        //{
        //    if (!description.V.vertexLayout.layouts[index].stride)
        //        break;

        //    const VertexBufferLayout* layout = &desc.vertexLayout.layouts[index];
        //    VkVertexInputBindingDescription* bindingDesc = &vertexBindings[vertexInputState.vertexBindingDescriptionCount++];
        //    bindingDesc->binding = index;
        //    bindingDesc->stride = layout->stride;
        //    bindingDesc->inputRate = ToVk(layout->stepMode);
        //}

        //for (var index = 0; index < MaxVertexAttributes; index++)
        //{
        //    const VertexAttribute* attribute = &desc.vertexLayout.attributes[index];
        //    if (attribute->format == VertexFormat::Undefined)
        //        continue;

        //    VkVertexInputAttributeDescription* attributeDesc = &vertexAttributes[vertexInputState.vertexAttributeDescriptionCount++];
        //    attributeDesc->location = index;
        //    attributeDesc->binding = attribute->bufferIndex;
        //    attributeDesc->format = ToVkFormat(attribute->format);
        //    attributeDesc->offset = attribute->offset;
        //}

        // InputAssemblyState
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = new();
        inputAssemblyState.topology = description.PrimitiveTopology.ToVk();
        switch (description.PrimitiveTopology)
        {
            case PrimitiveTopology.LineStrip:
            case PrimitiveTopology.TriangleStrip:
                inputAssemblyState.primitiveRestartEnable = true;
                break;
            default:
                inputAssemblyState.primitiveRestartEnable = false;
                break;
        }

        // TessellationState
        VkPipelineTessellationStateCreateInfo tessellationState = new();
        if (inputAssemblyState.topology == VkPrimitiveTopology.PatchList)
        {
            tessellationState.patchControlPoints = (uint)description.PatchControlPoints;
        }
        else
        {
            tessellationState.patchControlPoints = 0;
        }

        // ViewportState
        VkPipelineViewportStateCreateInfo viewportState = new()
        {
            viewportCount = 1,
            scissorCount = 1
        };

        // RasterizationState
        VkPipelineRasterizationStateCreateInfo rasterizationState = new();
        rasterizationState.depthClampEnable = false;

        //VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipStateInfo = new()
        //
        //if (desc.rasterizerState.depthClipMode == DepthClipMode::Clip &&
        //    depthClipEnableFeatures.depthClipEnable == VK_TRUE)
        //{
        //    rasterizationState.pNext = &depthClipStateInfo;
        //    depthClipStateInfo.depthClipEnable = VK_TRUE;
        //}
        //else
        //{
        //    rasterizationState.depthClampEnable = VK_TRUE;
        //}
        //
        //rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        //rasterizationState.polygonMode = ToVk(features2, desc.rasterizerState.fillMode);
        //rasterizationState.cullMode = ToVk(desc.rasterizerState.cullMode);
        //rasterizationState.frontFace = desc.rasterizerState.frontFaceCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        //// Can be managed by command buffer
        //rasterizationState.depthBiasEnable = desc.rasterizerState.depthBias != 0.0f || desc.rasterizerState.slopeScaledDepthBias != 0.0f;
        //rasterizationState.depthBiasConstantFactor = desc.rasterizerState.depthBias;
        //rasterizationState.depthBiasClamp = desc.rasterizerState.depthBiasClamp;
        //rasterizationState.depthBiasSlopeFactor = desc.rasterizerState.slopeScaledDepthBias;
        //rasterizationState.lineWidth = 1.0f;

        // MultisampleState
        VkPipelineMultisampleStateCreateInfo multisampleState = new();
        multisampleState.rasterizationSamples = description.SampleCount.ToVk();

        Debug.Assert((int)multisampleState.rasterizationSamples <= 32);
        if (multisampleState.rasterizationSamples > VkSampleCountFlags.Count1)
        {
            multisampleState.alphaToCoverageEnable = description.BlendState.AlphaToCoverageEnabled;
            multisampleState.alphaToOneEnable = false;
            multisampleState.sampleShadingEnable = false;
            multisampleState.minSampleShading = 1.0f;
        }
        uint sampleMask = uint.MaxValue;
        multisampleState.pSampleMask = &sampleMask;

        // DepthStencilState
        VkPipelineDepthStencilStateCreateInfo depthStencilState = new();
        depthStencilState.depthTestEnable = (description.DepthStencilState.DepthCompare != CompareFunction.Always || description.DepthStencilState.DepthWriteEnabled);
        depthStencilState.depthWriteEnable = description.DepthStencilState.DepthWriteEnabled;
        depthStencilState.depthCompareOp = description.DepthStencilState.DepthCompare.ToVk();
        if (_device.PhysicalDeviceFeatures2.features.depthBounds)
        {
            depthStencilState.depthBoundsTestEnable = description.DepthStencilState.DepthBoundsTestEnable;
        }
        else
        {
            depthStencilState.depthBoundsTestEnable = false;
        }
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        //depthStencilState.stencilTestEnable = StencilTestEnabled(&desc.depthStencilState) ? VK_TRUE : VK_FALSE;
        //depthStencilState.front.failOp = ToVk(desc.depthStencilState.frontFace.failOp);
        //depthStencilState.front.passOp = ToVk(desc.depthStencilState.frontFace.passOp);
        //depthStencilState.front.depthFailOp = ToVk(desc.depthStencilState.frontFace.depthFailOp);
        //depthStencilState.front.compareOp = ToVk(desc.depthStencilState.frontFace.compareFunc);
        //depthStencilState.front.compareMask = desc.depthStencilState.stencilReadMask;
        //depthStencilState.front.writeMask = desc.depthStencilState.stencilWriteMask;
        //depthStencilState.front.reference = 0;
        //
        //depthStencilState.back.failOp = ToVk(desc.depthStencilState.backFace.failOp);
        //depthStencilState.back.passOp = ToVk(desc.depthStencilState.backFace.passOp);
        //depthStencilState.back.depthFailOp = ToVk(desc.depthStencilState.backFace.depthFailOp);
        //depthStencilState.back.compareOp = ToVk(desc.depthStencilState.backFace.compareFunc);
        //depthStencilState.back.compareMask = desc.depthStencilState.stencilReadMask;
        //depthStencilState.back.writeMask = desc.depthStencilState.stencilWriteMask;
        //depthStencilState.back.reference = 0;

        // BlendState

        // DynamicState
        int dynamicStateCount = 4;
        var vkDynamicStates = stackalloc VkDynamicState[6] {
            VkDynamicState.Viewport,
            VkDynamicState.Scissor,
            VkDynamicState.StencilReference,
            VkDynamicState.BlendConstants,
            VkDynamicState.DepthBounds,
            VkDynamicState.FragmentShadingRateKHR
        };
        if (_device.PhysicalDeviceFeatures2.features.depthBounds)
        {
            dynamicStateCount++;
        }
        //if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
        //{
        //    dynamicStateCount++;
        //}

        var dynamicState = new VkPipelineDynamicStateCreateInfo
        {
            pNext = null,
            flags = 0,
            dynamicStateCount = (uint)dynamicStateCount,
            pDynamicStates = vkDynamicStates,
        };

        VkGraphicsPipelineCreateInfo createInfo = new()
        {
            stageCount = (uint)shaderStageCount,
            pStages = shaderStages,
            pVertexInputState = &vertexInputState,
            pInputAssemblyState = &inputAssemblyState,
            pTessellationState = (inputAssemblyState.topology == VkPrimitiveTopology.PatchList) ? &tessellationState : null,
            pViewportState = &viewportState,
            pRasterizationState = &rasterizationState,
            pMultisampleState = &multisampleState,
            pDepthStencilState = &depthStencilState,
            pColorBlendState = null,
            pDynamicState = &dynamicState,
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
