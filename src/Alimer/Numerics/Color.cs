// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Text;
using static Alimer.Numerics.VectorUtilities;

namespace Alimer.Numerics;

/// <summary>
/// Represents a floating-point RGBA color.
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly struct Color : IEquatable<Color>, IFormattable
{
    private readonly Vector128<float> _value;

    /// <summary>
    /// Initializes a new instance of the <see cref="Color"/> struct.
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
        _value = Vector128.Create(value, value, value, value);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Color" /> struct.
    /// </summary>
    /// <param name="red">The value of the red component.</param>
    /// <param name="green">The value of the green component.</param>
    /// <param name="blue">The value of the blue component.</param>
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
    /// <param name="value">The value of the vector.</param>
    public Color(Vector128<float> value)
    {
        _value = value;
    }

    /// <summary>Gets the value of the red component.</summary>
    public float R
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return _value.ToScalar();
        }
    }

    /// <summary>Gets the value of the green component.</summary>
    public float G
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return _value.GetElement(1);
        }
    }

    /// <summary>Gets the value of the blue component.</summary>
    public float B
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return _value.GetElement(2);
        }
    }

    /// <summary>Gets the value of the alpha component.</summary>
    public float A
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return _value.GetElement(3);
        }
    }

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Color left, Color right) => CompareEqualAll(left._value, right._value);

    /// <summary>Compares two colors to determine equality.</summary>
    /// <param name="left">The color to compare with <paramref name="right" />.</param>
    /// <param name="right">The color to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Color left, Color right) => CompareNotEqualAny(left._value, right._value);

    /// <inheritdoc />
    public override bool Equals(object? obj) => obj is Color other && Equals(other);

    /// <inheritdoc />
    public bool Equals(Color other) => this == other;

    /// <inheritdoc />
    public override int GetHashCode() => HashCode.Combine(R, G, B, A);

    /// <inheritdoc />
    public override string ToString() => ToString(format: null, formatProvider: null);

    /// <inheritdoc />
    public string ToString(string? format, IFormatProvider? formatProvider)
    {
        var separator = NumberFormatInfo.GetInstance(formatProvider).NumberGroupSeparator;

        return new StringBuilder(9 + (separator.Length * 3))
            .Append('<')
            .Append(R.ToString(format, formatProvider))
            .Append(separator)
            .Append(' ')
            .Append(G.ToString(format, formatProvider))
            .Append(separator)
            .Append(' ')
            .Append(B.ToString(format, formatProvider))
            .Append(separator)
            .Append(' ')
            .Append(A.ToString(format, formatProvider))
            .Append('>')
            .ToString();
    }
}
