// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Quaternion.h"
//#include "Alimer/Math/Matrix3x3.h"
#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

const Quaternion Quaternion::Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
const Quaternion Quaternion::One = { 1.0f, 1.0f, 1.0f, 1.0f };
const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"

Quaternion::Quaternion(simd_float4_param xyzw)
{
    simd_float4_store(reinterpret_cast<Vector4*>(this), xyzw);
}

simd_float4 Quaternion::ToSIMD() const
{
    return simd_float4_load(reinterpret_cast<const Vector4*>(this));
}
#endif


Quaternion Quaternion::Parse(StringView str)
{
    Quaternion result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return Quaternion::Identity;
}

bool Quaternion::TryParse(StringView str, Quaternion* result)
{
    ALIMER_ASSERT(result);

    size_t elements = StringUtils::CountElements(str);
    if (elements < 3)
        return false;

    char* ptr = const_cast<char*>(str.data());
    result->x = (float)strtod(ptr, &ptr);
    result->y = (float)strtod(ptr, &ptr);
    result->z = (float)strtod(ptr, &ptr);
    if (elements > 3)
        result->w = (float)strtod(ptr, &ptr);

    return true;
}

std::string Quaternion::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    sprintf(tempBuffer, "%g %g %g %g", x, y, z, w);
    return std::string(tempBuffer);
}

size_t Quaternion::GetHashCode() const
{
    size_t hash = 0;
    HashCombine(hash, x, y, z, w);
    return hash;
}

float Quaternion::Length() const noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    const simd_float4 simd = ToSIMD();
    const simd_float4 X = simd_float4_length(simd);
    return simd_float4_get_x(X);
#else
    return MathF::Sqrt(x * x + y * y + z * z + w * w);
#endif
}

float Quaternion::LengthSquared() const noexcept
{
    return x * x + y * y + z * z + w * w;
    //const float32x4 simd = ToSIMD();
    //const float32x4 X = Simd::Vec4::LengthSquared(simd);
    //return Simd::VectorGetX(X);
}

void Quaternion::Normalize() noexcept
{
    float lenSquared = LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }
}

Quaternion Quaternion::Normalized() const noexcept
{
    float lenSquared = LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        return *this * invLen;
    }

    return *this;
}

Vector3 Quaternion::EulerAngles() const noexcept
{
    // Derivation from http://www.geometrictools.com/Documentation/EulerAngles.pdf
    // Order of rotations: Z first, then X, then Y
    const float check = 2.0f * (-y * z + w * x);

    // More on singularity handling here:
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
    constexpr float singularityThreshold = 0.999999f;
    const float y2 = y * y;
    const float z2 = z * z;
    const float x2 = x * x;
    const float unit = w * w + x2 + y2 + z2;

    // In case of singularity treat rotation as yaw angle as it fits yaw/pitch 3D games better.
    if (Abs(check) > singularityThreshold * unit)
    {
        return Vector3(
            90.0f * Sign(check),
            -atan2f(2.0f * (x * z - w * y), 1.0f - 2.0f * (y2 + z2)) * RAD_TO_DEG,
            0.0f);
    }

    return Vector3(
        asinf(check) * RAD_TO_DEG,
        atan2f(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y2)) * RAD_TO_DEG,
        atan2f(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z2)) * RAD_TO_DEG);
}

float Quaternion::YawAngle() const noexcept
{
    return EulerAngles().y;
}

float Quaternion::PitchAngle() const noexcept
{
    return EulerAngles().x;
}

float Quaternion::RollAngle() const noexcept
{
    return EulerAngles().z;
}

Vector3 Quaternion::Axis() const noexcept
{
    float axisScaleInv = static_cast<float>(MathF::Sqrt(Max(0.0f, 1.0f - w * w)));
    if (axisScaleInv < 1e-6f)
        return Vector3::Up;

    return Vector3(x, y, z) / axisScaleInv;
}

float Quaternion::Angle() const noexcept
{
    return 2 * RAD_TO_DEG * MathF::Acos(w);
}

void Quaternion::Lerp(const Quaternion& value1, const Quaternion& value2, float amount, Quaternion& result) noexcept
{
    result.x = value1.x + (value2.x - value1.x) * amount;
    result.y = value1.y + (value2.y - value1.y) * amount;
    result.z = value1.z + (value2.z - value1.z) * amount;
    result.w = value1.w + (value2.w - value1.w) * amount;
}

Quaternion Quaternion::Lerp(const Quaternion& value1, const Quaternion& value2, float amount) noexcept
{
    return Quaternion(
        value1.x + (value2.x - value1.x) * amount,
        value1.y + (value2.y - value1.y) * amount,
        value1.z + (value2.z - value1.z) * amount,
        value1.w + (value2.w - value1.w) * amount
    );
}

