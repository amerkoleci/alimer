// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// Implementation and code based on DirectXMath: https://github.com/microsoft/DirectXMath/blob/main/LICENSE

#pragma once

#include "Alimer/Math/MathHelper.h"
#include "Alimer/Core/Assert.h"

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)

#if defined(ALIMER_USE_SSE)
#if defined(ALIMER_USE_AVX2)
#   define XM_PERMUTE_PS( v, c ) _mm_permute_ps((v), c )
#else
#   define XM_PERMUTE_PS( v, c ) _mm_shuffle_ps((v), (v), c )
#endif
#endif

#if defined(ALIMER_USE_FMADD)
#   define XM_FMADD_PS( a, b, c ) _mm_fmadd_ps((a), (b), (c))
#   define XM_FNMADD_PS( a, b, c ) _mm_fnmadd_ps((a), (b), (c))
#else
#   define XM_FMADD_PS( a, b, c ) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#   define XM_FNMADD_PS( a, b, c ) _mm_sub_ps((c), _mm_mul_ps((a), (b)))
#endif

#if defined(_MSC_VER) && !defined(_M_ARM) && !defined(_M_ARM64) && !defined(_M_HYBRID_X86_ARM64) && !defined(_M_ARM64EC) && (!_MANAGED) && (!_M_CEE) && (!defined(_M_IX86_FP) || (_M_IX86_FP > 1))
#define _XM_VECTORCALL_ 1
#endif

#if _XM_VECTORCALL_
#   define SIMD_CALLCONV __vectorcall
#elif defined(__GNUC__)
#   define SIMD_CALLCONV
#else
#   define SIMD_CALLCONV __fastcall
#endif

#if defined(__GNUC__) && !defined(__MINGW32__)
#   define GLOBAL_CONST extern const __attribute__((weak))
#else
#   define GLOBAL_CONST extern const __declspec(selectany)
#endif

namespace Alimer
{
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Matrix4x4;

    struct alignas(16) VectorF32
    {
        union
        {
            float f[4];
            simd_float4 v;
        };

        inline operator simd_float4() const noexcept { return v; }
        inline operator const float* () const noexcept { return f; }
#if defined(ALIMER_USE_SSE)
        inline operator __m128i() const noexcept { return _mm_castps_si128(v); }
        inline operator __m128d() const noexcept { return _mm_castps_pd(v); }
#elif defined(ALIMER_USE_NEON) && (defined(__GNUC__) || defined(_ARM64_DISTINCT_NEON_TYPES))
        inline operator int32x4_t() const noexcept { return vreinterpretq_s32_f32(v); }
        inline operator uint32x4_t() const noexcept { return vreinterpretq_u32_f32(v); }
#endif
    };

    struct alignas(16) VectorI32
    {
        union
        {
            int32_t i[4];
            simd_float4 v;
        };

        inline operator simd_float4() const noexcept { return v; }
#if defined(ALIMER_USE_SSE)
        inline operator __m128i() const noexcept { return _mm_castps_si128(v); }
        inline operator __m128d() const noexcept { return _mm_castps_pd(v); }
#elif defined(ALIMER_USE_NEON) && (defined(__GNUC__) || defined(_ARM64_DISTINCT_NEON_TYPES))
        inline operator int32x4_t() const noexcept { return vreinterpretq_s32_f32(v); }
        inline operator uint32x4_t() const noexcept { return vreinterpretq_u32_f32(v); }
#endif
    };

    struct alignas(16) VectorU32
    {
        union
        {
            uint32_t u[4];
            simd_float4 v;
        };

        inline operator simd_float4() const noexcept { return v; }
#if defined(ALIMER_USE_SSE)
        inline operator __m128i() const noexcept { return _mm_castps_si128(v); }
        inline operator __m128d() const noexcept { return _mm_castps_pd(v); }
#elif defined(ALIMER_USE_NEON) && (defined(__GNUC__) || defined(_ARM64_DISTINCT_NEON_TYPES))
        inline operator int32x4_t() const noexcept { return vreinterpretq_s32_f32(v); }
        inline operator uint32x4_t() const noexcept { return vreinterpretq_u32_f32(v); }
#endif
    };

