// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Math/Ray.h"
#include "Alimer/Math/Plane.h"

namespace Alimer
{
    class ALIMER_API BoundingFrustum final
    {
    public:
        /// Specifies the total number of planes (6) in the BoundingFrustum.
        static constexpr uint32_t kCornerCount = 6u;

        static constexpr uint32_t kNearPlaneIndex = 0;
        static constexpr uint32_t kFarPlaneIndex = 1;
        static constexpr uint32_t kLeftPlaneIndex = 2;
        static constexpr uint32_t kRightPlaneIndex = 3;
        static constexpr uint32_t kTopPlaneIndex = 4;
        static constexpr uint32_t kBottomPlaneIndex = 5;

        BoundingFrustum();

        /// Sets the frustum to the frustum corresponding to the specified view projection matrix.
        void Set(const Matrix4x4& matrix);

        [[nodiscard]] bool Intersects(const BoundingBox& box) const noexcept;

        /// Gets the near plane of the frustum.
        [[nodiscard]] const Plane& GetNear() const { return _planes[kNearPlaneIndex]; }

        /// Gets the far plane of the frustum.
        [[nodiscard]] const Plane& GetFar() const { return _planes[kFarPlaneIndex]; }

        /// Gets the left plane of the frustum.
        [[nodiscard]] const Plane& GetLeft() const { return _planes[kLeftPlaneIndex]; }

        /// Gets the right plane of the frustum.
        [[nodiscard]] const Plane& GetRight() const { return _planes[kRightPlaneIndex]; }

        /// Gets the top plane of the frustum.
        [[nodiscard]] const Plane& GetTop() const { return _planes[kTopPlaneIndex]; }

        /// Gets the bottom plane of the frustum.
        [[nodiscard]] const Plane& GetBottom() const { return _planes[kBottomPlaneIndex]; }

    private:
        /// Update the planes.
        void UpdatePlanes();

        Plane _planes[kCornerCount];
        Matrix4x4 _matrix;
    };
}
