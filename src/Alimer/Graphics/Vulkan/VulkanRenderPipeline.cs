// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Vortice.Vulkan.Vulkan;

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
        uint shaderStageCount = 0;
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
        uint vertexBufferLayoutsCount = 0;
        uint vertexAttributesCount = 0;
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
            vertexBindingDescriptionCount = vertexBufferLayoutsCount,
            pVertexBindingDescriptions = vertexBindings,
            vertexAttributeDescriptionCount = vertexAttributesCount,
            pVertexAttributeDescriptions = vertexAttributes
        };

        // InputAssemblyState
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = new()
        {
            topology = descriptor.PrimitiveTopology.ToVk(),
            primitiveRestartEnable = descriptor.PrimitiveTopology switch
            {
                PrimitiveTopology.LineStrip or PrimitiveTopology.TriangleStrip => true,
                _ => false,
            }
        };

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
            depthClipStateInfo.depthClipEnable = descriptor.RasterizerState.DepthClipMode == DepthClipMode.Clip;

            rasterizationState.depthClampEnable = true;
            rasterizationState.pNext = &depthClipStateInfo;

            // Requires {}
            *tail = &depthClipStateInfo;
            tail = &depthClipStateInfo.pNext;
        }

        VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeState = new();
        if (descriptor.RasterizerState.ConservativeRasterEnable)
        {
            rasterizationConservativeState.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
            //rasterizationConservativeState.extraPrimitiveOverestimationSize = 0.0f;

            *tail = &rasterizationConservativeState;
            tail = &rasterizationConservativeState.pNext;
        }

        rasterizationState.rasterizerDiscardEnable = false;

        switch (descriptor.RasterizerState.FillMode)
        {
            default:
            case FillMode.Solid:
                rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
                break;

            case FillMode.Wireframe:
                if (!_device.VkAdapter.Features2.features.fillModeNonSolid)
                {
                    Log.Warn("Vulkan: Wireframe fill mode is being used but it's not supported on this device");
                    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
                }

                rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
                break;
        }

        rasterizationState.cullMode = descriptor.RasterizerState.CullMode.ToVk();
        rasterizationState.frontFace = descriptor.RasterizerState.FrontFace.ToVk();
        // Can be managed by command buffer
        rasterizationState.depthBiasEnable = descriptor.RasterizerState.DepthBias != 0.0f || descriptor.RasterizerState.DepthBiasSlopeScale != 0.0f;
        rasterizationState.depthBiasConstantFactor = descriptor.RasterizerState.DepthBias;
        rasterizationState.depthBiasClamp = descriptor.RasterizerState.DepthBiasClamp;
        rasterizationState.depthBiasSlopeFactor = descriptor.RasterizerState.DepthBiasSlopeScale;
        rasterizationState.lineWidth = 1.0f;

        // MultisampleState
        VkPipelineMultisampleStateCreateInfo multisampleState = new()
        {
            rasterizationSamples = descriptor.SampleCount.ToVk()
        };

        Debug.Assert((int)multisampleState.rasterizationSamples <= 32);
        if (multisampleState.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT)
        {
            multisampleState.alphaToCoverageEnable = descriptor.BlendState.AlphaToCoverageEnabled;
            multisampleState.alphaToOneEnable = false;
            multisampleState.sampleShadingEnable = false;
            multisampleState.minSampleShading = 1.0f;
        }
        //uint sampleMask = uint.MaxValue;
        multisampleState.pSampleMask = null; // &sampleMask;

        // DepthStencilState
        VkPipelineDepthStencilStateCreateInfo depthStencilState = default;
        if (descriptor.DepthStencilFormat != PixelFormat.Undefined)
        {
            depthStencilState = new()
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

            StencilFaceState stencilFront = descriptor.DepthStencilState.StencilFront;
            depthStencilState.stencilTestEnable = GraphicsUtilities.StencilTestEnabled(descriptor.DepthStencilState);
            depthStencilState.front.failOp = stencilFront.FailOperation.ToVk();
            depthStencilState.front.passOp = stencilFront.PassOperation.ToVk();
            depthStencilState.front.depthFailOp = stencilFront.DepthFailOperation.ToVk();
            depthStencilState.front.compareOp = stencilFront.CompareFunction.ToVk();
            depthStencilState.front.compareMask = descriptor.DepthStencilState.StencilReadMask;
            depthStencilState.front.writeMask = descriptor.DepthStencilState.StencilWriteMask;
            depthStencilState.front.reference = 0;


            StencilFaceState stencilBack = descriptor.DepthStencilState.StencilBack;
            depthStencilState.back.failOp = stencilBack.FailOperation.ToVk();
            depthStencilState.back.passOp = stencilBack.PassOperation.ToVk();
            depthStencilState.back.depthFailOp = stencilBack.DepthFailOperation.ToVk();
            depthStencilState.back.compareOp = stencilBack.CompareFunction.ToVk();
            depthStencilState.back.compareMask = descriptor.DepthStencilState.StencilReadMask;
            depthStencilState.back.writeMask = descriptor.DepthStencilState.StencilWriteMask;
            depthStencilState.back.reference = 0;
        }

        // BlendState
        uint colorAttachmentCount = 0;
        VkFormat* pColorAttachmentFormats = stackalloc VkFormat[descriptor.ColorFormats.Length];
        VkPipelineColorBlendAttachmentState* blendAttachmentStates = stackalloc VkPipelineColorBlendAttachmentState[descriptor.ColorFormats.Length];

        for (int i = 0; i < descriptor.ColorFormats.Length; i++)
        {
            if (descriptor.ColorFormats[i] == PixelFormat.Undefined)
                continue;

            int attachmentIndex = 0;
            if (descriptor.BlendState.IndependentBlendEnable)
                attachmentIndex = i;

            ref readonly RenderTargetBlendState attachment = ref descriptor.BlendState.RenderTargets[attachmentIndex];

            blendAttachmentStates[colorAttachmentCount].blendEnable = GraphicsUtilities.BlendEnabled(in attachment);
            blendAttachmentStates[colorAttachmentCount].srcColorBlendFactor = attachment.SourceColorBlendFactor.ToVk();
            blendAttachmentStates[colorAttachmentCount].dstColorBlendFactor = attachment.DestinationColorBlendFactor.ToVk();
            blendAttachmentStates[colorAttachmentCount].colorBlendOp = attachment.ColorBlendOperation.ToVk();
            blendAttachmentStates[colorAttachmentCount].srcAlphaBlendFactor = attachment.SourceAlphaBlendFactor.ToVk();
            blendAttachmentStates[colorAttachmentCount].dstAlphaBlendFactor = attachment.DestinationAlphaBlendFactor.ToVk();
            blendAttachmentStates[colorAttachmentCount].alphaBlendOp = attachment.AlphaBlendOperation.ToVk();
            blendAttachmentStates[colorAttachmentCount].colorWriteMask = attachment.ColorWriteMask.ToVk();

            pColorAttachmentFormats[colorAttachmentCount] = _device.ToVkFormat(descriptor.ColorFormats[i]);
            colorAttachmentCount++;
        }

        VkPipelineColorBlendStateCreateInfo blendState = new()
        {
            logicOpEnable = false,
            logicOp = VK_LOGIC_OP_CLEAR,
            attachmentCount = colorAttachmentCount,
            pAttachments = blendAttachmentStates
        };
        blendState.blendConstants[0] = 0.0f;
        blendState.blendConstants[1] = 0.0f;
        blendState.blendConstants[2] = 0.0f;
        blendState.blendConstants[3] = 0.0f;

        VkFormat depthStencilFormat = _device.VkAdapter.ToVkFormat(descriptor.DepthStencilFormat);
        VkPipelineRenderingCreateInfo renderingInfo = new()
        {
            colorAttachmentCount = colorAttachmentCount,
            pColorAttachmentFormats = pColorAttachmentFormats,
            depthAttachmentFormat = depthStencilFormat,
            stencilAttachmentFormat = !descriptor.DepthStencilFormat.IsDepthOnlyFormat() ? depthStencilFormat : VK_FORMAT_UNDEFINED
        };

        VkPipelineDynamicStateCreateInfo dynamicState = device.DynamicStateInfo;

        VkGraphicsPipelineCreateInfo createInfo = new()
        {
            pNext = &renderingInfo,
            stageCount = shaderStageCount,
            pStages = shaderStages,
            pVertexInputState = &vertexInputState,
            pInputAssemblyState = &inputAssemblyState,
            pTessellationState = null,
            pViewportState = &viewportState,
            pRasterizationState = &rasterizationState,
            pMultisampleState = &multisampleState,
            pDepthStencilState = (descriptor.DepthStencilFormat == PixelFormat.Undefined) ? null : &depthStencilState,
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
