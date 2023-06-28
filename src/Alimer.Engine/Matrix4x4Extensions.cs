// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Engine;

public static class Matrix4x4Extensions
{
    public static void Deconstruct(this in Matrix4x4 matrix, out Vector3 scale, out Quaternion rotation, out Vector3 translation)
    {
        Matrix4x4.Decompose(matrix, out scale, out rotation, out translation);
    }
}
