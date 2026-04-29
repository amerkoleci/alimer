// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Serialization;
using static Alimer.Utilities.VectorUtilities;

namespace Alimer.Numerics;

/// <summary>
/// Defines a 32-bit RGBA color.
/// </summary>
[DataContract]
[StructLayout(LayoutKind.Sequential, Pack = 4, Size = 4)]
public readonly partial struct ColorRgba
    : IEquatable<ColorRgba>,
      IFormattable,
      ISpanFormattable,
      IUtf8SpanFormattable
{
    /// <summary>
    /// The red component of the color.
    /// </summary>
    public readonly byte R;

    /// <summary>
    /// The green component of the color.
    /// </summary>
    public readonly byte G;

    /// <summary>
    /// The blue component of the color.
    /// </summary>
    public readonly byte B;

    /// <summary>
    /// The alpha component of the color.
    /// </summary>
    public readonly byte A;

    /// <summary>
	/// Gets the Color Value in a RGBA 32-bit unsigned integer
	/// </summary>
	public readonly uint RGBA => ((uint)R << 24) | ((uint)G << 16) | ((uint)B << 8) | (uint)A;

    /// <summary>
    /// The Color Value in a ABGR 32-bit unsigned integer
    /// </summary>
    public readonly uint ABGR => ((uint)A << 24) | ((uint)B << 16) | ((uint)G << 8) | (uint)R;

    /// <summary>
    /// Initializes a new instance of the <see cref="ColorRgba"/> struct from packed rgba value.
    /// </summary>
    /// <param name="rgba">Packed rgba uint value.</param>
	public ColorRgba(uint rgba)
    {
        R = (byte)(rgba >> 24);
        G = (byte)(rgba >> 16);
        B = (byte)(rgba >> 08);
        A = (byte)(rgba);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ColorRgba"/> struct.
    /// </summary>
    /// <param name="red">The red component of the color.</param>
    /// <param name="green">The green component of the color.</param>
    /// <param name="blue">The blue component of the color.</param>
    public ColorRgba(byte red, byte green, byte blue)
    {
        R = red;
        G = green;
        B = blue;
        A = 255;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ColorRgba"/> struct.
    /// </summary>
    /// <param name="red">The red component of the color.</param>
    /// <param name="green">The green component of the color.</param>
    /// <param name="blue">The blue component of the color.</param>
    /// <param name="alpha">The alpha component of the color</param>
    public ColorRgba(byte red, byte green, byte blue, byte alpha)
    {
        R = red;
        G = green;
        B = blue;
        A = alpha;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ColorRgba"/> struct.
    /// </summary>
    /// <param name="value">The value that will be assigned to all components.</param>
    public ColorRgba(float value)
        : this(value, value, value, value)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="r">Red component.</param>
    /// <param name="g">Green component.</param>
    /// <param name="b">Blue component.</param>
    /// <param name="a">Alpha component.</param>
    public ColorRgba(float r, float g, float b, float a = 1.0f)
    {
        Vector128<float> result = Saturate(Vector128.Create(r, g, b, a));
        result = Vector128.Multiply(result, UByteMax);
        result = Vector128.Truncate(result);

        R = (byte)result.GetX();
        G = (byte)result.GetY();
        B = (byte)result.GetZ();
        A = (byte)result.GetW();
    }

    /// <summary>Reinterprets the current instance as a new <see cref="Vector128{Single}" />.</summary>
    /// <returns>The current instance reinterpreted as a new <see cref="Vector128{Single}" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Vector128<float> AsVector128() => Vector128.Create(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);

    /// <summary>
    /// Converts the color into a three component vector.
    /// </summary>
    /// <returns>A three component vector containing the red, green, and blue components of the color.</returns>
    public Vector3 ToVector3()
    {
        return new Vector3(R / 255.0f, G / 255.0f, B / 255.0f);
    }

    /// <summary>
    /// Gets a four-component vector representation for this object.
    /// </summary>
    public Vector4 ToVector4()
    {
        return new(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
    }

    /// <summary>
    /// Convert this instance to a <see cref="Color"/>
    /// </summary>
    public Color ToColor()
    {
        return new(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
    }

    /// <summary>
    /// Performs an implicit conversion from <see cref="ColorRgba"/> to <see cref="Color"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static implicit operator Color(ColorRgba value) => value.ToColor();

    /// <summary>
    /// Performs an explicit conversion from <see cref="Vector3"/> to <see cref="Color"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static explicit operator ColorRgba(in Vector3 value) => new(value.X, value.Y, value.Z, 1.0f);

    /// <summary>
    /// Performs an explicit conversion from <see cref="Vector4"/> to <see cref="Color"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static explicit operator ColorRgba(in Vector4 value) => new(value.X, value.Y, value.Z, value.W);

    /// <summary>
    /// Performs an explicit conversion from <see cref="Color4"/> to <see cref="Color"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static explicit operator ColorRgba(in Color value) => new(value.R, value.G, value.B, value.A);

    /// <summary>
    /// Performs an explicit conversion from <see cref="Color"/> to <see cref="uint"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static explicit operator uint(in ColorRgba value) => value.RGBA;

    /// <summary>
    /// Performs an explicit conversion from <see cref="int"/> to <see cref="ColorRgba"/>.
    /// </summary>
    /// <param name="value">The value.</param>
    /// <returns>The result of the conversion.</returns>
    public static implicit operator ColorRgba(uint value) => new(value);

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(ColorRgba left, ColorRgba right) => left.Equals(right);

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(ColorRgba left, ColorRgba right) => !left.Equals(right);

    /// <inheritdoc />
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is ColorRgba other && Equals(other);

    /// <inheritdoc />
    public bool Equals(ColorRgba other) => RGBA == other.RGBA;

    /// <inheritdoc />
    public override int GetHashCode() => RGBA.GetHashCode();

    /// <inheritdoc />
    public override string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public string ToString(string? format, IFormatProvider? formatProvider)
    => $"{nameof(ColorRgba)} {{ R = {R.ToString(format, formatProvider)}, G = {G.ToString(format, formatProvider)}, B = {B.ToString(format, formatProvider)}, A = {A.ToString(format, formatProvider)} }}";

    /// <inheritdoc />
    public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"ColorRgba { R = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "ColorRgba { R = ".Length;

        numWritten += partLength;
        destination = destination.Slice(numWritten);

        if (!R.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", G = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", G = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!G.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", B = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", B = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!B.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", A = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", A = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!A.TryFormat(destination, out partLength, format, provider))
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
    public bool TryFormat(Span<byte> utf8Destination, out int bytesWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"ColorRgba { R = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "ColorRgba { R = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(numWritten);

        if (!R.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", G = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", G = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!G.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", B = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", B = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!B.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", A = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", A = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!A.TryFormat(utf8Destination, out partLength, format, provider))
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
