// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Serialization;

public unsafe ref struct ReadByteStream
{
    private ReadOnlySpan<byte> _readSpan;
    private int _size;
    private int _position;

    public ReadByteStream(ReadOnlySpan<byte> data)
    {
        _readSpan = data;
        _size = data.Length;
        _position = 0;
    }

    /// <summary>
	/// The current read or write position. Values are clamped to valid range.
	/// </summary>
	public int Position
    {
        readonly get => _position;
        set
        {
            _position = int.Clamp(_position, 0, _size);
        }
    }

    public readonly int ReadRemaining
    {
        get
        {
            int remaining = _size - _position;
            return remaining < 0 ? 0 : remaining;
        }
    }

    /// <summary>
    /// Gets the data written to the underlying buffer so far, as a <see cref="ReadOnlySpan{T}"/>.
    /// </summary>
    [UnscopedRef]
    public readonly ReadOnlySpan<byte> RemainingSpan
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return _readSpan.Slice(_position);
        }
    }

    public T Read<T>()
        where T : unmanaged
    {
        if (_position + sizeof(T) > _size)
            throw new InvalidOperationException("Not enough data to read the requested type.");

        T value = Unsafe.ReadUnaligned<T>(ref MemoryMarshal.GetReference(_readSpan.Slice(_position)));
        _position += sizeof(T);
        return value;
    }

    public TEnum ReadEnum<TEnum>()
        where TEnum : struct, Enum
    {
        Type underlyingType = Enum.GetUnderlyingType(typeof(TEnum));
        object value = underlyingType switch
        {
            Type t when t == typeof(byte) => Read<byte>(),
            Type t when t == typeof(sbyte) => Read<sbyte>(),
            Type t when t == typeof(short) => Read<short>(),
            Type t when t == typeof(ushort) => Read<ushort>(),
            Type t when t == typeof(int) => Read<int>(),
            Type t when t == typeof(uint) => Read<uint>(),
            Type t when t == typeof(long) => Read<long>(),
            Type t when t == typeof(ulong) => Read<ulong>(),
            _ => throw new InvalidOperationException("Unsupported enum underlying type.")
        };

        return (TEnum)value;
    }

    public string? ReadString()
    {
        int length = Read<int>();
        if (length == -1) return null;
        if (length == 0) return string.Empty;

        if (_position + length > _size)
            throw new InvalidOperationException("Not enough data to read the requested string.");

        string value = System.Text.Encoding.UTF8.GetString(_readSpan.Slice(_position, length));
        _position += length;
        return value;
    }

    public T ReadByteSerializable<T>()
        where T : IByteSerializable
    {
        return (T)T.ReadObject(ref this);
    }
}
