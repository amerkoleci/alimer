// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Vector2.h"
#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Matrix3x2.h"
#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Math/Color.h"
#include "Alimer/Core/Hash.h"

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"
#endif

using namespace Alimer;

const Vector2 Vector2::Zero = { 0.0f, 0.0f };
const Vector2 Vector2::One = { 1.0f, 1.0f };
const Vector2 Vector2::UnitX = { 1.0f, 0.0f };
const Vector2 Vector2::UnitY = { 0.0f, 1.0f };

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"

Vector2::Vector2(simd_float4_param xy)
    : x(0.0f)
    , y(0.0f)
{
    // XMStoreFloat2
#if defined(ALIMER_USE_NEON)
    vst1_f32(reinterpret_cast<float*>(this), vget_low_f32(xy));
#elif defined(ALIMER_USE_SSE)
    _mm_store_sd(reinterpret_cast<double*>(this), _mm_castps_pd(xy));
#endif
}

simd_float4 Vector2::ToSIMD() const
{
    // XMLoadFloat2
#if defined(ALIMER_USE_NEON)
    float32x2_t x = vld1_f32(reinterpret_cast<const float*>(this));
    float32x2_t zero = vdup_n_f32(0);
    return vcombine_f32(x, zero);
#elif defined(ALIMER_USE_SSE)
    return _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(this)));
#endif
}
#endif

void Vector2::Set(float x, float y)
{
    this->x = x;
    this->y = y;
}

void Vector2::Set(const Vector2& rhs)
{
    x = rhs.x;
    y = rhs.y;
}

float Vector2::Dot(const Vector2& v1, const Vector2& v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

float Vector2::Cross(const Vector2& v1, const Vector2& v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}

Vector2 Vector2::Normalize(const Vector2& vector) noexcept
{
    float lenSquared = vector.LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        return vector * invLen;
    }

    return vector;
}

float Vector2::Distance(const Vector2& v1, const Vector2& v2)
{
    float distanceSquared = DistanceSquared(v1, v2);
    return MathF::Sqrt(distanceSquared);
}

float Vector2::DistanceSquared(const Vector2& v1, const Vector2& v2)
{
    Vector2 difference = v1 - v2;
    return Dot(difference, difference);
}

Vector2 Vector2::Min(const Vector2& value1, const Vector2& value2)
{
    return Vector2(
        (value1.x < value2.x) ? value1.x : value2.x,
        (value1.y < value2.y) ? value1.y : value2.y
    );
}

Vector2 Vector2::Max(const Vector2& value1, const Vector2& value2)
{
    return Vector2(
        (value1.x > value2.x) ? value1.x : value2.x,
        (value1.y > value2.y) ? value1.y : value2.y
    );
}

Vector2 Vector2::Round(const Vector2& value) noexcept
{
    return Vector2(MathF::Round(value.x), MathF::Round(value.y));
}

Vector2 Vector2::Truncate(const Vector2& value) noexcept
{
    return Vector2(MathF::Truncate(value.x), MathF::Truncate(value.y));
}

Vector2 Vector2::Floor(const Vector2& value) noexcept
{
    return Vector2(MathF::Floor(value.x), MathF::Floor(value.y));
}

Vector2 Vector2::Ceiling(const Vector2& value) noexcept
{
    return Vector2(MathF::Ceil(value.x), MathF::Ceil(value.y));
}

Vector2 Vector2::Clamp(const Vector2& value, const Vector2& min, const Vector2& max) noexcept
{
    return Vector2(
        Alimer::Clamp(value.x, min.x, max.x),
        Alimer::Clamp(value.y, min.y, max.y)
    );
}

Vector2 Vector2::Saturate(const Vector2& vector) noexcept
{
    return Clamp(vector, Zero, One);
}

Vector2 Vector2::Lerp(const Vector2& value1, const Vector2& value2, float amount) noexcept
{
    return Vector2(
        value1.x + (value2.x - value1.x) * amount,
        value1.y + (value2.y - value1.y) * amount
    );
}

Vector2 Vector2::SmoothStep(const Vector2& value1, const Vector2& value2, float amount) noexcept
{
    amount = (amount > 1.0f) ? 1.0f : ((amount < 0.0f) ? 0.0f : amount);  // Clamp value to 0 to 1
    amount = amount * amount * (3.0f - 2.0f * amount);
    return Vector2::Lerp(value1, value2, amount);
}

Vector2 Vector2::Transform(const Vector2& value, const Matrix3x2& matrix) noexcept
{
    return Vector2(
        (value.x * matrix.m11) + (value.y * matrix.m21) + matrix.m31,
        (value.x * matrix.m12) + (value.y * matrix.m22) + matrix.m32
        );
}

Vector2 Vector2::Transform(const Vector2& value, const Matrix4x4& matrix) noexcept
{
    return Vector2(
        (value.x * matrix.m11) + (value.y * matrix.m21) + matrix.m41,
        (value.x * matrix.m12) + (value.y * matrix.m22) + matrix.m42
        );
}

Vector2 Vector2::Transform(const Vector2& value, const Quaternion& rotation) noexcept
{
    float x2 = rotation.x + rotation.x;
    float y2 = rotation.y + rotation.y;
    float z2 = rotation.z + rotation.z;

    float wz2 = rotation.w * z2;
    float xx2 = rotation.x * x2;
    float xy2 = rotation.x * y2;
    float yy2 = rotation.y * y2;
    float zz2 = rotation.z * z2;

    return Vector2(
        value.x * (1.0f - yy2 - zz2) + value.y * (xy2 - wz2),
        value.x * (xy2 + wz2) + value.y * (1.0f - xx2 - zz2)
        );
}

