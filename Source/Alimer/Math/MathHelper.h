// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Assert.h"
#include <stdlib.h>
#include <cmath>
#include <limits>
#include <type_traits>

#if defined(ALIMER_USE_SSE)
#   include <immintrin.h>
#elif defined(ALIMER_USE_NEON)
#   if defined(_MSC_VER) && !defined(__clang__) && (defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC))
#       include <intrin.h>
#       include <arm64_neon.h>
#   else
#       include <arm_neon.h>
#   endif
#endif

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4244) // Conversion from 'double' to 'float'
#	pragma warning(disable : 4702) // unreachable code
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wfloat-equal"
#endif

#ifdef M_PI
#undef M_PI
#endif

namespace Alimer
{
#if defined(ALIMER_USE_SSE)
    using simd_float4 = __m128;
#elif defined(ALIMER_USE_NEON)
    using simd_float4 = float32x4_t;
#endif

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    // Fix-up for (1st-3rd) XMVECTOR parameters that are pass-in-register for x86, ARM, ARM64, and vector call; by reference otherwise
#if ( defined(_M_IX86) || defined(_M_ARM) || defined(_M_ARM64) || _XM_VECTORCALL_ || __i386__ || __arm__ || __aarch64__ )
    typedef const simd_float4 simd_float4_param;
#else
    typedef const simd_float4& simd_float4_param;
#endif
#endif /* defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON) */

    // 16 bit floating point number consisting of a sign bit, a 5 bit biased exponent, and a 10 bit mantissa
    using half = uint16_t;

    /// Describes how one bounding volume contains another.
    enum class ContainmentType : uint8_t
    {
        /// The two bounding volumes don't intersect at all.
        Disjoint,
        /// One bounding volume completely contains another.
        Contains,
        /// The two bounding volumes overlap.
        Intersects,
    };

    /// Describes the intersection between a plane and a bounding volume.
    enum class PlaneIntersectionType : uint8_t
    {
        /// There is no intersection, and the bounding volume is in the positive half-space of the Plane.
        Front,
        /// There is no intersection, and the bounding volume is in the negative half-space of the Plane.
        Back,
        /// The Plane is intersected.
        Intersecting
    };

    static constexpr float M_EPSILON = 1e-6f;
    static constexpr double M_EPSILON_DOUBLE = 1e-6;
    static constexpr float M_PI = 3.141592654f;
    static constexpr float M_2PI = 6.28318530718f; // XM_2PI
    static constexpr float M_1DIVPI = 0.318309886f;     // XM_1DIVPI 
    static constexpr float M_1DIV2PI = 0.159154943f;    // XM_1DIV2PI
    static constexpr float M_PIDIV2 = 1.570796327f;     // XM_PIDIV2 
    static constexpr float M_PIDIV4 = 0.785398163f;     // XM_PIDIV4 
    static constexpr float M_INFINITY = (float)HUGE_VALF;

    static constexpr float DEG_TO_RAD = M_PI / 180.0f;
    static constexpr float RAD_TO_DEG = 180.0f / M_PI;

    [[nodiscard]] constexpr float ToRadians(float degrees) noexcept { return degrees * DEG_TO_RAD; }
    [[nodiscard]] constexpr float ToDegrees(float radians) noexcept { return radians * RAD_TO_DEG; }

    /// Check whether a floating point value is NaN.
    template <typename T> inline bool IsNaN(T value) { return std::isnan(value); }

    /// Check whether a floating point value is positive or negative infinity.
    template <typename T> inline bool IsInf(T value) { return std::isinf(value); }

    /// Return the sign of a float (-1, 0 or 1).
    template<typename T> [[nodiscard]] inline T Sign(T value) { return value > 0 ? T(1) : (value < 0 ? T(-1) : T(0)); }

    /// Round up to next power of two.
    inline constexpr uint32_t NextPowerOfTwo(uint32_t value)
    {
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        --value;
        value |= value >> 1u;
        value |= value >> 2u;
        value |= value >> 4u;
        value |= value >> 8u;
        value |= value >> 16u;
        return ++value;
    }

