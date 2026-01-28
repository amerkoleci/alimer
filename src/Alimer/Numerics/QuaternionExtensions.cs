// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;

namespace Alimer.Numerics;

/// <summary>
/// Defines extension methods for <see cref="Quaternion"/>
/// </summary>
public static class QuaternionExtensions
{
    public static Vector3 ToEuler(this in Quaternion rotation)
    {
        float xx = rotation.X * rotation.X;
        float yy = rotation.Y * rotation.Y;
        float zz = rotation.Z * rotation.Z;

        float m31 = 2.0f * rotation.X * rotation.Z + 2.0f * rotation.Y * rotation.W;
        float m32 = 2.0f * rotation.Y * rotation.Z - 2.0f * rotation.X * rotation.W;
        float m33 = 1.0f - 2.0f * xx - 2.0f * yy;

        float cy = MathF.Sqrt(m33 * m33 + m31 * m31);
        float cx = MathF.Atan2(-m32, cy);
        if (cy > 16.0f * float.Epsilon)
        {
            float m12 = 2.0f * rotation.X * rotation.Y + 2.0f * rotation.Z * rotation.W;
            float m22 = 1.0f - 2.0f * xx - 2.0f * zz;

            return new Vector3(cx, MathF.Atan2(m31, m33), MathF.Atan2(m12, m22));
        }
        else
        {
            float m11 = 1.0f - 2.0f * yy - 2.0f * zz;
            float m21 = 2.0f * rotation.X * rotation.Y - 2.0f * rotation.Z * rotation.W;

            return new Vector3(cx, 0.0f, MathF.Atan2(-m21, m11));
        }
    }

    public static Quaternion FromEuler(float x, float y, float z)
    {
        float halfToRad = float.DegreesToRadians(0.5f);
        x *= halfToRad;
        y *= halfToRad;
        z *= halfToRad;

        (float fSinX, float fCosX) = float.SinCos(x);
        (float fSinY, float fCosY) = float.SinCos(y);
        (float fSinZ, float fCosZ) = float.SinCos(z);

        float fCosXY = fCosX * fCosY;
        float fSinXY = fSinX * fSinY;

        return new(
            fSinX * fCosY * fCosZ - fSinZ * fSinY * fCosX,
            fSinY * fCosX * fCosZ + fSinZ * fSinX * fCosY,
            fSinZ * fCosXY - fSinXY * fCosZ,
            fCosZ * fCosXY + fSinXY * fSinZ
            );
    }

    public static Quaternion FromEuler(this in Vector3 value)
    {
        return FromEuler(value.X, value.Y, value.Z);
    }
}
