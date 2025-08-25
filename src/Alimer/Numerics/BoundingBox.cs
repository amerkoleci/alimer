// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace Alimer.Numerics;

/// <summary>
/// Defines an axis-aligned box-shaped 3D volume.
/// </summary>
[DataContract]
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public partial struct BoundingBox : IEquatable<BoundingBox>, IFormattable
{
    /// <summary>
    /// Specifies the total number of corners (8) in the BoundingBox.
    /// </summary>
    public const int CornerCount = 8;

    /// <summary>
    /// A <see cref="BoundingBox"/> which represents an empty space.
    /// </summary>
    public static BoundingBox Zero => new(Vector3.Zero, Vector3.Zero);

    /// <summary>
    /// A <see cref="BoundingBox"/> which represents an infinite space.
    /// </summary>
    public static BoundingBox Infinite => new(float.MaxValue, float.MaxValue, float.MaxValue, float.MinValue, float.MinValue, float.MinValue);

    /// <summary>
    /// The minimum point of the box.
    /// </summary>
    public Vector3 Min;

    /// <summary>
    /// The maximum point of the box.
    /// </summary>
    public Vector3 Max;

    /// <summary>
    /// Initializes a new instance of the <see cref="BoundingBox"/> struct.
    /// </summary>
    /// <param name="min">The minimum vertex of the bounding box.</param>
    /// <param name="max">The maximum vertex of the bounding box.</param>
    public BoundingBox(Vector3 min, Vector3 max)
    {
        Min = min;
        Max = max;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="BoundingBox"/> struct.
    /// </summary>
    /// <param name="minX">The x coordinate of the minimum point of the bounding box.</param>
    /// <param name="minY">The y coordinate of the minimum point of the bounding box.</param>
    /// <param name="minZ">The z coordinate of the minimum point of the bounding box.</param>
    /// <param name="maxX">The x coordinate of the maximum point of the bounding box.</param>
    /// <param name="maxY">The y coordinate of the maximum point of the bounding box.</param>
    /// <param name="maxZ">The z coordinate of the maximum point of the bounding box.</param>
    public BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        Min = new Vector3(minX, minY, minZ);
        Max = new Vector3(maxX, maxY, maxZ);
    }

    /// <summary>
    /// Gets the center of this bouding box.
    /// </summary>
    public readonly Vector3 Center
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => (Min + Max) / 2;
    }

    /// <summary>
    /// Gets the extent of this bouding box.
    /// </summary>
    public readonly Vector3 Extent
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => (Max - Min) / 2;
    }

    /// <summary>
    /// Gets size  of this bouding box.
    /// </summary>
    public readonly Vector3 Size => Max - Min;

    /// <summary>
    /// Gets the width of the bounding box.
    /// </summary>
    public readonly float Width
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => Extent.X * 2.0f;
    }

    /// <summary>
    /// Gets the height of the bounding box.
    /// </summary>
    public readonly float Height
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => Extent.Y * 2.0f;
    }

    /// <summary>
    /// Gets the depth of the bounding box.
    /// </summary>
    public readonly float Depth
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => Extent.Z * 2.0f;
    }

    /// <summary>
    /// Gets the volume of the bounding box.
    /// </summary>
    public readonly float Volume
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            Vector3 sides = Max - Min;
            return sides.X * sides.Y * sides.Z;
        }
    }

    /// <summary>
    /// Get the perimeter length.
    /// </summary>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly float GetPerimeter()
    {
        Vector3 sides = Max - Min;
        return 4 * (sides.X + sides.Y + sides.Z);
    }

    /// <summary>
    /// Get the surface area.
    /// </summary>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly float GetSurfaceArea()
    {
        Vector3 sides = Max - Min;
        return 2 * (sides.X * sides.Y + sides.X * sides.Z + sides.Y * sides.Z);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly void GetCorners(Span<Vector3> corners)
    {
        if (corners.Length < CornerCount)
        {
            throw new ArgumentOutOfRangeException(nameof(corners), $"GetCorners need at least {CornerCount} elements to copy corners.");
        }

        corners[0] = new Vector3(Min.X, Max.Y, Max.Z);
        corners[1] = new Vector3(Max.X, Max.Y, Max.Z);
        corners[2] = new Vector3(Max.X, Min.Y, Max.Z);
        corners[3] = new Vector3(Min.X, Min.Y, Max.Z);
        corners[4] = new Vector3(Min.X, Max.Y, Min.Z);
        corners[5] = new Vector3(Max.X, Max.Y, Min.Z);
        corners[6] = new Vector3(Max.X, Min.Y, Min.Z);
        corners[7] = new Vector3(Min.X, Min.Y, Min.Z);
    }

    public static BoundingBox CreateFromPoints(Span<Vector3> points)
    {
        Vector3 min = new(float.MaxValue);
        Vector3 max = new(float.MinValue);

        for (int i = 0; i < points.Length; ++i)
        {
            min = Vector3.Min(min, points[i]);
            max = Vector3.Max(max, points[i]);
        }

        return new BoundingBox(min, max);
    }

    public static BoundingBox CreateFromSphere(in BoundingSphere sphere)
    {
        return new(
            new Vector3(sphere.Center.X - sphere.Radius, sphere.Center.Y - sphere.Radius, sphere.Center.Z - sphere.Radius),
            new Vector3(sphere.Center.X + sphere.Radius, sphere.Center.Y + sphere.Radius, sphere.Center.Z + sphere.Radius)
            );
    }

    public static BoundingBox CreateMerged(in BoundingBox original, in BoundingBox additional)
    {
        return new(
            Vector3.Min(original.Min, additional.Min),
            Vector3.Max(original.Max, additional.Max)
        );
    }

    /// <summary>
    /// Transforms given <see cref="BoundingBox"/> using a given <see cref="Matrix4x4"/>.
    /// </summary>
    /// <param name="box">The source <see cref="BoundingBox"/>.</param>
    /// <param name="transform">A transformation matrix that might include translation, rotation, or uniform scaling.</param>
    /// <returns>The transformed BoundingBox.</returns>
    public static BoundingBox Transform(in BoundingBox box, in Matrix4x4 transform)
    {
        Transform(box, transform, out BoundingBox result);
        return result;
    }

    /// <summary>
    /// Transforms given <see cref="BoundingBox"/> using a given <see cref="Matrix4x4"/>.
    /// </summary>
    /// <param name="box">The source <see cref="BoundingBox"/>.</param>
    /// <param name="transform">A transformation matrix that might include translation, rotation, or uniform scaling.</param>
    /// <param name="result">The transformed BoundingBox.</param>
    public static void Transform(in BoundingBox box, in Matrix4x4 transform, out BoundingBox result)
    {
        Vector3 newCenter = Vector3.Transform(box.Center, transform);
        Vector3 oldEdge = box.Size * 0.5f;

        Vector3 newEdge = new(
            MathF.Abs(transform.M11) * oldEdge.X + MathF.Abs(transform.M12) * oldEdge.Y + MathF.Abs(transform.M13) * oldEdge.Z,
            MathF.Abs(transform.M21) * oldEdge.X + MathF.Abs(transform.M22) * oldEdge.Y + MathF.Abs(transform.M23) * oldEdge.Z,
            MathF.Abs(transform.M31) * oldEdge.X + MathF.Abs(transform.M32) * oldEdge.Y + MathF.Abs(transform.M33) * oldEdge.Z
        );

        result = new(newCenter - newEdge, newCenter + newEdge);
    }


    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => (obj is BoundingBox other) && Equals(other);

    /// <summary>
    /// Determines whether the specified <see cref="BoundingBox"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="BoundingBox"/> to compare with this instance.</param>
    public readonly bool Equals(BoundingBox other)
    {
        return Min.Equals(other.Min)
            && Max.Equals(other.Max);
    }

    /// <summary>
    /// Compares two <see cref="BoundingBox"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingBox"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingBox"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(BoundingBox left, BoundingBox right)
    {
        return (left.Min == right.Min)
            && (left.Max == right.Max);
    }

    /// <summary>
    /// Compares two <see cref="BoundingBox"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingBox"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingBox"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(BoundingBox left, BoundingBox right)
    {
        return (left.Min != right.Min)
            || (left.Max != right.Max);
    }

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(Min, Max);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{{ {nameof(Min)} = {Min.ToString(format, formatProvider)}, {nameof(Max)} = {Max.ToString(format, formatProvider)} }}";
    }
}
