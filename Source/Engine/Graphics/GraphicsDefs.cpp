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
