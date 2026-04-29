// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Serialization;
using static Alimer.Numerics.MathUtilities;

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
    public Color(int red, int green, int blue, int alpha)
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
    public float R
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.ToScalar();
    }

    /// <summary>
    /// Gets the value of the green component.
    /// </summary>
    public float G
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(1);
    }

    /// <summary>
    /// Gets the value of the blue component.
    /// </summary>
    public float B
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(2);
    }

    /// <summary>
    /// Gets the value of the alpha component.
    /// </summary>
    public float A
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => _value.GetElement(3);
    }

    /// <summary>
    /// Return sum of RGB components.
    /// </summary>
    public readonly float SumRGB => R + G + B;

    /// <summary>
    /// Return average value of the RGB channels.
    /// </summary>
    public readonly float Average => (R + G + B) / 3.0f;

    /// <summary>
    /// Return the 'grayscale' representation of RGB values, as used by JPEG and PAL/NTSC among others.
    /// </summary>
    public readonly float Luma => R * 0.299f + G * 0.587f + B * 0.114f;

    /// <summary>
    /// Deconstructs the vector's components into named variables.
    /// </summary>
    /// <param name="r">The R component</param>
    /// <param name="g">The G component</param>
    /// <param name="b">The B component</param>
    /// <param name="a">The A component</param>
    public readonly void Deconstruct(out float r, out float g, out float b, out float a)
    {
        r = R;
        g = G;
        b = B;
        a = A;
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="R" /> set to the specified value.</summary>
    /// <param name="red">The new value of the red component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="R" /> set to <paramref name="red" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithRed(float red)
    {
        Vector128<float> result = _value.WithElement(0, red);
        return new Color(result);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="G" /> set to the specified value.</summary>
    /// <param name="green">The new value of the green component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="G" /> set to <paramref name="green" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithGreen(float green)
    {
        Vector128<float> result = _value.WithElement(0, green);
        return new Color(result);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="B" /> set to the specified value.</summary>
    /// <param name="blue">The new value of the blue component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="B" /> set to <paramref name="blue" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithBlue(float blue)
    {
        Vector128<float> result = _value.WithElement(0, blue);
        return new Color(result);
    }

    /// <summary>Creates a new <see cref="Color" /> instance with <see cref="A" /> set to the specified value.</summary>
    /// <param name="alpha">The new value of the alpha component.</param>
    /// <returns>A new <see cref="Color" /> instance with <see cref="A" /> set to <paramref name="alpha" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Color WithAlpha(float alpha)
    {
        Vector128<float> result = _value.WithElement(0, alpha);
        return new Color(result);
    }

    /// <summary>
    /// Converts the color into a packed integer.
    /// </summary>
    /// <returns>A packed integer containing all four color components.</returns>
    public int ToBgra()
    {
        uint a = (uint)(A * 255.0f) & 255;
        uint r = (uint)(R * 255.0f) & 255;
        uint g = (uint)(G * 255.0f) & 255;
        uint b = (uint)(B * 255.0f) & 255;

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
        b = (byte)(B * 255.0f);
        g = (byte)(G * 255.0f);
        r = (byte)(R * 255.0f);
        a = (byte)(A * 255.0f);
    }

    public readonly Vector128<float> AsVector128() => _value;

    public readonly Vector3 ToVector3() => new(R, G, B);
    public readonly Vector4 ToVector4() => new(R, G, B, A);
    public readonly ColorRgba ToRgba() => new(R, G, B, A);

    /// <summary>
    /// Converts this color from linear space to sRGB space.
    /// </summary>
    /// <returns>A color3 in sRGB space.</returns>
    public readonly Color ToSRgb()
    {
        return new(LinearToSRgb(R), LinearToSRgb(G), LinearToSRgb(B), A);
    }

    /// <summary>
    /// Converts this color from sRGB space to linear space.
    /// </summary>
    /// <returns>A color4 in linear space.</returns>
    public readonly Color ToLinear()
    {
        return new(SRgbToLinear(R), SRgbToLinear(G), SRgbToLinear(B), A);
    }

    /// <summary>
    /// Adds two colors.
    /// </summary>
    /// <param name="left">The first color to add.</param>
    /// <param name="right">The second color to add.</param>
    /// <returns>The sum of the two colors.</returns>
    public static Color Add(Color left, Color right)
    {
        return new(left._value + right._value);
    }

    /// <summary>
    /// Subtracts two colors.
    /// </summary>
    /// <param name="left">The first color to subtract.</param>
    /// <param name="right">The second color to subtract</param>
    /// <returns>The difference of the two colors.</returns>
    public static Color Subtract(Color left, Color right)
    {
        return new(left._value - right._value);
    }

    /// <summary>
    /// Modulates two colors.
    /// </summary>
    /// <param name="left">The first color to modulate.</param>
    /// <param name="right">The second color to modulate.</param>
    /// <returns>The modulated color.</returns>
    public static Color Modulate(Color left, Color right)
    {
        return new(left._value * right._value);
    }

    /// <summary>
    /// Scales a color.
    /// </summary>
    /// <param name="value">The color to scale.</param>
    /// <param name="scale">The amount by which to scale.</param>
    /// <returns>The scaled color.</returns>
    public static Color Scale(Color value, float scale)
    {
        return new(value._value * scale);
    }

    /// <summary>
    /// Negates a color.
    /// </summary>
    /// <param name="value">The color to negate.</param>
    /// <returns>The negated color.</returns>
    public static Color Negate(Color value) => new(Vector128.Negate(value._value));

    /// <summary>
    /// Restricts a value to be within a specified range.
    /// </summary>
    /// <param name="value">The value to clamp.</param>
    /// <param name="min">The minimum value.</param>
    /// <param name="max">The maximum value.</param>
    /// <returns>The clamped value.</returns>
    public static Color Clamp(Color value, Color min, Color max)
    {
        return new(Vector128.Clamp(value._value, min._value, max._value));
    }

    /// <summary>
    /// Performs a linear interpolation between two colors.
    /// </summary>
    /// <param name="start">Start color.</param>
    /// <param name="end">End color.</param>
    /// <param name="amount">Value between 0 and 1 indicating the weight of <paramref name="end"/>.</param>
    /// <returns>The linear interpolation of the two colors.</returns>
    /// <remarks>
    /// Passing <paramref name="amount"/> a value of 0 will cause <paramref name="start"/> to be returned; a value of 1 will cause <paramref name="end"/> to be returned. 
    /// </remarks>
    public static Color Lerp(Color start, Color end, float amount)
    {
        return new(Vector128.Lerp(start._value, end._value, Vector128.Create(amount)));
    }

    /// <summary>
    /// Performs a linear interpolation between two colors.
    /// </summary>
    /// <param name="start">Start color.</param>
    /// <param name="end">End color.</param>
    /// <param name="amount">Value between 0 and 1 indicating the weight of <paramref name="end"/>.</param>
    /// <returns>The linear interpolation of the two colors.</returns>
    /// <remarks>
    /// Passing <paramref name="amount"/> a value of 0 will cause <paramref name="start"/> to be returned; a value of 1 will cause <paramref name="end"/> to be returned. 
    /// </remarks>
    public static Color Lerp(Color start, Color end, Color amount)
    {
        return new(Vector128.Lerp(start._value, end._value, amount._value));
    }

    /// <summary>
    /// Performs a cubic interpolation between two colors.
    /// </summary>
    /// <param name="start">Start color.</param>
    /// <param name="end">End color.</param>
    /// <param name="amount">Value between 0 and 1 indicating the weight of <paramref name="end"/>.</param>
    /// <param name="result">When the method completes, contains the cubic interpolation of the two colors.</param>
    public static Color SmoothStep(Color start, Color end, float amount)
    {
        amount = MathUtilities.SmoothStep(amount);
        return Lerp(start, end, amount);
    }

    /// <summary>
    /// Returns a color containing the largest components of the specified colors.
    /// </summary>
    /// <param name="left">The first source color.</param>
    /// <param name="right">The second source color.</param>
    /// <returns>A color containing the largest components of the source colors.</returns>
    public static Color Max(Color left, Color right)
    {
        return new(Vector128.Max(left._value, right._value));
    }

    /// <summary>
    /// Returns a color containing the smallest components of the specified colors.
    /// </summary>
    /// <param name="left">The first source color.</param>
    /// <param name="right">The second source color.</param>
    /// <returns>A color containing the smallest components of the source colors.</returns>
    public static Color Min(Color left, Color right)
    {
        return new(Vector128.Min(left._value, right._value));
    }

    /// <summary>
    /// Premultiplies the color components by the alpha value.
    /// </summary>
    /// <param name="value">The color to premultiply.</param>
    /// <returns>A color with premultiplied alpha.</returns>
    public static Color PremultiplyAlpha(Color value)
    {
        return new(value.R * value.A, value.G * value.A, value.B * value.A, value.A);
    }

    /// <summary>
    /// Adds two colors.
    /// </summary>
    /// <param name="left">The first color to add.</param>
    /// <param name="right">The second color to add.</param>
    /// <returns>The sum of the two colors.</returns>
    public static Color operator +(Color left, Color right) => new(left._value + right._value);

    /// <summary>
    /// Assert a color (return it unchanged).
    /// </summary>
    /// <param name="value">The color to assert (unchanged).</param>
    /// <returns>The asserted (unchanged) color.</returns>
    public static Color operator +(Color value) => value;

    /// <summary>
    /// Subtracts two colors.
    /// </summary>
    /// <param name="left">The first color to subtract.</param>
    /// <param name="right">The second color to subtract.</param>
    /// <returns>The difference of the two colors.</returns>
    public static Color operator -(Color left, Color right) => new(left._value - right._value);

    /// <summary>
    /// Negates a color.
    /// </summary>
    /// <param name="value">The color to negate.</param>
    /// <returns>A negated color.</returns>
    public static Color operator -(Color value) => new(Vector128.Negate(value._value));

    /// <summary>
    /// Scales a color.
    /// </summary>
    /// <param name="scale">The factor by which to scale the color.</param>
    /// <param name="value">The color to scale.</param>
    /// <returns>The scaled color.</returns>
    public static Color operator *(float scale, Color value) => new(scale * value._value);

    /// <summary>
    /// Scales a color.
    /// </summary>
    /// <param name="value">The factor by which to scale the color.</param>
    /// <param name="scale">The color to scale.</param>
    /// <returns>The scaled color.</returns>
    public static Color operator *(Color value, float scale) => new(value._value * scale);

    /// <summary>
    /// Modulates two colors.
    /// </summary>
    /// <param name="left">The first color to modulate.</param>
    /// <param name="right">The second color to modulate.</param>
    /// <returns>The modulated color.</returns>
    public static Color operator *(Color left, Color right) => new(left._value * right._value);


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
    public override int GetHashCode() => HashCode.Combine(R, G, B, A);

    /// <inheritdoc />
    public override string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public string ToString(string? format, IFormatProvider? formatProvider)
    => $"{nameof(Color)} {{ R = {R.ToString(format, formatProvider)}, G = {G.ToString(format, formatProvider)}, B = {B.ToString(format, formatProvider)}, A = {A.ToString(format, formatProvider)} }}";

    /// <inheritdoc />
    public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
    {
        int numWritten = 0;

        if (!"Color { R = ".TryCopyTo(destination))
        {
            charsWritten = 0;
            return false;
        }
        int partLength = "Color { R = ".Length;

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

        if (!"Color { R = "u8.TryCopyTo(utf8Destination))
        {
            bytesWritten = 0;
            return false;
        }
        int partLength = "Color { R = "u8.Length;

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