    GLOBAL_CONST VectorF32 g_XMZero = { { { 0.0f, 0.0f, 0.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMOne = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
    GLOBAL_CONST VectorF32 g_ByteMin = { { { -127.0f, -127.0f, -127.0f, -127.0f } } };
    GLOBAL_CONST VectorF32 g_ByteMax = { { { 127.0f, 127.0f, 127.0f, 127.0f } } };
    GLOBAL_CONST VectorF32 g_UByteMax = { { { 255.0f, 255.0f, 255.0f, 255.0f } } };
    GLOBAL_CONST VectorF32 g_ShortMin = { { { -32767.0f, -32767.0f, -32767.0f, -32767.0f } } };
    GLOBAL_CONST VectorF32 g_ShortMax = { { { 32767.0f, 32767.0f, 32767.0f, 32767.0f } } };
    GLOBAL_CONST VectorF32 g_UShortMax = { { { 65535.0f, 65535.0f, 65535.0f, 65535.0f } } };
    GLOBAL_CONST VectorF32 g_XMIdentityR0 = { { { 1.0f, 0.0f, 0.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMIdentityR1 = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMIdentityR2 = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMIdentityR3 = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegIdentityR0 = { { { -1.0f, 0.0f, 0.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegIdentityR1 = { { { 0.0f, -1.0f, 0.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegIdentityR2 = { { { 0.0f, 0.0f, -1.0f, 0.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegIdentityR3 = { { { 0.0f, 0.0f, 0.0f, -1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegateX = { { { -1.0f, 1.0f, 1.0f, 1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegateY = { { { 1.0f, -1.0f, 1.0f, 1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegateZ = { { { 1.0f, 1.0f, -1.0f, 1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegateW = { { { 1.0f, 1.0f, 1.0f, -1.0f } } };
    GLOBAL_CONST VectorF32 g_XMNegativeZero = { { { 0x80000000, 0x80000000, 0x80000000, 0x80000000 } } };
    GLOBAL_CONST VectorF32 g_XMNoFraction = { { { 8388608.0f, 8388608.0f, 8388608.0f, 8388608.0f } } };
    GLOBAL_CONST VectorF32 g_XMAddUDec4 = { { { 0, 0, 0, 32768.0f * 65536.0f } } };

    GLOBAL_CONST VectorI32 g_XMAbsMask = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };

    GLOBAL_CONST VectorU32 g_XMMaskX = { { { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 } } };
    GLOBAL_CONST VectorU32 g_XMMaskY = { { { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 } } };
    GLOBAL_CONST VectorU32 g_XMMaskZ = { { { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 } } };
    GLOBAL_CONST VectorU32 g_XMMaskW = { { { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF } } };
    GLOBAL_CONST VectorU32 g_XMMask3 = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } } };
    GLOBAL_CONST VectorU32 g_XMMaskByte4 = { { { 0xFF, 0xFF00, 0xFF0000, 0xFF000000 } } };
    GLOBAL_CONST VectorU32 g_XMFlipW = { { { 0, 0, 0, 0x80000000 } } };

    struct alignas(16) simd_float4x4
    {
        simd_float4 row[4];

        simd_float4x4() = default;

        simd_float4x4(const simd_float4x4&) = default;
        simd_float4x4& operator=(const simd_float4x4&) = default;

        simd_float4x4(simd_float4x4&&) = default;
        simd_float4x4& operator=(simd_float4x4&&) = default;

        constexpr simd_float4x4(simd_float4_param R0, simd_float4_param R1, simd_float4_param R2, simd_float4_param R3) noexcept
            : row{ R0,R1,R2,R3 }
        {
        }

        simd_float4x4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33) noexcept;
        explicit simd_float4x4(_In_reads_(16) const float* pArray) noexcept;
    };

    // Fix-up for (1st) XMMATRIX parameter to pass in-register for ARM64 and vector call; by reference otherwise
#if ( defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || _XM_VECTORCALL_ || __aarch64__ ) && !defined(_XM_NO_INTRINSICS_)
    typedef const simd_float4x4 simd_float4x4_param;
#else
    typedef const simd_float4x4& simd_float4x4_param;
#endif

    inline simd_float4 simd_vector_zero() noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_setzero_ps();
#elif defined(ALIMER_USE_NEON)
        return vdupq_n_f32(0);
#endif
    }

    inline simd_float4 simd_vector_one() noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_set1_ps(1.0f);
#elif defined(ALIMER_USE_NEON)
        return vdupq_n_f32(1.0f);
#endif
    }

    inline simd_float4 simd_vector_nan() noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_set1_ps(std::numeric_limits<float>::quiet_NaN());
#elif defined(ALIMER_USE_NEON)
        return vdupq_n_f32(std::numeric_limits<float>::quiet_NaN());
#endif
    }

    // XMVectorSet
    inline simd_float4 simd_make_float4(float x, float y, float z, float w) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_set_ps(w, z, y, x);
#elif defined(ALIMER_USE_NEON)
        float32x2_t V0 = vcreate_f32(
            static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&x))
            | (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&y)) << 32));
        float32x2_t V1 = vcreate_f32(
            static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&z))
            | (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&w)) << 32));
        return vcombine_f32(V0, V1);
#endif
    }

    inline simd_float4 simd_make_float4_int(uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept
    {
#if defined(ALIMER_USE_SSE)
        __m128i v = _mm_set_epi32(static_cast<int>(w), static_cast<int>(z), static_cast<int>(y), static_cast<int>(x));
        return _mm_castsi128_ps(v);
#elif defined(ALIMER_USE_NEON)
        uint32x2_t V0 = vcreate_u32(static_cast<uint64_t>(x) | (static_cast<uint64_t>(y) << 32));
        uint32x2_t V1 = vcreate_u32(static_cast<uint64_t>(z) | (static_cast<uint64_t>(w) << 32));
        return vreinterpretq_f32_u32(vcombine_u32(V0, V1));
#endif
    }

    inline simd_float4 simd_make_float4(float value) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_set_ps1(value);
#elif defined(ALIMER_USE_NEON)
        return vdupq_n_f32(value);
#endif
    }

    inline simd_float4 simd_make_float4(_In_ const float* pValue) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_load_ps1(pValue);
#elif defined(ALIMER_USE_NEON)
        return vld1q_dup_f32(pValue);
#endif
    }

    /* Float4 */
    inline float simd_float4_get_by_index(simd_float4_param vector, size_t i) noexcept
    {
        ALIMER_ASSERT(i < 4);
        _Analysis_assume_(i < 4);
        VectorF32 U;
        U.v = vector;
        return U.f[i];
    }

    inline float simd_float4_get_x(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_cvtss_f32(vector);
#elif defined(ALIMER_USE_NEON)
        return vgetq_lane_f32(vector, 0);
#endif
    }

    inline float simd_float4_get_y(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_SSE)
        simd_float4 vTemp = XM_PERMUTE_PS(vector, _MM_SHUFFLE(1, 1, 1, 1));
        return _mm_cvtss_f32(vTemp);
#elif defined(ALIMER_USE_NEON)
        return vgetq_lane_f32(vector, 1);
#endif
    }

    inline float simd_float4_get_z(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_SSE)
        simd_float4 vTemp = XM_PERMUTE_PS(vector, _MM_SHUFFLE(2, 2, 2, 2));
        return _mm_cvtss_f32(vTemp);
#elif defined(ALIMER_USE_NEON)
        return vgetq_lane_f32(vector, 2);
#endif
    }

    inline float simd_float4_get_w(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_SSE)
        simd_float4 vTemp = XM_PERMUTE_PS(vector, _MM_SHUFFLE(3, 3, 3, 3));
        return _mm_cvtss_f32(vTemp);
#elif defined(ALIMER_USE_NEON)
        return vgetq_lane_f32(vector, 3);
#endif
    }

    inline simd_float4 simd_float4_min(simd_float4_param v1, simd_float4_param v2) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_min_ps(v1, v2);
#elif defined(ALIMER_USE_NEON)
        return vminq_f32(v1, v2);
#endif
    }

    inline simd_float4 simd_float4_max(simd_float4_param v1, simd_float4_param v2) noexcept
    {
#if defined(ALIMER_USE_SSE)
        return _mm_max_ps(v1, v2);
#elif defined(ALIMER_USE_NEON)
        return vmaxq_f32(v1, v2);
#endif
    }

    inline simd_float4 simd_float4_round(simd_float4_param v) noexcept
    {
#if defined(ALIMER_USE_SSE4_1)
        return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#elif defined(ALIMER_USE_SSE)
        __m128 sign = _mm_and_ps(v, g_XMNegativeZero);
        __m128 sMagic = _mm_or_ps(g_XMNoFraction, sign);
        __m128 R1 = _mm_add_ps(v, sMagic);
        R1 = _mm_sub_ps(R1, sMagic);
        __m128 R2 = _mm_and_ps(v, g_XMAbsMask);
        __m128 mask = _mm_cmple_ps(R2, g_XMNoFraction);
        R2 = _mm_andnot_ps(mask, v);
        R1 = _mm_and_ps(R1, mask);
        simd_float4 result = _mm_xor_ps(R1, R2);
        return result;
#elif defined(ALIMER_USE_NEON)
#if defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || __aarch64__
        return vrndnq_f32(v);
#else
        uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(v), g_XMNegativeZero);
        float32x4_t sMagic = vreinterpretq_f32_u32(vorrq_u32(g_XMNoFraction, sign));
        float32x4_t R1 = vaddq_f32(v, sMagic);
        R1 = vsubq_f32(R1, sMagic);
        float32x4_t R2 = vabsq_f32(v);
        uint32x4_t mask = vcleq_f32(R2, g_XMNoFraction);
        return vbslq_f32(mask, R1, v);
#endif
#endif
    }

    inline simd_float4 simd_float4_truncate(simd_float4_param v) noexcept
    {
#if defined(ALIMER_USE_SSE4_1)
        return _mm_round_ps(v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
#elif defined(ALIMER_USE_SSE)
        __m128 sign = _mm_and_ps(v, g_XMNegativeZero);
        __m128 sMagic = _mm_or_ps(g_XMNoFraction, sign);
        __m128 R1 = _mm_add_ps(v, sMagic);
        R1 = _mm_sub_ps(R1, sMagic);
        __m128 R2 = _mm_and_ps(v, g_XMAbsMask);
        __m128 mask = _mm_cmple_ps(R2, g_XMNoFraction);
        R2 = _mm_andnot_ps(mask, v);
        R1 = _mm_and_ps(R1, mask);
        simd_float4 result = _mm_xor_ps(R1, R2);
        return result;
#elif defined(ALIMER_USE_NEON)
#if defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || __aarch64__
        return vrndq_f32(v);
#else
        float32x4_t vTest = vabsq_f32(V);
        vTest = vreinterpretq_f32_u32(vcltq_f32(vTest, g_XMNoFraction));

        int32x4_t vInt = vcvtq_s32_f32(V);
        float32x4_t vResult = vcvtq_f32_s32(vInt);

        // All numbers less than 8388608 will use the round to int
        // All others, use the ORIGINAL value
        return vbslq_f32(vreinterpretq_u32_f32(vTest), vResult, V);
#endif
#endif
    }

    inline simd_float4 simd_float4_saturate(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_SSE)
        // Set <0 to 0
        simd_float4 vResult = _mm_max_ps(vector, g_XMZero);
        // Set>1 to 1
        return _mm_min_ps(vResult, g_XMOne);
#elif defined(ALIMER_USE_NEON)
        // Set <0 to 0
        float32x4_t vResult = vmaxq_f32(vector, vdupq_n_f32(0));
        // Set>1 to 1
        return vminq_f32(vResult, vdupq_n_f32(1.0f));
#endif
    }

    // XMVector4Length
    inline simd_float4 simd_float4_length(simd_float4_param vector) noexcept
    {
#if defined(ALIMER_USE_NEON)
        // Dot4
        float32x4_t vTemp = vmulq_f32(V, V);
        float32x2_t v1 = vget_low_f32(vTemp);
        float32x2_t v2 = vget_high_f32(vTemp);
        v1 = vadd_f32(v1, v2);
        v1 = vpadd_f32(v1, v1);
        const float32x2_t zero = vdup_n_f32(0);
        uint32x2_t VEqualsZero = vceq_f32(v1, zero);
        // Sqrt
        float32x2_t S0 = vrsqrte_f32(v1);
        float32x2_t P0 = vmul_f32(v1, S0);
        float32x2_t R0 = vrsqrts_f32(P0, S0);
        float32x2_t S1 = vmul_f32(S0, R0);
        float32x2_t P1 = vmul_f32(v1, S1);
        float32x2_t R1 = vrsqrts_f32(P1, S1);
        float32x2_t Result = vmul_f32(S1, R1);
        Result = vmul_f32(v1, Result);
        Result = vbsl_f32(VEqualsZero, zero, Result);
        return vcombine_f32(Result, Result);
#elif defined(ALIMER_USE_SSE4_1)
        __m128 vTemp = _mm_dp_ps(vector, vector, 0xff);
        return _mm_sqrt_ps(vTemp);
#elif defined(_XM_SSE3_INTRINSICS_)
        XMVECTOR vLengthSq = _mm_mul_ps(V, V);
        vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);
        vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);
        vLengthSq = _mm_sqrt_ps(vLengthSq);
        return vLengthSq;
#elif defined(ALIMER_USE_SSE)
        // Perform the dot product on x,y,z and w
        __m128 vLengthSq = _mm_mul_ps(vector, vector);
        // vTemp has z and w
        __m128 vTemp = XM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(3, 2, 3, 2));
        // x+z, y+w
        vLengthSq = _mm_add_ps(vLengthSq, vTemp);
        // x+z,x+z,x+z,y+w
        vLengthSq = XM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 0, 0, 0));
        // ??,??,y+w,y+w
        vTemp = _mm_shuffle_ps(vTemp, vLengthSq, _MM_SHUFFLE(3, 3, 0, 0));
        // ??,??,x+z+y+w,??
        vLengthSq = _mm_add_ps(vLengthSq, vTemp);
        // Splat the length
        vLengthSq = XM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 2, 2, 2));
        // Get the length
        vLengthSq = _mm_sqrt_ps(vLengthSq);
        return vLengthSq;
