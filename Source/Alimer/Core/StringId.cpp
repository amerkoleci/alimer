// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/StringId.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

namespace
{
    std::mutex s_HashToStringMapMutex;
    UnorderedMap<uint32_t, String> s_HashToStringMap;

    // Fast CRC32
    // https://create.stephan-brumme.com/crc32/#bitwise
    constexpr uint32_t crc32_bitwise(const char* data, size_t length)
    {
        constexpr uint32_t Polynomial = 0xEDB88320;
        uint32_t crc = 0;
        while (length--)
        {
            crc ^= *data++;

            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        }
        return ~crc;
    }
}

const StringId32 StringId32::Empty;

/* StringId32 */
StringId32::StringId32(const char* str) noexcept
    : hash(Hash(str))
{
    std::lock_guard lockGuard(s_HashToStringMapMutex);
    s_HashToStringMap[hash] = str;
}

StringId32::StringId32(const String& str) noexcept
    : hash(Hash(str.c_str()))
{
    std::lock_guard lockGuard(s_HashToStringMapMutex);
    s_HashToStringMap[hash] = str;
}

StringId32::StringId32(StringView str) noexcept
    : hash(Hash(str.data()))
{
    std::lock_guard lockGuard(s_HashToStringMapMutex);
    s_HashToStringMap[hash] = str;
}

StringId32::StringId32(const StringId32& other) noexcept
    : hash(other.hash)
{
}

uint32_t StringId32::Hash(const char* str)
{
    return crc32_bitwise(str, std::char_traits<char>::length(str));
}

String StringId32::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    sprintf(tempBuffer, "%08X", hash);
    return String(tempBuffer);
}

String StringId32::GetString() const
{
    auto it = s_HashToStringMap.find(hash);
    return it == s_HashToStringMap.end() ? "Unknown StringID" : it->second;
}
