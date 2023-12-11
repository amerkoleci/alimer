// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace Alimer.Numerics;

/// <summary>
/// Defines a ray.
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct Ray : IEquatable<Ray>, IFormattable
{
    /// <summary>
    /// The position in three dimensional space where the ray starts.
    /// </summary>
    public Vector3 Position;

    /// <summary>
    /// The normalized direction in which the ray points.
    /// </summary>
    public Vector3 Direction;

    /// <summary>
    /// Initializes a new instance of the <see cref="Ray"/> struct.
    /// </summary>
    /// <param name="position">The position in three dimensional space of the origin of the ray.</param>
    /// <param name="direction">The normalized direction of the ray.</param>
    public Ray(in Vector3 position, in Vector3 direction)
    {
        Position = position;
        Direction = direction;
    }

    /// <inheritdoc/>
    public override readonly bool Equals(object? obj) => obj is Ray value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="Ray"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Int4"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(Ray other)
    {
        return Position.Equals(other.Position)
            && Direction.Equals(other.Direction);
    }

    /// <summary>
    /// Compares two <see cref="Ray"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="Ray"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Ray"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Ray left, Ray right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="Ray"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="Ray"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Ray"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Ray left, Ray right) => !left.Equals(right);

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(Position, Direction);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{{ {nameof(Position)} = {Position.ToString(format, formatProvider)}, {nameof(Direction)} = {Direction.ToString(format, formatProvider)} }}";
    }
}
