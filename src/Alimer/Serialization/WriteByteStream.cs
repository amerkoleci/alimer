// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers;
using System.Diagnostics.CodeAnalysis;
using System.IO.Compression;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Serialization;

public unsafe ref struct WriteByteStream : IDisposable
{
    /// <summary>
    /// The default buffer size to use to expand empty arrays.
    /// </summary>
    public const int DefaultInitialBufferSize = 256;

    private byte[]? _array;
    private int _index;

    public WriteByteStream()
        : this(DefaultInitialBufferSize)
    {

    }

    public WriteByteStream(int capacity)
    {
        if (capacity <= 0) throw new ArgumentOutOfRangeException(nameof(capacity), $"Size must be larger than 0");

        _array = ArrayPool<byte>.Shared.Rent(capacity);
        _index = 0;
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_array is not null)
            ArrayPool<byte>.Shared.Return(_array);

        _index = 0;
    }

    /// <summary>
    /// Gets the data written to the underlying buffer so far, as a <see cref="ReadOnlySpan{T}"/>.
    /// </summary>
    [UnscopedRef]
    public readonly ReadOnlySpan<byte> WrittenSpan
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            byte[]? array = _array;

            ArgumentNullException.ThrowIfNull(array);

            return array.AsSpan(0, _index);
        }
    }

    /// <summary>
    /// Writes the raw value of type <typeparamref name="T"/> into the buffer.
    /// </summary>
    /// <typeparam name="T">The type of value to write.</typeparam>
    /// <param name="value">The value to write.</param>
    /// <remarks>This method will just blit the data of <paramref name="value"/> into the target buffer.</remarks>
    public void Write<T>(scoped in T value)
        where T : unmanaged
    {
        Span<byte> span = GetSpan(sizeof(T));

        Unsafe.WriteUnaligned(ref MemoryMarshal.GetReference(span), value);

        Advance(sizeof(T));
    }

    /// <summary>
    /// Writes the raw data from the input <see cref="ReadOnlySpan{T}"/> into the buffer.
    /// </summary>
    /// <param name="data">The data to write.</param>
    public void Write(scoped ReadOnlySpan<byte> data)
    {
        Span<byte> span = GetSpan(data.Length);

        data.CopyTo(span);

        Advance(data.Length);
    }

    public void Write<T>(scoped ReadOnlySpan<T> data)
        where T : unmanaged
    {
        int size = data.Length * sizeof(T);
        Span<byte> span = GetSpan(size);
        data.CopyTo(MemoryMarshal.Cast<byte, T>(span));

        Advance(size);
    }

    public void WriteEnum<TEnum>(TEnum value)
        where TEnum : unmanaged, Enum
    {
        Type underlyingType = Enum.GetUnderlyingType(typeof(TEnum));

        switch (Type.GetTypeCode(underlyingType))
        {
            case TypeCode.Byte:
                Write(Unsafe.As<TEnum, byte>(ref value));
                break;
            case TypeCode.SByte:
                Write(Unsafe.As<TEnum, sbyte>(ref value));
                break;
            case TypeCode.Int16:
                Write(Unsafe.As<TEnum, short>(ref value));
                break;
            case TypeCode.UInt16:
                Write(Unsafe.As<TEnum, ushort>(ref value));
                break;
            case TypeCode.Int32:
                Write(Unsafe.As<TEnum, int>(ref value));
                break;
            case TypeCode.UInt32:
                Write(Unsafe.As<TEnum, uint>(ref value));
                break;
            case TypeCode.Int64:
                Write(Unsafe.As<TEnum, long>(ref value));
                break;
            case TypeCode.UInt64:
                Write(Unsafe.As<TEnum, ulong>(ref value));
                break;
            default:
                throw new NotSupportedException($"Enum underlying type {underlyingType} is not supported.");
        }
    }

    public void Write(string? value)
    {
        if (value is null)
        {
            // We write a length of -1 to indicate a null string, since a length of 0 can be used to represent an empty string.
            Write(-1);
            return;
        }

        int byteCount = System.Text.Encoding.UTF8.GetByteCount(value);
        int bytesLength = byteCount + 4;

        Span<byte> span = GetSpan(bytesLength);
        // First write the length of the string in bytes, then the actual UTF-8 encoded string data.
        Unsafe.WriteUnaligned(ref MemoryMarshal.GetReference(span), byteCount);
        System.Text.Encoding.UTF8.GetBytes(value, span.Slice(4));
        Advance(bytesLength);
    }

    public void Write<T>(T data)
        where T : IByteSerializable
    {
        T.WriteObject(ref this, data);
    }

    /// <summary>
    /// Advances the current writer.
    /// </summary>
    /// <param name="count">The amount to advance.</param>
    /// <remarks>Must be called after <see cref="GetSpan"/>.</remarks>
    private void Advance(int count)
    {
        byte[]? array = _array;

        ArgumentNullException.ThrowIfNull(array);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(_index, array.Length - count);

        _index += count;
    }

    /// <summary>
    /// Gets a <see cref="Span{T}"/> to write data to, with an optional minimum capacity.
    /// </summary>
    /// <param name="sizeHint">The capacity to request.</param>
    /// <returns>A <see cref="Span{T}"/> to write data to.</returns>
    [UnscopedRef]
    private Span<byte> GetSpan(int sizeHint = 0)
    {
        CheckBufferAndEnsureCapacity(sizeHint);

        return _array.AsSpan(_index);
    }

    /// <summary>
    /// Ensures that <see cref="array"/> has enough free space to contain a given number of new items.
    /// </summary>
    /// <param name="sizeHint">The minimum number of items to ensure space for in <see cref="array"/>.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void CheckBufferAndEnsureCapacity(int sizeHint)
    {
        byte[]? array = _array;

        ArgumentNullException.ThrowIfNull(array);
        ArgumentOutOfRangeException.ThrowIfNegative(sizeHint);

        if (sizeHint == 0)
        {
            sizeHint = 1;
        }

        if (sizeHint > array.Length - _index)
        {
            ResizeBuffer(sizeHint);
        }
    }

    /// <summary>
    /// Resizes <see cref="array"/> to ensure it can fit the specified number of new items.
    /// </summary>
    /// <param name="sizeHint">The minimum number of items to ensure space for in <see cref="array"/>.</param>
    [MethodImpl(MethodImplOptions.NoInlining)]
    private void ResizeBuffer(int sizeHint)
    {
        uint minimumSize = (uint)_index + (uint)sizeHint;

        // The ArrayPool<T> class has a maximum threshold of 1024 * 1024 for the maximum length of
        // pooled arrays, and once this is exceeded it will just allocate a new array every time
        // of exactly the requested size. In that case, we manually round up the requested size to
        // the nearest power of two, to ensure that repeated consecutive writes when the array in
        // use is bigger than that threshold don't end up causing a resize every single time.
        if (minimumSize > 1024 * 1024)
        {
            minimumSize = BitOperations.RoundUpToPowerOf2(minimumSize);
        }

        byte[] newArray = ArrayPool<byte>.Shared.Rent((int)minimumSize);

        Buffer.BlockCopy(_array!, 0, newArray, 0, _index);

        ArrayPool<byte>.Shared.Return(_array!);
        _array = newArray;
    }

    public readonly ReadByteStream Compress(CompressionLevel compressionLevel = CompressionLevel.Optimal)
    {
        ReadOnlySpan<byte> data = WrittenSpan;

        // Pre-allocate with reasonable estimate (GZip typically 40-50% of original size + overhead)
        using var compressedStream = new MemoryStream(data.Length / 2 + 128);
        using (var gzipStream = new GZipStream(compressedStream, compressionLevel, leaveOpen: true))
        {
            gzipStream.Write(data);
        }

        return new ReadByteStream(compressedStream.ToArray());
    }

    public readonly ReadByteStream Decompress()
    {
        ReadOnlySpan<byte> data = WrittenSpan;

        using var compressedStream = new MemoryStream(data.ToArray());
        using var decompressedStream = new MemoryStream();
        using var gzipStream = new GZipStream(compressedStream, CompressionMode.Decompress);

        gzipStream.CopyTo(decompressedStream);

        return new ReadByteStream(decompressedStream.ToArray());
    }
}