    /// Round up to next power of two.
    inline uint64_t NextPowerOfTwo(uint64_t value)
    {
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        --value;
        value |= value >> 1u;
        value |= value >> 2u;
        value |= value >> 4u;
        value |= value >> 8u;
        value |= value >> 16u;
        value |= value >> 32u;
        return ++value;
    }

    /// Round up or down to the closest power of two.
    inline uint32_t ClosestPowerOfTwo(uint32_t value)
    {
        const uint32_t next = NextPowerOfTwo(value);
        const uint32_t prev = next >> 1u;
        return (value - prev) > (next - value) ? next : prev;
    }


    /// Returns true if value is a power of two.
    template <typename T>
    constexpr inline bool IsPow2(T value)
    {
        return (value & (value - 1)) == 0;
    }

    /// Aligns value up to the given alignment. Assumes alignment is power of two.
    template <typename T>
    constexpr T AlignUp(T value, T alignment)
    {
        ALIMER_HEAVY_ASSERT(IsPow2(alignment));
        return (value + alignment - 1) & ~(alignment - 1);
    }

    template <typename T>
    constexpr T AlignDown(T value, T alignment)
    {
        ALIMER_HEAVY_ASSERT(IsPow2(alignment));
        return value & ~(alignment - 1);
    }

    template <typename T>
    constexpr bool IsAligned(T value, size_t alignment)
    {
        return 0 == ((size_t)value & (alignment - 1));
    }

    template<typename T>
    constexpr T Align(T value, T alignment)
    {
        return ((value + alignment - T(1)) / alignment) * alignment;
    }

