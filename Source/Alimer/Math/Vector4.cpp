// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Vector4.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

const Vector4 Vector4::Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
const Vector4 Vector4::One = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vector4 Vector4::UnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 Vector4::UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vector4 Vector4::UnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vector4 Vector4::UnitW = { 0.0f, 0.0f, 0.0f, 1.0f };

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"

Vector4::Vector4(simd_float4_param xyzw)
{
    simd_float4_store(this, xyzw);
}

simd_float4 Vector4::ToSIMD() const
{
    return simd_float4_load(this);
}
#endif

void Vector4::Set(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

void Vector4::Set(const Vector4& rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    w = rhs.w;
}

float Vector4::Length() const noexcept
{
    return sqrtf(x * x + y * y + z * z + w * w);
    //const float32x4 simd = ToSIMD();
    //const float32x4 X = Simd::Vec4::Length(simd);
    //return Simd::VectorGetX(X);
}

float Vector4::LengthSquared() const noexcept
{
    return x * x + y * y + z * z + w * w;
    //const float32x4 simd = ToSIMD();
    //const float32x4 X = Simd::Vec4::LengthSquared(simd);
    //return Simd::VectorGetX(X);
}

size_t Vector4::GetHashCode() const
{
    size_t hash = 0;
    HashCombine(hash, x, y, z, w);
    return hash;
}

Vector4 Vector4::Parse(StringView str)
{
    Vector4 result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return Vector4::Zero;
}

bool Vector4::TryParse(StringView str, Vector4* result)
{
    ALIMER_ASSERT(result);

    size_t elements = StringUtils::CountElements(str);
    if (elements < 4)
        return false;

    char* ptr = const_cast<char*>(str.data());
    result->x = (float)strtod(ptr, &ptr);
    result->y = (float)strtod(ptr, &ptr);
    result->z = (float)strtod(ptr, &ptr);
    result->w = (float)strtod(ptr, &ptr);

    return true;
}

std::string Vector4::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    sprintf(tempBuffer, "%g %g %g %g", x, y, z, w);
    return std::string(tempBuffer);
}

Vector4 Vector4::Normalize(const Vector4& vector) noexcept
{
    float lenSquared = vector.LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        return vector * invLen;
    }

    return vector;
}

Vector4 Vector4::Min(const Vector4& value1, const Vector4& value2) noexcept
{
    return Vector4(
        (value1.x < value2.x) ? value1.x : value2.x,
        (value1.y < value2.y) ? value1.y : value2.y,
        (value1.z < value2.z) ? value1.z : value2.z,
        (value1.w < value2.w) ? value1.w : value2.w
    );
}

Vector4 Vector4::Max(const Vector4& value1, const Vector4& value2) noexcept
{
    return Vector4(
        (value1.x > value2.x) ? value1.x : value2.x,
        (value1.y > value2.y) ? value1.y : value2.y,
        (value1.z > value2.z) ? value1.z : value2.z,
        (value1.w > value2.w) ? value1.w : value2.w
    );
}

Vector4 Vector4::Round(const Vector4& vector) noexcept
{
    return Vector4(
        MathF::Round(vector.x),
        MathF::Round(vector.y),
        MathF::Round(vector.z),
        MathF::Round(vector.w)
    );
}

Vector4 Vector4::Truncate(const Vector4& vector) noexcept
{
    return Vector4(
        MathF::Truncate(vector.x),
        MathF::Truncate(vector.y),
        MathF::Truncate(vector.z),
        MathF::Truncate(vector.w)
    );
}

Vector4 Vector4::Floor(const Vector4& vector) noexcept
{
    return Vector4(
        MathF::Floor(vector.x),
        MathF::Floor(vector.y),
        MathF::Floor(vector.z),
        MathF::Floor(vector.w)
    );
}

Vector4 Vector4::Ceiling(const Vector4& vector) noexcept
{
    return Vector4(
        MathF::Ceil(vector.x),
        MathF::Ceil(vector.y),
        MathF::Ceil(vector.z),
        MathF::Ceil(vector.w)
    );
}

Vector4 Vector4::Clamp(const Vector4& value, const Vector4& min, const Vector4& max) noexcept
{
    return Vector4(
        Alimer::Clamp(value.x, min.x, max.x),
        Alimer::Clamp(value.y, min.y, max.y),
        Alimer::Clamp(value.z, min.z, max.z),
        Alimer::Clamp(value.w, min.w, max.w)
    );
}

Vector4 Vector4::Saturate(const Vector4& vector) noexcept
{
    return Clamp(vector, Zero, One);
}

Vector4 Vector4::Lerp(const Vector4& value1, const Vector4& value2, float amount) noexcept
{
    return Vector4(
        value1.x + (value2.x - value1.x) * amount,
        value1.y + (value2.y - value1.y) * amount,
        value1.z + (value2.z - value1.z) * amount,
        value1.w + (value2.w - value1.w) * amount
    );
}

/* Int4 */
const Int4 Int4::Zero = { 0, 0, 0, 0 };

/* UInt4 */
const UInt4 UInt4::Zero = { 0, 0, 0, 0 };

/* Double4 */
const Double4 Double4::Zero = { 0.0, 0.0, 0.0, 0.0 };

double Double4::Length() const noexcept
{
    return sqrt(x * x + y * y + z * z + w * w);
}

double Double4::LengthSquared() const noexcept
{
    return x * x + y * y + z * z + w * w;
}

/* Half4 */
Half4::Half4(float x_, float y_, float z_, float w_) noexcept
{
    x = FloatToHalf(x_);
    y = FloatToHalf(y_);
    z = FloatToHalf(z_);
    w = FloatToHalf(w_);
}

Half4::Half4(_In_reads_(4) const float* floatArray) noexcept
{
    ALIMER_ASSERT(floatArray != nullptr);

    x = FloatToHalf(floatArray[0]);
    y = FloatToHalf(floatArray[1]);
    z = FloatToHalf(floatArray[2]);
    w = FloatToHalf(floatArray[3]);
}

Half4::Half4(const Vector4& vector) noexcept
{
    x = FloatToHalf(vector.x);
    y = FloatToHalf(vector.y);
    z = FloatToHalf(vector.z);
    w = FloatToHalf(vector.w);
}

Vector4 Half4::ToVector4() const
{
    return Vector4(
        HalfToFloat(x),
        HalfToFloat(y),
        HalfToFloat(z),
        HalfToFloat(w)
    );
}

/* UShort4 */
static constexpr Vector4 UShortMax(65535.0f, 65535.0f, 65535.0f, 65535.0f);

UShort4::UShort4(float x_, float y_, float z_, float w_) noexcept
{
    Vector4 vector = Vector4::Clamp(Vector4(x_, y_, z_, w_), Vector4::Zero, UShortMax);
    vector = Vector4::Round(vector);

    x = static_cast<uint16_t>(vector.x);
    y = static_cast<uint16_t>(vector.y);
    z = static_cast<uint16_t>(vector.z);
    w = static_cast<uint16_t>(vector.w);
}

UShort4::UShort4(_In_reads_(4) const float* floatArray) noexcept
{
    ALIMER_ASSERT(floatArray != nullptr);

    Vector4 vector = Vector4::Clamp(Vector4(floatArray), Vector4::Zero, UShortMax);
    vector = Vector4::Round(vector);

    x = static_cast<uint16_t>(vector.x);
    y = static_cast<uint16_t>(vector.y);
    z = static_cast<uint16_t>(vector.z);
    w = static_cast<uint16_t>(vector.w);
}