void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float amount, Quaternion& result) noexcept
{
    static constexpr float SlerpEpsilon = 1e-6f;

    float t = amount;

    float cosOmega = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    bool flip = false;

    if (cosOmega < 0.0f)
    {
        flip = true;
        cosOmega = -cosOmega;
    }

    float s1, s2;

    if (cosOmega > (1.0f - SlerpEpsilon))
    {
        // Too close, do straight linear interpolation.
        s1 = 1.0f - t;
        s2 = (flip) ? -t : t;
    }
    else
    {
        float omega = std::acosf(cosOmega);
        float invSinOmega = 1 / std::sinf(omega);

        s1 = std::sinf((1.0f - t) * omega) * invSinOmega;
        s2 = (flip)
            ? -std::sinf(t * omega) * invSinOmega
            : std::sinf(t * omega) * invSinOmega;
    }

    result.x = s1 * q1.x + s2 * q2.x;
    result.y = s1 * q1.y + s2 * q2.y;
    result.z = s1 * q1.z + s2 * q2.z;
    result.w = s1 * q1.w + s2 * q2.w;
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float amount) noexcept
{
    Quaternion result;
    Slerp(q1, q2, amount, result);
    return result;
}

void Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept
{
    result = q1 * q2;
}

Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept
{
    Quaternion result;
    Concatenate(q1, q2, result);
    return result;
}

void Quaternion::Conjugate() noexcept
{
    x = 0.0f - x;
    y = 0.0f - y;
    z = 0.0f - z;
}

void Quaternion::Conjugate(const Quaternion& value, Quaternion& result) noexcept
{
    result.x = 0.0f - value.x;
    result.y = 0.0f - value.y;
    result.z = 0.0f - value.z;
    result.w = value.w;
}

Quaternion Quaternion::Conjugate(const Quaternion& value) noexcept
{
    Quaternion result;
    Conjugate(value, result);
    return result;
}

Quaternion Quaternion::Normalize(const Quaternion& quaternion) noexcept
{
    float lenSquared = quaternion.LengthSquared();
    if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
    {
        float invLen = 1.0f / sqrtf(lenSquared);
        return quaternion * invLen;
    }

    return quaternion;
}


Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float degrees) noexcept
{
    Vector3 normAxis = Vector3::Normalize(axis);

    float sinAngle;
    float cosAngle;
    SinCos(ToRadians(degrees), &sinAngle, &cosAngle);

    Quaternion result;
    result.x = normAxis.x * sinAngle;
    result.y = normAxis.y * sinAngle;
    result.z = normAxis.z * sinAngle;
    result.w = cosAngle;
    return result;
}

Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    //  Roll first, about axis the object is facing, then
    //  pitch upward, then yaw to face into the new heading
    float sr, cr, sp, cp, sy, cy;

    float halfRoll = ToRadians(roll) * 0.5f;
    sr = sin(halfRoll);
    cr = cos(halfRoll);

    float halfPitch = ToRadians(pitch) * 0.5f;
    sp = sin(halfPitch);
    cp = cos(halfPitch);

    float halfYaw = ToRadians(yaw) * 0.5f;
    sy = sin(halfYaw);
    cy = cos(halfYaw);

    Quaternion result;

    result.x = cy * sp * cr + sy * cp * sr;
    result.y = sy * cp * cr - cy * sp * sr;
    result.z = cy * cp * sr - sy * sp * cr;
    result.w = cy * cp * cr + sy * sp * sr;

    return result;
}

Quaternion Quaternion::CreateFromYawPitchRoll(const Vector3& angles) noexcept
{
    return CreateFromYawPitchRoll(angles.x, angles.y, angles.z);
}

Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& matrix) noexcept
{
    float trace = matrix.m11 + matrix.m22 + matrix.m33;

    Quaternion q = Quaternion::Zero;

    if (trace > 0.0f)
    {
        float s = sqrtf(trace + 1.0f);
        q.w = s * 0.5f;
        s = 0.5f / s;
        q.x = (matrix.m23 - matrix.m32) * s;
        q.y = (matrix.m31 - matrix.m13) * s;
        q.z = (matrix.m12 - matrix.m21) * s;
    }
    else
    {
        if (matrix.m11 >= matrix.m22 && matrix.m11 >= matrix.m33)
        {
            float s = sqrtf(1.0f + matrix.m11 - matrix.m22 - matrix.m33);
            float invS = 0.5f / s;
            q.x = 0.5f * s;
            q.y = (matrix.m12 + matrix.m21) * invS;
            q.z = (matrix.m13 + matrix.m31) * invS;
            q.w = (matrix.m23 - matrix.m32) * invS;
        }
        else if (matrix.m22 > matrix.m33)
        {
            float s = sqrtf(1.0f + matrix.m22 - matrix.m11 - matrix.m33);
            float invS = 0.5f / s;
            q.x = (matrix.m21 + matrix.m12) * invS;
            q.y = 0.5f * s;
            q.z = (matrix.m32 + matrix.m23) * invS;
            q.w = (matrix.m31 - matrix.m13) * invS;
        }
        else
        {
            float s = sqrtf(1.0f + matrix.m33 - matrix.m11 - matrix.m22);
            float invS = 0.5f / s;
            q.x = (matrix.m31 + matrix.m13) * invS;
            q.y = (matrix.m32 + matrix.m23) * invS;
            q.z = 0.5f * s;
            q.w = (matrix.m12 - matrix.m21) * invS;
        }
    }

    return q;
}

