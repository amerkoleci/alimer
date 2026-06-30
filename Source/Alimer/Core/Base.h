// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/AlimerConfig.h"
#include "Alimer/PlatformDef.h"
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <mutex>
#include <atomic>
#include <initializer_list>
#include <memory>
#include <utility>

#if defined(_MSC_VER)
#include <malloc.h> // for alloca
#endif

// Define macro to get current function name
#if ALIMER_COMPILER_CLANG || ALIMER_COMPILER_GCC
#define ALIMER_FUNCTION_NAME	__PRETTY_FUNCTION__
#elif ALIMER_COMPILER_MSVC
#define ALIMER_FUNCTION_NAME	__FUNCTION__
#else
#error Undefined function name macro
#endif

// Stack allocation
#define ALIMER_STACK_ALLOC(n)		alloca(n)

// SAL annotations
#if defined(_MSC_VER)
#   include "sal.h"
#else
#define _In_
#define _In_z_
#define _In_opt_
#define _In_opt_z_
#define _In_reads_(x)
#define _In_reads_opt_(x)
#define _In_reads_bytes_(x)
#define _In_reads_bytes_opt_(x)
#define _In_range_(x, y)
#define _In_bytecount_(x)
#define _Out_
#define _Out_opt_
#define _Outptr_
#define _Outptr_opt_result_z_
#define _Outptr_opt_result_bytebuffer_(x)
#define _Analysis_assume_(expr)
#endif

#define ALIMER_BIT(x) (1 << x)

#define ALIMER_DISABLE_COPY(Class) \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete
#define ALIMER_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete
#define ALIMER_DISABLE_COPY_MOVE(Class) \
    ALIMER_DISABLE_COPY(Class); \
    ALIMER_DISABLE_MOVE(Class)

#define ALIMER_ENUM_CLASS_FLAG_OPERATORS(EnumType) \
inline constexpr EnumType operator | (EnumType a, EnumType b) noexcept \
    { \
        using UnderlyingType = std::underlying_type_t<EnumType>; \
        return EnumType(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b)); \
    } \
inline constexpr EnumType& operator |= (EnumType &a, EnumType b) noexcept \
    { return a = a | b; } \
inline constexpr EnumType operator & (EnumType a, EnumType b) noexcept \
    { \
        using UnderlyingType = std::underlying_type_t<EnumType>; \
        return EnumType(static_cast<UnderlyingType>(a) & static_cast<UnderlyingType>(b)); \
    } \
inline constexpr EnumType& operator &= (EnumType &a, EnumType b) noexcept \
    { return a = a & b; } \
inline constexpr EnumType operator ~ (EnumType a) noexcept \
    { \
        using UnderlyingType = std::underlying_type_t<EnumType>; \
        return EnumType(~static_cast<UnderlyingType>(a)); \
    } \
inline constexpr EnumType operator ^ (EnumType a, EnumType b) noexcept \
    { \
        using UnderlyingType = std::underlying_type_t<EnumType>; \
        return EnumType(static_cast<UnderlyingType>(a) ^ static_cast<UnderlyingType>(b)); \
    } \
inline constexpr EnumType& operator ^= (EnumType &a, EnumType b) noexcept \
    { return a = a ^ b; } \
inline constexpr bool operator ==(EnumType a, std::underlying_type<EnumType>::type b) { return ((std::underlying_type<EnumType>::type)a) == b; } \
inline constexpr bool operator !=(EnumType a, std::underlying_type<EnumType>::type b) { return ((std::underlying_type<EnumType>::type)a) != b; }

namespace Alimer
{
    using String = std::string;
    using StringView = std::string_view;

    static constexpr uint32_t kConversionBufferLength = 128;
    static constexpr uint32_t kMatrixConversionBufferLength = 256;

    /// Integer format represents time using 10,000,000 ticks per second.
    static constexpr uint64_t TicksPerSecond = 10000000;

    static constexpr double TicksToSeconds(uint64_t ticks) noexcept { return static_cast<double>(ticks) / TicksPerSecond; }
    static constexpr uint64_t SecondsToTicks(double seconds) noexcept { return static_cast<uint64_t>(seconds * TicksPerSecond); }

    //---------------------------------------------
    // Basic comparisons
    //---------------------------------------------
    template<typename T> inline T Abs(T v) { return (v >= 0) ? v : -v; }
    template<typename T> inline T Min(T a, T b) { return (a < b) ? a : b; }
    template<typename T> inline T Max(T a, T b) { return (a < b) ? b : a; }
    template<typename T> inline T Clamp(T arg, T lo, T hi) { return (arg < lo) ? lo : (arg < hi) ? arg : hi; }

    //---------------------------------------------
    // Limits
    //---------------------------------------------
    template <typename T> struct Limits;
#define ALIMER_MAKE_LIMITS(type, lo, hi) \
template <> struct Limits<type> { \
    static constexpr type Min = lo; \
    static constexpr type Max = hi; \
}

    ALIMER_MAKE_LIMITS(char, SCHAR_MIN, SCHAR_MAX);
    ALIMER_MAKE_LIMITS(short, SHRT_MIN, SHRT_MAX);
    ALIMER_MAKE_LIMITS(int, INT_MIN, INT_MAX);
    ALIMER_MAKE_LIMITS(long, LONG_MIN, LONG_MAX);
    ALIMER_MAKE_LIMITS(unsigned char, 0, UCHAR_MAX);
    ALIMER_MAKE_LIMITS(unsigned short, 0, USHRT_MAX);
    ALIMER_MAKE_LIMITS(unsigned int, 0, UINT_MAX);
    ALIMER_MAKE_LIMITS(unsigned long, 0, ULONG_MAX);
#if defined(LLONG_MAX)
    ALIMER_MAKE_LIMITS(long long, LLONG_MIN, LLONG_MAX);
    ALIMER_MAKE_LIMITS(unsigned long long, 0, ULLONG_MAX);
#endif
    ALIMER_MAKE_LIMITS(float, -FLT_MAX, FLT_MAX);
    ALIMER_MAKE_LIMITS(double, -DBL_MAX, DBL_MAX);

    template <class T>
    constexpr std::underlying_type_t<T> ecast(T x) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(x);
    }

    /// Returns the value of the bit at bitIndex (1 if the bit is 1, 0 if 0)
    template <typename T> [[nodiscard]] constexpr T CheckBit(T v, uint8_t bitIndex)
    {
        return (v >> bitIndex) & 1;
    }

    /// Returns whether all the set bits in bits are set in v.
    template <typename T> [[nodiscard]] constexpr bool CheckBitsAll(T v, T bits)
    {
        return (v & bits) == bits;
    }

    /// Returns whether any of the set bits in bits are set in v.
    template <typename T> [[nodiscard]] constexpr bool CheckBitsAny(T v, T bits)
    {
        return (v & bits) != (T)0;
    }

    template <typename T>
    void SafeDelete(T*& resource)
    {
        delete resource;
        resource = nullptr;
    }

    template <typename T>
    void SafeDeleteArray(T*& resource)
    {
        delete[] resource;
        resource = nullptr;
    }

    template <typename T>
    void SafeDeleteContainer(T& resource)
    {
        for (auto& element : resource)
        {
            SafeDelete(element);
        }
        resource.clear();
    }
}

#undef ALIMER_MAKE_LIMITS
