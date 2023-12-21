// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Numerics;

public static class Matrix4x4Extensions
{
    public static Vector4 GetRow1(this in Matrix4x4 matrix) => new(matrix.M11, matrix.M12, matrix.M13, matrix.M14);
    public static Vector4 GetRow2(this in Matrix4x4 matrix) => new(matrix.M21, matrix.M22, matrix.M23, matrix.M24);
    public static Vector4 GetRow3(this in Matrix4x4 matrix) => new(matrix.M31, matrix.M32, matrix.M33, matrix.M34);
    public static Vector4 GetRow4(this in Matrix4x4 matrix) => new(matrix.M41, matrix.M42, matrix.M43, matrix.M44);

    public static Vector4 GetColumn1(this in Matrix4x4 matrix) => new(matrix.M11, matrix.M21, matrix.M31, matrix.M41);
    public static Vector4 GetColumn2(this in Matrix4x4 matrix) => new(matrix.M12, matrix.M22, matrix.M32, matrix.M42);
    public static Vector4 GetColumn3(this in Matrix4x4 matrix) => new(matrix.M13, matrix.M23, matrix.M33, matrix.M43);
    public static Vector4 GetColumn4(this in Matrix4x4 matrix) => new(matrix.M14, matrix.M24, matrix.M34, matrix.M44);

    public static void Deconstruct(this in Matrix4x4 matrix, out Vector3 scale, out Quaternion rotation, out Vector3 translation)
    {
        Matrix4x4.Decompose(matrix, out scale, out rotation, out translation);
    }
}
