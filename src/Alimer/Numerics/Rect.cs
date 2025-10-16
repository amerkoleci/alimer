// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Defines a floating-point rectangle.
/// </summary>
[DebuggerDisplay("X={X}, Y={Y}, Width={Width}, Height={Height}")]
public partial struct Rect : IEquatable<Rect>, IFormattable
{
    /// <summary>
    /// The x-coordinate of the rectangle.
    /// </summary>
    public float X;

    /// <summary>
    /// The y-coordinate of the rectangle.
    /// </summary>
    public float Y;

    /// <summary>
    /// The width of the rectangle.
    /// </summary>
    public float Width;

    /// <summary>
    /// The height of the rectangle.
    /// </summary>
    public float Height;

    public static Rect Zero => new();

    /// <summary>
    /// Initializes a new instance of the <see cref='Rect'/> struct.
    /// </summary>
    /// <param name="width">The width of the rectangle.</param>
    /// <param name="height">The height of the rectangle.</param>
    public Rect(float width, float height)
    {
        X = 0.0f;
        Y = 0.0f;
        Width = width;
        Height = height;
    }

    /// <summary>
    /// Initializes a new instance of <see cref="Rect"/> struct.
    /// </summary>
    /// <param name="x">The x-coordinate of the rectangle</param>
    /// <param name="y">The y-coordinate of the rectangle</param>
    /// <param name="width">The width of the rectangle</param>
    /// <param name="height">The height of the rectangle</param>
    public Rect(float x, float y, float width, float height)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref='Rect'/> struct with the specified location and size.
    /// </summary>
    /// <param name="location">The x and y-coordinate of the rectangle.</param>
    /// <param name="size">The size of the rectangle.</param>
    public Rect(in Vector2 location, in Size size)
    {
        X = location.X;
        Y = location.Y;
        Width = size.Width;
        Height = size.Height;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref='Rect'/> struct from the specified <see cref="Vector4"/>.
    /// </summary>
    /// <param name="vector">The vector containing x, y, width and height</param>
    public Rect(in Vector4 vector)
    {
        X = vector.X;
        Y = vector.Y;
        Width = vector.Z;
        Height = vector.W;
    }

    /// <summary>
    /// Gets the x-coordinate of the upper-left corner of the rectangular region defined by this
    /// <see cref='Rect'/> .
    /// </summary>
    [Browsable(false)]
    public readonly float Left => X;

    /// <summary>
    /// Gets the y-coordinate of the upper-left corner of the rectangular region defined by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public readonly float Top => Y;

    /// <summary>
    /// Gets the x-coordinate of the lower-right corner of the rectangular region defined by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public readonly float Right => X + Width;

    /// <summary>
    /// Gets the y-coordinate of the lower-right corner of the rectangular region defined by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public readonly float Bottom => Y + Height;

    /// <summary>
    /// Tests whether this <see cref='Rect'/> has a <see cref='Rect.Width'/> or a <see cref='Rect.Height'/> of 0.
    /// </summary>
    [Browsable(false)]
    public readonly bool IsEmpty => (Width <= 0) || (Height <= 0);

    /// <summary>
    /// Gets or sets the coordinates of the upper-left corner of the rectangular region represented by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public Vector2 Location
    {
        readonly get => new(X, Y);
        set
        {
            X = value.X;
            Y = value.Y;
        }
    }

    /// <summary>
    /// Gets or sets the size of this <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public Size Size
    {
        readonly get => new(Width, Height);
        set
        {
            Width = value.Width;
            Height = value.Height;
        }
    }

    /// <summary>
    /// Creates a new <see cref="Vector4"/> from this rectangle.
    /// </summary>
    public readonly Vector4 ToVector4() => new(X, Y, Width, Height);

    public static Rect FromLTRB(float left, float top, float right, float bottom) => new(left, top, right - left, bottom - top);

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is Rect value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="Rect"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Rect"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(Rect other)
    {
        return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
    }

    /// <summary>
    /// Compares two <see cref="Rect"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="Rect"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Rect"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Rect left, Rect right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="Rect"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="Rect"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Rect"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Rect left, Rect right) => !left.Equals(right);

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(X, Y, Width, Height);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{{ {nameof(X)} = {X.ToString(format, formatProvider)}, {nameof(Y)} = {Y.ToString(format, formatProvider)}, {nameof(Width)} = {Width.ToString(format, formatProvider)}, {nameof(Height)} = {Height.ToString(format, formatProvider)} }}";
    }
}
