// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/Win32/WindowsPlatform.h"
#include <type_traits>

namespace Alimer
{
    std::string ToUtf8(std::wstring_view str)
    {
        const int size = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr);
        if (size < 0)
            return "";

        auto buffer = std::make_unique<char[]>(size);
        WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), buffer.get(), size, nullptr, nullptr);
        return std::string(buffer.get(), static_cast<size_t>(size));
    }

    std::wstring ToUtf16(std::string_view str)
    {
        const int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
        if (size < 0)
            return L"";

        auto buffer = std::make_unique<wchar_t[]>(size);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), buffer.get(), size);
        return std::wstring(buffer.get(), static_cast<size_t>(size));
    }
}