Quaternion Quaternion::CreateFromEulerAngles(float x, float y, float z) noexcept
{
    const float halfToRad = 0.5f * DEG_TO_RAD;
    x *= halfToRad;
    y *= halfToRad;
    z *= halfToRad;

    float sx, cx;
    float sy, cy;
    float sz, cz;
    SinCos(x, &sx, &cx);
    SinCos(y, &sy, &cy);
    SinCos(z, &sz, &cz);

    Quaternion result;
    result.x = sx * cy * cz - cx * sy * sz;
    result.y = cx * sy * cz + sx * cy * sz;
    result.z = cx * cy * sz - sx * sy * cz;
    result.w = cx * cy * cz + sx * sy * sz;
    return result;
}

Quaternion Quaternion::CreateFromEulerAngles(const Vector3& angles) noexcept
{
    return CreateFromEulerAngles(angles.x, angles.y, angles.z);
}

Quaternion Quaternion::CreateFromRotationTo(const Vector3& start, const Vector3& end)
{
    Vector3 normStart = Vector3::Normalize(start);
    Vector3 normEnd = Vector3::Normalize(end);
    float d = Vector3::Dot(normStart, normEnd);

    if (d > -1.0f + M_EPSILON)
    {
        Vector3 c = Vector3::Cross(normStart, normEnd);
        float s = MathF::Sqrt((1.0f + d) * 2.0f);
        float invS = 1.0f / s;

        return Quaternion(c.x * invS, c.y * invS, c.z * invS, 0.5f * s);
    }
    else
    {
        Vector3 axis = Vector3::Cross(Vector3::Right, normStart);
        if (axis.Length() < M_EPSILON)
        {
            axis = Vector3::Cross(Vector3::Up, normStart);
        }

        return CreateFromAxisAngle(axis, 180.0f);
    }
}

Quaternion Quaternion::CreateFromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
{
    Matrix4x4 matrix(
        xAxis.x, yAxis.x, zAxis.x, 0.0f,
        xAxis.y, yAxis.y, zAxis.y, 0.0f,
        xAxis.z, yAxis.z, zAxis.z, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    );

    return CreateFromRotationMatrix(matrix);
}

bool Quaternion::CreateFromLookRotation(const Vector3& direction, Quaternion& result)
{
    return CreateFromLookRotation(direction, Vector3::Up, result);
}

bool Quaternion::CreateFromLookRotation(const Vector3& direction, const Vector3& up, Quaternion& result)
{
    Vector3 forward = Vector3::Normalize(direction);

    Vector3 v = Vector3::Cross(forward, up);
    // If direction & up are parallel and crossproduct becomes zero, use FromRotationTo() fallback
    if (v.LengthSquared() >= M_EPSILON)
    {
        v.Normalize();
        Vector3 up = Vector3::Cross(v, forward);
        Vector3 right = Vector3::Cross(up, forward);
        result = Quaternion::CreateFromAxes(right, up, forward);
    }
    else
    {
        result = CreateFromRotationTo(Vector3::Forward, forward);
    }

    if (!result.IsNaN())
    {
        return true;
    }

    return false;
}

Quaternion Quaternion::Inverse(const Quaternion& value) noexcept
{
    //  -1   (       a              -v       )
    // q   = ( -------------   ------------- )
    //       (  a^2 + |v|^2  ,  a^2 + |v|^2  )


    float ls = value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w;
    float invNorm = 1.0f / ls;

    Quaternion result;
    result.x = -value.x * invNorm;
    result.y = -value.y * invNorm;
    result.z = -value.z * invNorm;
    result.w = value.w * invNorm;
    return result;
}


float Quaternion::Angle(const Quaternion& q1, const Quaternion& q2) noexcept
{
    // We can use the conjugate here instead of inverse assuming q1 & q2 are normalized.
    Quaternion R = Conjugate(q1) * q2;

    const float rs = R.w;
    const float length = MathF::Sqrt(R.x * R.x + R.y * R.y + R.z * R.z);
    return 2.0f * MathF::Atan2(length, rs) * RAD_TO_DEG;
}
