// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System;
using System.Numerics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using static Vortice.UnsafeUtilities;

namespace Vortice;

/// <summary>
/// Provides a set of methods to supplement or replace <see cref="Math" /> and <see cref="MathF" />.
/// </summary>
public static unsafe class MathUtilities
{
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
        => Is64BitProcess ? BitOperations.IsPow2(value) : BitOperations.IsPow2((uint)value);

    /// <summary>Rounds a given address up to the nearest alignment.</summary>
    /// <param name="address">The address to be aligned.</param>
    /// <param name="alignment">The target alignment, which should be a power of two.</param>
    /// <returns><paramref name="address" /> rounded up to the specified <paramref name="alignment" />.</returns>
    /// <remarks>This method does not account for an <paramref name="alignment" /> which is not a <c>power of two</c>.</remarks>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint AlignUp(uint address, uint alignment)
    {
        Guard.IsTrue(IsPow2(alignment), nameof(alignment));
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
        Guard.IsTrue(IsPow2(alignment), nameof(alignment));
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
        Guard.IsTrue(IsPow2(alignment), nameof(alignment));
        return (address + (alignment - 1)) & ~(alignment - 1);
    }
}
