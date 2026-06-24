// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/BoundingFrustum.h"

namespace Alimer
{
    struct Matrix4x4;

    /// Defines an axis-aligned box-shaped 3D volume.
    struct ALIMER_API BoundingBox final
    {
        /// Specifies the total number of corners (8) in the BoundingBox.
        static constexpr uint32_t kCornerCount = 8u;

        /// The minimum point the BoundingBox contains.
        Vector3 min;
        /// The maximum point the BoundingBox contains.
        Vector3 max;

        BoundingBox() noexcept
            : min(0.0f, 0.0f, 0.0f)
            , max(0.0f, 0.0f, 0.0f)
        {
        }

        constexpr BoundingBox(const Vector3& min_, const Vector3& max_) noexcept
            : min(min_)
            , max(max_)
        {
        }

        constexpr BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) noexcept
            : min(minX, minY, minZ)
            , max(maxX, maxY, maxZ)
        {
        }

        BoundingBox(const BoundingBox&) = default;
        BoundingBox& operator=(const BoundingBox&) = default;
        BoundingBox(BoundingBox&&) = default;
        BoundingBox& operator=(BoundingBox&&) = default;

        /// Test for equality with another box.
        bool operator == (const BoundingBox& rhs) const noexcept { return min == rhs.min && max == rhs.max; }
        /// Test for inequality with another box.
        bool operator != (const BoundingBox& rhs) const noexcept { return min != rhs.min || max != rhs.max; }

        /// Sets this bounding box to the specified values.
        void Set(const Vector3& min, const Vector3& max);

        /// Sets this bounding box to the specified values.
        void Set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

        /// Sets this bounding box to the specified bounding box.
        void Set(const BoundingBox& box);

        /// Sets this box to tightly contain the specified bounding sphere.
        void Set(const BoundingSphere& sphere);

        /// Test for equality with another vector with epsilon.
        bool Equals(const BoundingBox& rhs, float epsilon = M_EPSILON) const
        {
            return
                min.Equals(rhs.min, epsilon) &&
                max.Equals(rhs.max, epsilon);
        }

        /// Gets the corners of the bounding box in the specified array.
        void GetCorners(Vector3* corners) const;

        [[nodiscard]] static BoundingBox CreateFromPoints(const Vector3* points, size_t count) noexcept;
        [[nodiscard]] static BoundingBox CreateFromSphere(const BoundingSphere& sphere) noexcept;
        [[nodiscard]] static BoundingBox CreateMerged(const BoundingBox& original, const BoundingBox& additional) noexcept;

        static void Transform(const BoundingBox& box, const Matrix4x4& transform, BoundingBox& result) noexcept;
        [[nodiscard]] static BoundingBox Transform(const BoundingBox& box, const Matrix4x4& transform) noexcept;

        [[nodiscard]] ContainmentType Contains(const Vector3& point) const noexcept;
        [[nodiscard]] ContainmentType Contains(const BoundingBox& box) const noexcept;
        [[nodiscard]] ContainmentType Contains(const BoundingSphere& sphere) const noexcept;

        [[nodiscard]] bool Intersects(const BoundingBox& box) const noexcept;
        [[nodiscard]] bool Intersects(const BoundingSphere& sphere) const noexcept;
        [[nodiscard]] bool Intersects(const BoundingFrustum& frustum) const noexcept;

        [[nodiscard]] PlaneIntersectionType Intersects(const Plane& plane) const noexcept;
        [[nodiscard]] bool Intersects(const Ray& ray, _Out_ float& distance) const noexcept;

        /// Gets the center point of the bounding box.
        [[nodiscard]] Vector3 GetCenter() const noexcept { return (max + min) * 0.5f; }

        /// Gets the size (extent) of the bouding box.
        [[nodiscard]] Vector3 GetSize() const noexcept { return (max - min); }

        /// Gets the half size (half extent) of the bouding box.
        [[nodiscard]] Vector3 GetHalfSize() const noexcept { return  (max - min) * 0.5f; }

        /// Gets the width of the bouding box.
        [[nodiscard]] float GetWidth() const noexcept;

        /// Gets the height of the bouding box.
        [[nodiscard]] float GetHeight() const noexcept;

        /// Gets the depth of the bouding box.
        [[nodiscard]] float GetDepth() const noexcept;

        /// Gets the volume of the bouding box.
        [[nodiscard]] float GetVolume() const noexcept;

        /// Gets the perimeter length of the bouding box.
        [[nodiscard]] float GetPerimeter() const noexcept;

        /// Gets the surface of the bouding box.
        [[nodiscard]] float GetSurfaceArea() const noexcept;

        /// Determines if this bounding box is empty.
        [[nodiscard]] bool IsEmpty() const noexcept;

        /// Return hash value.
        [[nodiscard]] size_t GetHashCode() const noexcept;
    };
}
