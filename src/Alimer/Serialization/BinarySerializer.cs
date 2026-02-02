// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Text;

namespace Alimer.Serialization;

/// <summary>
/// Class that provides serialization methods for types implementing the <see cref="IBinarySerializable"/>.
/// </summary>
public partial class BinarySerializer
{
    private SerializerMode _mode;

    /// <summary>
    /// Initializes a new instance of the <see cref="BinarySerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    /// <param name="mode">The read or write mode.</param>
    public BinarySerializer(Stream stream, SerializerMode mode)
    {
        Stream = stream;
        Mode = mode;

        Encoding = Encoding.UTF8;
    }

    /// <summary>
    /// Underlying stream this instance is reading/writing to.
    /// </summary>
    public Stream Stream { get; }

    /// <summary>
    /// Gets the serialization mode.
    /// </summary>
    public SerializerMode Mode { get; set; }

    /// <summary>
    /// Gets or sets the encoding used to serialized strings.
    /// </summary>
    public Encoding Encoding { get; set; }

    public bool IsReading => Mode == SerializerMode.Read;

    /// <summary>
    /// Serializes a single <strong>byte</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref byte value)
    {
        if (Mode == SerializerMode.Write)
        {
            Stream.WriteByte(value);
        }
        else
        {
            value = ReadByteInternal();
        }
    }

    /// <summary>
    /// Serializes a single <strong>sbyte</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref sbyte value)
    {
        if (Mode == SerializerMode.Write)
        {
            Stream.WriteByte((byte)value);
        }
        else
        {
            value = (sbyte)ReadByteInternal();
        }
    }

    /// <summary>
    /// Serializes a single <strong>boolean</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref bool value)
    {
        if (Mode == SerializerMode.Write)
        {
            Stream.WriteByte((byte)(value ? 1 : 0));
        }
        else
        {
            value = ReadByteInternal() != 0;
        }
    }

    /// <summary>
    /// Serializes a single <strong>short</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref short value)
    {
        Span<byte> buffer = stackalloc byte[2];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteInt16LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadInt16LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>ushort</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref ushort value)
    {
        Span<byte> buffer = stackalloc byte[2];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadUInt16LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>int</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref int value)
    {
        Span<byte> buffer = stackalloc byte[4];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteInt32LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadInt32LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>uint</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref uint value)
    {
        Span<byte> buffer = stackalloc byte[4];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteUInt32LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadUInt32LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>int</strong> as a packed value (from 1 byte to 5 byte. if value &lt; 128, then 1 byte...etc.)
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void SerializePackedInt(ref int value)
    {
        if (Mode == SerializerMode.Write)
        {
            Write7BitEncodedInt(value);
        }
        else
        {
            value = Read7BitEncodedInt();
        }
    }

    /// <summary>
    /// Serializes a single <strong>string</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref string? value)
    {
        if (Mode == SerializerMode.Write)
        {
            if (value == null)
            {
                // TODO: Add Stream Extensions method to write int32 directly
                Span<byte> buffer = stackalloc byte[4];
                BinaryPrimitives.WriteInt32LittleEndian(buffer, 0);
                Stream.Write(buffer);
                return;
            }

            int count = Encoding.GetByteCount(value);
            int bytes = count + 4;
            Span<byte> dst = bytes < 2048 ? stackalloc byte[bytes] : new byte[bytes];
            BinaryPrimitives.WriteInt32LittleEndian(dst, count);
            Encoding.GetBytes(value, dst[4..]);
            Stream.Write(dst);
        }
        else
        {
            // Read length first
            Span<byte> buffer = stackalloc byte[4];
            Stream.ReadExactly(buffer);

            int length = BinaryPrimitives.ReadInt32LittleEndian(buffer);
            if (length == 0)
            {
                value = default;
            }
            else
            {
                Span<byte> src = length < 2048 ? stackalloc byte[length] : new byte[length];
                Stream.ReadExactly(src);

                int charCount = Encoding.GetCharCount(src);
                Span<char> chars = charCount < 2048 ? stackalloc char[charCount] : new char[charCount];
                Encoding.GetChars(src, chars);
                value = new(chars);
            }
        }
    }

    /// <summary>
    /// Serializes a single <strong>long</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref long value)
    {
        Span<byte> buffer = stackalloc byte[8];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteInt64LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadInt64LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>ulong</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref ulong value)
    {
        Span<byte> buffer = stackalloc byte[8];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteUInt64LittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadUInt64LittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>float</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref float value)
    {
        Span<byte> buffer = stackalloc byte[4];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadSingleLittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>double</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize(ref double value)
    {
        Span<byte> buffer = stackalloc byte[8];
        if (Mode == SerializerMode.Write)
        {
            BinaryPrimitives.WriteDoubleLittleEndian(buffer, value);
            Stream.Write(buffer);
        }
        else
        {
            Stream.ReadExactly(buffer);
            value = BinaryPrimitives.ReadDoubleLittleEndian(buffer);
        }
    }

    /// <summary>
    /// Serializes a single <strong>boolean</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public void Serialize<T>(ref T value)
        where T : IBinarySerializable
    {
        value.Serialize(this);
    }

    private byte ReadByteInternal()
    {
        int b = Stream.ReadByte();
        if (b == -1)
        {
            throw new EndOfStreamException("Unable to read beyond the end of the stream.");
        }

        return (byte)b;
    }

    private int Read7BitEncodedInt()
    {
        // Unlike writing, we can't delegate to the 64-bit read on
        // 64-bit platforms. The reason for this is that we want to
        // stop consuming bytes if we encounter an integer overflow.

        uint result = 0;
        byte byteReadJustNow;

        // Read the integer 7 bits at a time. The high bit
        // of the byte when on means to continue reading more bytes.
        //
        // There are two failure cases: we've read more than 5 bytes,
        // or the fifth byte is about to cause integer overflow.
        // This means that we can read the first 4 bytes without
        // worrying about integer overflow.

        const int MaxBytesWithoutOverflow = 4;
        for (int shift = 0; shift < MaxBytesWithoutOverflow * 7; shift += 7)
        {
            // ReadByte handles end of stream cases for us.
            byteReadJustNow = ReadByteInternal();
            result |= (byteReadJustNow & 0x7Fu) << shift;

            if (byteReadJustNow <= 0x7Fu)
            {
                return (int)result; // early exit
            }
        }

        // Read the 5th byte. Since we already read 28 bits,
        // the value of this byte must fit within 4 bits (32 - 28),
        // and it must not have the high bit set.

        byteReadJustNow = ReadByteInternal();
        if (byteReadJustNow > 0b_1111u)
        {
            throw new FormatException("Too many bytes in what should have been a 7-bit encoded integer.");
        }

        result |= (uint)byteReadJustNow << (MaxBytesWithoutOverflow * 7);
        return (int)result;
    }

    private void Write7BitEncodedInt(int value)
    {
        uint uValue = (uint)value;

        // Write out an int 7 bits at a time. The high bit of the byte,
        // when on, tells reader to continue reading more bytes.
        //
        // Using the constants 0x7F and ~0x7F below offers smaller
        // codegen than using the constant 0x80.

        while (uValue > 0x7Fu)
        {
            Stream.WriteByte((byte)(uValue | ~0x7Fu));
            uValue >>= 7;
        }

        Stream.WriteByte((byte)uValue);
    }
}
