// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/StringUtils.h"
#include "PlatformInclude.h"
#include <vector>
#include <sstream>
#include <cstdio>

using namespace std;

namespace alimer
{
#ifdef _WIN32
    string ToUtf8(const wchar_t* wstr, size_t len)
    {
        vector<char> char_buffer;
        auto ret = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(len), nullptr, 0, nullptr, nullptr);
        if (ret < 0)
            return "";

        char_buffer.resize(ret);
        WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(len), char_buffer.data(), static_cast<int>(char_buffer.size()), nullptr, nullptr);
        return std::string(char_buffer.data(), static_cast<uint32_t>(char_buffer.size()));
    }

    string ToUtf8(const std::wstring& str)
    {
        return ToUtf8(str.data(), str.length());
    }

    string ToUtf8(const wstring_view& str)
    {
        return ToUtf8(str.data(), str.length());
    }

    wstring ToUtf16(const char* str, size_t len)
    {
        vector<wchar_t> wchar_buffer;
        auto ret = MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), nullptr, 0);
        if (ret < 0)
            return L"";
        wchar_buffer.resize(ret);
        MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(len), wchar_buffer.data(), static_cast<int>(wchar_buffer.size()));
        return std::wstring(wchar_buffer.data(), wchar_buffer.size());
    }

    wstring ToUtf16(const string& str)
    {
        return ToUtf16(str.data(), str.length());
    }

    wstring ToUtf16(const string_view& str)
    {
        return ToUtf16(str.data(), str.length());
    }
#endif
}
