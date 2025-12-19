// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// SIMD matrix is based on DirectXMath: https://github.com/microsoft/DirectXMath/blob/main/LICENSE

#include "Alimer/Math/Matrix4x4.h"
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"
#endif

using namespace Alimer;

const Matrix4x4 Matrix4x4::Zero = {
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f
};

const Matrix4x4 Matrix4x4::Identity = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

Matrix4x4::Matrix4x4(_In_reads_(16) const float* data) noexcept
{
    ALIMER_ASSERT(data != nullptr);

    m[0][0] = data[0];
    m[0][1] = data[1];
    m[0][2] = data[2];
    m[0][3] = data[3];

    m[1][0] = data[4];
    m[1][1] = data[5];
    m[1][2] = data[6];
    m[1][3] = data[7];

    m[2][0] = data[8];
    m[2][1] = data[9];
    m[2][2] = data[10];
    m[2][3] = data[11];

    m[3][0] = data[12];
    m[3][1] = data[13];
    m[3][2] = data[14];
    m[3][3] = data[15];
}

void Matrix4x4::Set(const Matrix4x4& other)
{
    memcpy(this->m, other.m, sizeof(float) * 16);
}

void Matrix4x4::SetIdentity()
{
    m11 = 1.0f;
    m12 = 0.0f;
    m13 = 0.0f;
    m14 = 0.0f;

    m21 = 0.0f;
    m22 = 1.0f;
    m23 = 0.0f;
    m24 = 0.0f;

    m31 = 0.0f;
    m32 = 0.0;
    m33 = 1.0f;
    m34 = 0.0f;

    m41 = 0.0f;
    m42 = 0.0;
    m43 = 1.0f;
    m44 = 0.0f;
}

void Matrix4x4::SetZero()
{
    memset(m, 0, sizeof(Matrix4x4));
}

float Matrix4x4::GetDeterminant() const noexcept
{
    // | a b c d |     | f g h |     | e g h |     | e f h |     | e f g |
    // | e f g h | = a | j k l | - b | i k l | + c | i j l | - d | i j k |
    // | i j k l |     | n o p |     | m o p |     | m n p |     | m n o |
    // | m n o p |
    //
    //   | f g h |
    // a | j k l | = a ( f ( kp - lo ) - g ( jp - ln ) + h ( jo - kn ) )
    //   | n o p |
    //
    //   | e g h |
    // b | i k l | = b ( e ( kp - lo ) - g ( ip - lm ) + h ( io - km ) )
    //   | m o p |
    //
    //   | e f h |
    // c | i j l | = c ( e ( jp - ln ) - f ( ip - lm ) + h ( in - jm ) )
    //   | m n p |
    //
    //   | e f g |
    // d | i j k | = d ( e ( jo - kn ) - f ( io - km ) + g ( in - jm ) )
    //   | m n o |
    //
    // Cost of operation
    // 17 adds and 28 muls.
    //
    // add: 6 + 8 + 3 = 17
    // mul: 12 + 16 = 28

    float a = m11, b = m12, c = m13, d = m14;
    float e = m21, f = m22, g = m23, h = m24;
    float i = m31, j = m32, k = m33, l = m34;
    float m = m41, n = m42, o = m43, p = m44;

    float kp_lo = k * p - l * o;
    float jp_ln = j * p - l * n;
    float jo_kn = j * o - k * n;
    float ip_lm = i * p - l * m;
    float io_km = i * o - k * m;
    float in_jm = i * n - j * m;

    return a * (f * kp_lo - g * jp_ln + h * jo_kn) -
        b * (e * kp_lo - g * ip_lm + h * io_km) +
        c * (e * jp_ln - f * ip_lm + h * in_jm) -
        d * (e * jo_kn - f * io_km + g * in_jm);
}


Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& position) noexcept
{
    return CreateTranslation(position.x, position.y, position.z);
}

Matrix4x4 Matrix4x4::CreateTranslation(float x, float y, float z) noexcept
{
    Matrix4x4 result;
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    simd_float4x4 simdMatrix;
    simdMatrix.row[0] = g_XMIdentityR0.v;
    simdMatrix.row[1] = g_XMIdentityR1.v;
    simdMatrix.row[2] = g_XMIdentityR2.v;
    simdMatrix.row[3] = simd_make_float4(x, y, z, 1.0f);
    simd_float4x4_store(simdMatrix, &result);
#else
    result.m11 = 1.0f;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 1.0f;
    result.m23 = 0.0f;
    result.m24 = 0.0f;
    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = 1.0f;
    result.m34 = 0.0f;
    result.m41 = x;
    result.m42 = y;
    result.m43 = z;
    result.m44 = 1.0f;
#endif
    return result;
}

// XMMatrixRotationX
Matrix4x4 Matrix4x4::CreateRotationX(float degrees) noexcept
{
    float fSinAngle;
    float fCosAngle;
    SinCos(ToRadians(degrees), &fSinAngle, &fCosAngle);

    Matrix4x4 result;
#if defined(ALIMER_USE_SSE)
    simd_float4 vSin = _mm_set_ss(fSinAngle);
    simd_float4 vCos = _mm_set_ss(fCosAngle);
    // x = 0,y = cos,z = sin, w = 0
    vCos = _mm_shuffle_ps(vCos, vSin, _MM_SHUFFLE(3, 0, 0, 3));

    simd_float4x4 simdMatrix;
    simdMatrix.row[0] = g_XMIdentityR0;
    simdMatrix.row[1] = vCos;
    // x = 0,y = sin,z = cos, w = 0
    vCos = XM_PERMUTE_PS(vCos, _MM_SHUFFLE(3, 1, 2, 0));
    // x = 0,y = -sin,z = cos, w = 0
    vCos = _mm_mul_ps(vCos, g_XMNegateY);
    simdMatrix.row[2] = vCos;
    simdMatrix.row[3] = g_XMIdentityR3;
    simd_float4x4_store(simdMatrix, &result);
#else
    // [  1  0  0  0 ]
    // [  0  c  s  0 ]
    // [  0 -s  c  0 ]
    // [  0  0  0  1 ]

    result.m[0][0] = 1.0f;
    result.m[0][1] = 0.0f;
    result.m[0][2] = 0.0f;
    result.m[0][3] = 0.0f;

    result.m21 = 0.0f;
    result.m22 = fCosAngle;
    result.m23 = fSinAngle;
    result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = -fSinAngle;
    result.m33 = fCosAngle;
    result.m34 = 0.0f;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;
#endif
    return result;
}

