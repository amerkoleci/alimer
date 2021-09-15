// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/StringId.h"

namespace Alimer
{
    const StringId32 StringId32::Zero;

    /* StringId32 */
    StringId32::StringId32(const char* str) noexcept
    {
        value = static_cast<uint32_t>(StringHash(str));
    }

    StringId32::StringId32(const std::string& str) noexcept
    {
        value = static_cast<uint32_t>(StringHash(str.c_str()));
    }

    std::string StringId32::ToString() const
    {
        char tempBuffer[kConversionBufferLength];
        sprintf(tempBuffer, "%08X", value);
        return std::string(tempBuffer);
    }
}
