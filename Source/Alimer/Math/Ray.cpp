// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Ray.h"
#include "Alimer/Math/Plane.h"
#include "Alimer/Math/BoundingFrustum.h"
#include "Alimer/Math/BoundingSphere.h"
#include "Alimer/Math/BoundingBox.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

bool Ray::Intersects(const BoundingSphere& sphere, _Out_ float& distance) const noexcept
{
    return sphere.Intersects(*this, distance);
}

bool Ray::Intersects(const BoundingBox& box, _Out_ float& distance) const noexcept
{
    return box.Intersects(*this, distance);
}

bool Ray::Intersects(const Plane& plane, _Out_ float& distance) const noexcept
{
    const Vector3& normal = plane.normal;

    // If the origin of the ray is on the plane then the distance is zero.
    float alpha = (Vector3::Dot(normal, position) + plane.distance);
    if (fabs(alpha) < M_EPSILON)
    {
        return 0.0f;
    }

    float dot = Vector3::Dot(normal, direction);

    // If the dot product of the plane's normal and this ray's direction is zero,
    // then the ray is parallel to the plane and does not intersect it.
    if (dot == 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    // Calculate the distance along the ray's direction vector to the point where
    // the ray intersects the plane (if it is negative the plane is behind the ray).
    float d = -alpha / dot;
    if (d < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    distance = d;
    return true;
}

size_t Ray::GetHashCode() const noexcept
{
    size_t hash = 0;
    HashCombine(hash, position, direction);
    return hash;
}


//bool Ray::Intersects(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, _Out_ float& distance) const noexcept
//{
//}
