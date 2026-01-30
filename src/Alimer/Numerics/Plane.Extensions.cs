// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics;

namespace Alimer.Numerics;

/// <summary>
/// Defines extension methods for <see cref="Plane"/>
/// </summary>
public static class PlaneExtensions
{
    /// <summary>Reinterprets a <see cref="Vector4" /> as a new <see cref="Plane" />.</summary>
    /// <param name="value">The vector to reinterpret.</param>
    /// <returns><paramref name="value" /> reinterpreted as a new <see cref="Plane" />.</returns>
    public static Plane AsPlane(this Vector4 value) => Unsafe.BitCast<Vector4, Plane>(value);

    /// <summary>Reinterprets a <see cref="Plane" /> as a new <see cref="Vector4" />.</summary>
    /// <param name="value">The plane to reinterpret.</param>
    /// <returns><paramref name="value" /> reinterpreted as a new <see cref="Vector4" />.</returns>
    public static Vector4 AsVector4(this Plane value) => Unsafe.BitCast<Plane, Vector4>(value);

    /// <summary>Reinterprets a <see cref="Plane" /> as a new <see langword="Vector128&lt;Single&gt;" />.</summary>
    /// <param name="value">The plane to reinterpret.</param>
    /// <returns><paramref name="value" /> reinterpreted as a new <see langword="Vector128&lt;Single&gt;" />.</returns>
    public static Vector128<float> AsVector128(this Plane value) => Unsafe.BitCast<Plane, Vector128<float>>(value);

}
