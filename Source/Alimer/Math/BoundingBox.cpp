// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/BoundingBox.h"
#include "Alimer/Math/BoundingSphere.h"
#include "Alimer/Math/Plane.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

void BoundingBox::Set(const Vector3& min, const Vector3& max)
{
    this->min = min;
    this->max = max;
}

void BoundingBox::Set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    min.Set(minX, minY, minZ);
    max.Set(maxX, maxY, maxZ);
}

void BoundingBox::Set(const BoundingBox& box)
{
    min = box.min;
    max = box.max;
}

void BoundingBox::Set(const BoundingSphere& sphere)
{
    const Vector3& center = sphere.center;
    float radius = sphere.radius;

    // Calculate the minimum point for the box.
    min.x = center.x - radius;
    min.y = center.y - radius;
    min.z = center.z - radius;

    // Calculate the maximum point for the box.
    max.x = center.x + radius;
    max.y = center.y + radius;
    max.z = center.z + radius;
}

void BoundingBox::GetCorners(Vector3* corners) const
{
    ALIMER_ASSERT(corners);

    corners[0].Set(min.x, max.y, max.z);
    corners[1].Set(max.x, max.y, max.z);
    corners[2].Set(max.x, min.y, max.z);
    corners[3].Set(min.x, min.y, max.z);
    corners[4].Set(min.x, max.y, min.z);
    corners[5].Set(max.x, max.y, min.z);
    corners[6].Set(max.x, min.y, min.z);
    corners[7].Set(min.x, min.y, min.z);
}

BoundingBox BoundingBox::CreateFromPoints(const Vector3* points, size_t count) noexcept
{
    Vector3 min(FLT_MAX);
    Vector3 max(-FLT_MAX);

    for (size_t i = 0; i < count; ++i)
    {
        min = Vector3::Min(min, points[i]);
        max = Vector3::Max(max, points[i]);
    }

    return BoundingBox(min, max);
}

BoundingBox BoundingBox::CreateFromSphere(const BoundingSphere& sphere) noexcept
{
    return BoundingBox(
        Vector3(sphere.center.x - sphere.radius, sphere.center.y - sphere.radius, sphere.center.z - sphere.radius),
        Vector3(sphere.center.x + sphere.radius, sphere.center.y + sphere.radius, sphere.center.z + sphere.radius)
        );
}

BoundingBox BoundingBox::CreateMerged(const BoundingBox& original, const BoundingBox& additional) noexcept
{
    return BoundingBox(
        Vector3::Min(original.min, additional.min),
        Vector3::Max(original.max, additional.max)
        );
}

void BoundingBox::Transform(const BoundingBox& box, const Matrix4x4& transform, BoundingBox& result) noexcept
{
    Vector3 newCenter = Vector3::Transform(box.GetCenter(), transform);
    Vector3 oldEdge = box.GetSize() * 0.5f;

    Vector3 newEdge(
        Abs(transform.m11) * oldEdge.x + Abs(transform.m12) * oldEdge.y + Abs(transform.m13) * oldEdge.z,
        Abs(transform.m21) * oldEdge.x + Abs(transform.m22) * oldEdge.y + Abs(transform.m23) * oldEdge.z,
        Abs(transform.m31) * oldEdge.x + Abs(transform.m32) * oldEdge.y + Abs(transform.m33) * oldEdge.z
        );

    result.min = newCenter - newEdge;
    result.max = newCenter + newEdge;
}

BoundingBox BoundingBox::Transform(const BoundingBox& box, const Matrix4x4& transform) noexcept
{
    BoundingBox result;
    Transform(box, transform, result);
    return result;
}

ContainmentType BoundingBox::Contains(const Vector3& point) const noexcept
{
    if (min.x <= point.x && max.x >= point.x &&
        min.y <= point.y && max.y >= point.y &&
        min.z <= point.z && max.z >= point.z)
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Disjoint;
}

ContainmentType BoundingBox::Contains(const BoundingBox& box) const noexcept
{
    if (max.x < box.min.x || min.x > box.max.x)
        return ContainmentType::Disjoint;

    if (max.y < box.min.y || min.y > box.max.y)
        return ContainmentType::Disjoint;

    if (max.z < box.min.z || min.z > box.max.z)
        return ContainmentType::Disjoint;

    if (min.x <= box.min.x && (box.max.x <= max.x
        && min.y <= box.min.y && box.max.y <= max.y)
        && min.z <= box.min.z && box.max.z <= max.z)
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Intersects;
}

ContainmentType BoundingBox::Contains(const BoundingSphere& sphere) const noexcept
{
    Vector3 vector = Vector3::Clamp(sphere.center, min, max);
    float distance = Vector3::DistanceSquared(sphere.center, vector);

    if (distance > sphere.radius * sphere.radius)
        return ContainmentType::Disjoint;

    if (((min.x + sphere.radius <= sphere.center.x) && (sphere.center.x <= max.x - sphere.radius) && (max.x - min.x > sphere.radius))
        && ((min.y + sphere.radius <= sphere.center.y) && (sphere.center.y <= max.y - sphere.radius) && (max.y - min.y > sphere.radius))
        && ((min.z + sphere.radius <= sphere.center.z) && (sphere.center.z <= max.z - sphere.radius) && (max.z - min.z > sphere.radius)))
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Intersects;
}

