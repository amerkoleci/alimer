// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	Pipeline::Pipeline(Type type)
		: type{ type }
	{

	}

    PipelineRef Pipeline::Create(const RenderPipelineStateCreateInfo& info)
	{
		ALIMER_ASSERT(gGraphics().IsInitialized());
		ALIMER_ASSERT(info.vertexShader != nullptr);

		RenderPipelineStateCreateInfo def = info;

		uint32_t autoOffsets[kMaxVertexBufferBindings] = {};

		bool useAutoOffset = true;
		for (uint32_t index = 0; index < kMaxVertexAttributes; index++) 
		{
			// To use computed offsets, all attribute offsets must be 0.
			if (info.vertexLayout.attributes[index].offset != 0) {
				useAutoOffset = false;
				break;
			}
		}

		for (uint32_t index = 0; index < kMaxVertexAttributes; index++) {
			VertexAttribute* attribute = &def.vertexLayout.attributes[index];
			if (attribute->format == VertexFormat::Invalid) {
				continue;
			}

			ALIMER_ASSERT(attribute->bufferIndex < kMaxVertexBufferBindings);
			if (useAutoOffset) {
				attribute->offset = autoOffsets[attribute->bufferIndex];
			}
			//autoOffsets[attribute->bufferIndex] += GetVertexFormatSize(attribute->format);
		}

		// Compute vertex strides if needed.
		for (uint32_t index = 0; index < kMaxVertexBufferBindings; index++) {
			VertexBufferLayout* layout = &def.vertexLayout.buffers[index];
			if (layout->stride == 0) {
				layout->stride = autoOffsets[index];
			}
		}

		return gGraphics().CreateRenderPipeline(&def);
	}

    PipelineRef Pipeline::Create(const ComputePipelineCreateInfo& info)
	{
		ALIMER_ASSERT(gGraphics().IsInitialized());
		ALIMER_ASSERT(info.shader != nullptr);

		return gGraphics().CreateComputePipeline(&info);
	}

	bool EnableBlend(const RenderTargetBlendState& state)
	{
		return state.blendOperation != BlendOperation::Add
			|| state.destBlend != BlendFactor::Zero
			|| state.srcBlend != BlendFactor::One
			|| state.blendOperationAlpha != BlendOperation::Add
			|| state.destBlendAlpha != BlendFactor::Zero
			|| state.srcBlendAlpha != BlendFactor::One;
	}

    bool StencilTestEnabled(const DepthStencilState* depthStencil)
    {
        return depthStencil->backFace.compareFunction != CompareFunction::Always ||
            depthStencil->backFace.failOperation != StencilOperation::Keep ||
            depthStencil->backFace.depthFailOperation != StencilOperation::Keep ||
            depthStencil->backFace.passOperation != StencilOperation::Keep ||
            depthStencil->frontFace.compareFunction != CompareFunction::Always ||
            depthStencil->frontFace.failOperation != StencilOperation::Keep ||
            depthStencil->frontFace.depthFailOperation != StencilOperation::Keep ||
            depthStencil->frontFace.passOperation != StencilOperation::Keep;
    }
}