// XMMatrixRotationY
Matrix4x4 Matrix4x4::CreateRotationY(float degrees) noexcept
{
    float sinAngle;
    float cosAngle;
    SinCos(ToRadians(degrees), &sinAngle, &cosAngle);

    // [  c  0 -s  0 ]
    // [  0  1  0  0 ]
    // [  s  0  c  0 ]
    // [  0  0  0  1 ]
    Matrix4x4 result;
#if ALIMER_SSE2 && !defined(ALIMER_SIMD_DISABLED)
    float32x4 vSin = _mm_set_ss(sinAngle);
    float32x4 vCos = _mm_set_ss(cosAngle);
    // x = sin,y = 0,z = cos, w = 0
    vSin = _mm_shuffle_ps(vSin, vCos, _MM_SHUFFLE(3, 0, 3, 0));
    SimdMatrix4x4 simdMatrix;
    simdMatrix.row[2] = vSin;
    simdMatrix.row[1] = g_XMIdentityR1;
    // x = cos,y = 0,z = sin, w = 0
    vSin = SSE_PERMUTE_PS(vSin, _MM_SHUFFLE(3, 0, 1, 2));
    // x = cos,y = 0,z = -sin, w = 0
    vSin = _mm_mul_ps(vSin, g_XMNegateZ);
    simdMatrix.row[0] = vSin;
    simdMatrix.row[3] = g_XMIdentityR3;
    SimdStoreFloat4x4(&result, simdMatrix);
#else
    result.m11 = cosAngle;
    result.m12 = 0.0f;
    result.m13 = -sinAngle;
    result.m14 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 1.0f;
    result.m23 = 0.0f;
    result.m24 = 0.0f;
    result.m31 = sinAngle;
    result.m32 = 0.0f;
    result.m33 = cosAngle;
    result.m34 = 0.0f;
    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = 0.0f;
    result.m44 = 1.0f;
#endif
    return result;
}

// XMMatrixRotationY
Matrix4x4 Matrix4x4::CreateRotationZ(float degrees) noexcept
{
    float sinAngle;
    float cosAngle;
    SinCos(ToRadians(degrees), &sinAngle, &cosAngle);

    // [  c  s  0  0 ]
    // [ -s  c  0  0 ]
    // [  0  0  1  0 ]
    // [  0  0  0  1 ]
    Matrix4x4 result;
#if ALIMER_SSE2 && defined(TODO)
#else
    result.m11 = cosAngle;
    result.m12 = sinAngle;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m21 = -sinAngle;
    result.m22 = cosAngle;
    result.m23 = 0.0f;
    result.m24 = 0.0f;
    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = 1.0f;
    result.m34 = 0.0f;
    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = 0.0f;
    result.m44 = 1.0f;
#endif
    return result;
}

Matrix4x4 Matrix4x4::CreateScale(const Vector3& scale) noexcept
{
    return CreateScale(scale.x, scale.y, scale.z);
}

Matrix4x4 Matrix4x4::CreateScale(float scaleX, float scaleY, float scaleZ) noexcept
{
    Matrix4x4 result;
#if ALIMER_SSE2 && !defined(ALIMER_SIMD_DISABLED)
    SimdMatrix4x4 simdMatrix;
    simdMatrix.row[0] = _mm_set_ps(0, 0, 0, scaleX);
    simdMatrix.row[1] = _mm_set_ps(0, 0, scaleY, 0);
    simdMatrix.row[2] = _mm_set_ps(0, scaleZ, 0, 0);
    simdMatrix.row[3] = g_XMIdentityR3.v;
    SimdStoreFloat4x4(&result, simdMatrix);
#else
    result.m11 = scaleX;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = scaleY;
    result.m23 = 0.0f;
    result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = scaleZ;
    result.m34 = 0.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = 0.0f;
    result.m44 = 1.0f;
#endif
    return result;
}

Matrix4x4 Matrix4x4::CreateScale(float scale) noexcept
{
    return CreateScale(scale, scale, scale);
}

Matrix4x4 Matrix4x4::CreateScale(const Vector3& scale, const Vector3& centerPoint) noexcept
{
    float tx = centerPoint.x * (1 - scale.x);
    float ty = centerPoint.y * (1 - scale.y);
    float tz = centerPoint.z * (1 - scale.z);

    Matrix4x4 result;
    result.m11 = scale.x;
    result.m22 = scale.y;
    result.m33 = scale.z;
    result.m41 = tx;
    result.m42 = ty;
    result.m43 = tz;
    return result;
}

Matrix4x4 Matrix4x4::CreateScale(float scale, const Vector3& centerPoint) noexcept
{
    return CreateScale({ scale, scale, scale }, centerPoint);
}

Matrix4x4 Matrix4x4::CreateFromAxisAngle(const Vector3& axis, float degrees) noexcept
{
    // a: angle
    // x, y, z: unit vector for axis.
    //
    // Rotation matrix M can compute by using below equation.
    //
    //        T               T
    //  M = uu + (cos a)( I-uu ) + (sin a)S
    //
    // Where:
    //
    //  u = ( x, y, z )
    //
    //      [  0 -z  y ]
    //  S = [  z  0 -x ]
    //      [ -y  x  0 ]
    //
    //      [ 1 0 0 ]
    //  I = [ 0 1 0 ]
    //      [ 0 0 1 ]
    //
    //
    //     [  xx+cosa*(1-xx)   yx-cosa*yx-sina*z zx-cosa*xz+sina*y ]
    // M = [ xy-cosa*yx+sina*z    yy+cosa(1-yy)  yz-cosa*yz-sina*x ]
    //     [ zx-cosa*zx-sina*y zy-cosa*zy+sina*x   zz+cosa*(1-zz)  ]
    //

    float sinAngle;
    float cosAngle;
    SinCos(ToRadians(degrees), &sinAngle, &cosAngle);

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;

    Matrix4x4 result = Identity;

    result.m11 = xx + cosAngle * (1.0f - xx);
    result.m12 = xy - cosAngle * xy + sinAngle * z;
    result.m13 = xz - cosAngle * xz - sinAngle * y;

    result.m21 = xy - cosAngle * xy - sinAngle * z;
    result.m22 = yy + cosAngle * (1.0f - yy);
    result.m23 = yz - cosAngle * yz + sinAngle * x;

    result.m31 = xz - cosAngle * xz + sinAngle * y;
    result.m32 = yz - cosAngle * yz - sinAngle * x;
    result.m33 = zz + cosAngle * (1.0f - zz);

    return result;
}

