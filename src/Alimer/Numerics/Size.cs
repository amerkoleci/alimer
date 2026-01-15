// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Defines a floating-point size.
/// </summary>
[DebuggerDisplay("Width={Width}, Height={Height}")]
public partial struct Size
    : IEquatable<Size>,
      IFormattable,
      ISpanFormattable,
      IUtf8SpanFormattable
{
    /// <summary>
    /// The width of the size.
    /// </summary>
    public float Width;

    /// <summary>
    /// The height of the size.
    /// </summary>
    public float Height;

    /// <summary>
    /// A size with both width and height set to zero.
    /// </summary>
    public static Size Empty => new();

    /// <summary>
    /// Initializes a new instance of <see cref="Size"/> struct.
    /// </summary>
    /// <param name="width">The width of the size.</param>
    /// <param name="height">The height of the size.</param>
    public Size(float width, float height)
    {
        Width = width;
        Height = height;
    }

    /// <summary>
    /// Initializes a new instance of <see cref="Size"/> structure.
    /// </summary>
    /// <param name="vector">The width/height vector.</param>
    public Size(in Vector2 vector)
    {
        Width = vector.X;
        Height = vector.Y;
    }

    /// <summary>
    /// Tests whether this <see cref='Size'/> has a <see cref='Size.Width'/> or a <see cref='Size.Height'/> of 0.
    /// </summary>
    [Browsable(false)]
    public readonly bool IsEmpty => Width == 0 && Height == 0;

    /// <summary>
    /// Deconstructs this size into two float values.
    /// </summary>
    /// <param name="width">The out value for the width.</param>
    /// <param name="height">The out value for the height.</param>
    public readonly void Deconstruct(out float width, out float height)
    {
        width = Width;
        height = Height;
    }

    /// <summary>
    /// Adds two sizes.
    /// </summary>
    /// <param name="size1">First size.</param>
    /// <param name="size2">Second size.</param>
    /// <returns></returns>
    public static Size Add(Size size1, Size size2) => new(size1.Width + size2.Width, size1.Height + size2.Height);

    /// <summary>
    /// Subtracts two sizes.
    /// </summary>
    /// <param name="size1">First size.</param>
    /// <param name="size2">Second size.</param>
    /// <returns></returns>
    public static Size Subtract(Size size1, Size size2) => new(size1.Width - size2.Width, size1.Height - size2.Height);

    /// <summary>
    /// Multiplies two sizes.
    /// </summary>
    /// <param name="size1">First size.</param>
    /// <param name="size2">Second size.</param>
    /// <returns></returns>
    public static Size Multiply(Size size1, Size size2) => new(size1.Width * size2.Width, size1.Height * size2.Height);

    /// <summary>
    /// Multiplies a size by a scalar value.
    /// </summary>
    /// <param name="size">First size.</param>
    /// <param name="scalar">Scalar value.</param>
    /// <returns></returns>
    public static Size Multiply(Size size, float scalar) => new(size.Width * scalar, size.Height * scalar);

    /// <summary>
    /// Divides two sizes.
    /// </summary>
    /// <param name="size1">First size.</param>
    /// <param name="size2">Second size.</param>
    /// <returns></returns>
    public static Size Divide(Size size1, Size size2) => new(size1.Width / size2.Width, size1.Height / size2.Height);

    /// <summary>
    /// Divides a size by a scalar value.
    /// </summary>
    /// <param name="size">Source size.</param>
    /// <param name="divider">The divider.</param>
    /// <returns></returns>
    public static Size Divide(Size size, float divider)
    {
        float invRhs = 1f / divider;
        return new(size.Width * invRhs, size.Height * invRhs);
    }

    /// <summary>
    /// Creates a new <see cref="Vector2"/> from this size.
    /// </summary>
    public readonly Vector2 ToVector2() => new(Width, Height);

    /// <summary>
    /// Performs vector addition of two <see cref='Size'/>.
    /// </summary>
    public static Size operator +(Size size1, Size size2) => new(size1.Width + size2.Width, size1.Height + size2.Height);

    /// <summary>
    /// Contracts a <see cref='Size'/> by another <see cref='Size'/>
    /// </summary>
    public static Size operator -(Size size1, Size size2) => new(size1.Width - size2.Width, size1.Height - size2.Height);

    /// <summary>
    /// Multiplies <see cref="Size"/> by a <see cref="float"/> producing <see cref="Size"/>.
    /// </summary>
    /// <param name="left">Multiplier of type <see cref="float"/>.</param>
    /// <param name="right">Multiplicand of type <see cref="Size"/>.</param>
    /// <returns>Product of type <see cref="Size"/>.</returns>
    public static Size operator *(Size left, Size right) => Multiply(right, left);

    /// <summary>
    /// Multiplies <see cref="Size"/> by a <see cref="float"/> producing <see cref="Size"/>.
    /// </summary>
    /// <param name="left">Multiplier of type <see cref="float"/>.</param>
    /// <param name="right">Multiplicand of type <see cref="Size"/>.</param>
    /// <returns>Product of type <see cref="Size"/>.</returns>
    public static Size operator *(float left, Size right) => Multiply(right, left);

    /// <summary>
    /// Multiplies <see cref="Size"/> by a <see cref="float"/> producing <see cref="Size"/>.
    /// </summary>
    /// <param name="left">Multiplicand of type <see cref="Size"/>.</param>
    /// <param name="right">Multiplier of type <see cref="float"/>.</param>
    /// <returns>Product of type <see cref="Size"/>.</returns>
    public static Size operator *(Size left, float right) => Multiply(left, right);

    /// <summary>
    /// Divides <see cref="Size"/> by a <see cref="Size"/> producing <see cref="Size"/>.
    /// </summary>
    /// <param name="left">Dividend of type <see cref="Size"/>.</param>
    /// <param name="right">Divisor of type <see cref="Size"/>.</param>
    /// <returns>Result of type <see cref="Size"/>.</returns>
    public static Size operator /(Size left, Size right) => new(left.Width / right.Width, left.Height / right.Height);

    /// <summary>
    /// Divides <see cref="Size"/> by a <see cref="float"/> producing <see cref="Size"/>.
    /// </summary>
    /// <param name="left">Dividend of type <see cref="Size"/>.</param>
    /// <param name="right">Divisor of type <see cref="int"/>.</param>
    /// <returns>Result of type <see cref="Size"/>.</returns>
    public static Size operator /(Size left, float right) => new(left.Width / right, left.Height / right);

    /// <summary>
    /// Converts the specified <see cref="Size"/> to a <see cref="Vector2"/>.
    /// </summary>
    public static explicit operator Vector2(in Size size) => size.ToVector2();

    /// <summary>
    /// Converts the specified <see cref="Vector2"/> to a <see cref="Size"/>.
    /// </summary>
    public static explicit operator Size(in Vector2 vector) => new (vector);

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is Size value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="Size"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Size"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(Size other)
    {
        return Width.Equals(other.Width) && Height.Equals(other.Height);
    }

    /// <summary>
    /// Compares two <see cref="Size"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="Size"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Size"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Size left, Size right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="Size"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="Size"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Size"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Size left, Size right) => !left.Equals(right);

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(Width, Height);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{nameof(Size)} {{ Width = {Width.ToString(format, formatProvider)}, Height = {Height.ToString(format, formatProvider)} }}"; ;
    }

    /// <inheritdoc />
    public readonly bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"Size { Width = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "Size { Width = ".Length;

        numWritten += partLength;
        destination = destination.Slice(numWritten);

        if (!Width.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", Height = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", Height = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!Height.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!" }".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = " }".Length;

        charsWritten = numWritten + partLength;
        return true;
    }

    /// <inheritdoc />
    public readonly bool TryFormat(Span<byte> utf8Destination, out int bytesWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"Size { Width = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "Size { Width = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(numWritten);

        if (!Width.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", Height = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", Height = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!Height.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!" }"u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = " }"u8.Length;

        bytesWritten = numWritten + partLength;
        return true;
    }
}
