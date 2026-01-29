// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace Alimer.Numerics;

/// <summary>
/// Defines an sphere in three dimensional space.
/// </summary>
[DataContract]
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public partial struct BoundingSphere : IEquatable<BoundingSphere>, IFormattable
{
    /// <summary>
    /// Gets a bounding sphere with zero radius.
    /// </summary>
    public static BoundingSphere Zero => new(Vector3.Zero, 0.0f);

    /// <summary>
    /// The center of the sphere.
    /// </summary>
    public Vector3 Center;

    /// <summary>
    /// The radius of the sphere.
    /// </summary>
    public float Radius;

    /// <summary>
    /// Initializes a new instance of the <see cref="BoundingSphere"/> struct.
    /// </summary>
    /// <param name="center">The center of the sphere.</param>
    /// <param name="radius">The radius of the sphere.</param>
    public BoundingSphere(Vector3 center, float radius)
    {
        Center = center;
        Radius = radius;
    }

    public static BoundingSphere CreateFromPoints(Span<Vector3> points)
    {
        Vector3 MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
        MinX = MaxX = MinY = MaxY = MinZ = MaxZ = points[0];

        for (int i = 0; i < points.Length; ++i)
        {
            Vector3 point = points[i];
            if (point.X < MinX.X)
                MinX = point;

            if (point.X > MaxX.X)
                MaxX = point;

            if (point.Y < MinY.Y)
                MinY = point;

            if (point.Y > MaxY.Y)
                MaxY = point;

            if (point.Z < MinZ.Z)
                MinZ = point;

            if (point.Z > MaxZ.Z)
                MaxZ = point;
        }

        // Use the min/max pair that are farthest apart to form the initial sphere.
        Vector3 DeltaX = Vector3.Subtract(MaxX, MinX);
        Vector3 DeltaY = Vector3.Subtract(MaxY, MinY);
        Vector3 DeltaZ = Vector3.Subtract(MaxZ, MinZ);

        float DistX = DeltaX.Length();
        float DistY = DeltaY.Length();
        float DistZ = DeltaZ.Length();

        Vector3 center;
        float radius;

        if (DistX > DistY)
        {
            if (DistX > DistZ)
            {
                // Use min/max x.
                center = Vector3.Lerp(MaxX, MinX, 0.5f);
                radius = DistX * 0.5f;
            }
            else
            {
                // Use min/max z.
                center = Vector3.Lerp(MaxZ, MinZ, 0.5f);
                radius = DistZ * 0.5f;
            }
        }
        else // Y >= X
        {
            if (DistY > DistZ)
            {
                // Use min/max y.
                center = Vector3.Lerp(MaxY, MinY, 0.5f);
                radius = DistY * 0.5f;
            }
            else
            {
                // Use min/max z.
                center = Vector3.Lerp(MaxZ, MinZ, 0.5f);
                radius = DistZ * 0.5f;
            }
        }

        // Add any points not inside the sphere.
        for (int i = 0; i < points.Length; ++i)
        {
            Vector3 point = points[i];

            Vector3 Delta = Vector3.Subtract(point, center);
            float Dist = Delta.Length();

            if (Dist > radius)
            {
                // Adjust sphere to include the new point.
                radius = (radius + Dist) * 0.5f;
                center += (1.0f - (radius / Dist)) * Delta;
            }
        }

        return new BoundingSphere(center, radius);
    }

    public readonly ContainmentType Contains(in Vector3 point)
    {
        float distanceSquared = Vector3.DistanceSquared(point, Center);
        float radiusSquared = Radius * Radius;

        return distanceSquared <= radiusSquared ? ContainmentType.Contains : ContainmentType.Disjoint;
    }

    public readonly ContainmentType Contains(in BoundingSphere sphere)
    {
        float d = Vector3.Distance(Center, sphere.Center);
        float r1 = Radius;
        float r2 = sphere.Radius;

        return (r1 + r2 >= d) ? ((r1 - r2 >= d) ? ContainmentType.Contains : ContainmentType.Intersects) : ContainmentType.Disjoint;
    }

    public readonly bool Intersects(in BoundingSphere sphere)
    {
        // If the distance between the spheres' centers is less than or equal
        // to the sum of their radii, then the spheres intersect.
        float vx = sphere.Center.X - Center.Y;
        float vy = sphere.Center.Y - Center.Y;
        float vz = sphere.Center.Z - Center.Z;

        return float.Sqrt(vx * vx + vy * vy + vz * vz) <= (Radius + sphere.Radius);
    }

    public readonly bool Intersects(in BoundingBox box)
    {
        // Determine what point is closest; if the distance to that
        // point is less than the radius, then this sphere intersects.
        float cpX = Center.X;
        float cpY = Center.Y;
        float cpZ = Center.Z;

        Vector3 boxMin = box.Min;
        Vector3 boxMax = box.Max;
        // Closest x value.
        if (Center.X < boxMin.X)
        {
            cpX = boxMin.X;
        }
        else if (Center.X > boxMax.X)
        {
            cpX = boxMax.X;
        }

        // Closest y value.
        if (Center.Y < boxMin.Y)
        {
            cpY = boxMin.Y;
        }
        else if (Center.Y > boxMax.Y)
        {
            cpY = boxMax.Y;
        }

        // Closest z value.
        if (Center.Z < boxMin.Z)
        {
            cpZ = boxMin.Z;
        }
        else if (Center.Z > boxMax.Z)
        {
            cpZ = boxMax.Z;
        }

        // Find the distance to the closest point and see if it is less than or equal to the radius.
        cpX -= Center.X;
        cpY -= Center.Y;
        cpZ -= Center.Z;

        return float.Sqrt(cpX * cpX + cpY * cpY + cpZ * cpZ) <= Radius;
    }

    public readonly PlaneIntersectionType Intersects(in Plane plane)
    {
        float distance = plane.Distance(Center);

        if (float.Abs(distance) <= Radius)
        {
            return PlaneIntersectionType.Intersecting;
        }
        else if (distance > 0.0f)
        {
            return PlaneIntersectionType.Front;
        }
        else
        {
            return PlaneIntersectionType.Back;
        }
    }

    public readonly bool Intersects(in Ray ray, out float distance)
    {
         Vector3 origin = ray.Position;
         Vector3 direction = ray.Direction;

        // Calculate the vector and the square of the distance from the ray's origin to this sphere's center.
        float vx = origin.X - Center.X;
        float vy = origin.Y - Center.Y;
        float vz = origin.Z - Center.Z;
        float d2 = vx * vx + vy * vy + vz * vz;

        // Solve the quadratic equation using the ray's and sphere's equations together.
        // Since the ray's direction is guaranteed to be 1 by the Ray, we don't need to
        // calculate and use A (A=ray.getDirection().lengthSquared()).
        float B = 2.0f * (vx * direction.X + vy * direction.Y + vz * direction.Z);
        float C = d2 - Radius * Radius;
        float discriminant = B * B - 4.0f * C;

        // If the discriminant is negative, then there is no intersection.
        if (discriminant < 0.0f)
        {
            distance = .0f;
            return false;
        }

        // The intersection is at the smaller positive root.
        float sqrtDisc = float.Sqrt(discriminant);
        float t0 = (-B - sqrtDisc) * 0.5f;
        float t1 = (-B + sqrtDisc) * 0.5f;
        distance = (t0 > 0.0f && t0 < t1) ? t0 : t1;
        return true;
    }

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => (obj is BoundingSphere other) && Equals(other);

    /// <summary>
    /// Determines whether the specified <see cref="BoundingSphere"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="BoundingSphere"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(BoundingSphere other)
    {
        return Center.Equals(other.Center)
            && Radius.Equals(other.Radius);
    }

    /// <summary>
    /// Compares two <see cref="BoundingSphere"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingSphere"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingSphere"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(BoundingSphere left, BoundingSphere right)
    {
        return (left.Center == right.Center)
            && (left.Radius == right.Radius);
    }

    /// <summary>
    /// Compares two <see cref="BoundingSphere"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingSphere"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingSphere"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(BoundingSphere left, BoundingSphere right)
    {
        return (left.Center != right.Center)
            || (left.Radius != right.Radius);
    }

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(Center, Radius);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{{ {nameof(Center)} = {Center.ToString(format, formatProvider)}, {nameof(Radius)} = {Radius.ToString(format, formatProvider)} }}";
    }
}
