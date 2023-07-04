// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright © Amer Koleci and Contributors. Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Numerics;

/// <summary>
/// Provides a set of methods to supplement or replace <see cref="Math" /> and <see cref="MathF" />.
/// </summary>
public static class MathHelper
{
    /// <summary>Computes the absolute value of a given 16-bit signed integer.</summary>
    /// <param name="value">The integer for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    /// <remarks>This method does not account for <see cref="short.MinValue" />.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static short Abs(short value)
    {
        Debug.Assert(value != short.MinValue);
        var mask = value >> ((sizeof(short) * 8) - 1);
        return (short)((value + mask) ^ mask);
    }

    /// <summary>Computes the absolute value of a given 32-bit signed integer.</summary>
    /// <param name="value">The integer for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    /// <remarks>This method does not account for <see cref="int.MinValue" />.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int Abs(int value)
    {
        Debug.Assert(value != int.MinValue);
        var mask = value >> ((sizeof(int) * 8) - 1);
        return (value + mask) ^ mask;
    }

    /// <summary>Computes the absolute value of a given signed native integer.</summary>
    /// <param name="value">The integer for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    /// <remarks>This method does not account for <see cref="nint.MinValue" />.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static unsafe nint Abs(nint value)
    {
        Debug.Assert(value != nint.MinValue);
        var mask = value >> ((sizeof(nint) * 8) - 1);
        return (value + mask) ^ mask;
    }

    /// <summary>Computes the absolute value of a given 64-bit float.</summary>
    /// <param name="value">The float for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static double Abs(double value) => Math.Abs(value);

    /// <summary>Computes the absolute value of a given 8-bit signed integer.</summary>
    /// <param name="value">The integer for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    /// <remarks>This method does not account for <see cref="sbyte.MinValue" />.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static sbyte Abs(sbyte value)
    {
        Debug.Assert(value != sbyte.MinValue);
        var mask = value >> ((sizeof(int) * 8) - 1);
        return (sbyte)((value + mask) ^ mask);
    }

    /// <summary>Computes the absolute value of a given 32-bit float.</summary>
    /// <param name="value">The float for which to compute its absolute.</param>
    /// <returns>The absolute value of <paramref name="value" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Abs(float value) => MathF.Abs(value);

    /// <summary>Computes the arc-cosine for a given 64-bit float.</summary>
    /// <param name="value">The float, in radians, for which to compute the arc-cosine.</param>
    /// <returns>The arc-cosine of <paramref name="value" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static double Acos(double value) => Math.Acos(value);

    /// <summary>Computes the arc-cosine for a given 32-bit float.</summary>
    /// <param name="value">The float, in radians, for which to compute the arc-cosine.</param>
    /// <returns>The arc-cosine of <paramref name="value" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Acos(float value) => MathF.Acos(value);

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
    {
        return Platform.Is64BitProcess ? BitOperations.IsPow2(value) : BitOperations.IsPow2((uint)value);
    }

    // <summary>Rounds a given address down to the nearest alignment.</summary>
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
}
