// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include <spdlog/fmt/fmt.h>
#ifdef SPDLOG_USE_STD_FORMAT
#define FMT std
#else
#define FMT fmt
#endif

namespace Alimer
{
    /// Vector of strings.
    using StringVector = Vector<String>;

    ALIMER_API extern const String kEmptyString;
    ALIMER_API extern const StringView kEmptyStringView;

    /// Convert a char to uppercase.
    inline char ToUpper(char c) { return (c >= 'a' && c <= 'z') ? c - 0x20 : c; }
    /// Convert a char to lowercase.
    inline char ToLower(char c) { return (c >= 'A' && c <= 'Z') ? c + 0x20 : c; }
    /// Return whether a char is an alphabet letter.
    inline bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    /// Return whether a char is a digit.
    inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

    /// Return length of a C string.
    ALIMER_API size_t CStringLength(const char* str);

    /// Convert to lower-case.
    ALIMER_API String ToLower(const String& str);

    /// Convert to upper-case.
    ALIMER_API String ToUpper(const String& str);

    /// Return comparison result with a string.
    ALIMER_API int Compare(const char* lhs, const char* rhs, bool caseSensitive);

    /// Return a formatted string.
    ALIMER_API std::string ToString(const char* formatString, ...);

    /// Convert a boolean to string (returns true or false).
    ALIMER_API std::string ToString(bool value);

    /// Convert value to string.
    ALIMER_API std::string ToString(int16_t value);
    /// Convert value to string.
    ALIMER_API std::string ToString(int32_t value);

    /// Convert value to string.
    ALIMER_API std::string ToString(int64_t value);
    /// Convert value to string.
    ALIMER_API std::string ToString(uint16_t value);
    /// Convert value to string.
    ALIMER_API std::string ToString(uint32_t value);
    /// Convert value to string.
    ALIMER_API std::string ToString(uint64_t value);
    /// Convert value to string.
    ALIMER_API std::string ToString(float value);
    /// Convert value to string.
    ALIMER_API std::string ToString(double value);

    class ALIMER_API StringUtils
    {
    public:
        /// Parse a bool from a string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
        static bool ToBool(const std::string& source);
        /// Parse a bool from a C string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
        static bool ToBool(const char* source);
        /// Parse an integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static int32_t ToInt32(const std::string& source, int base = 10);
        /// Parse an integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static int32_t ToInt32(const char* source, int base = 10);
        /// Parse an unsigned integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static uint32_t ToUInt32(const std::string& source, int base = 10);
        /// Parse an unsigned integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static uint32_t ToUInt32(const char* source, int base = 10);
        /// Parse an 64 bit integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static int64_t ToInt64(const std::string& source, int base = 10);
        /// Parse an 64 bit integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static int64_t ToInt64(const char* source, int base = 10);
        /// Parse an unsigned 64 bit integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static uint64_t ToUInt64(const std::string& source, int base = 10);
        /// Parse an unsigned 64 bit integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
        static uint64_t ToUInt64(const char* source, int base = 10);
        /// Parse a float from a string.
        static float ToFloat(const std::string& source);
        /// Parse a float from a C string.
        static float ToFloat(const char* source);
        /// Parse a double from a string.
        static double ToDouble(const std::string& source);
        /// Parse a double from a C string.
        static double ToDouble(const char* source);

        /// Return the amount of substrings split by a separator char.
        static size_t CountElements(StringView str, char separator = ' ');

        /// Return the amount of substrings split by a separator char.
        static size_t CountElements(const char* str, size_t length, char separator = ' ');

        static std::string Trim(const std::string& str, bool left = true, bool right = true);

        /// Replace all instances of a sub-string with a another sub-string.
        static std::string ReplaceAll(std::string_view source, const std::string& replaceWhat, const std::string& replaceWithWhat);

        static StringVector Split(const std::string& str, const char* delim);
        static StringVector SplitNoEmpty(const std::string& str, const char* delim);

        static String Join(const String& separator, const StringVector& values);

        /// Return an index to a string list corresponding to the given string, or a default value if not found. The string list must be empty-terminated.
        static uint32_t GetStringListIndex(const std::string& value, const std::string* strings, uint32_t defaultIndex, bool caseSensitive = false);
        /// Return an index to a string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
        static uint32_t GetStringListIndex(const char* value, const std::string* strings, uint32_t defaultIndex, bool caseSensitive = false);
        /// Return an index to a C string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
        static uint32_t GetStringListIndex(const char* value, const char* const* strings, uint32_t defaultIndex, bool caseSensitive = false);
    };
}
