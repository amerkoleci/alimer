// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Plane.h"
#include "Alimer/Math/Ray.h"
#include "Alimer/Math/BoundingSphere.h"
#include "Alimer/Math/BoundingBox.h"
#include "Alimer/Math/BoundingFrustum.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

Plane::Plane(const Vector3& point, const Vector3& normal)
    : normal(normal)
    , distance(-Vector3::Dot(normal, point))
{}

Plane::Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3) noexcept
{
    float ax = point2.x - point1.x;
    float ay = point2.y - point1.y;
    float az = point2.z - point1.z;

    float bx = point3.x - point1.x;
    float by = point3.y - point1.y;
    float bz = point3.z - point1.z;

    // N=Cross(a,b)
    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;

    // Normalize(N)
    float ls = nx * nx + ny * ny + nz * nz;
    float invNorm = 1.0f / sqrtf(ls);

    normal.x = nx * invNorm;
    normal.y = ny * invNorm;
    normal.z = nz * invNorm;
    distance = -(normal.x * point1.x + normal.y * point1.y + normal.z * point1.z);
}

Plane Plane::Normalize(const Plane& plane) noexcept
{
    return Normalize(plane.normal, plane.distance);
}

Plane Plane::Normalize(const Vector3& normal, float distance) noexcept
{
    float magnitude = 1.0f / MathF::Sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

    return Plane(normal.x * magnitude, normal.y * magnitude, normal.z * magnitude, distance * magnitude);
}

float Plane::Dot(const Vector4& value) const
{
    return normal.x * value.x + normal.y * value.y + normal.z * value.z + distance * value.w;
}

float Plane::DotCoordinate(const Vector3& value) const
{
    return normal.x * value.x + normal.y * value.y + normal.z * value.z + distance;
}

float Plane::DotNormal(const Vector3& value) const
{
    return normal.x * value.x + normal.y * value.y + normal.z * value.z;
}

float Plane::Distance(const Vector3& point) const
{
    return normal.x * point.x + normal.y * point.y + normal.z * point.z + distance;
}

PlaneIntersectionType Plane::Intersects(const BoundingSphere& sphere) const
{
    return sphere.Intersects(*this);
}

PlaneIntersectionType Plane::Intersects(const BoundingBox& box) const
{
    return box.Intersects(*this);
}

void Plane::Transform(const Plane& plane, const Matrix4x4& matrix, Plane& result)
{
    Matrix4x4 inverse;
    if (Matrix4x4::Invert(matrix, &inverse))
    {
        const float x = plane.normal.x;
        const float y = plane.normal.y;
        const float z = plane.normal.z;
        const float d = plane.distance;

        result.normal.x = x * inverse.m11 + y * inverse.m12 + z * inverse.m13 + d * inverse.m14;
        result.normal.y = x * inverse.m21 + y * inverse.m22 + z * inverse.m23 + d * inverse.m24;
        result.normal.z = x * inverse.m31 + y * inverse.m32 + z * inverse.m33 + d * inverse.m34;
        result.distance = x * inverse.m41 + y * inverse.m42 + z * inverse.m43 + d * inverse.m44;
    }
}

Plane Plane::Transform(const Plane& plane, const Matrix4x4& matrix)
{
    Plane result;
    Transform(plane, matrix, result);
    return result;
}

void Plane::Transform(const Plane& plane, const Quaternion& rotation, Plane& result)
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
    float zz2 = rotation.z * z2;

    float m11 = 1.0f - yy2 - zz2;
    float m21 = xy2 - wz2;
    float m31 = xz2 + wy2;

    float m12 = xy2 + wz2;
    float m22 = 1.0f - xx2 - zz2;
    float m32 = yz2 - wx2;

    float m13 = xz2 - wy2;
    float m23 = yz2 + wx2;
    float m33 = 1.0f - xx2 - yy2;

    float x = plane.normal.x, y = plane.normal.y, z = plane.normal.z;

    result.normal.x = x * m11 + y * m21 + z * m31;
    result.normal.y = x * m12 + y * m22 + z * m32;
    result.normal.z = x * m13 + y * m23 + z * m33;
    result.distance = plane.distance;
}

Plane Plane::Transform(const Plane& plane, const Quaternion& rotation)
{
    Plane result;
    Transform(plane, rotation, result);
    return result;
}

size_t Plane::GetHashCode() const noexcept
{
    size_t hash = 0;
    HashCombine(hash, normal, distance);
    return hash;
}

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"

Plane::Plane(simd_float4_param xyzw)
{
    // XMStoreFloat4
#if defined(ALIMER_USE_SSE)
    _mm_storeu_ps(&reinterpret_cast<Vector4*>(this)->x, xyzw);
#elif defined(ALIMER_USE_NEON)
    vst1q_f32(&reinterpret_cast<Vector4*>(this)->x, xyzw);
#endif
}

simd_float4 Plane::ToSIMD() const
{
    return reinterpret_cast<const Vector4*>(this)->ToSIMD();
}
#endif