Vector2 Vector2::TransformNormal(const Vector2& value, const Matrix3x2& matrix) noexcept
{
    return Vector2(
        (value.x * matrix.m11) + (value.y * matrix.m21),
        (value.x * matrix.m12) + (value.y * matrix.m22)
        );
}

Vector2 Vector2::TransformNormal(const Vector2& value, const Matrix4x4& matrix) noexcept
{
    return Vector2(
        (value.x * matrix.m11) + (value.y * matrix.m21),
        (value.x * matrix.m12) + (value.y * matrix.m22)
        );
}

Vector2 Vector2::Parse(StringView str)
{
    Vector2 result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return Vector2::Zero;
}

bool Vector2::TryParse(StringView str, Vector2* result)
{
    ALIMER_ASSERT(result);

    size_t elements = StringUtils::CountElements(str);
    if (elements < 2)
        return false;

    char* ptr = (char*)str.data();
    result->x = (float)strtod(ptr, &ptr);
    result->y = (float)strtod(ptr, &ptr);

    return true;
}

size_t Vector2::GetHashCode() const
{
    size_t hash = 0;
    HashCombine(hash, x, y);
    return hash;
}


std::string Vector2::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    snprintf(tempBuffer, kConversionBufferLength, "%g %g", x, y);
    return std::string(tempBuffer);
}

/* Int2 */
const Int2 Int2::Zero = { 0, 0 };
const Int2 Int2::One = { 1, 1 };
const Int2 Int2::UnitX = { 1, 0 };
const Int2 Int2::UnitY = { 0, 1 };

std::string Int2::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    snprintf(tempBuffer, kConversionBufferLength, "%d %d", x, y);
    return std::string(tempBuffer);
}

/* UInt2 */
const UInt2 UInt2::Zero = { 0u, 0u };
const UInt2 UInt2::One = { 1u, 1u };
const UInt2 UInt2::UnitX = { 1u, 0u };
const UInt2 UInt2::UnitY = { 0u, 1u };

std::string UInt2::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    snprintf(tempBuffer, kConversionBufferLength, "%u %u", x, y);
    return std::string(tempBuffer);
}

/* UInt2 */
const Double2 Double2::Zero = { 0.0, 0.0 };
const Double2 Double2::One = { 1.0, 1.0 };
const Double2 Double2::UnitX = { 1.0, 0.0 };
const Double2 Double2::UnitY = { 0.0, 1.0 };

/* Half2 */
Half2::Half2(float x_, float y_) noexcept
{
    x = FloatToHalf(x_);
    y = FloatToHalf(y_);
}

Half2::Half2(_In_reads_(2) const float* floatArray) noexcept
{
    ALIMER_ASSERT(floatArray != nullptr);

    x = FloatToHalf(floatArray[0]);
    y = FloatToHalf(floatArray[1]);
}

Half2::Half2(const Vector2& vector) noexcept
{
    x = FloatToHalf(vector.x);
    y = FloatToHalf(vector.y);
}

Vector2 Half2::ToVector2() const
{
    return Vector2(HalfToFloat(x), HalfToFloat(y));
}

/* UShort2 */
UShort2::UShort2(float x_, float y_) noexcept
{
#if defined(ALIMER_USE_SSE)
    // Bounds check
    __m128 vResult = simd_float4_max(simd_make_float4(x_, y_, 0.0f, 0.0f), simd_vector_zero());
    vResult = simd_float4_min(vResult, _mm_set_ps1(65535.0f));
    // Convert to int with rounding
    __m128i vInt = _mm_cvtps_epi32(vResult);
    // Since the SSE pack instruction clamps using signed rules,
    // manually extract the values to store them to memory
    x = static_cast<uint16_t>(_mm_extract_epi16(vInt, 0));
    y = static_cast<uint16_t>(_mm_extract_epi16(vInt, 2));
#elif defined(ALIMER_USE_NEON)
    float32x4_t R = vmaxq_f32(simd_make_float4(x_, y_, 0.0f, 0.0f), vdupq_n_f32(0.f));
    R = vminq_f32(R, vdupq_n_f32(65535.0f));
    uint32x4_t vInt32 = vcvtq_u32_f32(R);
    uint16x4_t vInt16 = vqmovn_u32(vInt32);
    vst1_lane_u32(&packedValue, vreinterpret_u32_u16(vInt16), 0);
#else
    static constexpr Vector2 UShortMax(65535.0f, 65535.0f);

    Vector2 vector = Vector2::Clamp(Vector2(x_, y_), Vector2::Zero, UShortMax);
    vector = Vector2::Round(vector);

    x = static_cast<uint16_t>(vector.x);
    y = static_cast<uint16_t>(vector.y);
#endif
}

UShort2::UShort2(_In_reads_(2) const float* pArray) noexcept
    : UShort2(pArray[0], pArray[1])
{

}

Vector2 UShort2::ToVector2() const noexcept
{
    return Vector2(static_cast<float>(x), static_cast<float>(y));
}

UInt2 Alimer::PackHalf4(float x, float y, float z, float w)
{
    return UInt2(
        (uint32_t)FloatToHalf(x) | ((uint32_t)FloatToHalf(y) << 16u),
        (uint32_t)FloatToHalf(z) | ((uint32_t)FloatToHalf(w) << 16u)
    );
}

UInt2 Alimer::PackHalf4(const Vector4& value)
{
    return PackHalf4(value.x, value.y, value.z, value.w);
}

UInt2 Alimer::PackHalf4(const Color& value)
{
    return PackHalf4(value.r, value.g, value.b, value.a);
}
