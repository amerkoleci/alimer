// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/String.h"
#include <cstdarg>
#include <cstring>
#include <sstream>

namespace Alimer
{
    namespace
    {
        StringVector SplitInternal(const std::string& str, const char* delim, bool allow_empty)
        {
            if (str.empty())
                return {};

            StringVector ret;

            size_t start_index = 0;
            size_t index = 0;
            while ((index = str.find_first_of(delim, start_index)) != std::string::npos)
            {
                if (allow_empty || index > start_index)
                    ret.push_back(str.substr(start_index, index - start_index));
                start_index = index + 1;

                if (allow_empty && (index == str.size() - 1))
                    ret.push_back(kEmptyString);
            }

            if (start_index < str.size())
                ret.push_back(str.substr(start_index));
            return ret;
        }
    }

    const String kEmptyString{};
    const StringView kEmptyStringView{};

    size_t CStringLength(const char* str)
    {
        return str ? strlen(str) : 0;
    }

    String ToLower(const String& str)
    {
        String result;
        for (const char& ch : str)
        {
            result += Alimer::ToLower(ch);
        }

        return result;
    }

    String ToUpper(const String& str)
    {
        String result;
        for (const char& ch : str)
        {
            result += Alimer::ToUpper(ch);
        }

        return result;
    }

    int Compare(const char* lhs, const char* rhs, bool caseSensitive)
    {
        if (!lhs || !rhs)
            return lhs ? 1 : (rhs ? -1 : 0);

        if (caseSensitive)
        {
            return strcmp(lhs, rhs);
        }
        else
        {
            for (;;)
            {
                auto l = (char)tolower(*lhs);
                auto r = (char)tolower(*rhs);
                if (!l || !r)
                    return l ? 1 : (r ? -1 : 0);
                if (l < r)
                    return -1;
                if (l > r)
                    return 1;

                ++lhs;
                ++rhs;
            }
        }
    }

    std::string ToString(const char* formatString, ...)
    {
        char formatBuffer[1024];
        va_list args;
        va_start(args, formatString);
        vsnprintf(formatBuffer, 1024, formatString, args);
        va_end(args);
        return std::string(formatBuffer);
    }

    std::string ToString(bool value)
    {
        return value ? "true" : "false";
    }

    std::string ToString(int16_t value)
    {
        return ToString("%d", value);
    }

    std::string ToString(int32_t value)
    {
        return ToString("%d", value);
    }

    std::string ToString(int64_t value)
    {
        return ToString("%lld", value);
    }

    std::string ToString(uint16_t value)
    {
        return ToString("%u", value);
    }

    std::string ToString(uint32_t value)
    {
        return ToString("%u", value);
    }

    std::string ToString(uint64_t value)
    {
        return ToString("%llu", value);
    }

    std::string ToString(float value)
    {
        return ToString("%g", value);
    }

    std::string ToString(double value)
    {
        return ToString("%.15g", value);
    }

    bool StringUtils::ToBool(const std::string& source)
    {
        return ToBool(source.c_str());
    }

    bool StringUtils::ToBool(const char* source)
    {
        size_t length = CStringLength(source);

        for (size_t i = 0; i < length; ++i)
        {
            auto c = Alimer::ToLower(source[i]);
            if (c == 't' || c == 'y' || c == '1')
                return true;
            else if (c != ' ' && c != '\t')
                break;
        }

        return false;
    }

    int32_t StringUtils::ToInt32(const std::string& source, int base)
    {
        return ToInt32(source.c_str(), base);
    }

    int32_t StringUtils::ToInt32(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return (int)strtol(source, nullptr, base);
    }

    uint32_t StringUtils::ToUInt32(const std::string& source, int base)
    {
        return ToUInt32(source.c_str(), base);
    }

    uint32_t StringUtils::ToUInt32(const char* source, int base)
    {
        if (!source)
            return 0;

        if (base < 2 || base > 36)
            base = 0;

        return (uint32_t)strtoul(source, nullptr, base);
    }

    int64_t StringUtils::ToInt64(const std::string& source, int base)
    {
        return ToInt64(source.c_str(), base);
    }

