// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Defines a signed-integer point.
/// </summary>
[DebuggerDisplay("Width={Width}, Height={Height}")]
public partial struct PointI
    : IEquatable<PointI>,
      IFormattable,
      ISpanFormattable,
      IUtf8SpanFormattable
{
    /// <summary>
    /// The x-coordinate of the point.
    /// </summary>
    public int X;

    /// <summary>
    /// The y-coordinate of the point.
    /// </summary>
    public int Y;

    /// <summary>
    /// A point with both x and y-coordinate set to zero.
    /// </summary>
    public static PointI Zero => new();

    /// <summary>
    /// Initializes a new instance of <see cref="PointI"/> struct.
    /// </summary>
    /// <param name="x"></param>
    /// <param name="y"></param>
    public PointI(int x, int y)
    {
        X = x;
        Y = y;
    }

    
    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is PointI value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="Size"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Size"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(PointI other)
    {
        return X.Equals(other.X) && Y.Equals(other.Y);
    }

    /// <summary>
    /// Compares two <see cref="PointI"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="PointI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="PointI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(PointI left, PointI right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="PointI"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="PointI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="PointI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(PointI left, PointI right) => !left.Equals(right);

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(X, Y);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{nameof(PointI)} {{ X = {X.ToString(format, formatProvider)}, Y = {Y.ToString(format, formatProvider)} }}"; ;
    }

    /// <inheritdoc />
    public readonly bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"PointI { X = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "PointI { X = ".Length;

        numWritten += partLength;
        destination = destination.Slice(numWritten);

        if (!X.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", Y = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", Y = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!Y.TryFormat(destination, out partLength, format, provider))
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

        if (!"PointI { X = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "PointI { X = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(numWritten);

        if (!X.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", Y = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", Y = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!Y.TryFormat(utf8Destination, out partLength, format, provider))
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