bool BoundingBox::Intersects(const BoundingBox& box) const noexcept
{
    if (max.x < box.min.x || min.x > box.max.x)
    {
        return false;
    }
    if (max.y < box.min.y || min.y > box.max.y)
    {
        return false;
    }

    if (max.z < box.min.z || min.z > box.max.z)
    {
        return false;
    }

    return true;
}

bool BoundingBox::Intersects(const BoundingSphere& sphere) const noexcept
{
    return sphere.Intersects(*this);
}

bool BoundingBox::Intersects(const BoundingFrustum& frustum) const noexcept
{
    return frustum.Intersects(*this);
}

PlaneIntersectionType BoundingBox::Intersects(const Plane& plane) const noexcept
{
    // Calculate the distance from the center of the box to the plane.
    Vector3 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
    float distance = plane.Distance(center);

    // Get the extents of the box from its center along each axis.
    float extentX = (max.x - min.x) * 0.5f;
    float extentY = (max.y - min.y) * 0.5f;
    float extentZ = (max.z - min.z) * 0.5f;

    const Vector3& planeNormal = plane.normal;
    if (fabsf(distance) <= (fabsf(extentX * planeNormal.x) + fabsf(extentY * planeNormal.y) + fabsf(
        extentZ * planeNormal.z)))
    {
        return PlaneIntersectionType::Intersecting;
    }

    return (distance > 0.0f) ? PlaneIntersectionType::Front : PlaneIntersectionType::Back;
}

bool BoundingBox::Intersects(const Ray& ray, _Out_ float& distance) const noexcept
{
    // Intermediate calculation variables.
    float dnear = 0.0f;
    float dfar = 0.0f;
    float tmin = 0.0f;
    float tmax = 0.0f;

    const Vector3& origin = ray.position;
    const Vector3& direction = ray.direction;

    // X direction.
    float div = 1.0f / direction.x;
    if (div >= 0.0f)
    {
        tmin = (min.x - origin.x) * div;
        tmax = (max.x - origin.x) * div;
    }
    else
    {
        tmin = (max.x - origin.x) * div;
        tmax = (min.x - origin.x) * div;
    }
    dnear = tmin;
    dfar = tmax;

    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    // Y direction.
    div = 1.0f / direction.y;
    if (div >= 0.0f)
    {
        tmin = (min.y - origin.y) * div;
        tmax = (max.y - origin.y) * div;
    }
    else
    {
        tmin = (max.y - origin.y) * div;
        tmax = (min.y - origin.y) * div;
    }

    // Update the near and far intersection distances.
    if (tmin > dnear)
    {
        dnear = tmin;
    }
    if (tmax < dfar)
    {
        dfar = tmax;
    }
    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    // Z direction.
    div = 1.0f / direction.z;
    if (div >= 0.0f)
    {
        tmin = (min.z - origin.z) * div;
        tmax = (max.z - origin.z) * div;
    }
    else
    {
        tmin = (max.z - origin.z) * div;
        tmax = (min.z - origin.z) * div;
    }

    // Update the near and far intersection distances.
    if (tmin > dnear)
    {
        dnear = tmin;
    }
    if (tmax < dfar)
    {
        dfar = tmax;
    }

    // Check if the ray misses the box.
    if (dnear > dfar || dfar < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    // The ray intersects the box (and since the direction of a Ray is normalized, dnear is the distance to the ray).
    distance = dnear;
    return true;
}
float BoundingBox::GetWidth() const noexcept
{
    return GetHalfSize().x * 2.0f;
}

float BoundingBox::GetHeight() const noexcept
{
    return GetHalfSize().y * 2.0f;
}

float BoundingBox::GetDepth() const noexcept
{
    return GetHalfSize().z * 2.0f;
}

float BoundingBox::GetVolume() const noexcept
{
    Vector3 sides = max - min;
    return sides.x * sides.y * sides.z;
}

float BoundingBox::GetPerimeter() const noexcept
{
    Vector3 sides = max - min;
    return 4 * (sides.x + sides.y + sides.z);
}

float BoundingBox::GetSurfaceArea() const noexcept
{
    Vector3 sides = max - min;
    return 2 * (sides.x * sides.y + sides.x * sides.z + sides.y * sides.z);
}

bool BoundingBox::IsEmpty() const noexcept
{
    return min.x == max.x && min.y == max.y && min.z == max.z;
}

size_t BoundingBox::GetHashCode() const noexcept
{
    size_t hash = 0;
    HashCombine(hash, min, max);
    return hash;
}
