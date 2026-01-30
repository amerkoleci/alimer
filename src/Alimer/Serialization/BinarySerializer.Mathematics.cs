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
        Span<byte> buf = stackalloc byte[8];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buf, value.X);
            BinaryPrimitives.WriteSingleLittleEndian(buf[4..], value.Y);
            Stream.Write(buf);
        }
        else
        {
            Stream.ReadExactly(buf);
            value.X = BinaryPrimitives.ReadSingleLittleEndian(buf);
            value.Y = BinaryPrimitives.ReadSingleLittleEndian(buf[4..]);
        }
    }
}
