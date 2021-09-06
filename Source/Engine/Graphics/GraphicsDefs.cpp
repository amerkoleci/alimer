// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GraphicsDefs.h"

namespace Alimer
{
    uint32_t GetVertexFormatNumComponents(VertexElementFormat format)
    {
        switch (format)
        {
            case VertexElementFormat::Float:
            case VertexElementFormat::UInt:
            case VertexElementFormat::Int:
                return 1;

            case VertexElementFormat::UChar2:
            case VertexElementFormat::Char2:
            case VertexElementFormat::UChar2Norm:
            case VertexElementFormat::Char2Norm:
            case VertexElementFormat::UShort2:
            case VertexElementFormat::Short2:
            case VertexElementFormat::UShort2Norm:
            case VertexElementFormat::Short2Norm:
            case VertexElementFormat::Half2:
            case VertexElementFormat::Float2:
            case VertexElementFormat::UInt2:
            case VertexElementFormat::Int2:
                return 2;

            case VertexElementFormat::Float3:
            case VertexElementFormat::UInt3:
            case VertexElementFormat::Int3:
                return 3;

            case VertexElementFormat::UChar4:
            case VertexElementFormat::Char4:
            case VertexElementFormat::UChar4Norm:
            case VertexElementFormat::Char4Norm:
            case VertexElementFormat::UShort4:
            case VertexElementFormat::Short4:
            case VertexElementFormat::UShort4Norm:
            case VertexElementFormat::Short4Norm:
            case VertexElementFormat::Half4:
            case VertexElementFormat::Float4:
            case VertexElementFormat::UInt4:
            case VertexElementFormat::Int4:
            case VertexElementFormat::RGB10A2Unorm:
                return 4;

            default:
                ALIMER_UNREACHABLE();
                return 0;
        }
    }

    uint32_t GetVertexFormatComponentSize(VertexElementFormat format)
    {
        switch (format)
        {
            case VertexElementFormat::UChar2:
            case VertexElementFormat::UChar4:
            case VertexElementFormat::Char2:
            case VertexElementFormat::Char4:
            case VertexElementFormat::UChar2Norm:
            case VertexElementFormat::UChar4Norm:
            case VertexElementFormat::Char2Norm:
            case VertexElementFormat::Char4Norm:
                return sizeof(char);

            case VertexElementFormat::UShort2:
            case VertexElementFormat::UShort4:
            case VertexElementFormat::UShort2Norm:
            case VertexElementFormat::UShort4Norm:
            case VertexElementFormat::Short2:
            case VertexElementFormat::Short4:
            case VertexElementFormat::Short2Norm:
            case VertexElementFormat::Short4Norm:
            case VertexElementFormat::Half2:
            case VertexElementFormat::Half4:
                return sizeof(uint16_t);

            case VertexElementFormat::Float:
            case VertexElementFormat::Float2:
            case VertexElementFormat::Float3:
            case VertexElementFormat::Float4:
                return sizeof(float);
            case VertexElementFormat::UInt:
            case VertexElementFormat::UInt2:
            case VertexElementFormat::UInt3:
            case VertexElementFormat::UInt4:
            case VertexElementFormat::Int:
            case VertexElementFormat::Int2:
            case VertexElementFormat::Int3:
            case VertexElementFormat::Int4:
            case VertexElementFormat::RGB10A2Unorm:
                return sizeof(int32_t);

            default:
                ALIMER_UNREACHABLE();
        }
    }

    uint32_t GetVertexFormatSize(VertexElementFormat format)
    {
        return GetVertexFormatNumComponents(format) * GetVertexFormatComponentSize(format);
    }
}