bool Matrix4x4::Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const
{
    const float DecomposeEpsilon = 0.0001f;

    Vector3* scaleBase = &scale;
    float* pfScales = (float*)scaleBase;
    float det;

    struct CanonicalBasis
    {
        Vector3 Row0;
        Vector3 Row1;
        Vector3 Row2;
    };

    struct VectorBasis
    {
        Vector3* Element0;
        Vector3* Element1;
        Vector3* Element2;
    };

    VectorBasis vectorBasis;
    Vector3** pVectorBasis = (Vector3**)&vectorBasis;

    Matrix4x4 matTemp = Identity;
    CanonicalBasis canonicalBasis = {};
    Vector3* pCanonicalBasis = &canonicalBasis.Row0;

    canonicalBasis.Row0 = Vector3(1.0f, 0.0f, 0.0f);
    canonicalBasis.Row1 = Vector3(0.0f, 1.0f, 0.0f);
    canonicalBasis.Row2 = Vector3(0.0f, 0.0f, 1.0f);

    translation.x = m41;
    translation.y = m42;
    translation.z = m43;

    pVectorBasis[0] = (Vector3*)&matTemp.m11;
    pVectorBasis[1] = (Vector3*)&matTemp.m21;
    pVectorBasis[2] = (Vector3*)&matTemp.m31;

    *(pVectorBasis[0]) = Vector3(m11, m12, m13);
    *(pVectorBasis[1]) = Vector3(m21, m22, m23);
    *(pVectorBasis[2]) = Vector3(m31, m32, m33);

    scale.x = pVectorBasis[0]->Length();
    scale.y = pVectorBasis[1]->Length();
    scale.z = pVectorBasis[2]->Length();

    uint32_t a, b, c;
    float x = pfScales[0], y = pfScales[1], z = pfScales[2];
    if (x < y)
    {
        if (y < z)
        {
            a = 2;
            b = 1;
            c = 0;
        }
        else
        {
            a = 1;

            if (x < z)
            {
                b = 2;
                c = 0;
            }
            else
            {
                b = 0;
                c = 2;
            }
        }
    }
    else
    {
        if (x < z)
        {
            a = 2;
            b = 0;
            c = 1;
        }
        else
        {
            a = 0;

            if (y < z)
            {
                b = 2;
                c = 1;
            }
            else
            {
                b = 1;
                c = 2;
            }
        }
    }

    if (pfScales[a] < DecomposeEpsilon)
    {
        *(pVectorBasis[a]) = pCanonicalBasis[a];
    }

    *pVectorBasis[a] = Vector3::Normalize(*pVectorBasis[a]);

    if (pfScales[b] < DecomposeEpsilon)
    {
        uint32_t cc;
        float fAbsX, fAbsY, fAbsZ;

        fAbsX = Alimer::Abs(pVectorBasis[a]->x);
        fAbsY = Alimer::Abs(pVectorBasis[a]->y);
        fAbsZ = Alimer::Abs(pVectorBasis[a]->z);

        if (fAbsX < fAbsY)
        {
            if (fAbsY < fAbsZ)
            {
                cc = 0;
            }
            else
            {
                if (fAbsX < fAbsZ)
                {
                    cc = 0;
                }
                else
                {
                    cc = 2;
                }
            }
        }
        else
        {
            if (fAbsX < fAbsZ)
            {
                cc = 1;
            }
            else
            {
                if (fAbsY < fAbsZ)
                {
                    cc = 1;
                }
                else
                {
                    cc = 2;
                }
            }
        }

        *pVectorBasis[b] = Vector3::Cross(*pVectorBasis[a], *(pCanonicalBasis + cc));
    }

    *pVectorBasis[b] = Vector3::Normalize(*pVectorBasis[b]);

    if (pfScales[c] < DecomposeEpsilon)
    {
        *pVectorBasis[c] = Vector3::Cross(*pVectorBasis[a], *pVectorBasis[b]);
    }

    *pVectorBasis[c] = Vector3::Normalize(*pVectorBasis[c]);

    det = matTemp.GetDeterminant();

    // use Kramer's rule to check for handedness of coordinate system
    if (det < 0.0f)
    {
        // switch coordinate system by negating the scale and inverting the basis vector on the x-axis
        pfScales[a] = -pfScales[a];
        *pVectorBasis[a] = -(*pVectorBasis[a]);

        det = -det;
    }

    det -= 1.0f;
    det *= det;

    if ((DecomposeEpsilon < det))
    {
        // Non-SRT matrix encountered
        return false;
    }

    // generate the quaternion from the matrix
    rotation = Quaternion::CreateFromRotationMatrix(matTemp);
    return true;
}

Vector3 Matrix4x4::ToEuler() const noexcept
{
    const float cy = sqrtf(m33 * m33 + m31 * m31);
    const float cx = atan2f(-m32, cy);
    if (cy > 16.f * FLT_EPSILON)
    {
        return Vector3(cx, atan2f(m31, m33), atan2f(m12, m22));
    }

    return Vector3(cx, 0.0f, atan2f(-m21, m11));
}

