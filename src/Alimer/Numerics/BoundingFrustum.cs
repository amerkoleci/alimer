// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

public readonly partial struct BoundingFrustum : IEquatable<BoundingFrustum>
{
    public const int NearPlaneIndex = 0;
    public const int FarPlaneIndex = 1;
    public const int LeftPlaneIndex = 2;
    public const int RightPlaneIndex = 3;
    public const int TopPlaneIndex = 4;
    public const int BottomPlaneIndex = 5;
    public const int CornerCount = 8;

    private readonly Plane[] _planes;

    public BoundingFrustum(in Matrix4x4 viewProjection)
    {
        // Gribb-Hartmann plane extraction for row-major, RIGHT-HANDED coordinates
        // (OpenGL/Vulkan style: Z points toward camera, near < far in view space)
        // Planes point inward (normals point toward the inside of the frustum)

        _planes =
        [
            // Near plane (note the negation for right-handed)
            Plane.Normalize(new Plane(
            -viewProjection.M13,
            -viewProjection.M23,
            -viewProjection.M33,
            -viewProjection.M43)),
        
            // Far plane
            Plane.Normalize(new Plane(
                viewProjection.M14 - viewProjection.M13,
                viewProjection.M24 - viewProjection.M23,
                viewProjection.M34 - viewProjection.M33,
                viewProjection.M44 - viewProjection.M43)),
        
            // Left plane
            Plane.Normalize(new Plane(
                viewProjection.M14 + viewProjection.M11,
                viewProjection.M24 + viewProjection.M21,
                viewProjection.M34 + viewProjection.M31,
                viewProjection.M44 + viewProjection.M41)),
        
            // Right plane
            Plane.Normalize(new Plane(
                viewProjection.M14 - viewProjection.M11,
                viewProjection.M24 - viewProjection.M21,
                viewProjection.M34 - viewProjection.M31,
                viewProjection.M44 - viewProjection.M41)),
        
            // Top plane
            Plane.Normalize(new Plane(
                viewProjection.M14 - viewProjection.M12,
                viewProjection.M24 - viewProjection.M22,
                viewProjection.M34 - viewProjection.M32,
                viewProjection.M44 - viewProjection.M42)),
        
            // Bottom plane
            Plane.Normalize(new Plane(
                viewProjection.M14 + viewProjection.M12,
                viewProjection.M24 + viewProjection.M22,
                viewProjection.M34 + viewProjection.M32,
                viewProjection.M44 + viewProjection.M42)),
        ];
    }

    /// <summary>
    /// The near plane of this frustum.
    /// </summary>
    public ref readonly Plane Near => ref _planes[NearPlaneIndex];

    /// <summary>
    /// The far plane of this frustum.
    /// </summary>
    public ref readonly Plane Far => ref _planes[FarPlaneIndex];

    /// <summary>
    /// The left plane of this frustum.
    /// </summary>
    public ref readonly Plane Left => ref _planes[LeftPlaneIndex];

    /// <summary>
    /// The right plane of this frustum.
    /// </summary>
    public ref readonly Plane Right => ref _planes[RightPlaneIndex];

    /// <summary>
    /// The top  plane of this frustum.
    /// </summary>
    public ref readonly Plane Top => ref _planes[TopPlaneIndex];

    /// <summary>
    /// The bottom plane of this frustum.
    /// </summary>
    public ref readonly Plane Bottom => ref _planes[BottomPlaneIndex];

    /// <summary>
    /// Retrieves the eight corners of the bounding frustum.
    /// </summary>
    /// <returns>An array of points representing the eight corners of the bounding frustum.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly void GetCorners(Span<Vector3> corners)
    {
        if (corners.Length < CornerCount)
        {
            throw new ArgumentOutOfRangeException(nameof(corners), $"GetCorners need at least {CornerCount} elements to copy corners.");
        }

        corners[0] = IntersectionPoint(_planes[0], _planes[2], _planes[4]);
        corners[1] = IntersectionPoint(_planes[0], _planes[3], _planes[4]);
        corners[2] = IntersectionPoint(_planes[0], _planes[3], _planes[5]);
        corners[3] = IntersectionPoint(_planes[0], _planes[2], _planes[5]);
        corners[4] = IntersectionPoint(_planes[1], _planes[2], _planes[4]);
        corners[5] = IntersectionPoint(_planes[1], _planes[3], _planes[4]);
        corners[6] = IntersectionPoint(_planes[1], _planes[3], _planes[5]);
        corners[7] = IntersectionPoint(_planes[1], _planes[2], _planes[5]);
    }

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is BoundingFrustum value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="BoundingFrustum"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="BoundingFrustum"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(BoundingFrustum other)
    {
        for (int i = 0; i < _planes.Length; i++)
        {
            if (!_planes[i].Equals(other._planes[i]))
            {
                return false;
            }
        }

        return true;
    }

    /// <summary>
    /// Compares two <see cref="BoundingFrustum"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingFrustum"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingFrustum"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(BoundingFrustum left, BoundingFrustum right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="BoundingFrustum"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="BoundingFrustum"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="BoundingFrustum"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(BoundingFrustum left, BoundingFrustum right) => !left.Equals(right);

    /// <inheritdoc/>
    public override int GetHashCode() => HashCode.Combine(Near, Far, Left, Right, Top, Bottom);

    /// <inheritdoc />
    public override string ToString() => $"{nameof(BoundingFrustum)}";

    public readonly bool Intersects(in BoundingBox box)
    {
        // The box must either intersect or be in the positive half-space of all six planes of the frustum.
        return (box.Intersects(_planes[NearPlaneIndex]) != PlaneIntersectionType.Back
            && box.Intersects(_planes[FarPlaneIndex]) != PlaneIntersectionType.Back
            && box.Intersects(_planes[LeftPlaneIndex]) != PlaneIntersectionType.Back
            && box.Intersects(_planes[RightPlaneIndex]) != PlaneIntersectionType.Back
            && box.Intersects(_planes[TopPlaneIndex]) != PlaneIntersectionType.Back
            && box.Intersects(_planes[BottomPlaneIndex]) != PlaneIntersectionType.Back);
    }

    private static Vector3 IntersectionPoint(Plane a, Plane b, Plane c)
    {
        var cross = Vector3.Cross(b.Normal, c.Normal);

        float f = Vector3.Dot(a.Normal, cross);
        f *= -1.0f;

        var v1 = Vector3.Multiply(cross, a.D);

        cross = Vector3.Cross(c.Normal, a.Normal);
        var v2 = Vector3.Multiply(cross, b.D);

        cross = Vector3.Cross(a.Normal, b.Normal);
        var v3 = Vector3.Multiply(cross, c.D);

        Vector3 result;
        result.X = (v1.X + v2.X + v3.X) / f;
        result.Y = (v1.Y + v2.Y + v3.Y) / f;
        result.Z = (v1.Z + v2.Z + v3.Z) / f;

        return result;
    }
}
