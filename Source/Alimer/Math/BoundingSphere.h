// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/BoundingFrustum.h"

namespace Alimer
{
    struct Matrix4x4;

    /// Represents a bounding sphere in three dimensional space.
    struct ALIMER_API BoundingSphere final
    {
        /// The center point of the sphere.
        Vector3 center;

        /// The radius of the sphere.
        float radius;

        BoundingSphere() noexcept
            : center(0.0f, 0.0f, 0.0f)
            , radius(1.0f)
        {
        }

        /// Construct from coordinates.
        constexpr BoundingSphere(const Vector3& center_, float radius_) noexcept
            : center(center_)
            , radius(radius_)
        {
        }

        BoundingSphere(const BoundingSphere&) = default;
        BoundingSphere& operator=(const BoundingSphere&) = default;
        BoundingSphere(BoundingSphere&&) = default;
        BoundingSphere& operator=(BoundingSphere&&) = default;

        /// Test for equality with another sphere.
        bool operator == (const BoundingSphere& rhs) const noexcept { return center == rhs.center && radius == rhs.radius; }
        /// Test for inequality with another sphere.
        bool operator != (const BoundingSphere& rhs) const noexcept { return center != rhs.center || radius != rhs.radius; }

        static void Transform(const BoundingSphere& sphere, const Matrix4x4& transform, BoundingSphere& result) noexcept;
        [[nodiscard]] static BoundingSphere Transform(const BoundingSphere& sphere, const Matrix4x4& transform) noexcept;

        [[nodiscard]] ContainmentType Contains(_In_ Vector3& point) const noexcept;
        [[nodiscard]] ContainmentType Contains(_In_ const BoundingSphere& sphere) const noexcept;

        [[nodiscard]] bool Intersects(const BoundingSphere& sphere) const noexcept;
        [[nodiscard]] bool Intersects(const BoundingBox& box) const noexcept;
        [[nodiscard]] PlaneIntersectionType Intersects(const Plane& plane) const noexcept;
        [[nodiscard]] bool Intersects(const Ray& ray, _Out_ float& distance) const noexcept;

        /// Return hash value.
        [[nodiscard]] size_t GetHashCode() const noexcept;

        // Static methods
        [[nodiscard]] static BoundingSphere CreateFromPoints(const Vector3* points, size_t count) noexcept;
        [[nodiscard]] static BoundingSphere CreateMerged(const BoundingSphere& original, const BoundingSphere& additional) noexcept;
    };
}