Matrix4x4 Matrix4x4::CreatePerspectiveInfiniteReverseZ(float fieldOfView, float aspectRatio, float zNearPlane) noexcept
{
    ALIMER_ASSERT(zNearPlane > 0.0f);
    ALIMER_ASSERT(!MathF::NearEqual(fieldOfView, 0.0f, 0.00001f * 2.0f));
    ALIMER_ASSERT(!MathF::NearEqual(aspectRatio, 0.0f, 0.00001f));

    const float height = 1.0f / MathF::Tan(fieldOfView * 0.5f);
    const float width = height / aspectRatio;

    Matrix4x4 result;
#if defined(ALIMER_USE_SSE4_1)
    __m128 vTemp = _mm_setr_ps(width, height, 0.0f, zNearPlane);
    const __m128 vOne = _mm_set1_ps(-1.0f);

    simd_float4x4 simdMatrix;
    simdMatrix.row[0] = _mm_insert_ps(vTemp, vTemp, 0xe);
    simdMatrix.row[1] = _mm_insert_ps(vTemp, vTemp, 0xd);
    simdMatrix.row[2] = _mm_insert_ps(vTemp, vOne, 0x33);
    simdMatrix.row[3] = _mm_insert_ps(vTemp, vTemp, 0xeb);
    simd_float4x4_store(simdMatrix, &result);
#elif defined(ALIMER_USE_SSE)
    __m128 vTemp = _mm_setr_ps(width, height, 0.0f, zNearPlane);
    const __m128 vOne = _mm_set1_ps(-1.0f);
    const __m128 vZero = _mm_setzero_ps();

    simd_float4x4 simdMatrix;
    simdMatrix.row[0] = _mm_setr_ps(width, 0.0f, 0.0f, 0.0f);
    simdMatrix.row[1] = _mm_setr_ps(0.0f, height, 0.0f, 0.0f);
    simdMatrix.row[2] = _mm_setr_ps(0.0f, 0.0f, 0.0f, -1.0f);
    simdMatrix.row[3] = _mm_setr_ps(0.0f, 0.0f, zNearPlane, 0.0f);
    simd_float4x4_store(simdMatrix, &result);
#else
    result.m11 = width;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = height;
    result.m23 = 0.0f;
    result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = 0.0f;
    result.m34 = -1.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = zNearPlane;
    result.m44 = 0.0f;
#endif

    return result;
}

// XMMatrixPerspectiveFovRH
Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfView(float fovAngleY, float aspectRatio, float zNearPlane, float zFarPlane) noexcept
{
    ALIMER_ASSERT(zNearPlane > 0.0f && zFarPlane > 0.0f);
    ALIMER_ASSERT(!MathF::NearEqual(ToRadians(fovAngleY), 0.0f, 0.00001f * 2.0f));
    ALIMER_ASSERT(!MathF::NearEqual(aspectRatio, 0.0f, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(zFarPlane, zNearPlane, 0.00001f));

    float sinFov;
    float cosFov;
    SinCos(0.5f * fovAngleY, &sinFov, &cosFov);

    const float height = cosFov / sinFov;
    const float width = height / aspectRatio;
    const float range = Alimer::IsInf(zFarPlane) ? -1.0f : zFarPlane / (zNearPlane - zFarPlane);

    Matrix4x4 result;
    result.m11 = width;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = height;
    result.m23 = 0.0f;
    result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = range;
    result.m34 = -1.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = range * zNearPlane;
    result.m44 = 0.0f;

    return result;
}

Matrix4x4 Matrix4x4::CreatePerspective(float width, float height, float zNearPlane, float zFarPlane) noexcept
{
    ALIMER_ASSERT(zNearPlane > 0.0f && zFarPlane > 0.0f);
    ALIMER_ASSERT(!MathF::NearEqual(width, 0.0f, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(height, 0.0f, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(zFarPlane, zNearPlane, 0.00001f));

    float twoNearZ = zNearPlane + zNearPlane;
    float range = zFarPlane / (zFarPlane - zNearPlane);

    Matrix4x4 result;
    result.m11 = twoNearZ / width;
    result.m12 = result.m13 = result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = twoNearZ / height;
    result.m23 = result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = range;
    result.m34 = 1.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = -range * zNearPlane;
    result.m44 = 0.0f;
    return result;
}

Matrix4x4 Matrix4x4::CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept
{
    ALIMER_ASSERT(zNearPlane > 0.0f && zFarPlane > 0.0f);
    ALIMER_ASSERT(!MathF::NearEqual(zFarPlane, zNearPlane, 0.00001f));

    float dblNearPlaneDistance = zNearPlane + zNearPlane;
    float reciprocalWidth = 1.0f / (right - left);
    float reciprocalHeight = 1.0f / (top - bottom);
    float range = Alimer::IsInf(zFarPlane) ? 1.0f : zFarPlane / (zFarPlane - zNearPlane);

    Matrix4x4 result;
    result.m11 = dblNearPlaneDistance * reciprocalWidth;
    result.m12 = result.m13 = result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = dblNearPlaneDistance * reciprocalHeight;
    result.m23 = result.m24 = 0.0f;

    result.m31 = -(left + right) * reciprocalWidth;
    result.m32 = -(top + bottom) * reciprocalHeight;
    result.m33 = range;
    result.m34 = 1.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = -range * zNearPlane;
    result.m44 = 0.0f;

    return result;
}

Matrix4x4 Matrix4x4::CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept
{
    ALIMER_ASSERT(!MathF::NearEqual(width, 0.0f, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(height, 0.0f, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(zFarPlane, zNearPlane, 0.00001f));

    const float range = 1.0f / (zNearPlane - zFarPlane);

    Matrix4x4 result = Identity;

    result.m11 = 2.0f / width;
    result.m22 = 2.0f / height;
    result.m33 = range;
    result.m43 = range * zNearPlane;
    return result;
}

Matrix4x4 Matrix4x4::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept
{
    ALIMER_ASSERT(!MathF::NearEqual(right, left, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(top, bottom, 0.00001f));
    ALIMER_ASSERT(!MathF::NearEqual(zFarPlane, zNearPlane, 0.00001f));

    const float reciprocalWidth = 1.0f / (right - left);
    const float reciprocalHeight = 1.0f / (top - bottom);
    const float range = 1.0f / (zNearPlane - zFarPlane);

    Matrix4x4 result;
    result.m11 = reciprocalWidth + reciprocalWidth;
    result.m12 = result.m13 = result.m14 = 0.0f;

    result.m22 = reciprocalHeight + reciprocalHeight;
    result.m21 = result.m23 = result.m24 = 0.0f;

    result.m33 = range;
    result.m31 = result.m32 = result.m34 = 0.0f;

    result.m41 = -(left + right) * reciprocalWidth;
    result.m42 = -(top + bottom) * reciprocalHeight;
    result.m43 = range * zNearPlane;
    result.m44 = 1.0f;
    return result;
}

Matrix4x4 Matrix4x4::CreateLookTo(const Vector3& cameraPosition, const Vector3& cameraDirection, const Vector3& cameraUpVector) noexcept
{
    ALIMER_ASSERT(!cameraDirection.IsZero());
    ALIMER_ASSERT(!cameraDirection.IsInf());
    ALIMER_ASSERT(!cameraUpVector.IsZero());
    ALIMER_ASSERT(!cameraUpVector.IsInf());

    Vector3 axisZ = Vector3::Normalize(cameraDirection);
    Vector3 axisX = Vector3::Normalize(Vector3::Cross(cameraUpVector, axisZ));
    Vector3 axisY = Vector3::Cross(axisZ, axisX);
    Vector3 negativeCameraPosition = -cameraPosition;

    Matrix4x4 result;
    result.m11 = axisX.x;
    result.m12 = axisY.x;
    result.m13 = axisZ.x;
    result.m14 = 0.0f;

    result.m21 = axisX.y;
    result.m22 = axisY.y;
    result.m23 = axisZ.y;
    result.m24 = 0.0f;

    result.m31 = axisX.z;
    result.m32 = axisY.z;
    result.m33 = axisZ.z;
    result.m34 = 0.0f;

    result.m41 = Vector3::Dot(axisX, negativeCameraPosition);
    result.m42 = Vector3::Dot(axisY, negativeCameraPosition);
    result.m43 = Vector3::Dot(axisZ, negativeCameraPosition);
    result.m44 = 1.0f;

    return result;
}

Matrix4x4 Matrix4x4::CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector) noexcept
{
    Vector3 cameraDirection = Vector3::Subtract(cameraTarget, cameraPosition);
    return CreateLookTo(cameraPosition, cameraDirection, cameraUpVector);
}

// XMMatrixRotationQuaternion
Matrix4x4 Matrix4x4::CreateFromQuaternion(const Quaternion& rotation) noexcept
{
    Matrix4x4 result = Identity;
#if defined(ALIMER_USE_SSE)
    static const VectorF32  Constant1110 = { { { 1.0f, 1.0f, 1.0f, 0.0f } } };

    __m128 Q0 = _mm_add_ps(rotation, rotation);
    __m128 Q1 = _mm_mul_ps(rotation, Q0);

    __m128 V0 = XM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 0, 0, 1));
    V0 = _mm_and_ps(V0, g_XMMask3);
    __m128 V1 = XM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 1, 2, 2));
    V1 = _mm_and_ps(V1, g_XMMask3);
    __m128 R0 = _mm_sub_ps(Constant1110, V0);
    R0 = _mm_sub_ps(R0, V1);

    V0 = XM_PERMUTE_PS(rotation, _MM_SHUFFLE(3, 1, 0, 0));
    V1 = XM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 2, 1, 2));
    V0 = _mm_mul_ps(V0, V1);

    V1 = XM_PERMUTE_PS(rotation, _MM_SHUFFLE(3, 3, 3, 3));
    __m128 V2 = XM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 0, 2, 1));
    V1 = _mm_mul_ps(V1, V2);

    __m128 R1 = _mm_add_ps(V0, V1);
    __m128 R2 = _mm_sub_ps(V0, V1);

    V0 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(1, 0, 2, 1));
    V0 = XM_PERMUTE_PS(V0, _MM_SHUFFLE(1, 3, 2, 0));
    V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 2, 0, 0));
    V1 = XM_PERMUTE_PS(V1, _MM_SHUFFLE(2, 0, 2, 0));

    Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(1, 0, 3, 0));
    Q1 = XM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 2, 0));

    simd_float4x4 simdMatrix;
    simdMatrix.row[0] = Q1;

    Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(3, 2, 3, 1));
    Q1 = XM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 0, 2));
    simdMatrix.row[1] = Q1;

    Q1 = _mm_shuffle_ps(V1, R0, _MM_SHUFFLE(3, 2, 1, 0));
    simdMatrix.row[2] = Q1;
    simdMatrix.row[3] = g_XMIdentityR3;
    simd_float4x4_store(simdMatrix, &result);
#else
    const float xx = rotation.x * rotation.x;
    const float yy = rotation.y * rotation.y;
    const float zz = rotation.z * rotation.z;

    const float xy = rotation.x * rotation.y;
    const float wz = rotation.z * rotation.w;
    const float xz = rotation.z * rotation.x;
    const float wy = rotation.y * rotation.w;
    const float yz = rotation.y * rotation.z;
    const float wx = rotation.x * rotation.w;

    result.m11 = 1.0f - 2.0f * (yy + zz);
    result.m12 = 2.0f * (xy + wz);
    result.m13 = 2.0f * (xz - wy);

    result.m21 = 2.0f * (xy - wz);
    result.m22 = 1.0f - 2.0f * (zz + xx);
    result.m23 = 2.0f * (yz + wx);

    result.m31 = 2.0f * (xz + wy);
    result.m32 = 2.0f * (yz - wx);
    result.m33 = 1.0f - 2.0f * (yy + xx);
#endif
    return result;
}

Matrix4x4 Matrix4x4::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    Quaternion q = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
    return CreateFromQuaternion(q);
}

Matrix4x4 Matrix4x4::CreateViewport(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept
{
    Matrix4x4 result;

    result.m41 = width * 0.5f;
    result.m42 = height * 0.5f;
    result.m43 = 0.0f;
    result.m44 = 1.0f;

    result.m11 = result.m41;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = -result.m42;
    result.m23 = 0.0f;
    result.m24 = 0.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    result.m33 = minDepth - maxDepth;
    result.m34 = 0.0f;

    result.m41 += x;
    result.m42 += y;
    result.m43 += minDepth;
    result.m44 = 1.0f;

    return result;
}

Matrix4x4 Matrix4x4::operator *(const Matrix4x4& rhs) const noexcept
{
    Matrix4x4 result;
    // First row
    result.m11 = m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31 + m14 * rhs.m41;
    result.m12 = m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32 + m14 * rhs.m42;
    result.m13 = m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33 + m14 * rhs.m43;
    result.m14 = m11 * rhs.m14 + m12 * rhs.m24 + m13 * rhs.m34 + m14 * rhs.m44;

    // Second row
    result.m21 = m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31 + m24 * rhs.m41;
    result.m22 = m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32 + m24 * rhs.m42;
    result.m23 = m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33 + m24 * rhs.m43;
    result.m24 = m21 * rhs.m14 + m22 * rhs.m24 + m23 * rhs.m34 + m24 * rhs.m44;

    // Third row
    result.m31 = m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31 + m34 * rhs.m41;
    result.m32 = m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32 + m34 * rhs.m42;
    result.m33 = m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33 + m34 * rhs.m43;
    result.m34 = m31 * rhs.m14 + m32 * rhs.m24 + m33 * rhs.m34 + m34 * rhs.m44;

    // Fourth row
    result.m41 = m41 * rhs.m11 + m42 * rhs.m21 + m43 * rhs.m31 + m44 * rhs.m41;
    result.m42 = m41 * rhs.m12 + m42 * rhs.m22 + m43 * rhs.m32 + m44 * rhs.m42;
    result.m43 = m41 * rhs.m13 + m42 * rhs.m23 + m43 * rhs.m33 + m44 * rhs.m43;
    result.m44 = m41 * rhs.m14 + m42 * rhs.m24 + m43 * rhs.m34 + m44 * rhs.m44;
    return result;
}

Matrix4x4 Matrix4x4::operator *(float scalar) const noexcept
{
    Matrix4x4 result;

    result.m11 = m11 * scalar;
    result.m12 = m12 * scalar;
    result.m13 = m13 * scalar;
    result.m14 = m14 * scalar;
    result.m21 = m21 * scalar;
    result.m22 = m22 * scalar;
    result.m23 = m23 * scalar;
    result.m24 = m24 * scalar;
    result.m31 = m31 * scalar;
    result.m32 = m32 * scalar;
    result.m33 = m33 * scalar;
    result.m34 = m34 * scalar;
    result.m41 = m41 * scalar;
    result.m42 = m42 * scalar;
    result.m43 = m43 * scalar;
    result.m44 = m44 * scalar;
    return result;
}

bool Matrix4x4::Invert(const Matrix4x4& matrix, Matrix4x4* result) noexcept
{
    ALIMER_ASSERT(result);

#if ALIMER_SSE2 && !defined(ALIMER_SIMD_DISABLED)
    SimdMatrix4x4 simdMatrix = SimdLoadFloat4x4(&matrix);
    // Transpose matrix
    float32x4 vTemp1 = _mm_shuffle_ps(simdMatrix.row[0], simdMatrix.row[1], _MM_SHUFFLE(1, 0, 1, 0));
    float32x4 vTemp3 = _mm_shuffle_ps(simdMatrix.row[0], simdMatrix.row[1], _MM_SHUFFLE(3, 2, 3, 2));
    float32x4 vTemp2 = _mm_shuffle_ps(simdMatrix.row[2], simdMatrix.row[3], _MM_SHUFFLE(1, 0, 1, 0));
    float32x4 vTemp4 = _mm_shuffle_ps(simdMatrix.row[2], simdMatrix.row[3], _MM_SHUFFLE(3, 2, 3, 2));

    SimdMatrix4x4 MT;
    MT.row[0] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(2, 0, 2, 0));
    MT.row[1] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(3, 1, 3, 1));
    MT.row[2] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(2, 0, 2, 0));
    MT.row[3] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(3, 1, 3, 1));

    float32x4 V00 = SSE_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(1, 1, 0, 0));
    float32x4 V10 = SSE_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(3, 2, 3, 2));
    float32x4 V01 = SSE_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(1, 1, 0, 0));
    float32x4 V11 = SSE_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(3, 2, 3, 2));
    float32x4 V02 = _mm_shuffle_ps(MT.row[2], MT.row[0], _MM_SHUFFLE(2, 0, 2, 0));
    float32x4 V12 = _mm_shuffle_ps(MT.row[3], MT.row[1], _MM_SHUFFLE(3, 1, 3, 1));

    float32x4 D0 = _mm_mul_ps(V00, V10);
    float32x4 D1 = _mm_mul_ps(V01, V11);
    float32x4 D2 = _mm_mul_ps(V02, V12);

    V00 = SSE_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(3, 2, 3, 2));
    V10 = SSE_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(1, 1, 0, 0));
    V01 = SSE_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(3, 2, 3, 2));
    V11 = SSE_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(1, 1, 0, 0));
    V02 = _mm_shuffle_ps(MT.row[2], MT.row[0], _MM_SHUFFLE(3, 1, 3, 1));
    V12 = _mm_shuffle_ps(MT.row[3], MT.row[1], _MM_SHUFFLE(2, 0, 2, 0));

    D0 = XM_FNMADD_PS(V00, V10, D0);
    D1 = XM_FNMADD_PS(V01, V11, D1);
    D2 = XM_FNMADD_PS(V02, V12, D2);
    // V11 = D0Y,D0W,D2Y,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));
    V00 = SSE_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(1, 0, 2, 1));
    V10 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(0, 3, 0, 2));
    V01 = SSE_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(0, 1, 0, 2));
    V11 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(2, 1, 2, 1));
    // V13 = D1Y,D1W,D2W,D2W
    float32x4 V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));
    V02 = SSE_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(1, 0, 2, 1));
    V12 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(0, 3, 0, 2));
    float32x4 V03 = SSE_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(0, 1, 0, 2));
    V13 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(2, 1, 2, 1));

    float32x4 C0 = _mm_mul_ps(V00, V10);
    float32x4 C2 = _mm_mul_ps(V01, V11);
    float32x4 C4 = _mm_mul_ps(V02, V12);
    float32x4 C6 = _mm_mul_ps(V03, V13);

    // V11 = D0X,D0Y,D2X,D2X
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));
    V00 = SSE_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(2, 1, 3, 2));
    V10 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V01 = SSE_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(1, 3, 2, 3));
    V11 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(0, 2, 1, 2));
    // V13 = D1X,D1Y,D2Z,D2Z
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));
    V02 = SSE_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(2, 1, 3, 2));
    V12 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(2, 1, 0, 3));
    V03 = SSE_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(1, 3, 2, 3));
    V13 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(0, 2, 1, 2));

    C0 = XM_FNMADD_PS(V00, V10, C0);
    C2 = XM_FNMADD_PS(V01, V11, C2);
    C4 = XM_FNMADD_PS(V02, V12, C4);
    C6 = XM_FNMADD_PS(V03, V13, C6);

    V00 = SSE_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(0, 3, 0, 3));
    // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
    V10 = SSE_PERMUTE_PS(V10, _MM_SHUFFLE(0, 2, 3, 0));
    V01 = SSE_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(2, 0, 3, 1));
    // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
    V11 = SSE_PERMUTE_PS(V11, _MM_SHUFFLE(2, 1, 0, 3));
    V02 = SSE_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(0, 3, 0, 3));
    // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
    V12 = SSE_PERMUTE_PS(V12, _MM_SHUFFLE(0, 2, 3, 0));
    V03 = SSE_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(2, 0, 3, 1));
    // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
    V13 = SSE_PERMUTE_PS(V13, _MM_SHUFFLE(2, 1, 0, 3));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    float32x4 C1 = _mm_sub_ps(C0, V00);
    C0 = _mm_add_ps(C0, V00);
    float32x4 C3 = _mm_add_ps(C2, V01);
    C2 = _mm_sub_ps(C2, V01);
    float32x4 C5 = _mm_sub_ps(C4, V02);
    C4 = _mm_add_ps(C4, V02);
    float32x4 C7 = _mm_add_ps(C6, V03);
    C6 = _mm_sub_ps(C6, V03);

    C0 = _mm_shuffle_ps(C0, C1, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C3, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C5, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C7, _MM_SHUFFLE(3, 1, 2, 0));
    C0 = SSE_PERMUTE_PS(C0, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = SSE_PERMUTE_PS(C2, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = SSE_PERMUTE_PS(C4, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = SSE_PERMUTE_PS(C6, _MM_SHUFFLE(3, 1, 2, 0));
    // Get the determinant
    float32x4 vTemp = Simd::Vec4::Dot(C0, MT.row[0]);
    //if (pDeterminant != nullptr)
    //    *pDeterminant = vTemp;
    vTemp = _mm_div_ps(g_XMOne, vTemp);
    SimdMatrix4x4 simdResult;
    simdResult.row[0] = _mm_mul_ps(C0, vTemp);
    simdResult.row[1] = _mm_mul_ps(C2, vTemp);
    simdResult.row[2] = _mm_mul_ps(C4, vTemp);
    simdResult.row[3] = _mm_mul_ps(C6, vTemp);
    Matrix4x4 result;
    SimdStoreFloat4x4(&result, simdResult);
    return result;
#else
    float a = matrix.m11, b = matrix.m12, c = matrix.m13, d = matrix.m14;
    float e = matrix.m21, f = matrix.m22, g = matrix.m23, h = matrix.m24;
    float i = matrix.m31, j = matrix.m32, k = matrix.m33, l = matrix.m34;
    float m = matrix.m41, n = matrix.m42, o = matrix.m43, p = matrix.m44;

    float kp_lo = k * p - l * o;
    float jp_ln = j * p - l * n;
    float jo_kn = j * o - k * n;
    float ip_lm = i * p - l * m;
    float io_km = i * o - k * m;
    float in_jm = i * n - j * m;

    float a11 = +(f * kp_lo - g * jp_ln + h * jo_kn);
    float a12 = -(e * kp_lo - g * ip_lm + h * io_km);
    float a13 = +(e * jp_ln - f * ip_lm + h * in_jm);
    float a14 = -(e * jo_kn - f * io_km + g * in_jm);

    float det = a * a11 + b * a12 + c * a13 + d * a14;

    if (MathF::Abs(det) < FLT_EPSILON)
    {
        const float NaN = std::numeric_limits<float>::quiet_NaN();

        result->m11 = NaN;
        result->m12 = NaN;
        result->m13 = NaN;
        result->m14 = NaN;

        result->m21 = NaN;
        result->m22 = NaN;
        result->m23 = NaN;
        result->m24 = NaN;

        result->m31 = NaN;
        result->m32 = NaN;
        result->m33 = NaN;
        result->m34 = NaN;

        result->m41 = NaN;
        result->m42 = NaN;
        result->m43 = NaN;
        result->m44 = NaN;

        return false;
    }

    float invDet = 1.0f / det;

    result->m11 = a11 * invDet;
    result->m21 = a12 * invDet;
    result->m31 = a13 * invDet;
    result->m41 = a14 * invDet;

    result->m12 = -(b * kp_lo - c * jp_ln + d * jo_kn) * invDet;
    result->m22 = +(a * kp_lo - c * ip_lm + d * io_km) * invDet;
    result->m32 = -(a * jp_ln - b * ip_lm + d * in_jm) * invDet;
    result->m42 = +(a * jo_kn - b * io_km + c * in_jm) * invDet;

    float gp_ho = g * p - h * o;
    float fp_hn = f * p - h * n;
    float fo_gn = f * o - g * n;
    float ep_hm = e * p - h * m;
    float eo_gm = e * o - g * m;
    float en_fm = e * n - f * m;

    result->m13 = +(b * gp_ho - c * fp_hn + d * fo_gn) * invDet;
    result->m23 = -(a * gp_ho - c * ep_hm + d * eo_gm) * invDet;
    result->m33 = +(a * fp_hn - b * ep_hm + d * en_fm) * invDet;
    result->m43 = -(a * fo_gn - b * eo_gm + c * en_fm) * invDet;

    float gl_hk = g * l - h * k;
    float fl_hj = f * l - h * j;
    float fk_gj = f * k - g * j;
    float el_hi = e * l - h * i;
    float ek_gi = e * k - g * i;
    float ej_fi = e * j - f * i;

    result->m14 = -(b * gl_hk - c * fl_hj + d * fk_gj) * invDet;
    result->m24 = +(a * gl_hk - c * el_hi + d * ek_gi) * invDet;
    result->m34 = -(a * fl_hj - b * el_hi + d * ej_fi) * invDet;
    result->m44 = +(a * fk_gj - b * ek_gi + c * ej_fi) * invDet;

    return true;
#endif
}

Matrix4x4 Matrix4x4::Transpose(const Matrix4x4& value)
{
    Matrix4x4 result;
    Matrix4x4::Transpose(value, result);
    return result;
}

void Matrix4x4::Transpose(const Matrix4x4& value, Matrix4x4& result)
{
    result.m11 = value.m11;
    result.m12 = value.m21;
    result.m13 = value.m31;
    result.m14 = value.m41;
    result.m21 = value.m12;
    result.m22 = value.m22;
    result.m23 = value.m32;
    result.m24 = value.m42;
    result.m31 = value.m13;
    result.m32 = value.m23;
    result.m33 = value.m33;
    result.m34 = value.m43;
    result.m41 = value.m14;
    result.m42 = value.m24;
    result.m43 = value.m34;
    result.m44 = value.m44;
}

void Matrix4x4::Lerp(const Matrix4x4& matrix1, const Matrix4x4& matrix2, float amount, Matrix4x4& result) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    simd_float4 x1 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m11));
    simd_float4 x2 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m21));
    simd_float4 x3 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m31));
    simd_float4 x4 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m41));

    const simd_float4 y1 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m11));
    const simd_float4 y2 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m21));
    const simd_float4 y3 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m31));
    const simd_float4 y4 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m41));

    x1 = simd_float4_lerp(x1, y1, amount);
    x2 = simd_float4_lerp(x2, y2, amount);
    x3 = simd_float4_lerp(x3, y3, amount);
    x4 = simd_float4_lerp(x4, y4, amount);

    simd_float4_store(reinterpret_cast<Vector4*>(&result.m11), x1);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m21), x2);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m31), x3);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m41), x4);
#else
    result.m11 = MathF::Lerp(matrix1.m11, matrix2.m11, amount);
    result.m12 = MathF::Lerp(matrix1.m12, matrix2.m12, amount);
    result.m13 = MathF::Lerp(matrix1.m13, matrix2.m13, amount);
    result.m14 = MathF::Lerp(matrix1.m14, matrix2.m14, amount);
    result.m21 = MathF::Lerp(matrix1.m21, matrix2.m21, amount);
    result.m22 = MathF::Lerp(matrix1.m22, matrix2.m22, amount);
    result.m23 = MathF::Lerp(matrix1.m23, matrix2.m23, amount);
    result.m24 = MathF::Lerp(matrix1.m24, matrix2.m24, amount);
    result.m31 = MathF::Lerp(matrix1.m31, matrix2.m31, amount);
    result.m32 = MathF::Lerp(matrix1.m32, matrix2.m32, amount);
    result.m33 = MathF::Lerp(matrix1.m33, matrix2.m33, amount);
    result.m34 = MathF::Lerp(matrix1.m34, matrix2.m34, amount);
    result.m41 = MathF::Lerp(matrix1.m41, matrix2.m41, amount);
    result.m42 = MathF::Lerp(matrix1.m42, matrix2.m42, amount);
    result.m43 = MathF::Lerp(matrix1.m43, matrix2.m43, amount);
    result.m44 = MathF::Lerp(matrix1.m44, matrix2.m44, amount);
