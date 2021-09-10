// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "AlimerConfig.h"
#include "PlatformDef.h"
#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <assert.h>
#include <type_traits>
#include <mutex>
#include <memory>
#include <string>
#include <string_view>

namespace Alimer
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
    using isize = intptr_t;
    using usize = uintptr_t;

    using string = std::string;
    using string_view = std::string_view;

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

    /**
     * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
     *
     * Until C++14, std::hash<T> is not defined if T is a enum (see
     * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).
     * But even with C++14, as of october 2016, std::hash for enums is not widely
     * implemented by compilers, so here when T is a enum, we use EnumClassHash
     * instead of std::hash. (For instance, in Alimer::HashCombine(), or
     * Alimer::UnorderedMap.)
     */
    struct EnumClassHash
    {
        template <typename T>
        constexpr std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    /** Hasher that handles custom enums automatically. */
    template <typename Key>
    using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

    /** Generates a hash for the provided type. Type must have a std::hash specialization. */
    template <class T>
    size_t Hash(const T& v)
    {
        using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

        HashType hasher;
        return hasher(v);
    }

    // Source: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
    template <typename T>
    inline void HashCombine(std::size_t& seed, const T& v)
    {
        HashType<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T, typename... Rest>
    inline void HashCombine(size_t& seed, const T& v, Rest&&... rest)
    {
        HashType<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        HashCombine(seed, std::forward<Rest>(rest)...);
    }

    constexpr size_t StringHash(const char* input)
    {
        // https://stackoverflow.com/questions/2111667/compile-time-string-hashing
        size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
        const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

        while (*input)
        {
            hash ^= static_cast<size_t>(*input);
            hash *= prime;
            ++input;
        }

        return hash;
    }

    /// Returns whether all the set bits in bits are set in v.
    template <typename T> inline bool CheckBitsAll(T v, T bits) { return (v & bits) == bits; }

    /// Returns whether any of the set bits in bits are set in v.
    template <typename T> inline bool CheckBitsAny(T v, T bits) { return (v & bits) != (T)0; }

    // A type cast that is safer than static_cast in debug builds, and is a simple static_cast in release builds.
    // Used for downcasting various ISomething* pointers to their implementation classes in the backends.
    template <typename T, typename U>
    T checked_cast(U u)
    {
        static_assert(!std::is_same<T, U>::value, "Redundant checked_cast");
#ifdef _DEBUG
        if (!u) return nullptr;
        T t = dynamic_cast<T>(u);
        if (!t) assert(!"Invalid type cast");  // NOLINT(clang-diagnostic-string-conversion)
        return t;
#else
        return static_cast<T>(u);
#endif
    }
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
