// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Matrix4x4.h"

namespace Alimer
{
    struct BoundingSphere;
    struct BoundingBox;
    struct Ray;
    class BoundingFrustum;

    /// Represents a plane.
    struct ALIMER_API Plane final
    {
        /// The normal vector of the Plane.
        Vector3 normal;

        /// The distance of the Plane along its normal from the origin.
        float distance;

        Plane() noexcept
            : normal(Vector3::UnitY)
            , distance(0.0f)
        {
        }

        constexpr Plane(float normalX, float normalY, float normalZ, float distance_) noexcept
            : normal(normalX, normalY, normalZ)
            , distance(distance_)
        {
        }

        Plane(const Vector3& normal_, float distance_) noexcept
            : normal(normal_)
            , distance(distance_)
        {

        }

        Plane(const Vector3& point, const Vector3& normal);
        Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3) noexcept;

        explicit Plane(const Vector4& vector) noexcept
            : normal(vector.x, vector.y, vector.z)
            , distance(vector.w)
        {
        }

        explicit Plane(_In_reads_(4) const float* pArray) noexcept
            : normal(pArray[0], pArray[1], pArray[2])
            , distance(pArray[3])
        {
        }

        Plane(const Plane&) = default;
        Plane& operator=(const Plane&) = default;
        Plane(Plane&&) = default;
        Plane& operator=(Plane&&) = default;

        /// Test for equality with another plane.
        bool operator == (const Plane& rhs) const noexcept { return normal == rhs.normal && distance == rhs.distance; }
        /// Test for inequality with another plane.
        bool operator != (const Plane& rhs) const noexcept { return normal != rhs.normal || distance != rhs.distance; }

        // Plane operations
        [[nodiscard]] static Plane Normalize(const Plane& plane) noexcept;
        [[nodiscard]] static Plane Normalize(const Vector3& normal, float distance) noexcept;

        [[nodiscard]] float Dot(const Vector4& value) const;
        [[nodiscard]] float DotCoordinate(const Vector3& value) const;
        [[nodiscard]] float DotNormal(const Vector3& value) const;

        /// Calculates the distance from this plane to the specified point.
        [[nodiscard]] float Distance(const Vector3& point) const;

        // Intersects operations
        [[nodiscard]] PlaneIntersectionType Intersects(const BoundingSphere& sphere) const;
        [[nodiscard]] PlaneIntersectionType Intersects(const BoundingBox& box) const;

        // Static functions
        static void Transform(const Plane& plane, const Matrix4x4& matrix, Plane& result);
        [[nodiscard]] static Plane Transform(const Plane& plane, const Matrix4x4& matrix);

        static void Transform(const Plane& plane, const Quaternion& rotation, Plane& result);
        [[nodiscard]] static Plane Transform(const Plane& plane, const Quaternion& rotation);

        /// Return hash value.
        [[nodiscard]] size_t GetHashCode() const noexcept;

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
        explicit Plane(simd_float4_param xyzw);

        /// Return SIMD vector.
        simd_float4 ToSIMD() const;

        operator simd_float4() const noexcept { return ToSIMD(); }
#endif
    };
}
