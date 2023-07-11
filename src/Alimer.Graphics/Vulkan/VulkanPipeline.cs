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
        BindPoint = VkPipelineBindPoint.Graphics;

        // ShaderStages
        int shaderStageCount = description.ShaderStages.Length;
        VkPipelineShaderStageCreateInfo* shaderStages = stackalloc VkPipelineShaderStageCreateInfo[shaderStageCount];
        sbyte** vkShaderNames = stackalloc sbyte*[shaderStageCount];

        VkResult result;
        for (int i = 0; i < shaderStageCount; i++)
        {
            ref ShaderStageDescription shaderDesc = ref description.ShaderStages[i];

            ReadOnlySpan<sbyte> entryPointName = shaderDesc.EntryPoint.GetUtf8Span();
            int entryPointNameLength = entryPointName.Length + 1;

            sbyte* pName = Interop.AllocateArray<sbyte>((uint)entryPointNameLength);
            Span<sbyte> destination = new(pName, entryPointNameLength);

            entryPointName.CopyTo(destination);
            destination[entryPointName.Length] = 0x00;

            vkShaderNames[i] = pName;

            shaderStages[i] = new()
            {
                stage = shaderDesc.Stage.ToVk(),
                pName = pName,
            };

            result = vkCreateShaderModule(device.Handle, shaderDesc.ByteCode, null, out shaderStages[i].module);
            if (result != VkResult.Success)
            {
                Log.Error("Failed to create a pipeline shader module");
                return;
            }
        }

        // VertexInputState
        int vertexBufferLayoutsCount = 0;
        int vertexAttributesCount = 0;
        VkVertexInputBindingDescription* vertexBindings = stackalloc VkVertexInputBindingDescription[description.VertexBufferLayouts.Length];
        VkVertexInputAttributeDescription* vertexAttributes = stackalloc VkVertexInputAttributeDescription[MaxVertexAttributes];

        for (uint binding = 0; binding < description.VertexBufferLayouts.Length; binding++)
        {
            ref readonly VertexBufferLayout layout = ref description.VertexBufferLayouts[binding];

            if (layout.Stride == 0)
                continue;

            VkVertexInputBindingDescription* bindingDesc = &vertexBindings[vertexBufferLayoutsCount++];
            bindingDesc->binding = binding;
            bindingDesc->stride = layout.Stride;
            bindingDesc->inputRate = layout.StepMode.ToVk();

            for (int i = 0; i < layout.Attributes.Length; i++)
            {
                ref readonly VertexAttribute attribute = ref layout.Attributes[i];

                VkVertexInputAttributeDescription* attributeDesc = &vertexAttributes[vertexAttributesCount++];
                attributeDesc->location = attribute.ShaderLocation;
                attributeDesc->binding = binding;
                attributeDesc->format = attribute.Format.ToVk();
                attributeDesc->offset = attribute.Offset;
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState = new()
        {
            vertexBindingDescriptionCount = (uint)vertexBufferLayoutsCount,
            pVertexBindingDescriptions = vertexBindings,
            vertexAttributeDescriptionCount = (uint)vertexAttributesCount,
            pVertexAttributeDescriptions = vertexAttributes
        };

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

        VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipStateInfo = new();

        if (description.RasterizerState.DepthClipMode == DepthClipMode.Clip && device.DepthClipEnable)
        {
            rasterizationState.pNext = &depthClipStateInfo;
            depthClipStateInfo.depthClipEnable = true;
        }
        else
        {
            rasterizationState.depthClampEnable = true;
        }

        rasterizationState.rasterizerDiscardEnable = false;
        rasterizationState.polygonMode = VkPolygonMode.Fill; // ToVk(features2, desc.rasterizerState.fillMode);
        rasterizationState.cullMode = description.RasterizerState.CullMode.ToVk();
        rasterizationState.frontFace = description.RasterizerState.FrontFaceCounterClockwise ? VkFrontFace.CounterClockwise : VkFrontFace.Clockwise;
        // Can be managed by command buffer
        //rasterizationState.depthBiasEnable = desc.rasterizerState.depthBias != 0.0f || desc.rasterizerState.slopeScaledDepthBias != 0.0f;
        //rasterizationState.depthBiasConstantFactor = desc.rasterizerState.depthBias;
        //rasterizationState.depthBiasClamp = desc.rasterizerState.depthBiasClamp;
        //rasterizationState.depthBiasSlopeFactor = desc.rasterizerState.slopeScaledDepthBias;
        rasterizationState.lineWidth = 1.0f;

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
        VkPipelineDepthStencilStateCreateInfo depthStencilState = new()
        {
            depthTestEnable = (description.DepthStencilState.DepthCompare != CompareFunction.Always || description.DepthStencilState.DepthWriteEnabled),
            depthWriteEnable = description.DepthStencilState.DepthWriteEnabled,
            depthCompareOp = description.DepthStencilState.DepthCompare.ToVk(),
            minDepthBounds = 0.0f,
            maxDepthBounds = 1.0f
        };
        if (_device.PhysicalDeviceFeatures2.features.depthBounds)
        {
            depthStencilState.depthBoundsTestEnable = description.DepthStencilState.DepthBoundsTestEnable;
        }
        else
        {
            depthStencilState.depthBoundsTestEnable = false;
        }

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
        VkPipelineRenderingCreateInfo renderingInfo = new();
        VkFormat* pColorAttachmentFormats = stackalloc VkFormat[description.ColorFormats.Length];
        VkPipelineColorBlendAttachmentState* blendAttachmentStates = stackalloc VkPipelineColorBlendAttachmentState[description.ColorFormats.Length];

        for (int i = 0; i < description.ColorFormats.Length; i++)
        {
            if (description.ColorFormats[i] == PixelFormat.Undefined)
                continue;

            ref readonly RenderTargetBlendState attachment = ref description.BlendState.RenderTargets[i];

            blendAttachmentStates[renderingInfo.colorAttachmentCount].blendEnable = GraphicsUtilities.BlendEnabled(in attachment);
            blendAttachmentStates[renderingInfo.colorAttachmentCount].srcColorBlendFactor = attachment.SourceColorBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].dstColorBlendFactor = attachment.DestinationColorBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].colorBlendOp = attachment.ColorBlendOperation.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].srcAlphaBlendFactor = attachment.SourceAlphaBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].dstAlphaBlendFactor = attachment.DestinationAlphaBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].alphaBlendOp = attachment.AlphaBlendOperation.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].colorWriteMask = attachment.ColorWriteMask.ToVk();

            pColorAttachmentFormats[renderingInfo.colorAttachmentCount] = _device.ToVkFormat(description.ColorFormats[i]);
            renderingInfo.colorAttachmentCount++;
        }

        VkPipelineColorBlendStateCreateInfo blendState = new()
        {
            logicOpEnable = false,
            logicOp = VkLogicOp.Clear,
            attachmentCount = renderingInfo.colorAttachmentCount,
            pAttachments = blendAttachmentStates
        };
        blendState.blendConstants[0] = 0.0f;
        blendState.blendConstants[1] = 0.0f;
        blendState.blendConstants[2] = 0.0f;
        blendState.blendConstants[3] = 0.0f;

        renderingInfo.pColorAttachmentFormats = pColorAttachmentFormats;
        renderingInfo.depthAttachmentFormat = _device.ToVkFormat(description.DepthStencilFormat);
        if (!description.DepthStencilFormat.IsDepthOnlyFormat())
        {
            renderingInfo.stencilAttachmentFormat = renderingInfo.depthAttachmentFormat;
        }

        // DynamicState
        int dynamicStateCount = 4;
        VkDynamicState* vkDynamicStates = stackalloc VkDynamicState[6] {
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

        VkPipelineDynamicStateCreateInfo dynamicState = new()
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
            pColorBlendState = &blendState,
            pDynamicState = &dynamicState,
            layout = _layout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        if (_device.PhysicalDeviceFeatures1_3.dynamicRendering)
        {
            createInfo.pNext = &renderingInfo;
        }
        else
        {

        }

        VkPipeline pipeline;
        result = vkCreateGraphicsPipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        for (int i = 0; i < shaderStageCount; i++)
        {
            vkDestroyShaderModule(device.Handle, shaderStages[i].module, null);
        }

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
        BindPoint = VkPipelineBindPoint.Compute;

        VkString entryPoint = new(description.ComputeShader.EntryPoint);

        VkPipelineShaderStageCreateInfo stage = new()
        {
            stage = VkShaderStageFlags.Compute,
            pName = entryPoint
        };

        VkResult result = vkCreateShaderModule(device.Handle, description.ComputeShader.ByteCode, null, out stage.module);
        if (result != VkResult.Success)
        {
            Log.Error("Failed to create a pipeline shader module");
            return;
        }

        VkComputePipelineCreateInfo createInfo = new()
        {
            stage = stage,
            layout = _layout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        result = vkCreateComputePipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        // Delete shader module.
        vkDestroyShaderModule(device.Handle, stage.module);

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
    public VkPipelineBindPoint BindPoint { get; }

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
