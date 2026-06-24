// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/BoundingSphere.h"
#include "Alimer/Math/BoundingBox.h"
#include "Alimer/Core/Hash.h"

using namespace Alimer;

void BoundingSphere::Transform(const BoundingSphere& sphere, const Matrix4x4& transform, BoundingSphere& result) noexcept
{
    // Transform the center of the sphere.
    result.center = Vector3::Transform(sphere.center, transform);

    float x = transform.m11 * transform.m11 + transform.m12 * transform.m12 + transform.m13 * transform.m13;
    float y = transform.m21 * transform.m21 + transform.m22 * transform.m22 + transform.m23 * transform.m23;
    float z = transform.m31 * transform.m31 + transform.m32 * transform.m32 + transform.m33 * transform.m33;

    float d = Alimer::Max(x, Alimer::Max(y, z));

    // Scale the radius of the sphere.
    result.radius = sphere.radius * sqrtf(d);
}

BoundingSphere BoundingSphere::Transform(const BoundingSphere& sphere, const Matrix4x4& transform) noexcept
{
    BoundingSphere result;
    Transform(sphere, transform, result);
    return result;
}

ContainmentType BoundingSphere::Contains(_In_ Vector3& point) const noexcept
{
    const float distanceSquared = Vector3::DistanceSquared(point, center);
    const float radiusSquared = radius * radius;

    return distanceSquared <= radiusSquared ? ContainmentType::Contains : ContainmentType::Disjoint;
}

ContainmentType BoundingSphere::Contains(_In_ const BoundingSphere& sphere) const noexcept
{
    float d = Vector3::Distance(center, sphere.center);
    float r1 = radius;
    float r2 = sphere.radius;

    return (r1 + r2 >= d) ? ((r1 - r2 >= d) ? ContainmentType::Contains : ContainmentType::Intersects) : ContainmentType::Disjoint;
}

bool BoundingSphere::Intersects(const BoundingSphere& sphere) const noexcept
{
    // If the distance between the spheres' centers is less than or equal
    // to the sum of their radii, then the spheres intersect.
    float vx = sphere.center.x - center.x;
    float vy = sphere.center.y - center.y;
    float vz = sphere.center.z - center.z;

    return sqrt(vx * vx + vy * vy + vz * vz) <= (radius + sphere.radius);
}

bool BoundingSphere::Intersects(const BoundingBox& box) const noexcept
{
    // Determine what point is closest; if the distance to that
    // point is less than the radius, then this sphere intersects.
    float cpX = center.x;
    float cpY = center.y;
    float cpZ = center.z;

    const Vector3& boxMin = box.min;
    const Vector3& boxMax = box.max;
    // Closest x value.
    if (center.x < boxMin.x)
    {
        cpX = boxMin.x;
    }
    else if (center.x > boxMax.x)
    {
        cpX = boxMax.x;
    }

    // Closest y value.
    if (center.y < boxMin.y)
    {
        cpY = boxMin.y;
    }
    else if (center.y > boxMax.y)
    {
        cpY = boxMax.y;
    }

    // Closest z value.
    if (center.z < boxMin.z)
    {
        cpZ = boxMin.z;
    }
    else if (center.z > boxMax.z)
    {
        cpZ = boxMax.z;
    }

    // Find the distance to the closest point and see if it is less than or equal to the radius.
    cpX -= center.x;
    cpY -= center.y;
    cpZ -= center.z;

    return sqrt(cpX * cpX + cpY * cpY + cpZ * cpZ) <= radius;
}

PlaneIntersectionType BoundingSphere::Intersects(const Plane& plane) const noexcept
{
    float distance = plane.Distance(center);

    if (fabsf(distance) <= radius)
    {
        return PlaneIntersectionType::Intersecting;
    }
    else if (distance > 0.0f)
    {
        return PlaneIntersectionType::Front;
    }
    else
    {
        return PlaneIntersectionType::Back;
    }
}

