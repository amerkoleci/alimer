// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Pipeline.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Pipeline::Pipeline(Type type)
        : type{ type }
    {
    }


    PipelineRef Pipeline::Create(const RenderPipelineDesc& desc)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        RenderPipelineDesc descDef = desc;

        uint32_t autoOffsets[kMaxVertexBufferBindings] = {};

        bool useAutoOffset = true;
        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            // To use computed offsets, all attribute offsets must be 0.
            if (desc.vertexLayout.attributes[index].offset != 0) {
                useAutoOffset = false;
                break;
            }
        }

        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            VertexAttribute* attribute = &descDef.vertexLayout.attributes[index];
            if (attribute->format == VertexFormat::Undefined) {
                continue;
            }

            ALIMER_ASSERT(attribute->bufferIndex < kMaxVertexBufferBindings);
            if (useAutoOffset) {
                attribute->offset = autoOffsets[attribute->bufferIndex];
            }
            autoOffsets[attribute->bufferIndex] += GetVertexFormatSize(attribute->format);
        }

        // Compute vertex strides if needed.
        for (uint32_t index = 0; index < kMaxVertexBufferBindings; index++)
        {
            VertexBufferLayout* layout = &descDef.vertexLayout.buffers[index];
            if (layout->stride == 0) {
                layout->stride = autoOffsets[index];
            }
        }

        return gGraphics().CreateRenderPipeline(descDef);
    }

    bool StencilTestEnabled(const DepthStencilState* depthStencil)
    {
        return depthStencil->backFace.compare != CompareFunction::Always ||
            depthStencil->backFace.failOp != StencilOperation::Keep ||
            depthStencil->backFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->backFace.passOp != StencilOperation::Keep ||
            depthStencil->frontFace.compare != CompareFunction::Always ||
            depthStencil->frontFace.failOp != StencilOperation::Keep ||
            depthStencil->frontFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->frontFace.passOp != StencilOperation::Keep;
    }
}
