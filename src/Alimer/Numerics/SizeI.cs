// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Defines an integer size.
/// </summary>
[DebuggerDisplay("Width={Width}, Height={Height}")]
public partial struct SizeI
    : IEquatable<SizeI>,
      IFormattable,
      ISpanFormattable,
      IUtf8SpanFormattable
{
    /// <summary>
    /// The width of the size.
    /// </summary>
    public int Width;

    /// <summary>
    /// The height of the size.
    /// </summary>
    public int Height;

    /// <summary>
    /// A size with both width and height set to zero.
    /// </summary>
    public static SizeI Empty => new();

    /// <summary>
    /// Initializes a new instance of <see cref="SizeI"/> struct.
    /// </summary>
    /// <param name="width">The width of the size.</param>
    /// <param name="height">The height of the size.</param>
    public SizeI(int width, int height)
    {
        Width = width;
        Height = height;
    }

    /// <summary>
    /// Tests whether this <see cref='SizeI'/> has a <see cref='SizeI.Width'/> or a <see cref='SizeI.Height'/> of 0.
    /// </summary>
    [Browsable(false)]
    public readonly bool IsEmpty => Width == 0 && Height == 0;

    /// <summary>
    /// Deconstructs this size into two float values.
    /// </summary>
    /// <param name="width">The out value for the width.</param>
    /// <param name="height">The out value for the height.</param>
    public readonly void Deconstruct(out int width, out int height)
    {
        width = Width;
        height = Height;
    }

    /// <summary>
    /// Converts the specified <see cref='SizeI'/> to a <see cref='Size'/>.
    /// </summary>
    public static implicit operator Size(SizeI value) => new(value.Width, value.Height);

    /// <inheritdoc/>
    public override readonly bool Equals([NotNullWhen(true)] object? obj) => obj is SizeI value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="SizeI"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="SizeI"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly bool Equals(SizeI other)
    {
        return Width.Equals(other.Width) && Height.Equals(other.Height);
    }

    /// <summary>
    /// Compares two <see cref="SizeI"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="SizeI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="SizeI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(SizeI left, SizeI right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="SizeI"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="SizeI"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="SizeI"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(SizeI left, SizeI right) => !left.Equals(right);

    /// <inheritdoc/>
    public override readonly int GetHashCode() => HashCode.Combine(Width, Height);

    /// <inheritdoc />
    public override readonly string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public readonly string ToString(string? format, IFormatProvider? formatProvider)
    {
        return $"{nameof(SizeI)} {{ Width = {Width.ToString(format, formatProvider)}, Height = {Height.ToString(format, formatProvider)} }}"; ;
    }

    /// <inheritdoc />
    public readonly bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"SizeInt { Width = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "SizeInt { Width = ".Length;

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

        if (!"SizeInt { Width = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "SizeInt { Width = "u8.Length;

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