    template <typename T>
    constexpr T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }


    /// Return remainder of X/Y for float values.
    template <class T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    inline T Mod(T x, T y) { return fmod(x, y); }

    /// Return remainder of X/Y for integer values.
    template <class T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    inline T Mod(T x, T y) { return x % y; }

    /// Return always positive remainder of X/Y.
    template <class T>
    inline T AbsMod(T x, T y)
    {
        const T result = Mod(x, y);
        return result < 0 ? result + y : result;
    }

    /// Return fractional part of passed value in range [0, 1).
    template <typename T> inline T Fract(T value) { return value - floor(value); }

    /// Round value down to nearest number that can be represented as i*y, where i is integer.
    template <typename T> inline T SnapFloor(T x, T y) { return floor(x / y) * y; }

    /// Calculate both sine and cosine, with angle in radians.
    ALIMER_API void SinCos(float angle, float* sin, float* cos) noexcept;

    ALIMER_API float HalfToFloat(half value) noexcept;
    ALIMER_API half FloatToHalf(float value) noexcept;

    /// Compute number of trailing zero bits (how many low bits are zero)
    ALIMER_API uint32_t CountTrailingZeros(uint32_t value);

    namespace MathF
    {
        /// Check whether two floating point values are equal within accuracy.
        [[nodiscard]] constexpr bool Equals(float lhs, float rhs, float eps = M_EPSILON)
        {
            return lhs + eps >= rhs && lhs - eps <= rhs;
        }

        [[nodiscard]] static ALIMER_FORCE_INLINE bool IsZero(float value)
        {
            return Abs(value) < M_EPSILON;
        }

        [[nodiscard]] static ALIMER_FORCE_INLINE bool IsOne(float value)
        {
            return IsZero(value - 1.0f);
        }

        /// Return sine of an angle in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Sin(float radians) { return sinf(radians); }

        /// Return cosine of an angle in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Cos(float radians) { return cosf(radians); }

        /// Return tangent of an angle in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Tan(float radians) { return tanf(radians); }

        /// Return arc sine in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Asin(float radians) { return asinf(Clamp(radians, -1.0f, 1.0f)); }

        /// Return arc cosine in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Acos(float radians) { return acosf(Clamp(radians, -1.0f, 1.0f)); }

        /// Return arc tangent in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Atan(float radians) { return atanf(radians); }

        /// Return arc tangent of y/x in radians.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Atan2(float y, float x) { return atan2f(y, x); }

        /// Return natural logarithm of X.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Log(float x) { return logf(x); }

        [[nodiscard]] static ALIMER_FORCE_INLINE float Log2(float x) { return log2f(x); }
        [[nodiscard]] static ALIMER_FORCE_INLINE float Log10(float x) { return log10f(x); }

        /// Return square root of X.
        [[nodiscard]] static ALIMER_FORCE_INLINE float Sqrt(float x) { return sqrtf(x); }

        /// Linear interpolation between two values.
        [[nodiscard]] constexpr float Lerp(float lhs, float rhs, float t) noexcept
        {
            return lhs * (1.0f - t) + rhs * t;
        }

        /// Inverse linear interpolation between two values.
        [[nodiscard]] constexpr float InverseLerp(float lhs, float rhs, float x) noexcept
        {
            return (x - lhs) / (rhs - lhs);
        }

        /// Performs smooth (cubic Hermite) interpolation between 0 and 1.
        [[nodiscard]] constexpr float SmoothStep(float amount) noexcept
        {
            return (amount <= 0) ? 0 : (amount >= 1) ? 1 : amount * amount * (3 - (2 * amount));
        }

        static ALIMER_FORCE_INLINE float Abs(float value)
        {
            return fabsf(value);
        }

        static ALIMER_FORCE_INLINE float Pow(float x, float y)
        {
            return powf(x, y);
        }

        /// Round value down.
        static ALIMER_FORCE_INLINE float Floor(float x)
        {
            return floorf(x);
        }

        /// Round value down. Returns integer value.
        static inline int32_t FloorToInt(float x)
        {
            return (int32_t)floorf(x);
        }

        static ALIMER_FORCE_INLINE int32_t RoundToInt(float value)
        {
            return FloorToInt(value + 0.5f);
        }

        /// Round value to nearest integer.
        static ALIMER_FORCE_INLINE float Round(const float x)
        {
            return roundf(x);
        }

        /// Clamp value to be between 0 and 1 range, inclusive
        static ALIMER_FORCE_INLINE float Saturate(const float value)
        {
            return value < 0.0f ? 0.0f : value < 1.0f ? value : 1.0f;
        }

        static ALIMER_FORCE_INLINE float Truncate(float x)
        {
            return truncf(x);
        }

        /// Round value up.
        static ALIMER_FORCE_INLINE float Ceil(float value)
        {
            return ceilf(value);
        }

        /// Round value up to nearest number that can be represented as i*y, where i is integer.
        static ALIMER_FORCE_INLINE float SnapCeil(float x, float y)
        {
            return ceil(x / y) * y;;
        }

        /// Round value up.
        static ALIMER_FORCE_INLINE int CeilToInt(float x)
        {
            return static_cast<int>(ceilf(x));
        }

        ALIMER_API bool NearEqual(float value1, float value2, float epsilon) noexcept;

        ALIMER_API bool AreClose(float value1, float value2) noexcept;

        /// Converts a float value from linear to sRGB (gamma) space.
        ALIMER_API float LinearToSRgb(float linearValue) noexcept;

        /// Convert a float value from sRGB ( gamma) to linear.
        ALIMER_API float SRgbToLinear(float sRgbValue) noexcept;
    }

    namespace MathD
    {
        static ALIMER_FORCE_INLINE double Abs(double value)
        {
            return fabs(value);
        }

        [[nodiscard]] static ALIMER_FORCE_INLINE bool IsZero(double value)
        {
            return fabs(value) < M_EPSILON_DOUBLE;
        }

        [[nodiscard]] static ALIMER_FORCE_INLINE bool IsOne(float value)
        {
            return IsZero(value - 1.);
        }
    }
}

#ifdef _MSC_VER
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#endif
