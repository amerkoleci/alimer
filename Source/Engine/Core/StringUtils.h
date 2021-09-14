// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include <vector>

namespace Alimer
{
    /// Return whether a char is an alphabet letter.
    inline bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    /// Return whether a char is a digit.
    inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

    ALIMER_API std::vector<std::string> Split(const std::string& str, const char* delim);
    ALIMER_API std::vector<std::string> SplitNoEmpty(const std::string& str, const char* delim);

    // Convert to lower-case.
    ALIMER_API string ToLower(const string& str);

    // Convert to upper-case.
    ALIMER_API string ToUpper(const string& str);

    /// Replace all instances of a sub-string with a another sub-string.
    ALIMER_API string ReplaceAll(const string& source, const string& replaceWhat, const string& replaceWithWhat);

#ifdef _WIN32
    ALIMER_API std::string ToUtf8(const wchar_t* wstr, size_t len);
    ALIMER_API std::string ToUtf8(const std::wstring& str);
    ALIMER_API std::string ToUtf16(const std::wstring_view& str);

    ALIMER_API std::wstring ToUtf16(const char* str, size_t len);
    ALIMER_API std::wstring ToUtf16(const std::string& str);
    ALIMER_API std::wstring ToUtf16(const std::string_view& str);
#endif
}
