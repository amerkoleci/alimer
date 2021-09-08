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

    uint32_t GetVertexFormatNumComponents(VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat::UByte:
            case VertexFormat::Byte:
            case VertexFormat::UByteNorm:
            case VertexFormat::ByteNorm:
            case VertexFormat::UShort:
            case VertexFormat::Short:
            case VertexFormat::UShortNorm:
            case VertexFormat::ShortNorm:
            case VertexFormat::Half:
            case VertexFormat::Float:
            case VertexFormat::UInt:
            case VertexFormat::Int:
                return 1;

            case VertexFormat::UByte2:
            case VertexFormat::Byte2:
            case VertexFormat::UByte2Norm:
            case VertexFormat::Byte2Norm:
            case VertexFormat::UShort2:
            case VertexFormat::Short2:
            case VertexFormat::UShort2Norm:
            case VertexFormat::Short2Norm:
            case VertexFormat::Half2:
            case VertexFormat::Float2:
            case VertexFormat::UInt2:
            case VertexFormat::Int2:
                return 2;

            case VertexFormat::Float3:
            case VertexFormat::UInt3:
            case VertexFormat::Int3:
                return 3;

            case VertexFormat::UByte4:
            case VertexFormat::Byte4:
            case VertexFormat::UByte4Norm:
            case VertexFormat::Byte4Norm:
            case VertexFormat::UShort4:
            case VertexFormat::Short4:
            case VertexFormat::UShort4Norm:
            case VertexFormat::Short4Norm:
            case VertexFormat::Half4:
            case VertexFormat::Float4:
            case VertexFormat::UInt4:
            case VertexFormat::Int4:
            case VertexFormat::RGB10A2Unorm:
                return 4;

            default:
                ALIMER_UNREACHABLE();
                return 0;
        }
    }

    uint32_t GetVertexFormatComponentSize(VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat::UByte:
            case VertexFormat::UByte2:
            case VertexFormat::UByte4:
            case VertexFormat::Byte:
            case VertexFormat::Byte2:
            case VertexFormat::Byte4:
            case VertexFormat::UByteNorm:
            case VertexFormat::UByte2Norm:
            case VertexFormat::UByte4Norm:
            case VertexFormat::ByteNorm:
            case VertexFormat::Byte2Norm:
            case VertexFormat::Byte4Norm:
                return sizeof(char);

            case VertexFormat::UShort:
            case VertexFormat::UShort2:
            case VertexFormat::UShort4:
            case VertexFormat::Short:
            case VertexFormat::Short2:
            case VertexFormat::Short4:
            case VertexFormat::UShortNorm:
            case VertexFormat::UShort2Norm:
            case VertexFormat::UShort4Norm:
            case VertexFormat::ShortNorm:
            case VertexFormat::Short2Norm:
            case VertexFormat::Short4Norm:
            case VertexFormat::Half:
            case VertexFormat::Half2:
            case VertexFormat::Half4:
                return sizeof(uint16_t);

            case VertexFormat::Float:
            case VertexFormat::Float2:
            case VertexFormat::Float3:
            case VertexFormat::Float4:
                return sizeof(float);
            case VertexFormat::UInt:
            case VertexFormat::UInt2:
            case VertexFormat::UInt3:
            case VertexFormat::UInt4:
            case VertexFormat::Int:
            case VertexFormat::Int2:
            case VertexFormat::Int3:
            case VertexFormat::Int4:
            case VertexFormat::RGB10A2Unorm:
                return sizeof(int32_t);

            default:
                ALIMER_UNREACHABLE();
        }
    }

    uint32_t GetVertexFormatSize(VertexFormat format)
    {
        return GetVertexFormatNumComponents(format) * GetVertexFormatComponentSize(format);
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
