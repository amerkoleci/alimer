// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Defines a signed-integer rectangle.
/// </summary>
[DebuggerDisplay("X={X}, Y={Y}, Width={Width}, Height={Height}")]
public partial struct RectI : IEquatable<RectI>, IFormattable
{
    /// <summary>
    /// The x-coordinate of the rectangle.
    /// </summary>
    public int X;

    /// <summary>
    /// The y-coordinate of the rectangle.
    /// </summary>
    public int Y;

    /// <summary>
    /// The width of the rectangle.
    /// </summary>
    public int Width;

    /// <summary>
    /// The height of the rectangle.
    /// </summary>
    public int Height;

    /// <summary>
    /// Gets a rectangle with all coordinates and dimensions set to zero.
    /// </summary>
    public static RectI Zero => new();

    /// <summary>
    /// Initializes a new instance of the <see cref='RectI'/> struct.
    /// </summary>
    /// <param name="width">The width of the rectangle.</param>
    /// <param name="height">The height of the rectangle.</param>
    public RectI(int width, int height)
    {
        X = 0;
        Y = 0;
        Width = width;
        Height = height;
    }

    /// <summary>
    /// Initializes a new instance of <see cref="RectI"/> struct.
    /// </summary>
    /// <param name="x">The x-coordinate of the rectangle</param>
    /// <param name="y">The y-coordinate of the rectangle</param>
    /// <param name="width">The width of the rectangle</param>
    /// <param name="height">The height of the rectangle</param>
    public RectI(int x, int y, int width, int height)
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
    public RectI(in PointI location, in SizeI size)
    {
        X = location.X;
        Y = location.Y;
        Width = size.Width;
        Height = size.Height;
    }

    /// <summary>
    /// Gets the x-coordinate of the upper-left corner of the rectangular region defined by this
    /// <see cref='RectI'/> .
    /// </summary>
    [Browsable(false)]
    public readonly int Left => X;

    /// <summary>
    /// Gets the y-coordinate of the upper-left corner of the rectangular region defined by this
    /// <see cref='RectI'/>.
    /// </summary>
    [Browsable(false)]
    public readonly int Top => Y;

    /// <summary>
    /// Gets the x-coordinate of the lower-right corner of the rectangular region defined by this
    /// <see cref='RectI'/>.
    /// </summary>
    [Browsable(false)]
    public readonly int Right => X + Width;

    /// <summary>
    /// Gets the y-coordinate of the lower-right corner of the rectangular region defined by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public readonly int Bottom => Y + Height;

    /// <summary>
    /// Gets a value indicating whether the rectangle is empty.
    /// </summary>
    [Browsable(false)]
    public readonly bool IsEmpty => Width == 0 || Height == 0;

    /// <summary>
    /// Gets or sets the coordinates of the upper-left corner of the rectangular region represented by this
    /// <see cref='Rect'/>.
    /// </summary>
    [Browsable(false)]
    public PointI Location
    {
        readonly get => new(X, Y);
        set
        {
            X = value.X;
            Y = value.Y;
        }
    }

    /// <summary>
    /// Gets or sets the size of this rectangle.
    /// </summary>
    [Browsable(false)]
    public SizeI Size
    {
        readonly get => new(Width, Height);
        set
        {
            Width = value.Width;
            Height = value.Height;
        }
    }

    public static RectI FromLTRB(int left, int top, int right, int bottom) => new(left, top, right - left, bottom - top);

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is RectI value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="RectI"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="RectI"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(RectI other)
    {
        return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
    }

    /// <summary>
    /// Compares two <see cref="RectI"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="RectI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="RectI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(RectI left, RectI right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="Rect"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="RectI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="RectI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(RectI left, RectI right) => !left.Equals(right);

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
