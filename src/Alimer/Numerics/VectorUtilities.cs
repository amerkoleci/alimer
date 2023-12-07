// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright (c) Amer Koleci and Contributors.Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

// This file includes code based on code from https://github.com/microsoft/DirectXMath
// The original code is Copyright © Microsoft. All rights reserved. Licensed under the MIT License (MIT).

using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.Arm;
using System.Runtime.Intrinsics.X86;

namespace Alimer.Numerics;

/// <summary>
/// Provides a set of methods to supplement or replace <see cref="Vector128" /> and <see cref="Vector128{T}" />.
/// </summary>
public static class VectorUtilities
{
    /// <summary>Gets the x-component of the vector.</summary>
    /// <param name="self">The vector.</param>
    /// <returns>The x-component of <paramref name="self" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float GetX(this Vector128<float> self) => self.ToScalar();

    /// <summary>Gets the y-component of the vector.</summary>
    /// <param name="self">The vector.</param>
    /// <returns>The y-component of <paramref name="self" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float GetY(this Vector128<float> self) => self.GetElement(1);

    /// <summary>Gets the z-component of the vector.</summary>
    /// <param name="self">The vector.</param>
    /// <returns>The z-component of <paramref name="self" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float GetZ(this Vector128<float> self) => self.GetElement(2);

    /// <summary>Gets the w-component of the vector.</summary>
    /// <param name="self">The vector.</param>
    /// <returns>The w-component of <paramref name="self" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float GetW(this Vector128<float> self) => self.GetElement(3);

    /// <summary>Compares two vectors to determine equality.</summary>
    /// <param name="left">The vector to compare with <paramref name="right" />.</param>
    /// <param name="right">The vector to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareEqualAll(Vector128<float> left, Vector128<float> right)
    {
        if (Sse41.IsSupported)
        {
            Vector128<float> result = Sse.CompareNotEqual(left, right);
            return Sse.MoveMask(result) == 0x00;
        }
        else if (AdvSimd.Arm64.IsSupported)
        {
            Vector128<float> result = AdvSimd.CompareEqual(left, right);
            return AdvSimd.Arm64.MinAcross(result).ToScalar() != 0;
        }
        else
        {
            return SoftwareFallback(left, right);
        }

        static bool SoftwareFallback(Vector128<float> left, Vector128<float> right)
        {
            return (left.GetX() == right.GetX())
                && (left.GetY() == right.GetY())
                && (left.GetZ() == right.GetZ())
                && (left.GetW() == right.GetW());
        }
    }

    /// <summary>Compares two vectors to determine equality.</summary>
    /// <param name="left">The vector to compare with <paramref name="right" />.</param>
    /// <param name="right">The vector to compare with <paramref name="left" />.</param>
    /// <returns><c>true</c> if <paramref name="left" /> and <paramref name="right" /> are equal; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool CompareNotEqualAny(Vector128<float> left, Vector128<float> right)
    {
        if (Sse41.IsSupported)
        {
            Vector128<float> result = Sse.CompareNotEqual(left, right);
            return Sse.MoveMask(result) != 0x00;
        }
        else if (AdvSimd.Arm64.IsSupported)
        {
            Vector128<float> result = AdvSimd.CompareEqual(left, right);
            return AdvSimd.Arm64.MaxAcross(result).ToScalar() == 0;
        }
        else
        {
            return SoftwareFallback(left, right);
        }

        static bool SoftwareFallback(Vector128<float> left, Vector128<float> right)
        {
            return (left.GetX() != right.GetX())
                || (left.GetY() != right.GetY())
                || (left.GetZ() != right.GetZ())
                || (left.GetW() != right.GetW());
        }
    }
}
