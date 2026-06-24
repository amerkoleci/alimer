// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Vector3.h"

namespace Alimer
{
    struct Plane;
    struct BoundingSphere;
    struct BoundingBox;
    class BoundingFrustum;

    /// Defines a 3-dimensional ray.
    struct ALIMER_API Ray final
    {
        /// Specifies the starting point of the Ray.
        Vector3 position;

        /// The unit vector specifying the direction the Ray is pointing.
        Vector3 direction;

        Ray() noexcept
            : position(0.0f, 0.0f, 0.0f)
            , direction(0.0f, 0.0f, 0.0f)
        {
        }

        constexpr Ray(const Vector3& position_, const Vector3& direction_) noexcept
            : position(position_)
            , direction(direction_)
        {
        }

        Ray(const Ray&) = default;
        Ray& operator=(const Ray&) = default;

        Ray(Ray&&) = default;
        Ray& operator=(Ray&&) = default;

        /// Test for equality with another ray.
        bool operator == (const Ray& rhs) const noexcept { return position == rhs.position && direction == rhs.direction; }
        /// Test for inequality with another ray.
        bool operator != (const Ray& rhs) const noexcept { return position != rhs.position || direction != rhs.direction; }

        [[nodiscard]] bool Intersects(const BoundingSphere& sphere, _Out_ float& distance) const noexcept;
        [[nodiscard]] bool Intersects(const BoundingBox& box, _Out_ float& distance) const noexcept;
        [[nodiscard]] bool Intersects(const Plane& plane, _Out_ float& distance) const noexcept;
        //bool Intersects(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, _Out_ float& distance) const noexcept;

        /// Return hash value.
        [[nodiscard]] size_t GetHashCode() const noexcept;
    };
}
