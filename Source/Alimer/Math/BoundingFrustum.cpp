// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/BoundingFrustum.h"
#include "Alimer/Math/BoundingSphere.h"
#include "Alimer/Math/BoundingBox.h"

using namespace Alimer;

BoundingFrustum::BoundingFrustum()
{
    Set(Matrix4x4::Identity);
}

void BoundingFrustum::Set(const Matrix4x4& matrix)
{
    _matrix.Set(matrix);

    UpdatePlanes();
}

void BoundingFrustum::UpdatePlanes()
{
    _planes[kNearPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 + _matrix.m13, _matrix.m24 + _matrix.m23, _matrix.m34 + _matrix.m33), _matrix.m44 + _matrix.m43);
    _planes[kFarPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 - _matrix.m13, _matrix.m24 - _matrix.m23, _matrix.m34 - _matrix.m33), _matrix.m44 - _matrix.m43);
    _planes[kLeftPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 + _matrix.m11, _matrix.m24 + _matrix.m21, _matrix.m34 + _matrix.m31), _matrix.m44 + _matrix.m41);
    _planes[kRightPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 - _matrix.m11, _matrix.m24 - _matrix.m21, _matrix.m34 - _matrix.m31), _matrix.m44 - _matrix.m41);
    _planes[kTopPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 - _matrix.m12, _matrix.m24 - _matrix.m22, _matrix.m34 - _matrix.m32), _matrix.m44 - _matrix.m42);
    _planes[kBottomPlaneIndex] = Plane::Normalize(Vector3(_matrix.m14 + _matrix.m12, _matrix.m24 + _matrix.m22, _matrix.m34 + _matrix.m32), _matrix.m44 + _matrix.m42);
}

bool BoundingFrustum::Intersects(const BoundingBox& box) const noexcept
{
    // The box must either intersect or be in the positive half-space of all six planes of the frustum.
    return (
        box.Intersects(_planes[kNearPlaneIndex]) != PlaneIntersectionType::Back
        && box.Intersects(_planes[kFarPlaneIndex]) != PlaneIntersectionType::Back
        && box.Intersects(_planes[kLeftPlaneIndex]) != PlaneIntersectionType::Back
        && box.Intersects(_planes[kRightPlaneIndex]) != PlaneIntersectionType::Back
        && box.Intersects(_planes[kTopPlaneIndex]) != PlaneIntersectionType::Back
        && box.Intersects(_planes[kBottomPlaneIndex]) != PlaneIntersectionType::Back);
}
