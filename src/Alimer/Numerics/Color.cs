// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Serialization;
//using static Alimer.Utilities.VectorUtilities;

namespace Alimer.Numerics;

/// <summary>
/// Defines a floating-point RGBA color.
/// </summary>
[DataContract]
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly partial struct Color
    : IEquatable<Color>,
      IFormattable,
      ISpanFormattable,
      IUtf8SpanFormattable
{
    private readonly Vector128<float> _value;

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct with default values (Red=0, Green=0, Blue=0, Alpha=1).
    /// </summary>
    public Color()
    {
        _value = Vector128.Create(0.0f, 0.0f, 0.0f, 1.0f);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="value">The value that will be assigned to all components.</param>
    public Color(float value)
    {
        _value = Vector128.Create(value);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="red">The red component of the color.</param>
    /// <param name="green">The green component of the color.</param>
    /// <param name="blue">The blue component of the color.</param>
    public Color(float red, float green, float blue)
    {
        _value = Vector128.Create(red, green, blue, 1.0f);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct.
    /// </summary>
    /// <param name="red">The value of the red component.</param>
    /// <param name="green">The value of the green component.</param>
    /// <param name="blue">The value of the blue component.</param>
    /// <param name="alpha">The value of the alpha component.</param>
    public Color(float red, float green, float blue, float alpha)
    {
        _value = Vector128.Create(red, green, blue, alpha);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct.
    /// </summary>
    /// <param name="red">The value of the red component.</param>
    /// <param name="green">The value of the green component.</param>
    /// <param name="blue">The value of the blue component.</param>
    public Color(int red, int green, int blue)
    {
        _value = Vector128.Create(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct.
    /// </summary>
    /// <param name="red">The value of the red component.</param>
    /// <param name="green">The value of the green component.</param>
    /// <param name="blue">The value of the blue component.</param>
    public Color(int red, int green, int blue, float alpha)
    {
        _value = Vector128.Create(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="value">The red, green, blue, and alpha components of the color.</param>
    public Color(in Vector4 value)
    {
        _value = value.AsVector128();
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="value">The red, green, and blue components of the color.</param>
    /// <param name="alpha">The alpha component of the color.</param>
    public Color(in Vector3 value, float alpha)
    {
        _value = Vector128.Create(value.X, value.Y, value.Z, alpha);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
    /// </summary>
    /// <param name="rgba">A packed integer containing all four color components in RGBA order.</param>
    public Color(uint rgba)
    {
        float red = (rgba & 255) / 255.0f;
        float green = ((rgba >> 8) & 255) / 255.0f;
        float blue = ((rgba >> 16) & 255) / 255.0f;
        float alpha = ((rgba >> 24) & 255) / 255.0f;

        _value = Vector128.Create(red, green, blue, alpha);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color4"/> struct.
    /// </summary>
    /// <param name="rgba">A packed integer containing all four color components in RGBA order.</param>
    public Color(int rgba)
    {
        float red = (rgba & 255) / 255.0f;
        float green = ((rgba >> 8) & 255) / 255.0f;
        float blue = ((rgba >> 16) & 255) / 255.0f;
        float alpha = ((rgba >> 24) & 255) / 255.0f;

        _value = Vector128.Create(red, green, blue, alpha);
    }


    /// <summary>Constructs a color from the given <see cref="ReadOnlySpan{Single}" />. The span must contain at least 4 elements.</summary>
    /// <param name="values">The span of elements to assign to the vector.</param>
    public Color(ReadOnlySpan<float> values)
    {
        _value = Vector128.Create(values);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct.
    /// </summary>
    /// <param name="value">The value of the vector.</param>
    public Color(Vector128<float> value)
    {
        _value = value;
    }

    /// <summary>
    /// Gets the value of the red component.
    /// </summary>
    public float Red
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.ToScalar();
    }

    /// <summary>
    /// Gets the value of the green component.
    /// </summary>
    public float Green
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(1);
    }

    /// <summary>
    /// Gets the value of the blue component.
    /// </summary>
    public float Blue
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(2);
    }

    /// <summary>
    /// Gets the value of the alpha component.
    /// </summary>
    public float Alpha
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(3);
    }

    /// <summary>
    /// Return sum of RGB components.
    /// </summary>
    public readonly float SumRGB => Red + Green + Blue;

    /// <summary>
    /// Return average value of the RGB channels.
    /// </summary>
    public readonly float Average => (Red + Green + Blue) / 3.0f;

    /// <summary>
    /// Return the 'grayscale' representation of RGB values, as used by JPEG and PAL/NTSC among others.
    /// </summary>
    public readonly float Luma => Red * 0.299f + Green * 0.587f + Blue * 0.114f;

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="Red" /> set to the specified value.</summary>
    /// <param name="red">The new value of the red component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="Red" /> set to <paramref name="red" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithRed(float red)
    {
        Vector128<float> result = _value.WithElement(0, red);
        return new Color(result);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="Green" /> set to the specified value.</summary>
    /// <param name="green">The new value of the green component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="Green" /> set to <paramref name="green" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithGreen(float green)
    {
        Vector128<float> result = _value.WithElement(0, green);
        return new Color(result);
    }

    /// <summary>
    /// Converts the color into a packed integer.
    /// </summary>
    /// <returns>A packed integer containing all four color components.</returns>
    public int ToBgra()
    {
        uint a = (uint)(Alpha * 255.0f) & 255;
        uint r = (uint)(Red * 255.0f) & 255;
        uint g = (uint)(Green * 255.0f) & 255;
        uint b = (uint)(Blue * 255.0f) & 255;

        uint value = b;
        value |= g << 8;
        value |= r << 16;
        value |= a << 24;

        return (int)value;
    }

    /// <summary>
    /// Converts the color into a packed integer.
    /// </summary>
    /// <returns>A packed integer containing all four color components.</returns>
    public void ToBgra(out byte r, out byte g, out byte b, out byte a)
    {
        b = (byte)(Blue * 255.0f);
        g = (byte)(Green * 255.0f);
        r = (byte)(Red * 255.0f);
        a = (byte)(Alpha * 255.0f);
    }

    /// <summary>
    /// Converts the color into a packed integer.
    /// </summary>
    /// <returns>A packed integer containing all four color components.</returns>
    public uint ToRgba()
    {
        uint r = (uint)(Red * 255.0f) & 255;
        uint g = (uint)(Green * 255.0f) & 255;
        uint b = (uint)(Blue * 255.0f) & 255;
        uint a = (uint)(Alpha * 255.0f) & 255;

        uint value = r;
        value |= g << 8;
        value |= b << 16;
        value |= a << 24;

        return value;
    }

    /// <summary>
    /// Converts the color into a packed integer.
    /// </summary>
    /// <returns>A packed integer containing all four color components.</returns>
    public void ToRgba(out byte r, out byte g, out byte b, out byte a)
    {
        r = (byte)(Red * 255.0f);
        g = (byte)(Green * 255.0f);
        b = (byte)(Blue * 255.0f);
        a = (byte)(Alpha * 255.0f);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="Blue" /> set to the specified value.</summary>
    /// <param name="blue">The new value of the blue component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="Blue" /> set to <paramref name="blue" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithBlue(float blue)
    {
        Vector128<float> result = _value.WithElement(0, blue);
        return new Color(result);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="Alpha" /> set to the specified value.</summary>
    /// <param name="alpha">The new value of the alpha component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="Alpha" /> set to <paramref name="alpha" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithAlpha(float alpha)
    {
        Vector128<float> result = _value.WithElement(0, alpha);
        return new Color(result);
    }

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Color left, Color right) => Vector128.EqualsAll(left._value, right._value);

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Color left, Color right) => !Vector128.EqualsAny(left._value, right._value);

    /// <inheritdoc />
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is Color other && Equals(other);

    /// <inheritdoc />
    public bool Equals(Color other) => this == other;

    /// <inheritdoc />
    public override int GetHashCode() => HashCode.Combine(Red, Green, Blue, Alpha);

    /// <inheritdoc />
    public override string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public string ToString(string? format, IFormatProvider? formatProvider)
    => $"{nameof(Color)} {{ Red = {Red.ToString(format, formatProvider)}, Green = {Green.ToString(format, formatProvider)}, Blue = {Blue.ToString(format, formatProvider)}, Alpha = {Alpha.ToString(format, formatProvider)} }}";

    /// <inheritdoc />
    public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"Color { Red = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "Color { Red = ".Length;

        numWritten += partLength;
        destination = destination.Slice(numWritten);

        if (!Red.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", Green = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", Green = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!Green.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", Blue = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", Blue = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!Blue.TryFormat(destination, out partLength, format, provider))
        {
            charsWritten = 0;
            return false;
        }

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!", Alpha = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        partLength = ", Alpha = ".Length;

        numWritten += partLength;
        destination = destination.Slice(partLength);

        if (!Alpha.TryFormat(destination, out partLength, format, provider))
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

        if (!"Color { Red = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "Color { Red = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(numWritten);

        if (!Red.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", Green = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", Green = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!Green.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", Blue = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", Blue = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!Blue.TryFormat(utf8Destination, out partLength, format, provider))
        {
            bytesWritten = 0;
            return false;
        }

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!", Alpha = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        partLength = ", Alpha = "u8.Length;

        numWritten += partLength;
        utf8Destination = utf8Destination.Slice(partLength);

        if (!Alpha.TryFormat(utf8Destination, out partLength, format, provider))
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
