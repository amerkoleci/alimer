// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "PlatformDef.h"
#include <cassert>
#include <cinttypes>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <type_traits>
#include <mutex>
#include <memory>
#include <string>
#include <string_view>

namespace alimer
{
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using f32 = float;
    using f64 = double;

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

    //---------------------------------------------
    // Basic comparisons
    //---------------------------------------------
    template<typename T> inline T Abs(T v) { return (v >= 0) ? v : -v; }
    template<typename T> inline T Min(T a, T b) { return (a < b) ? a : b; }
    template<typename T> inline T Max(T a, T b) { return (a < b) ? b : a; }
    template<typename T> inline T Clamp(T arg, T lo, T hi) { return (arg < lo) ? lo : (arg < hi) ? arg : hi; }

    /// Returns whether all the set bits in bits are set in v.
    template <typename T> inline bool All(T v, T bits) { return (v & bits) == bits; }

    /// Returns whether any of the set bits in bits are set in v.
    template <typename T> inline bool Any(T v, T bits) { return (v & bits) != (T)0; }
}

#ifdef NDEBUG
#   define ALIMER_DEBUG 0
#else
#   define ALIMER_DEBUG 1
#endif

#ifndef ALIMER_ASSERT
#   if ALIMER_DEBUG
#       define ALIMER_ASSERT(cond, ...) assert(cond)
#   else
#       define ALIMER_ASSERT
#   endif
#endif

#define ALIMER_DISABLE_COPY(_Class) \
    _Class(const _Class&) = delete; _Class& operator=(const _Class&) = delete; \
    _Class(_Class&&) = default; _Class& operator=(_Class&&) = default;

#define ALIMER_DISABLE_MOVE(_Class) \
    _Class(_Class&&) = delete; _Class& operator=(_Class&&) = delete;

#define ALIMER_DISABLE_COPY_MOVE(_Class) \
    _Class(const _Class&) = delete; _Class& operator=(const _Class&) = delete; ALIMER_DISABLE_MOVE(_Class)

#define ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(T) \
inline constexpr T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
inline constexpr T& operator |= (T &a, T b) { return a = a | b; } \
inline constexpr T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } \
inline constexpr T& operator &= (T &a, T b) { return a = a & b; } \
inline constexpr T operator ~ (T a) { return T(~uint32_t(a)); } \
inline constexpr bool operator !(T a) { return uint32_t(a) == 0; } \
inline constexpr T operator ^ (T a, T b) { return T(uint32_t(a) ^ uint32_t(b)); } \
inline constexpr T& operator ^= (T &a, T b) { return a = a ^ b; } \
inline constexpr bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
inline constexpr bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }
