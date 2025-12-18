// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Vector3.h"
#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

const Vector3 Vector3::Zero = { 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::One = { 1.0f, 1.0f, 1.0f };
const Vector3 Vector3::UnitX = { 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::UnitY = { 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::UnitZ = { 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::Left = { -1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Right = { 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Up = { 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::Down = { 0.0f, -1.0f, 0.0f };
const Vector3 Vector3::Forward = { 0.0f, 0.0f, -1.0f };
const Vector3 Vector3::Backward = { 0.0f, 0.0f, 1.0f };

void Vector3::Set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector3::Set(const Vector3& rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
}

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"

Vector3::Vector3(simd_float4_param xyz)
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
{
#if defined(ALIMER_USE_NEON)
    float32x2_t VL = vget_low_f32(xyz);
    vst1_f32(reinterpret_cast<float*>(this), VL);
    vst1q_lane_f32(reinterpret_cast<float*>(this) + 2, xyz, 2);
#elif defined(ALIMER_USE_SSE4_1)
    * reinterpret_cast<int*>(&x) = _mm_extract_ps(xyz, 0);
    *reinterpret_cast<int*>(&y) = _mm_extract_ps(xyz, 1);
    *reinterpret_cast<int*>(&z) = _mm_extract_ps(xyz, 2);
#elif defined(ALIMER_USE_SSE)
    _mm_store_sd(reinterpret_cast<double*>(this), _mm_castps_pd(xyz));
    __m128 z = XM_PERMUTE_PS(xyz, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&this->z, z);
#endif
}

simd_float4 Vector3::ToSIMD() const
{
#if defined(ALIMER_USE_NEON)
    float32x2_t x = vld1_f32(reinterpret_cast<const float*>(this));
    float32x2_t zero = vdup_n_f32(0);
    float32x2_t y = vld1_lane_f32(reinterpret_cast<const float*>(this) + 2, zero, 0);
    return vcombine_f32(x, y);
#elif defined(ALIMER_USE_SSE4_1)
    __m128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(this)));
    __m128 z = _mm_load_ss(&this->z);
    return _mm_insert_ps(xy, z, 0x20);
#elif defined(ALIMER_USE_SSE)
    __m128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(this)));
    __m128 z = _mm_load_ss(&this->z);
    return _mm_movelh_ps(xy, z);
#endif
}
#endif

size_t Vector3::GetHashCode() const
{
    size_t hash = 0;
    HashCombine(hash, x, y, z);
    return hash;
}

Vector3 Vector3::Parse(StringView str)
{
    Vector3 result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return Vector3::Zero;
}

bool Vector3::TryParse(StringView str, Vector3* result)
{
    ALIMER_ASSERT(result);

    size_t elements = StringUtils::CountElements(str);
    if (elements < 3)
        return false;

    char* ptr = (char*)str.data();
    result->x = (float)strtod(ptr, &ptr);
    result->y = (float)strtod(ptr, &ptr);
    result->z = (float)strtod(ptr, &ptr);

    return true;
}

std::string Vector3::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    sprintf(tempBuffer, "%g %g %g", x, y, z);
    return std::string(tempBuffer);
}

//float32x4 Vector3::ToSIMD() const
//{
//    return Simd::LoadFloat3(&x);
//}

Vector3 Vector3::Add(const Vector3& value1, const Vector3& value2) noexcept
{
    return Vector3(value1.x + value2.x, value1.y + value2.y, value1.z + value2.z);
}

Vector3 Vector3::Subtract(const Vector3& value1, const Vector3& value2) noexcept
{
    return Vector3(value1.x - value2.x, value1.y - value2.y, value1.z - value2.z);
}

Vector3 Vector3::Multiply(const Vector3& value1, const Vector3& value2) noexcept
{
    return Vector3(value1.x * value2.x, value1.y * value2.y, value1.z * value2.z);
}

Vector3 Vector3::MultiplyAdd(const Vector3& value1, const Vector3& value2, const Vector3& value3) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    simd_float4 simd_result = simd_float4_multiply_add(value1.ToSIMD(), value2.ToSIMD(), value3.ToSIMD());
    Vector3 result;
    simd_float3_store(&result, simd_result);
    return result;
#else
    return Vector3(value1.x * value2.x + value3.x, value1.y * value2.y + value3.y, value1.z * value2.z + value3.z);
#endif
}

Vector3 Vector3::MultiplyAdd(const Vector3& value1, float value2, const Vector3& value3) noexcept
{
    return Vector3(value1.x * value2 + value3.x, value1.y * value2 + value3.y, value1.z * value2 + value3.z);
}

Vector3 Vector3::Negate(const Vector3& value) noexcept
{
    return Vector3(-value.x, -value.y, -value.z);
}

Vector3 Vector3::Min(const Vector3& value1, const Vector3& value2) noexcept
{
    return Vector3(
        (value1.x < value2.x) ? value1.x : value2.x,
        (value1.y < value2.y) ? value1.y : value2.y,
        (value1.z < value2.z) ? value1.z : value2.z
    );
}

Vector3 Vector3::Max(const Vector3& value1, const Vector3& value2) noexcept
{
    return Vector3(
        (value1.x > value2.x) ? value1.x : value2.x,
        (value1.y > value2.y) ? value1.y : value2.y,
        (value1.z > value2.z) ? value1.z : value2.z
    );
}

Vector3 Vector3::Round(const Vector3& value) noexcept
{
    return Vector3(MathF::Round(value.x), MathF::Round(value.y), MathF::Round(value.z));
}

Vector3 Vector3::Truncate(const Vector3& value) noexcept
{
    return Vector3(MathF::Truncate(value.x), MathF::Truncate(value.y), MathF::Truncate(value.z));
}

Vector3 Vector3::Floor(const Vector3& value) noexcept
{
    return Vector3(MathF::Floor(value.x), MathF::Floor(value.y), MathF::Floor(value.z));
}

Vector3 Vector3::Ceiling(const Vector3& value) noexcept
{
    return Vector3(MathF::Ceil(value.x), MathF::Ceil(value.y), MathF::Ceil(value.z));
}

Vector3 Vector3::Clamp(const Vector3& value, const Vector3& min, const Vector3& max) noexcept
{
    return Vector3(
        Alimer::Clamp(value.x, min.x, max.x),
        Alimer::Clamp(value.y, min.y, max.y),
        Alimer::Clamp(value.z, min.z, max.z)
    );
}

Vector3 Vector3::Saturate(const Vector3& vector) noexcept
{
    return Clamp(vector, Zero, One);
}

float Vector3::Dot(const Vector3& v1, const Vector3& v2) noexcept
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
    //const float32x4 X = Simd::Vec3::Dot(v1, v2);
    //return Simd::VectorGetX(X);
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2) noexcept
{
    return Vector3(
        (v1.y * v2.z) - (v1.z * v2.y),
        (v1.z * v2.x) - (v1.x * v2.z),
        (v1.x * v2.y) - (v1.y * v2.x)
    );
    //const float32x4 result = Simd::Vec3::Cross(v1, v2);
    //return Vector3(result);
}

Vector3 Vector3::Normalize(const Vector3& vector) noexcept
{
    float lenSquared = vector.LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        return vector * invLen;
    }

    return vector;
}

float Vector3::Distance(const Vector3& v1, const Vector3& v2) noexcept
{
    float distanceSquared = DistanceSquared(v1, v2);
    return sqrtf(distanceSquared);
}

float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept
{
    Vector3 difference = v1 - v2;
    return Dot(difference, difference);
}

Vector3 Vector3::Reflect(const Vector3& vector, const Vector3& normal) noexcept
{
    float dot = Dot(vector, normal);
    return vector - (2 * dot * normal);
}

Vector3 Vector3::Lerp(const Vector3& value1, const Vector3& value2, float amount) noexcept
{
    return Vector3(
        value1.x + (value2.x - value1.x) * amount,
        value1.y + (value2.y - value1.y) * amount,
        value1.z + (value2.z - value1.z) * amount
    );
}

Vector3 Vector3::Transform(const Vector3& position, const Matrix4x4& matrix) noexcept
{
    Vector3 result;
    result.x = (position.x * matrix.m11) + (position.y * matrix.m21) + (position.z * matrix.m31) + matrix.m41;
    result.y = (position.x * matrix.m12) + (position.y * matrix.m22) + (position.z * matrix.m32) + matrix.m42;
    result.z = (position.x * matrix.m13) + (position.y * matrix.m23) + (position.z * matrix.m33) + matrix.m43;
    return result;
}

Vector3 Vector3::Transform(const Vector3& value, const Quaternion& rotation) noexcept
{
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
    float zz2 = rotation.z * z2;

    Vector3 result;
    result.x = value.x * (1.0f - yy2 - zz2) + value.y * (xy2 - wz2) + value.z * (xz2 + wy2);
    result.y = value.x * (xy2 + wz2) + value.y * (1.0f - xx2 - zz2) + value.z * (yz2 - wx2);
    result.z = value.x * (xz2 - wy2) + value.y * (yz2 + wx2) + value.z * (1.0f - xx2 - yy2);
    return result;
}

Vector3 Vector3::TransformNormal(const Vector3& value, const Matrix4x4& matrix) noexcept
{
    Vector3 result;
    result.x = (value.x * matrix.m11) + (value.y * matrix.m21) + (value.z * matrix.m31);
    result.y = (value.x * matrix.m12) + (value.y * matrix.m22) + (value.z * matrix.m32);
    result.z = (value.x * matrix.m13) + (value.y * matrix.m23) + (value.z * matrix.m33);
    return result;
}

/* Int3 */
std::string Int3::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    sprintf(tempBuffer, "%u %u %u", x, y, z);
    return std::string(tempBuffer);
}

const Int3 Int3::Zero = { 0, 0, 0 };

/* UInt3 */
const UInt3 UInt3::Zero = { 0, 0, 0 };

/* Double3 */
const Double3 Double3::Zero = { 0.0, 0.0, 0.0 };
