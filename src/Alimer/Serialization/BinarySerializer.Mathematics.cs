// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Numerics;

namespace Alimer.Serialization;

partial class BinarySerializer
{
    /// <summary>
    /// Serializes a single <see cref="Vector2"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Vector2 value)
    {
        Span<byte> buffer = stackalloc byte[8];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer, value.X);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.Y);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.X = BinaryPrimitives.ReadSingleLittleEndian(buffer);
            value.Y = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
        }
    }

    /// <summary>
    /// Serializes a single <see cref="Vector3"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Vector3 value)
    {
        Span<byte> buffer = stackalloc byte[12];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer, value.X);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.Y);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[8..], value.Z);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.X = BinaryPrimitives.ReadSingleLittleEndian(buffer);
            value.Y = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
            value.Z = BinaryPrimitives.ReadSingleLittleEndian(buffer[8..]);
        }
    }

    /// <summary>
    /// Serializes a single <see cref="Vector4"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Vector4 value)
    {
        Span<byte> buffer = stackalloc byte[16];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer, value.X);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.Y);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[8..], value.Z);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[12..], value.W);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.X = BinaryPrimitives.ReadSingleLittleEndian(buffer);
            value.Y = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
            value.Z = BinaryPrimitives.ReadSingleLittleEndian(buffer[8..]);
            value.W = BinaryPrimitives.ReadSingleLittleEndian(buffer[12..]);
        }
    }

    /// <summary>
    /// Serializes a single <see cref="Quaternion"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Quaternion value)
    {
        Span<byte> buffer = stackalloc byte[16];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer, value.X);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.Y);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[8..], value.Z);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[12..], value.W);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.X = BinaryPrimitives.ReadSingleLittleEndian(buffer);
            value.Y = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
            value.Z = BinaryPrimitives.ReadSingleLittleEndian(buffer[8..]);
            value.W = BinaryPrimitives.ReadSingleLittleEndian(buffer[12..]);
        }
    }

    /// <summary>
    /// Serializes a single <see cref="Matrix3x2"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Matrix3x2 value)
    {
        Span<byte> buffer = stackalloc byte[24];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer[0..], value.M11);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.M12);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[8..], value.M21);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[12..], value.M22);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[16..], value.M31);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[20..], value.M32);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.M11 = BinaryPrimitives.ReadSingleLittleEndian(buffer[0..]);
            value.M12 = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
            value.M21 = BinaryPrimitives.ReadSingleLittleEndian(buffer[8..]);
            value.M22 = BinaryPrimitives.ReadSingleLittleEndian(buffer[12..]);
            value.M31 = BinaryPrimitives.ReadSingleLittleEndian(buffer[16..]);
            value.M32 = BinaryPrimitives.ReadSingleLittleEndian(buffer[20..]);
        }
    }

    /// <summary>
    /// Serializes a single <see cref="Matrix4x4"/> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref Matrix4x4 value)
    {
        Span<byte> buffer = stackalloc byte[64];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer[0..], value.M11);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[4..], value.M12);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[8..], value.M13);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[12..], value.M14);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[16..], value.M21);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[20..], value.M22);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[24..], value.M23);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[28..], value.M24);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[32..], value.M31);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[36..], value.M32);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[40..], value.M33);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[44..], value.M34);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[48..], value.M41);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[52..], value.M42);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[56..], value.M43);
            BinaryPrimitives.WriteSingleLittleEndian(buffer[60..], value.M44);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value.M11 = BinaryPrimitives.ReadSingleLittleEndian(buffer[0..]);
            value.M12 = BinaryPrimitives.ReadSingleLittleEndian(buffer[4..]);
            value.M13 = BinaryPrimitives.ReadSingleLittleEndian(buffer[8..]);
            value.M14 = BinaryPrimitives.ReadSingleLittleEndian(buffer[12..]);
            value.M21 = BinaryPrimitives.ReadSingleLittleEndian(buffer[16..]);
            value.M22 = BinaryPrimitives.ReadSingleLittleEndian(buffer[20..]);
            value.M23 = BinaryPrimitives.ReadSingleLittleEndian(buffer[24..]);
            value.M24 = BinaryPrimitives.ReadSingleLittleEndian(buffer[28..]);
            value.M31 = BinaryPrimitives.ReadSingleLittleEndian(buffer[32..]);
            value.M32 = BinaryPrimitives.ReadSingleLittleEndian(buffer[36..]);
            value.M33 = BinaryPrimitives.ReadSingleLittleEndian(buffer[40..]);
            value.M34 = BinaryPrimitives.ReadSingleLittleEndian(buffer[44..]);
            value.M41 = BinaryPrimitives.ReadSingleLittleEndian(buffer[48..]);
            value.M42 = BinaryPrimitives.ReadSingleLittleEndian(buffer[52..]);
            value.M43 = BinaryPrimitives.ReadSingleLittleEndian(buffer[56..]);
            value.M44 = BinaryPrimitives.ReadSingleLittleEndian(buffer[60..]);
        }
    }
}
