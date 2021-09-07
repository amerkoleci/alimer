// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GraphicsDefs.h"

namespace Alimer
{
    uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth)
    {
        uint32_t numMips = 0;
        uint32_t size = std::max(std::max(width, height), depth);
        while (1u << numMips <= size)
        {
            ++numMips;
        }

        if (1u << numMips < size)
        {
            ++numMips;
        }

        return numMips;
    }

    uint32_t GetVertexFormatNumComponents(VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat::Float:
            case VertexFormat::UInt:
            case VertexFormat::Int:
                return 1;

            case VertexFormat::UChar2:
            case VertexFormat::Char2:
            case VertexFormat::UChar2Norm:
            case VertexFormat::Char2Norm:
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

            case VertexFormat::UChar4:
            case VertexFormat::Char4:
            case VertexFormat::UChar4Norm:
            case VertexFormat::Char4Norm:
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
            case VertexFormat::UChar2:
            case VertexFormat::UChar4:
            case VertexFormat::Char2:
            case VertexFormat::Char4:
            case VertexFormat::UChar2Norm:
            case VertexFormat::UChar4Norm:
            case VertexFormat::Char2Norm:
            case VertexFormat::Char4Norm:
                return sizeof(char);

            case VertexFormat::UShort2:
            case VertexFormat::UShort4:
            case VertexFormat::UShort2Norm:
            case VertexFormat::UShort4Norm:
            case VertexFormat::Short2:
            case VertexFormat::Short4:
            case VertexFormat::Short2Norm:
            case VertexFormat::Short4Norm:
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

    const char* ToString(CompareFunction func)
    {
        switch (func)
        {
            case CompareFunction::Never:        return "Never";
            case CompareFunction::Less:         return "Less";
            case CompareFunction::Equal:        return "Equal";
            case CompareFunction::LessEqual:    return "LessEqual";
            case CompareFunction::Greater:      return "Greater";
            case CompareFunction::NotEqual:     return "NotEqual";
            case CompareFunction::GreaterEqual: return "GreaterEqual";
            case CompareFunction::Always:       return "Always";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }
}