#endif
}

Matrix4x4 Matrix4x4::Lerp(const Matrix4x4& matrix1, const Matrix4x4& matrix2, float amount) noexcept
{
    Matrix4x4 result;
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    simd_float4 x1 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m11));
    simd_float4 x2 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m21));
    simd_float4 x3 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m31));
    simd_float4 x4 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix1.m41));

    const simd_float4 y1 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix2.m11));
    const simd_float4 y2 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix2.m21));
    const simd_float4 y3 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix2.m31));
    const simd_float4 y4 = simd_float4_load(reinterpret_cast<const Vector4*>(&matrix2.m41));

    x1 = simd_float4_lerp(x1, y1, amount);
    x2 = simd_float4_lerp(x2, y2, amount);
    x3 = simd_float4_lerp(x3, y3, amount);
    x4 = simd_float4_lerp(x4, y4, amount);

    simd_float4_store(reinterpret_cast<Vector4*>(&result.m11), x1);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m21), x2);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m31), x3);
    simd_float4_store(reinterpret_cast<Vector4*>(&result.m41), x4);
#else
    Matrix4x4::Lerp(matrix1, matrix2, amount, result);
#endif
    return result;
}

void Matrix4x4::Transform(const Matrix4x4& matrix, const Quaternion& rotation, Matrix4x4& result) noexcept
{
    // Compute rotation matrix.
    float x2 = rotation.x + rotation.x;
    float y2 = rotation.y + rotation.y;
    float z2 = rotation.z + rotation.z;

    float wx2 = rotation.w * x2;
    float wy2 = rotation.w * y2;
    float wz2 = rotation.w * z2;

    float xx2 = rotation.x * x2;
    float xy2 = rotation.x * y2;
    float xz2 = rotation.x * z2;

    float yy2 = rotation.y * y2;
    float yz2 = rotation.y * z2;
    float zz2 = rotation.y * z2;

    float q11 = 1.0f - yy2 - zz2;
    float q21 = xy2 - wz2;
    float q31 = xz2 + wy2;

    float q12 = xy2 + wz2;
    float q22 = 1.0f - xx2 - zz2;
    float q32 = yz2 - wx2;

    float q13 = xz2 - wy2;
    float q23 = yz2 + wx2;
    float q33 = 1.0f - xx2 - yy2;

    result.m11 = matrix.m11 * q11 + matrix.m12 * q21 + matrix.m13 * q31;
    result.m12 = matrix.m11 * q12 + matrix.m12 * q22 + matrix.m13 * q32;
    result.m13 = matrix.m11 * q13 + matrix.m12 * q23 + matrix.m13 * q33;
    result.m14 = matrix.m14;

    result.m21 = matrix.m21 * q11 + matrix.m22 * q21 + matrix.m23 * q31;
    result.m22 = matrix.m21 * q12 + matrix.m22 * q22 + matrix.m23 * q32;
    result.m23 = matrix.m21 * q13 + matrix.m22 * q23 + matrix.m23 * q33;
    result.m24 = matrix.m24;

    result.m31 = matrix.m31 * q11 + matrix.m32 * q21 + matrix.m33 * q31;
    result.m32 = matrix.m31 * q12 + matrix.m32 * q22 + matrix.m33 * q32;
    result.m33 = matrix.m31 * q13 + matrix.m32 * q23 + matrix.m33 * q33;
    result.m34 = matrix.m34;

    result.m41 = matrix.m41 * q11 + matrix.m42 * q21 + matrix.m43 * q31;
    result.m42 = matrix.m41 * q12 + matrix.m42 * q22 + matrix.m43 * q32;
    result.m43 = matrix.m41 * q13 + matrix.m42 * q23 + matrix.m43 * q33;
    result.m44 = matrix.m44;
}

Matrix4x4 Matrix4x4::Transform(const Matrix4x4& matrix, const Quaternion& rotation) noexcept
{
    Matrix4x4 result;
    Matrix4x4::Transform(matrix, rotation, result);
    return result;
}


std::string Matrix4x4::ToString() const
{
    char tempBuffer[kMatrixConversionBufferLength];
    sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
        m11, m12, m13, m14,
        m21, m22, m23, m24,
        m31, m32, m33, m34,
        m41, m42, m43, m44);
    return std::string(tempBuffer);
}