    int64_t StringUtils::ToInt64(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return strtoll(source, nullptr, base);
    }

    uint64_t StringUtils::ToUInt64(const std::string& source, int base)
    {
        return ToUInt64(source.c_str(), base);
    }

    uint64_t StringUtils::ToUInt64(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return strtoull(source, nullptr, base);
    }

    float StringUtils::ToFloat(const std::string& source)
    {
        return ToFloat(source.c_str());
    }

    float StringUtils::ToFloat(const char* source)
    {
        if (!source)
            return 0;

        return (float)strtod(source, nullptr);
    }

    double StringUtils::ToDouble(const std::string& source)
    {
        return ToDouble(source.c_str());
    }

    double StringUtils::ToDouble(const char* source)
    {
        if (!source)
            return 0;

        return strtod(source, nullptr);
    }

    size_t StringUtils::CountElements(StringView str, char separator)
    {
        return CountElements(str.data(), str.length(), separator);
    }

    size_t StringUtils::CountElements(const char* str, size_t length, char separator)
    {
        if (!str)
            return 0;

        if (!length)
            length = CStringLength(str);

        const char* endPos = str + length;
        const char* pos = str;
        size_t ret = 0;

        while (pos < endPos)
        {
            if (*pos != separator)
                break;
            ++pos;
        }

        while (pos < endPos)
        {
            const char* start = pos;

            while (start < endPos)
            {
                if (*start == separator)
                    break;

                ++start;
            }

            if (start == endPos)
            {
                ++ret;
                break;
            }

            const char* end = start;

            while (end < endPos)
            {
                if (*end != separator)
                    break;

                ++end;
            }

            ++ret;
            pos = end;
        }

        return ret;
    }

    std::string StringUtils::Trim(const std::string& str, bool left, bool right)
    {
        std::string result = str;
        static const std::string delims = " \t\r";
        if (right)
            result.erase(str.find_last_not_of(delims) + 1);  // trim right
        if (left)
            result.erase(0, str.find_first_not_of(delims));  // trim left
        return result;
    }

    std::string StringUtils::ReplaceAll(std::string_view source, const std::string& replaceWhat, const std::string& replaceWithWhat)
    {
        std::string result(source);
        std::string::size_type pos = 0;
        while (1)
        {
            pos = result.find(replaceWhat, pos);
            if (pos == std::string::npos) break;
            result.replace(pos, replaceWhat.size(), replaceWithWhat);
            pos += replaceWithWhat.size();
        }
        return result;
    }

    StringVector StringUtils::Split(const std::string& str, const char* delim)
    {
        return SplitInternal(str, delim, true);
    }

    StringVector StringUtils::SplitNoEmpty(const std::string& str, const char* delim)
    {
        return SplitInternal(str, delim, false);
    }

    String StringUtils::Join(const String& separator, const StringVector& values)
    {
        if (values.empty())
            return kEmptyString;

        std::stringstream ss;
        auto it = values.cbegin();
        while (true)
        {
            ss << *it++;
            if (it != values.cend())
                ss << separator;
            else
                return ss.str();
        }
    }

    uint32_t StringUtils::GetStringListIndex(const std::string& value, const std::string* strings, uint32_t defaultIndex, bool caseSensitive)
    {
        return GetStringListIndex(value.c_str(), strings, defaultIndex, caseSensitive);
    }

    uint32_t StringUtils::GetStringListIndex(const char* value, const std::string* strings, uint32_t defaultIndex, bool caseSensitive)
    {
        uint32_t i = 0;

        while (!strings[i].empty())
        {
            if (!Compare(strings[i].c_str(), value, caseSensitive))
                return i;
            ++i;
        }

        return defaultIndex;
    }

    uint32_t StringUtils::GetStringListIndex(const char* value, const char* const* strings, uint32_t defaultIndex, bool caseSensitive)
    {
        uint32_t i = 0;

        while (strings[i])
        {
            if (!Compare(value, strings[i], caseSensitive))
                return i;
            ++i;
        }

        return defaultIndex;
    }
}
