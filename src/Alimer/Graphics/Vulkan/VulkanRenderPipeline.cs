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

internal unsafe class VulkanRenderPipeline : RenderPipeline
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipeline _handle = VkPipeline.Null;

    public VulkanRenderPipeline(VulkanGraphicsDevice device, in RenderPipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        VkLayout = (VulkanPipelineLayout)descriptor.Layout;

        // ShaderStages
        int shaderStageCount = 0;
        // Mesh Pipeline (D3DX12_MESH_SHADER_PIPELINE_STATE_DESC)
        VkPipelineShaderStageCreateInfo* shaderStages = stackalloc VkPipelineShaderStageCreateInfo[3];
        if (descriptor.MeshShader is not null)
        {
            shaderStages[shaderStageCount++] = ((VulkanShaderModule)descriptor.MeshShader!).StageInfo;

            if (descriptor.AmplificationShader != null)
            {
                shaderStages[shaderStageCount++] = ((VulkanShaderModule)descriptor.AmplificationShader!).StageInfo;
            }
        }
        else
        {
            shaderStages[shaderStageCount++] = ((VulkanShaderModule)descriptor.VertexShader!).StageInfo;
        }

        if (descriptor.FragmentShader != null)
        {
            shaderStages[shaderStageCount++] = ((VulkanShaderModule)descriptor.FragmentShader!).StageInfo;
        }

        // VertexInputState
        int vertexBufferLayoutsCount = 0;
        int vertexAttributesCount = 0;
        VkVertexInputBindingDescription* vertexBindings = stackalloc VkVertexInputBindingDescription[descriptor.VertexBufferLayouts.Length];
        VkVertexInputAttributeDescription* vertexAttributes = stackalloc VkVertexInputAttributeDescription[MaxVertexAttributes];

        for (uint binding = 0; binding < descriptor.VertexBufferLayouts.Length; binding++)
        {
            ref readonly VertexBufferLayout layout = ref descriptor.VertexBufferLayouts[binding];

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
        inputAssemblyState.topology = descriptor.PrimitiveTopology.ToVk();
        switch (descriptor.PrimitiveTopology)
        {
            case PrimitiveTopology.LineStrip:
            case PrimitiveTopology.TriangleStrip:
                inputAssemblyState.primitiveRestartEnable = true;
                break;
            default:
                inputAssemblyState.primitiveRestartEnable = false;
                break;
        }

        // ViewportState
        VkPipelineViewportStateCreateInfo viewportState = new()
        {
            viewportCount = 1,
            scissorCount = 1
        };

        // RasterizationState
        VkPipelineRasterizationStateCreateInfo rasterizationState = new()
        {
            depthClampEnable = descriptor.RasterizerState.DepthClipMode == DepthClipMode.Clamp
        };

        VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipStateInfo = new();
        void** tail = &rasterizationState.pNext;
        if (device.VkAdapter.DepthClipEnableFeatures.depthClipEnable)
        {
            rasterizationState.depthClampEnable = true;
            depthClipStateInfo.depthClipEnable = descriptor.RasterizerState.DepthClipMode == DepthClipMode.Clip;

            *tail = &depthClipStateInfo;
            tail = &depthClipStateInfo.pNext;
        }

        VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeState = new();
        if (descriptor.RasterizerState.ConservativeRaster)
        {
            rasterizationConservativeState.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
            rasterizationConservativeState.extraPrimitiveOverestimationSize = 0.0f;

            *tail = &rasterizationConservativeState;
            tail = &rasterizationConservativeState.pNext;
        }

        rasterizationState.rasterizerDiscardEnable = false;
        rasterizationState.polygonMode = VkPolygonMode.Fill; // ToVk(features2, desc.rasterizerState.fillMode);
        rasterizationState.cullMode = descriptor.RasterizerState.CullMode.ToVk();
        rasterizationState.frontFace = (descriptor.RasterizerState.FrontFace == FrontFace.Clockwise) ? VkFrontFace.Clockwise : VkFrontFace.CounterClockwise;
        // Can be managed by command buffer
        //rasterizationState.depthBiasEnable = desc.rasterizerState.depthBias != 0.0f || desc.rasterizerState.slopeScaledDepthBias != 0.0f;
        //rasterizationState.depthBiasConstantFactor = desc.rasterizerState.depthBias;
        //rasterizationState.depthBiasClamp = desc.rasterizerState.depthBiasClamp;
        //rasterizationState.depthBiasSlopeFactor = desc.rasterizerState.slopeScaledDepthBias;
        rasterizationState.lineWidth = 1.0f;

        // MultisampleState
        VkPipelineMultisampleStateCreateInfo multisampleState = new()
        {
            rasterizationSamples = descriptor.SampleCount.ToVk()
        };

        Debug.Assert((int)multisampleState.rasterizationSamples <= 32);
        if (multisampleState.rasterizationSamples > VkSampleCountFlags.Count1)
        {
            multisampleState.alphaToCoverageEnable = descriptor.BlendState.AlphaToCoverageEnabled;
            multisampleState.alphaToOneEnable = false;
            multisampleState.sampleShadingEnable = false;
            multisampleState.minSampleShading = 1.0f;
        }
        uint sampleMask = uint.MaxValue;
        multisampleState.pSampleMask = &sampleMask;

        // DepthStencilState
        VkPipelineDepthStencilStateCreateInfo depthStencilState = new()
        {
            depthTestEnable = (descriptor.DepthStencilState.DepthCompare != CompareFunction.Always || descriptor.DepthStencilState.DepthWriteEnabled),
            depthWriteEnable = descriptor.DepthStencilState.DepthWriteEnabled,
            depthCompareOp = descriptor.DepthStencilState.DepthCompare.ToVk(),
            minDepthBounds = 0.0f,
            maxDepthBounds = 1.0f
        };
        if (_device.VkAdapter.Features2.features.depthBounds)
        {
            depthStencilState.depthBoundsTestEnable = descriptor.DepthStencilState.DepthBoundsTestEnable;
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
        VkFormat* pColorAttachmentFormats = stackalloc VkFormat[descriptor.ColorFormats.Length];
        VkPipelineColorBlendAttachmentState* blendAttachmentStates = stackalloc VkPipelineColorBlendAttachmentState[descriptor.ColorFormats.Length];

        for (int i = 0; i < descriptor.ColorFormats.Length; i++)
        {
            if (descriptor.ColorFormats[i] == PixelFormat.Undefined)
                continue;

            ref readonly RenderTargetBlendState attachment = ref descriptor.BlendState.RenderTargets[i];

            blendAttachmentStates[renderingInfo.colorAttachmentCount].blendEnable = GraphicsUtilities.BlendEnabled(in attachment);
            blendAttachmentStates[renderingInfo.colorAttachmentCount].srcColorBlendFactor = attachment.SourceColorBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].dstColorBlendFactor = attachment.DestinationColorBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].colorBlendOp = attachment.ColorBlendOperation.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].srcAlphaBlendFactor = attachment.SourceAlphaBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].dstAlphaBlendFactor = attachment.DestinationAlphaBlendFactor.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].alphaBlendOp = attachment.AlphaBlendOperation.ToVk();
            blendAttachmentStates[renderingInfo.colorAttachmentCount].colorWriteMask = attachment.ColorWriteMask.ToVk();

            pColorAttachmentFormats[renderingInfo.colorAttachmentCount] = _device.ToVkFormat(descriptor.ColorFormats[i]);
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
        renderingInfo.depthAttachmentFormat = _device.VkAdapter.ToVkFormat(descriptor.DepthStencilFormat);
        if (!descriptor.DepthStencilFormat.IsDepthOnlyFormat())
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
        if (_device.VkAdapter.Features2.features.depthBounds)
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
            pNext = &renderingInfo,
            stageCount = (uint)shaderStageCount,
            pStages = shaderStages,
            pVertexInputState = &vertexInputState,
            pInputAssemblyState = &inputAssemblyState,
            pTessellationState = null,
            pViewportState = &viewportState,
            pRasterizationState = &rasterizationState,
            pMultisampleState = &multisampleState,
            pDepthStencilState = &depthStencilState,
            pColorBlendState = &blendState,
            pDynamicState = &dynamicState,
            layout = VkLayout.Handle,
            basePipelineHandle = VkPipeline.Null,
            basePipelineIndex = 0
        };

        VkPipeline pipeline;
        VkResult result = _device.DeviceApi.vkCreateGraphicsPipelines(device.Handle, device.PipelineCache, 1, &createInfo, null, &pipeline);

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create Render Pipeline.");
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
    ~VulkanRenderPipeline() => Dispose(disposing: false);

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
