// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>Provides a set of methods to supplement or replace <see cref="Math" /> and <see cref="MathF" />.</summary>
public static partial class MathUtilities
{
    /// <summary>
    /// Represents the value of pi.
    /// </summary>
    public const float Pi = MathF.PI;

    /// <summary>
    /// Represents the value of pi times two.
    /// </summary>
    public const float TwoPi = 2 * MathF.PI;

    /// <summary>
    /// Represents the value of pi divided by two.
    /// </summary>
    public const float PiOver2 = MathF.PI / 2;

    /// <summary>
    /// Represents the value of pi divided by four.
    /// </summary>
    public const float PiOver4 = MathF.PI / 4;

    /// <summary>Gets a value used to determine if a value is near zero.</summary>
    public static float NearZeroEpsilon
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => 4.7683716E-07f; // 2^-21: 0x35000000
    }

    /// <summary>
    /// Converts radians to degrees.
    /// </summary>
    /// <param name="radians">The angle in radians.</param>
    /// <returns>The converted value.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float ToDegrees(this float radians) => radians * (180.0f / MathF.PI);

    /// <summary>
    /// Converts degrees to radians.
    /// </summary>
    /// <param name="degrees">Converts degrees to radians.</param>
    /// <returns>The converted value.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float ToRadians(this float degrees) => degrees * (MathF.PI / 180.0f);

    /// <summary>Compares two 64-bit floats to determine approximate equality.</summary>
    /// <param name="left">The float to compare with <paramref name="right" />.</param>
    /// <param name="right">The float to compare with <paramref name="left" />.</param>
    /// <param name="epsilon">The maximum (inclusive) difference between <paramref name="left" /> and <paramref name="right" /> for which they should be considered equivalent.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> differ by no more than <paramref name="epsilon" />; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareEqual(double left, double right, double epsilon) => Math.Abs(left - right) <= epsilon;

    /// <summary>Compares two 64-bit floats to determine approximate equality.</summary>
    /// <param name="left">The float to compare with <paramref name="right" />.</param>
    /// <param name="right">The float to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> differ by no more than <see cref="NearZeroEpsilon"/>; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareEqual(double left, double right) => Math.Abs(left - right) <= NearZeroEpsilon;

    /// <summary>Compares two 32-bit floats to determine approximate equality.</summary>
    /// <param name="left">The float to compare with <paramref name="right" />.</param>
    /// <param name="right">The float to compare with <paramref name="left" />.</param>
    /// <param name="epsilon">The maximum (inclusive) difference between <paramref name="left" /> and <paramref name="right" /> for which they should be considered equivalent.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> differ by no more than <paramref name="epsilon" />; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareEqual(float left, float right, float epsilon) => MathF.Abs(left - right) <= epsilon;

    /// <summary>Compares two 32-bit floats to determine approximate equality.</summary>
    /// <param name="left">The float to compare with <paramref name="right" />.</param>
    /// <param name="right">The float to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> differ by no more than <see cref="NearZeroEpsilon"/>; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareEqual(float left, float right) => MathF.Abs(left - right) <= NearZeroEpsilon;

    /// <summary>
    /// Determines whether the specified value is close to zero (0.0f).
    /// </summary>
    /// <param name="a">The floating value.</param>
    /// <returns><c>true</c> if the specified value is close to zero (0.0f); otherwise, <c>false</c>.</returns>
    public static bool IsZero(float a) => MathF.Abs(a) < NearZeroEpsilon;

    /// <summary>
    /// Determines whether the specified value is close to one (1.0f).
    /// </summary>
    /// <param name="a">The floating value.</param>
    /// <returns><c>true</c> if the specified value is close to one (1.0f); otherwise, <c>false</c>.</returns>
    public static bool IsOne(float a) => IsZero(a - 1.0f);

    /// <summary>
    /// Calculates the greatest common divisor (GCD) of two unsigned integers using Euclidean algorithm.
    /// </summary>
    public static uint GreatestCommonDivisor(uint a, uint b)
    {
        while (b != 0)
        {
            uint temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    /// <summary>
    /// Calculates the least common multiple (LCM) of two unsigned integers.
    /// Equivalent to std::lcm from C++ &lt;numeric&gt;.
    /// </summary>
    public static uint LeastCommonMultiple(uint a, uint b)
    {
        if (a == 0 || b == 0)
            return 0;

        // LCM(a, b) = (a * b) / GCD(a, b)
        // Rewritten as (a / GCD(a, b)) * b to avoid overflow
        return (a / GreatestCommonDivisor(a, b)) * b;
    }

    /// <summary>Determines whether a given value is a power of two.</summary>
    /// <param name="value">The value to check.</param>
    /// <returns><c>true</c> if <paramref name="value" /> is a power of two; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsPow2(uint value) => BitOperations.IsPow2(value);

    /// <summary>Determines whether a given value is a power of two.</summary>
    /// <param name="value">The value to check.</param>
    /// <returns><c>true</c> if <paramref name="value" /> is a power of two; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsPow2(ulong value) => BitOperations.IsPow2(value);

    /// <summary>Determines whether a given value is a power of two.</summary>
    /// <param name="value">The value to check.</param>
    /// <returns><c>true</c> if <paramref name="value" /> is a power of two; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsPow2(nuint value)
        => (Unsafe.SizeOf<nuint>() == 8) ? BitOperations.IsPow2(value) : BitOperations.IsPow2((uint)value);

    /// <summary>Rounds a given address down to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded down to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint AlignDown(uint address, uint alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return address & ~(alignment - 1);
    }

    /// <summary>Rounds a given address down to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded down to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong AlignDown(ulong address, ulong alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return address & ~(alignment - 1);
    }

    /// <summary>Rounds a given address down to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded down to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static nuint AlignDown(nuint address, nuint alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return address & ~(alignment - 1);
    }

    /// <summary>Rounds a given address up to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded up to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int AlignUp(int address, int alignment)
    {
        Debug.Assert(BitOperations.IsPow2(alignment));

        return (address + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>Rounds a given address up to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded up to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint AlignUp(uint address, uint alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return (address + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>Rounds a given address up to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded up to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong AlignUp(ulong address, ulong alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return (address + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>Rounds a given address up to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded up to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static nuint AlignUp(nuint address, nuint alignment)
    {
        Debug.Assert(IsPow2(alignment));

        return (address + (alignment - 1)) & ~(alignment - 1);
    }

    public static T AlignUp<T>(T value, T size) where T : IBinaryInteger<T>
    {
        return (value + (size - T.One)) & -size;
    }

    public static T AlignDown<T>(T value, T size) where T : IBinaryInteger<T>
    {
        return value & -size;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint DivideByMultiple(uint value, uint alignment)
    {
        return ((value + alignment - 1) / alignment);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong DivideByMultiple(ulong value, ulong alignment)
    {
        return ((value + alignment - 1) / alignment);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsNegativeOrNonFinite(double value)
    {
        ulong bits = BitConverter.DoubleToUInt64Bits(value);
        return bits >= 0x7FF0_0000_0000_0000;
    }
}