bool BoundingSphere::Intersects(const Ray& ray, _Out_ float& distance) const noexcept
{
    const Vector3& origin = ray.position;
    const Vector3& direction = ray.direction;

    // Calculate the vector and the square of the distance from the ray's origin to this sphere's center.
    float vx = origin.x - center.x;
    float vy = origin.y - center.y;
    float vz = origin.z - center.z;
    float d2 = vx * vx + vy * vy + vz * vz;

    // Solve the quadratic equation using the ray's and sphere's equations together.
    // Since the ray's direction is guaranteed to be 1 by the Ray, we don't need to
    // calculate and use A (A=ray.getDirection().lengthSquared()).
    float B = 2.0f * (vx * direction.x + vy * direction.y + vz * direction.z);
    float C = d2 - radius * radius;
    float discriminant = B * B - 4.0f * C;

    // If the discriminant is negative, then there is no intersection.
    if (discriminant < 0.0f)
    {
        distance = 0.f;
        return false;
    }

    // The intersection is at the smaller positive root.
    float sqrtDisc = sqrt(discriminant);
    float t0 = (-B - sqrtDisc) * 0.5f;
    float t1 = (-B + sqrtDisc) * 0.5f;
    distance = (t0 > 0.0f && t0 < t1) ? t0 : t1;
    return true;
}

size_t BoundingSphere::GetHashCode() const noexcept
{
    size_t hash = 0;
    HashCombine(hash, center, radius);
    return hash;
}

BoundingSphere BoundingSphere::CreateFromPoints(const Vector3* points, size_t count) noexcept
{
    Vector3 MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
    MinX = MaxX = MinY = MaxY = MinZ = MaxZ = points[0];

    for (size_t i = 0; i < count; ++i)
    {
        Vector3 point = points[i];
        if (point.x < MinX.x)
            MinX = point;

        if (point.x > MaxX.x)
            MaxX = point;

        if (point.y < MinY.y)
            MinY = point;

        if (point.y > MaxY.y)
            MaxY = point;

        if (point.z < MinZ.z)
            MinZ = point;

        if (point.z > MaxZ.z)
            MaxZ = point;
    }

    // Use the min/max pair that are farthest apart to form the initial sphere.
    Vector3 deltaX = Vector3::Subtract(MaxX, MinX);
    Vector3 deltaY = Vector3::Subtract(MaxY, MinY);
    Vector3 deltaZ = Vector3::Subtract(MaxZ, MinZ);

    const float distX = deltaX.Length();
    const float distY = deltaY.Length();
    const float distZ = deltaZ.Length();

    Vector3 center;
    float radius;

    if (distX > distY)
    {
        if (distX > distZ)
        {
            // Use min/max x.
            center = Vector3::Lerp(MaxX, MinX, 0.5f);
            radius = distX * 0.5f;
        }
        else
        {
            // Use min/max z.
            center = Vector3::Lerp(MaxZ, MinZ, 0.5f);
            radius = distZ * 0.5f;
        }
    }
    else // Y >= X
    {
        if (distY > distZ)
        {
            // Use min/max y.
            center = Vector3::Lerp(MaxY, MinY, 0.5f);
            radius = distY * 0.5f;
        }
        else
        {
            // Use min/max z.
            center = Vector3::Lerp(MaxZ, MinZ, 0.5f);
            radius = distZ * 0.5f;
        }
    }

    // Add any points not inside the sphere.
    for (size_t i = 0; i < count; ++i)
    {
        Vector3 point = points[i];

        Vector3 Delta = Vector3::Subtract(point, center);
        float Dist = Delta.Length();

        if (Dist > radius)
        {
            // Adjust sphere to include the new point.
            radius = (radius + Dist) * 0.5f;
            center += (1.0f - (radius / Dist)) * Delta;
        }
    }

    return BoundingSphere(center, radius);
}

BoundingSphere BoundingSphere::CreateMerged(const BoundingSphere& original, const BoundingSphere& additional) noexcept
{
    Vector3 center1 = original.center;
    float r1 = original.radius;

    Vector3 center2 = additional.center;
    float r2 = additional.radius;

    Vector3 V = center2 - center1;

    float d = V.Length();

    if (r1 + r2 >= d)
    {
        if (r1 - r2 >= d)
        {
            return original;
        }
        else if (r2 - r1 >= d)
        {
            return additional;
        }
    }

    Vector3 N = V / d;

    float t1 = Min(-r1, d - r2);
    float t2 = Max(r1, d + r2);
    float t_5 = (t2 - t1) * 0.5f;

    BoundingSphere result;
    result.center = Vector3::Add(center1, N * (t_5 + t1));
    result.radius = t_5;
    return result;
}
