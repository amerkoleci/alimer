// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Text;
using Alimer.Numerics;

namespace Alimer.Assets;

public sealed class AssetWriter : BinaryWriter
{
    public AssetWriter(Stream stream)
         : base(stream, Encoding.UTF8)
    {

    }

    public void WriteUTF8String(string value)
    {
        Write(Encoding.ASCII.GetBytes(value));
    }

    public void Write(in Size value)
    {
        Write(value.Width);
        Write(value.Height);
    }

    public void Write(in SizeI value)
    {
        Write(value.Width);
        Write(value.Height);
    }

    public void Write(in Rect value)
    {
        Write(value.X);
        Write(value.Y);
        Write(value.Width);
        Write(value.Height);
    }

    public void Write(in Vector2 value)
    {
        Write(value.X);
        Write(value.Y);
    }

    public void Write(in Vector3 value)
    {
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
    }

    public void Write(in Vector4 value)
    {
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
        Write(value.W);
    }

    public void Write(in Quaternion value)
    {
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
        Write(value.W);
    }

    public void Write(in Color value)
    {
        Write(value.Red);
        Write(value.Green);
        Write(value.Blue);
        Write(value.Alpha);
    }

    public void Write(in Matrix3x2 value)
    {
        Write(value.M11);
        Write(value.M12);
        Write(value.M21);
        Write(value.M22);
        Write(value.M31);
        Write(value.M32);
    }

    public void Write(in Matrix4x4 value)
    {
        Write(value.M11);
        Write(value.M12);
        Write(value.M13);
        Write(value.M14);
        Write(value.M21);
        Write(value.M22);
        Write(value.M23);
        Write(value.M24);
        Write(value.M31);
        Write(value.M32);
        Write(value.M33);
        Write(value.M34);
        Write(value.M41);
        Write(value.M42);
        Write(value.M43);
        Write(value.M44);
    }
}
