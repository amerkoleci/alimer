// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>

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
    template<typename T> inline T abs(T v) { return (v >= 0) ? v : -v; }
    template<typename T> inline T min(T a, T b) { return (a < b) ? a : b; }
    template<typename T> inline T max(T a, T b) { return (a < b) ? b : a; }
    template<typename T> inline T clamp(T arg, T lo, T hi) { return (arg < lo) ? lo : (arg < hi) ? arg : hi; }
}
