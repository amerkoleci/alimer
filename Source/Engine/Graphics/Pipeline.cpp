// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
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
			if (attribute->format == VertexFormat::Undefined) {
				continue;
			}

			ALIMER_ASSERT(attribute->bufferIndex < kMaxVertexBufferBindings);
			if (useAutoOffset) {
				attribute->offset = autoOffsets[attribute->bufferIndex];
			}

            const VertexFormatInfo& vertexFormatInfo = GetVertexFormatInfo(attribute->format);
			autoOffsets[attribute->bufferIndex] += vertexFormatInfo.byteSize;
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

    PipelineRef Pipeline::Create(const ComputePipelineDesc& desc)
	{
		ALIMER_ASSERT(gGraphics().IsInitialized());
		ALIMER_ASSERT(desc.shader != nullptr);
        ALIMER_ASSERT(desc.shader->GetStage() == ShaderStages::Compute);

		return gGraphics().CreateComputePipeline(&desc);
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
