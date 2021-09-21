// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/StringUtils.h"
#include "PlatformInclude.h"
#include <vector>
#include <sstream>
#include <cstdio>

namespace Alimer
{
    namespace
    {
        std::vector<std::string> SplitInternal(const std::string& str, const char* delim, bool allow_empty)
        {
            if (str.empty())
                return {};

            std::vector<std::string> ret;

            size_t start_index = 0;
            size_t index = 0;
            while ((index = str.find_first_of(delim, start_index)) != std::string::npos)
            {
                if (allow_empty || index > start_index)
                    ret.push_back(str.substr(start_index, index - start_index));
                start_index = index + 1;

                if (allow_empty && (index == str.size() - 1))
                    ret.emplace_back();
            }

            if (start_index < str.size())
                ret.push_back(str.substr(start_index));
            return ret;
        }
    }

    const std::string kEmptyString{};

    std::vector<std::string> Split(const std::string& str, const char* delim)
    {
        return SplitInternal(str, delim, true);
    }

    std::vector<std::string> SplitNoEmpty(const std::string& str, const char* delim)
    {
        return SplitInternal(str, delim, false);
    }

    std::string ToLower(const std::string& str)
    {
        std::string result;
        for (const char& ch : str)
        {
            result += static_cast<char>(::tolower(ch));
        }

        return result;
    }

    std::string ToUpper(const std::string& str)
    {
        std::string result;
        for (const char& ch : str)
        {
            result += static_cast<char>(::toupper(ch));
        }

        return result;
    }

    std::string ReplaceAll(const std::string& source, const std::string& replaceWhat, const std::string& replaceWithWhat)
    {
        std::string result = source;
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

#ifdef _WIN32
    std::string ToUtf8(const wchar_t* wstr, size_t len)
    {
        std::vector<char> char_buffer;
        auto ret = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(len), nullptr, 0, nullptr, nullptr);
        if (ret < 0)
            return "";

        char_buffer.resize(ret);
        WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(len), char_buffer.data(), static_cast<int>(char_buffer.size()), nullptr, nullptr);
        return std::string(char_buffer.data(), static_cast<uint32_t>(char_buffer.size()));
    }

    std::string ToUtf8(const std::wstring& str)
    {
        return ToUtf8(str.data(), str.length());
    }

    std::string ToUtf8(const std::wstring_view& str)
    {
        return ToUtf8(str.data(), str.length());
    }

    std::wstring ToUtf16(const char* str, size_t len)
    {
        std::vector<wchar_t> wchar_buffer;
        auto ret = MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), nullptr, 0);
        if (ret < 0)
            return L"";
        wchar_buffer.resize(ret);
        MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), wchar_buffer.data(), static_cast<int>(wchar_buffer.size()));
        return std::wstring(wchar_buffer.data(), wchar_buffer.size());
    }

    std::wstring ToUtf16(const std::string& str)
    {
        return ToUtf16(str.data(), str.length());
    }

    std::wstring ToUtf16(const std::string_view& str)
    {
        return ToUtf16(str.data(), str.length());
    }
#endif
}
