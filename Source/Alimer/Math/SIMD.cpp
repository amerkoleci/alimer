// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
//-------------------------------------------------------------------------------------
// DirectXMath.h -- SIMD C++ Math library
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615560
//-------------------------------------------------------------------------------------
// Implementation and code based on DirectXMath: https://github.com/microsoft/DirectXMath/blob/main/LICENSE

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"
#include "Alimer/Math/Vector4.h"
#include "Alimer/Math/Matrix4x4.h"

namespace Alimer
{
    // XMStoreFloat3
    void simd_float3_store(_Out_ Vector3* pDestination, _In_ simd_float4_param vector) noexcept
    {
        ALIMER_ASSERT(pDestination);
#if defined(ALIMER_USE_NEON)
        float32x2_t VL = vget_low_f32(vector);
        vst1_f32(reinterpret_cast<float*>(pDestination), VL);
        vst1q_lane_f32(reinterpret_cast<float*>(pDestination) + 2, V, 2);
#elif defined(ALIMER_USE_SSE4_1)
        * reinterpret_cast<int*>(&pDestination->x) = _mm_extract_ps(vector, 0);
        *reinterpret_cast<int*>(&pDestination->y) = _mm_extract_ps(vector, 1);
        *reinterpret_cast<int*>(&pDestination->z) = _mm_extract_ps(vector, 2);
#elif defined(ALIMER_USE_SSE)
        _mm_store_sd(reinterpret_cast<double*>(pDestination), _mm_castps_pd(vector));
        __m128 z = XM_PERMUTE_PS(vector, _MM_SHUFFLE(2, 2, 2, 2));
        _mm_store_ss(&pDestination->z, z);
#endif
    }

    // XMStoreFloat4
    void simd_float4_store(_Out_ Vector4* pDestination, _In_ simd_float4_param vector) noexcept
    {
        ALIMER_ASSERT(pDestination);

#if defined(ALIMER_USE_SSE)
        _mm_storeu_ps(&pDestination->x, vector);
#elif defined(ALIMER_USE_NEON)
        vst1q_f32(reinterpret_cast<float*>(pDestination), vector);
#endif
    }

    // XMLoadFloat4
    simd_float4 simd_float4_load(_In_ const Vector4* pSource) noexcept
    {
        ALIMER_ASSERT(pSource);

#if defined(ALIMER_USE_SSE)
        return _mm_loadu_ps(&pSource->x);
#elif defined(ALIMER_USE_NEON)
        return vld1q_f32(reinterpret_cast<const float*>(pSource));
#endif
    }

    // XMStoreFloat4x4
    void simd_float4x4_store(_In_ simd_float4x4_param matrix, _Out_ Matrix4x4* pDestination) noexcept
    {
        ALIMER_ASSERT(pDestination);

#if defined(ALIMER_USE_SSE)
        _mm_storeu_ps(&pDestination->m11, matrix.row[0]);
        _mm_storeu_ps(&pDestination->m21, matrix.row[1]);
        _mm_storeu_ps(&pDestination->m31, matrix.row[2]);
        _mm_storeu_ps(&pDestination->m41, matrix.row[3]);
#elif defined(ALIMER_USE_NEON)
        vst1q_f32(reinterpret_cast<float*>(&pDestination->m11), matrix.row[0]);
        vst1q_f32(reinterpret_cast<float*>(&pDestination->m21), matrix.row[1]);
        vst1q_f32(reinterpret_cast<float*>(&pDestination->m31), matrix.row[2]);
        vst1q_f32(reinterpret_cast<float*>(&pDestination->m41), matrix.row[3]);
#endif
    }

    // XMLoadFloat4x4
    simd_float4x4 simd_float4x4_load(_In_ const Matrix4x4* pSource) noexcept
    {
        ALIMER_ASSERT(pSource);

        simd_float4x4 result;
#if defined(ALIMER_USE_SSE)
        result.row[0] = _mm_loadu_ps(&pSource->m11);
        result.row[1] = _mm_loadu_ps(&pSource->m21);
        result.row[2] = _mm_loadu_ps(&pSource->m31);
        result.row[3] = _mm_loadu_ps(&pSource->m41);
        
#elif defined(ALIMER_USE_NEON)
        result.row[0] = vld1q_f32(reinterpret_cast<const float*>(&pSource->m11));
        result.row[1] = vld1q_f32(reinterpret_cast<const float*>(&pSource->m21));
        result.row[2] = vld1q_f32(reinterpret_cast<const float*>(&pSource->m31));
        result.row[3] = vld1q_f32(reinterpret_cast<const float*>(&pSource->m41));
#endif
        return result;
    }
}

#endif /* defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON) */