#endif
    }

    // XMVectorLerp
    inline simd_float4 SIMD_CALLCONV simd_float4_lerp(simd_float4_param v0, simd_float4_param v1, float t) noexcept
    {
#if defined(ALIMER_USE_NEON)
        simd_float4 L = vsubq_f32(v1, v0);
        return vmlaq_n_f32(v0, L, t);
#elif defined(ALIMER_USE_SSE)
        simd_float4 L = _mm_sub_ps(v1, v0);
        simd_float4 S = _mm_set_ps1(t);
        return XM_FMADD_PS(L, S, v0);
#endif
    }

    // XMVectorMultiplyAdd
    inline simd_float4 SIMD_CALLCONV simd_float4_multiply_add(simd_float4_param v1, simd_float4_param v2, simd_float4_param v3) noexcept
    {
#if defined(ALIMER_USE_NEON)
#if defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC) || __aarch64__
        return vfmaq_f32(v3, v1, v2);
#else
        return vmlaq_f32(v3, v1, v2);
#endif
#else
        return XM_FMADD_PS(v1, v2, v3);
#endif
    }

    ALIMER_API void simd_float3_store(_Out_ Vector3* pDestination, _In_ simd_float4_param vector) noexcept;         // XMStoreFloat3
    ALIMER_API void simd_float4_store(_Out_ Vector4* pDestination, _In_ simd_float4_param vector) noexcept;         // XMStoreFloat4
    ALIMER_API simd_float4 simd_float4_load(_In_ const Vector4* pSource) noexcept;                                  // XMLoadFloat4
    ALIMER_API void simd_float4x4_store(_In_ simd_float4x4_param matrix, _Out_ Matrix4x4* pDestination) noexcept;   // XMStoreFloat4x4
    ALIMER_API simd_float4x4 simd_float4x4_load(_In_ const Matrix4x4* pSource) noexcept;   // XMLoadFloat4x4

    inline simd_float4x4::simd_float4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) noexcept
    {
        row[0] = simd_make_float4(m00, m01, m02, m03);
        row[1] = simd_make_float4(m10, m11, m12, m13);
        row[2] = simd_make_float4(m20, m21, m22, m23);
        row[3] = simd_make_float4(m30, m31, m32, m33);
    }

    inline simd_float4x4::simd_float4x4(const float* pArray) noexcept
    {
        ALIMER_ASSERT(pArray != nullptr);
        row[0] = simd_float4_load(reinterpret_cast<const Vector4*>(pArray));
        row[1] = simd_float4_load(reinterpret_cast<const Vector4*>(pArray + 4));
        row[2] = simd_float4_load(reinterpret_cast<const Vector4*>(pArray + 8));
        row[3] = simd_float4_load(reinterpret_cast<const Vector4*>(pArray + 12));
    }
}

#endif /* defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON) */
