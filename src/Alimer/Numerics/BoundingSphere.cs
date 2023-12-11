// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Globalization;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Serialization;
using System.Text;
using static Alimer.Numerics.VectorUtilities;

namespace Alimer.Numerics;

/// <summary>
/// Defines an sphere in three dimensional space.
/// </summary>
public struct BoundingSphere : IEquatable<BoundingSphere>, IFormattable
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

    public static BoundingSphere CreateFromPoints(Vector3[] points)
    {
        Span<Vector3> span = points.AsSpan();
        return CreateFromPoints(span);
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

    /// <inheritdoc/>
    public override readonly bool Equals(object? obj) => (obj is BoundingSphere other) && Equals(other);

    /// <summary>
    /// Determines whether the specified <see cref="BoundingSphere"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Int4"/> to compare with this instance.</param>
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
