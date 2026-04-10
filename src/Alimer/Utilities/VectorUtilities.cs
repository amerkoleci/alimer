// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// This file includes code based on code from https://github.com/microsoft/DirectXMath
// The original code is Copyright © Microsoft. All rights reserved. Licensed under the MIT License (MIT).

using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics;

namespace Alimer.Utilities;

/// <summary>Provides a set of methods to supplement or replace <see cref="Vector128" /> and <see cref="Vector128{T}" />.</summary>
public static unsafe class VectorUtilities
{
    /// <summary>Gets a vector where the x-component is one and all other components are zero.</summary>
    public static Vector128<float> UnitX
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Vector128.Create(1.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    /// <summary>Gets a vector where the y-component is one and all other components are zero.</summary>
    public static Vector128<float> UnitY
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Vector128.Create(0.0f, 1.0f, 0.0f, 0.0f);
        }
    }

    /// <summary>Gets a vector where the z-component is one and all other components are zero.</summary>
    public static Vector128<float> UnitZ
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Vector128.Create(0.0f, 0.0f, 1.0f, 0.0f);
        }
    }

    /// <summary>Gets a vector where the w-component is one and all other components are zero.</summary>
    public static Vector128<float> UnitW
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Vector128.Create(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    /// <summary>Gets a vector where all components are 255.0f.</summary>
    public static Vector128<float> UByteMax
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Vector128.Create(255.0f, 255.0f, 255.0f, 255.0f);
        }
    }

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

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector128<float> Saturate(Vector128<float> vector)
    {
        return Vector128.Clamp(vector, Vector128<float>.Zero, Vector128<float>.One);
    }
}
