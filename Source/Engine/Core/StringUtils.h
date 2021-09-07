// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"

namespace Alimer
{
#ifdef _WIN32
    ALIMER_API std::string ToUtf8(const wchar_t* wstr, size_t len);
    ALIMER_API std::string ToUtf8(const std::wstring& str);
    ALIMER_API std::string ToUtf16(const std::wstring_view& str);

    ALIMER_API std::wstring ToUtf16(const char* str, size_t len);
    ALIMER_API std::wstring ToUtf16(const std::string& str);
    ALIMER_API std::wstring ToUtf16(const std::string_view& str);
#endif
}
