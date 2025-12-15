// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/MathHelper.h"
#include <limits>

using namespace Alimer;

bool MathF::NearEqual(float value1, float value2, float epsilon) noexcept
{
    float delta = value1 - value2;
    return (MathF::Abs(delta) <= epsilon);
}

bool MathF::AreClose(float value1, float value2) noexcept
{
    //in case they are Infinities (then epsilon check does not work)
    if (value1 == value2)
        return true;

    float eps = (Abs(value1) + Abs(value2) + 10.0f) * M_EPSILON;
    float delta = value1 - value2;
    return (-eps < delta) && (eps > delta);
}

void Alimer::SinCos(float angle, float* sin, float* cos) noexcept
{
    ALIMER_ASSERT(sin);
    ALIMER_ASSERT(cos);

#if defined(HAVE_SINCOSF)
    sincosf(angle, sin, cos);
#elif defined(HAVE___SINCOSF)
    __sincosf(angle, sin, cos);
#else
    // XMScalarSinCos
    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = M_1DIV2PI * angle;
    if (angle >= 0.0f)
    {
        quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
    }
    else
    {
        quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
    }
    float y = angle - M_2PI * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign;
    if (y > M_PIDIV2)
    {
        y = M_PI - y;
        sign = -1.0f;
    }
    else if (y < -M_PIDIV2)
    {
        y = -M_PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

    // 11-degree minimax approximation
    *sin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *cos = sign * p;
#endif
}

float MathF::LinearToSRgb(float linearValue) noexcept
{
    if (linearValue <= 0.0f)
        return 0.0f;
    else if (linearValue <= 0.0031308f)
        return 12.92f * linearValue;
    else if (linearValue < 1.0f)
        return 1.055f * MathF::Pow(linearValue, 0.4166667f) - 0.055f;
    else
        return MathF::Pow(linearValue, 0.45454545f);
}

float MathF::SRgbToLinear(float sRgbValue) noexcept
{
    if (sRgbValue <= 0.04045f)
        return sRgbValue / 12.92f;
    else if (sRgbValue < 1.0f)
        return MathF::Pow((sRgbValue + 0.055f) / 1.055f, 2.4f);
    else
        return MathF::Pow(sRgbValue, 2.2f);
}

float Alimer::HalfToFloat(half value) noexcept
{
#if defined(ALIMER_USE_F16C)
    __m128i V1 = _mm_cvtsi32_si128(static_cast<int>(value));
    __m128 V2 = _mm_cvtph_ps(V1);
    return _mm_cvtss_f32(V2);
#elif defined(ALIMER_USE_NEON) && (defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || __aarch64__) && (!defined(__GNUC__) || (__ARM_FP & 2))
    uint16x4_t vHalf = vdup_n_u16(value);
    float32x4_t vFloat = vcvt_f32_f16(vreinterpret_f16_u16(vHalf));
    return vgetq_lane_f32(vFloat, 0);
#else
    auto mantissa = static_cast<uint32_t>(value & 0x03FF);

    uint32_t exponent = (value & 0x7C00);
    if (exponent == 0x7C00) // INF/NAN
    {
        exponent = 0x8f;
    }
    else if (exponent != 0)  // The value is normalized
    {
        exponent = static_cast<uint32_t>((static_cast<int>(value) >> 10) & 0x1F);
    }
    else if (mantissa != 0)     // The value is denormalized
    {
        // Normalize the value in the resulting float
        exponent = 1;

        do
        {
            exponent--;
            mantissa <<= 1;
        } while ((mantissa & 0x0400) == 0);

        mantissa &= 0x03FF;
    }
    else                        // The value is zero
    {
        exponent = static_cast<uint32_t>(-112);
    }

    uint32_t result =
        ((static_cast<uint32_t>(value) & 0x8000) << 16) // Sign
        | ((exponent + 112) << 23)                      // Exponent
        | (mantissa << 13);                             // Mantissa

    return reinterpret_cast<float*>(&result)[0];
#endif
}

half Alimer::FloatToHalf(float value) noexcept
{
#if defined(ALIMER_USE_F16C)
    __m128 V1 = _mm_set_ss(value);
    __m128i V2 = _mm_cvtps_ph(V1, _MM_FROUND_TO_NEAREST_INT);
    return static_cast<half>(_mm_extract_epi16(V2, 0));
#elif defined(ALIMER_USE_NEON) && (defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || __aarch64__) && (!defined(__GNUC__) || (__ARM_FP & 2))
    float32x4_t vFloat = vdupq_n_f32(value);
    float16x4_t vHalf = vcvt_f16_f32(vFloat);
    return vget_lane_u16(vreinterpret_u16_f16(vHalf), 0);
#else
    uint32_t result;

    auto intValue = reinterpret_cast<uint32_t*>(&value)[0];
    uint32_t sign = (intValue & 0x80000000U) >> 16U;
    intValue = intValue & 0x7FFFFFFFU;      // Hack off the sign
    if (intValue >= 0x47800000 /*e+16*/)
    {
        // The number is too large to be represented as a half. Return infinity or NaN
        result = 0x7C00U | ((intValue > 0x7F800000) ? (0x200 | ((intValue >> 13U) & 0x3FFU)) : 0U);
    }
    else if (intValue <= 0x33000000U /*e-25*/)
    {
        result = 0;
    }
    else if (intValue < 0x38800000U /*e-14*/)
    {
        // The number is too small to be represented as a normalized half.
        // Convert it to a denormalized value.
        uint32_t Shift = 125U - (intValue >> 23U);
        intValue = 0x800000U | (intValue & 0x7FFFFFU);
        result = intValue >> (Shift + 1);
        uint32_t s = (intValue & ((1U << Shift) - 1)) != 0;
        result += (result | s) & ((intValue >> Shift) & 1U);
    }
    else
    {
        // Rebias the exponent to represent the value as a normalized half.
        intValue += 0xC8000000U;
        result = ((intValue + 0x0FFFU + ((intValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
    }

    return static_cast<half>(result | sign);
#endif
}

uint32_t Alimer::CountTrailingZeros(uint32_t value)
{
#if ALIMER_ARCH_X64 || ALIMER_ARCH_X86 || defined(ALIMER_USE_WASM)
#if defined(ALIMER_USE_TZCNT)
    return _tzcnt_u32(value);
#elif ALIMER_COMPILER_MSVC
    if (value == 0)
        return 32;
    unsigned long result;
    _BitScanForward(&result, value);
    return result;
#else
    if (value == 0)
        return 32;
    return __builtin_ctz(value);
#endif
#elif ALIMER_ARCH_ARM || ALIMER_ARCH_A64
#if ALIMER_COMPILER_MSVC
    if (value == 0)
        return 32;
    unsigned long result;
    _BitScanForward(&result, value);
    return result;
#else
    return __builtin_clz(__builtin_bitreverse32(value));
#endif
#else
#error Undefined
#endif
}
