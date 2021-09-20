// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanGraphics.h"

namespace Alimer
{
	namespace
	{
		[[nodiscard]] constexpr VkPrimitiveTopology ToVulkan(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::PointList:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveTopology::LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::LineStrip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopology::TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			default:
				ALIMER_UNREACHABLE();
				return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
			}
		}

		[[nodiscard]] constexpr VkBool32 EnablePrimitiveRestart(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::LineStrip:
			case PrimitiveTopology::TriangleStrip:
				return true;
			default:
				return false;
			}
		}

		[[nodiscard]] constexpr VkVertexInputRate ToVulkan(VertexStepRate stepMode)
		{
			switch (stepMode)
			{
			case VertexStepRate::Instance:
				return VK_VERTEX_INPUT_RATE_INSTANCE;
			default:
				return VK_VERTEX_INPUT_RATE_VERTEX;
			}
		}

		[[nodiscard]] constexpr VkFormat ToVulkan(VertexFormat format)
		{
			switch (format)
			{
            case VertexFormat::Uint8x2:
                return VK_FORMAT_R8G8_UINT;
            case VertexFormat::Uint8x4:
                return VK_FORMAT_R8G8B8A8_UINT;
            case VertexFormat::Sint8x2:
                return VK_FORMAT_R8G8_SINT;
            case VertexFormat::Sint8x4:
                return VK_FORMAT_R8G8B8A8_SINT;
            case VertexFormat::Unorm8x2:
                return VK_FORMAT_R8G8_UNORM;
            case VertexFormat::Unorm8x4:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case VertexFormat::Snorm8x2:
                return VK_FORMAT_R8G8_SNORM;
            case VertexFormat::Snorm8x4:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case VertexFormat::Uint16x2:
                return VK_FORMAT_R16G16_UINT;
            case VertexFormat::Uint16x4:
                return VK_FORMAT_R16G16B16A16_UINT;
            case VertexFormat::Sint16x2:
                return VK_FORMAT_R16G16_SINT;
            case VertexFormat::Sint16x4:
                return VK_FORMAT_R16G16B16A16_SINT;
            case VertexFormat::Unorm16x2:
                return VK_FORMAT_R16G16_UNORM;
            case VertexFormat::Unorm16x4:
                return VK_FORMAT_R16G16B16A16_UNORM;
            case VertexFormat::Snorm16x2:
                return VK_FORMAT_R16G16_SNORM;
            case VertexFormat::Snorm16x4:
                return VK_FORMAT_R16G16B16A16_SNORM;
            case VertexFormat::Float16x2:
                return VK_FORMAT_R16G16_SFLOAT;
            case VertexFormat::Float16x4:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case VertexFormat::Float32:
                return VK_FORMAT_R32_SFLOAT;
            case VertexFormat::Float32x2:
                return VK_FORMAT_R32G32_SFLOAT;
            case VertexFormat::Float32x3:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case VertexFormat::Float32x4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case VertexFormat::Uint32:
                return VK_FORMAT_R32_UINT;
            case VertexFormat::Uint32x2:
                return VK_FORMAT_R32G32_UINT;
            case VertexFormat::Uint32x3:
                return VK_FORMAT_R32G32B32_UINT;
            case VertexFormat::Uint32x4:
                return VK_FORMAT_R32G32B32A32_UINT;
            case VertexFormat::Sint32:
                return VK_FORMAT_R32_SINT;
            case VertexFormat::Sint32x2:
                return VK_FORMAT_R32G32_SINT;
            case VertexFormat::Sint32x3:
                return VK_FORMAT_R32G32B32_SINT;
            case VertexFormat::Sint32x4:
                return VK_FORMAT_R32G32B32A32_SINT;

			default:
				ALIMER_UNREACHABLE();
			}
		}

		[[nodiscard]] constexpr VkPolygonMode ToVulkan(FillMode mode)
		{
			switch (mode)
			{
			default:
			case FillMode::Solid:
				return VK_POLYGON_MODE_FILL;
			case FillMode::Wireframe:
				return VK_POLYGON_MODE_LINE;
			}
		}

		[[nodiscard]] constexpr VkCullModeFlagBits ToVulkan(CullMode mode)
		{
			switch (mode)
			{
			default:
			case CullMode::None:
				return VK_CULL_MODE_NONE;
			case CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			}
		}

		[[nodiscard]] constexpr VkFrontFace ToVulkan(FaceWinding winding)
		{
			switch (winding)
			{
			default:
			case FaceWinding::Clockwise:
				return VK_FRONT_FACE_CLOCKWISE;
			case FaceWinding::CounterClockwise:
				return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			}
		}

		[[nodiscard]] constexpr VkStencilOp ToVulkan(StencilOperation op) {
			switch (op) {
			case StencilOperation::Keep:
				return VK_STENCIL_OP_KEEP;
			case StencilOperation::Zero:
				return VK_STENCIL_OP_ZERO;
			case StencilOperation::Replace:
				return VK_STENCIL_OP_REPLACE;
			case StencilOperation::IncrementClamp:
				return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case StencilOperation::DecrementClamp:
				return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case StencilOperation::Invert:
				return VK_STENCIL_OP_INVERT;
			case StencilOperation::IncrementWrap:
				return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case StencilOperation::DecrementWrap:
				return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			default:
				ALIMER_UNREACHABLE();
			}
		}

		[[nodiscard]] constexpr VkBlendFactor ToVulkan(BlendFactor factor)
		{
			switch (factor) {
			case BlendFactor::Zero:
				return VK_BLEND_FACTOR_ZERO;
			case BlendFactor::One:
				return VK_BLEND_FACTOR_ONE;
			case BlendFactor::SourceColor:
				return VK_BLEND_FACTOR_SRC_COLOR;
			case BlendFactor::OneMinusSourceColor:
				return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::SourceAlpha:
				return VK_BLEND_FACTOR_SRC_ALPHA;
			case BlendFactor::OneMinusSourceAlpha:
				return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::DestinationColor:
				return VK_BLEND_FACTOR_DST_COLOR;
			case BlendFactor::OneMinusDestinationColor:
				return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::DestinationAlpha:
				return VK_BLEND_FACTOR_DST_ALPHA;
			case BlendFactor::OneMinusDestinationAlpha:
				return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case BlendFactor::SourceAlphaSaturated:
				return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			case BlendFactor::BlendColor:
				return VK_BLEND_FACTOR_CONSTANT_COLOR;
			case BlendFactor::OneMinusBlendColor:
				return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactor::Source1Color:
				return VK_BLEND_FACTOR_SRC1_COLOR;
			case BlendFactor::OneMinusSource1Color:
				return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
			case BlendFactor::Source1Alpha:
				return VK_BLEND_FACTOR_SRC1_ALPHA;
			case BlendFactor::OneMinusSource1Alpha:
				return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
			default:
				ALIMER_UNREACHABLE();
			}
		}

		[[nodiscard]] constexpr VkBlendOp ToVulkan(BlendOperation operation) {
			switch (operation) {
			case BlendOperation::Add:
				return VK_BLEND_OP_ADD;
			case BlendOperation::Subtract:
				return VK_BLEND_OP_SUBTRACT;
			case BlendOperation::ReverseSubtract:
				return VK_BLEND_OP_REVERSE_SUBTRACT;
			case BlendOperation::Min:
				return VK_BLEND_OP_MIN;
			case BlendOperation::Max:
				return VK_BLEND_OP_MAX;
			default:
				ALIMER_UNREACHABLE();
			}
		}

		VkColorComponentFlags constexpr ToVulkan(ColorWriteMask mask)
		{
			static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Red) == VK_COLOR_COMPONENT_R_BIT, "Vulkan ColorWriteMask mismatch");
			static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Green) == VK_COLOR_COMPONENT_G_BIT, "Vulkan ColorWriteMask mismatch");
			static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Blue) == VK_COLOR_COMPONENT_B_BIT, "Vulkan ColorWriteMask mismatch");
			static_assert(static_cast<VkColorComponentFlagBits>(ColorWriteMask::Alpha) == VK_COLOR_COMPONENT_A_BIT, "Vulkan ColorWriteMask mismatch");
			return static_cast<VkColorComponentFlags>(mask);
		}
	}

	VulkanPipeline::VulkanPipeline(VulkanGraphics& device_, const RenderPipelineStateCreateInfo* info)
		: Pipeline(Type::RenderPipeline)
		, device(device_)
		, bindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		std::vector<VulkanShader*> shaders;
		shaders.push_back(ToVulkan(info->vertexShader));

		if (info->pixelShader != nullptr)
		{
			shaders.push_back(ToVulkan(info->pixelShader));
		}

		pipelineLayout = &device.RequestPipelineLayout(shaders);

		uint32_t vertexBindingCount = 0;
		uint32_t vertexAttributeCount = 0;
		VkVertexInputBindingDescription vertexBindings[kMaxVertexBufferBindings];
		VkVertexInputAttributeDescription vertexAttributes[kMaxVertexAttributes];

		for (uint32_t index = 0; index < kMaxVertexBufferBindings; ++index)
		{
			if (!info->vertexLayout.buffers[index].stride)
				break;

			const VertexBufferLayout* layout = &info->vertexLayout.buffers[index];
			VkVertexInputBindingDescription* bindingDesc = &vertexBindings[vertexBindingCount++];
			bindingDesc->binding = index;
			bindingDesc->stride = layout->stride;
			bindingDesc->inputRate = ToVulkan(layout->stepRate);
		}

		for (uint32_t index = 0; index < kMaxVertexAttributes; index++) {
			const VertexAttribute* attribute = &info->vertexLayout.attributes[index];
			if (attribute->format == VertexFormat::Undefined) {
				continue;
			}

			VkVertexInputAttributeDescription* attributeDesc = &vertexAttributes[vertexAttributeCount++];
			attributeDesc->location = index;
			attributeDesc->binding = attribute->bufferIndex;
			attributeDesc->format = ToVulkan(attribute->format);
			attributeDesc->offset = attribute->offset;
		}

		VulkanRenderPassKey renderPassKey;
		renderPassKey.depthStencilAttachment.format = info->depthStencilFormat;
		renderPassKey.depthStencilAttachment.loadAction = LoadAction::Discard;
		renderPassKey.depthStencilAttachment.storeAction = StoreAction::Discard;
        renderPassKey.sampleCount = info->sampleCount;

		// BlendState
		std::array<VkPipelineColorBlendAttachmentState, kMaxColorAttachments> blendAttachmentState = {};

		VkPipelineColorBlendStateCreateInfo blendState{};
		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_CLEAR;
		blendState.attachmentCount = 0;
		blendState.pAttachments = blendAttachmentState.data();

		for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
		{
            if (info->colorFormats[i] == PixelFormat::Undefined)
                break;

            uint32_t rtIndex = 0;
            if (info->blendState.independentBlendEnable)
                rtIndex = i;

			const RenderTargetBlendState& renderTarget = info->blendState.renderTargets[rtIndex];

			blendAttachmentState[blendState.attachmentCount].blendEnable = EnableBlend(renderTarget) ? VK_TRUE : VK_FALSE;
			blendAttachmentState[blendState.attachmentCount].srcColorBlendFactor = ToVulkan(renderTarget.srcBlend);
			blendAttachmentState[blendState.attachmentCount].dstColorBlendFactor = ToVulkan(renderTarget.destBlend);
			blendAttachmentState[blendState.attachmentCount].colorBlendOp = ToVulkan(renderTarget.blendOperation);
			blendAttachmentState[blendState.attachmentCount].srcAlphaBlendFactor = ToVulkan(renderTarget.srcBlendAlpha);
			blendAttachmentState[blendState.attachmentCount].dstAlphaBlendFactor = ToVulkan(renderTarget.destBlendAlpha);
			blendAttachmentState[blendState.attachmentCount].alphaBlendOp = ToVulkan(renderTarget.blendOperationAlpha);
			blendAttachmentState[blendState.attachmentCount].colorWriteMask = ToVulkan(renderTarget.writeMask);
			blendState.attachmentCount++;

			renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].format = info->colorFormats[i];
			renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].loadAction = LoadAction::Discard;
			renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].storeAction = StoreAction::Store;
			renderPassKey.colorAttachmentCount++;
		}

		// Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.pNext = nullptr;
		rasterizationState.flags = 0u;
		rasterizationState.depthClampEnable = VK_TRUE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = ToVulkan(info->rasterizerState.fillMode);
		rasterizationState.cullMode = ToVulkan(info->rasterizerState.cullMode);
		rasterizationState.frontFace = ToVulkan(info->rasterizerState.frontFace);
		rasterizationState.depthBiasEnable = info->rasterizerState.depthBias != 0 || info->rasterizerState.depthBiasSlopeScale != 0;
		rasterizationState.depthBiasConstantFactor = info->rasterizerState.depthBias;
		rasterizationState.depthBiasClamp = info->rasterizerState.depthBiasClamp;
		rasterizationState.depthBiasSlopeFactor = info->rasterizerState.depthBiasSlopeScale;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.flags = 0;
		// Depth writes only occur if depth is enabled
		depthStencilState.depthTestEnable = (info->depthStencilState.depthCompare != CompareFunction::Always || info->depthStencilState.depthWriteEnabled)
			? VK_TRUE
			: VK_FALSE;

		depthStencilState.depthWriteEnable = info->depthStencilState.depthWriteEnabled ? VK_TRUE : VK_FALSE;
		depthStencilState.depthCompareOp = ToVkCompareOp(info->depthStencilState.depthCompare);
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = StencilTestEnabled(&info->depthStencilState) ? VK_TRUE : VK_FALSE;

		depthStencilState.front.failOp = ToVulkan(info->depthStencilState.frontFace.failOperation);
		depthStencilState.front.passOp = ToVulkan(info->depthStencilState.frontFace.passOperation);
		depthStencilState.front.depthFailOp = ToVulkan(info->depthStencilState.frontFace.depthFailOperation);
		depthStencilState.front.compareOp = ToVkCompareOp(info->depthStencilState.frontFace.compareFunction);
		depthStencilState.front.compareMask = info->depthStencilState.stencilReadMask;
		depthStencilState.front.writeMask = info->depthStencilState.stencilWriteMask;
		depthStencilState.front.reference = 0; // The stencil reference is always dynamic

		depthStencilState.back.failOp = ToVulkan(info->depthStencilState.backFace.failOperation);
		depthStencilState.back.passOp = ToVulkan(info->depthStencilState.backFace.passOperation);
		depthStencilState.back.depthFailOp = ToVulkan(info->depthStencilState.backFace.depthFailOperation);
		depthStencilState.back.compareOp = ToVkCompareOp(info->depthStencilState.backFace.compareFunction);
		depthStencilState.back.compareMask = info->depthStencilState.stencilReadMask;
		depthStencilState.back.writeMask = info->depthStencilState.stencilWriteMask;
		depthStencilState.back.reference = 0; // The stencil reference is always dynamic

		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		for (VulkanShader* shader : shaders)
		{
			VkPipelineShaderStageCreateInfo stageCreateInfo{};
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = (VkShaderStageFlagBits)ToVulkan(shader->GetStage());
			stageCreateInfo.module = shader->handle;
			stageCreateInfo.pName = shader->GetEntryPoint().c_str();
			stageCreateInfo.pSpecializationInfo = nullptr;
			stages.push_back(stageCreateInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputState.vertexBindingDescriptionCount = vertexBindingCount;
		vertexInputState.pVertexBindingDescriptions = vertexBindings;
		vertexInputState.vertexAttributeDescriptionCount = vertexAttributeCount;
		vertexInputState.pVertexAttributeDescriptions = vertexAttributes;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.topology = ToVulkan(info->primitiveTopology);
		inputAssemblyState.primitiveRestartEnable = EnablePrimitiveRestart(info->primitiveTopology);

		VkPipelineTessellationStateCreateInfo tessellationState;
		tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = nullptr;
        tessellationState.flags = 0;
        tessellationState.patchControlPoints = info->patchControlPoints;

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pNext = nullptr;
		multisampleState.flags = 0;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // static_cast<VkSampleCountFlagBits>(info.sampleCount);
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 0.0f;
		//ALIMER_ASSERT(multisampleState.rasterizationSamples <= 32);
		//VkSampleMask sampleMask = info->sampleMask;
		//multisampleState.pSampleMask = &sampleMask;
		multisampleState.pSampleMask = nullptr;
		multisampleState.alphaToCoverageEnable = info->blendState.alphaToCoverageEnable;
		multisampleState.alphaToOneEnable = VK_FALSE;

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = static_cast<uint32_t>(stages.size());
		createInfo.pStages = stages.data();
		createInfo.pVertexInputState = &vertexInputState;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pTessellationState = &tessellationState;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pColorBlendState = &blendState;
		createInfo.pDynamicState = &device.dynamicStateInfo;
		createInfo.layout = pipelineLayout->GetHandle();
		createInfo.renderPass = device.GetVkRenderPass(renderPassKey);
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = 0;

		VkResult result = vkCreateGraphicsPipelines(device.GetHandle(), VK_NULL_HANDLE,
			1, &createInfo,
			nullptr,
			&handle);

		if (result != VK_SUCCESS)
		{
			VK_LOG_ERROR(result, "Failed to create pipeline");
			return;
		}

        //OnCreated();

		if (info->label != nullptr)
		{
			device.SetObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)handle, info->label);
		}
	}

	VulkanPipeline::VulkanPipeline(VulkanGraphics& device_, const ComputePipelineCreateInfo* info)
		: Pipeline(Type::ComputePipeline)
		, device(device_)
		, bindPoint(VK_PIPELINE_BIND_POINT_COMPUTE)
	{
		VulkanShader* shader = ToVulkan(info->shader);
		pipelineLayout = &device.RequestPipelineLayout({ shader });

		VkComputePipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage.stage = (VkShaderStageFlagBits)ToVulkan(shader->GetStage());
		createInfo.stage.module = shader->handle;
		createInfo.stage.pName = shader->GetEntryPoint().c_str();
		createInfo.stage.pSpecializationInfo = nullptr;
		createInfo.layout = pipelineLayout->GetHandle();

		VkResult result = vkCreateComputePipelines(device.GetHandle(), VK_NULL_HANDLE,
			1, &createInfo,
			nullptr, &handle);

		if (result != VK_SUCCESS)
		{
			VK_LOG_ERROR(result, "Failed to create compute pipeline");
			return;
		}

        //OnCreated();

		if (info->label != nullptr)
		{
			device.SetObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)handle, info->label);
		}
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Destroy();
	}

	void VulkanPipeline::Destroy()
	{
		if (handle != VK_NULL_HANDLE)
		{
			device.DeferDestroy(handle);
			handle = VK_NULL_HANDLE;
		}

        //OnDestroyed();
	}
}

