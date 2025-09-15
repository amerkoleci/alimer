// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer.Utilities;

/// <summary>
/// Represents a null terminated UTF8 encoded text buffer.
/// </summary>
public readonly unsafe struct Utf8String : IEquatable<Utf8String>
{
    /// <summary>
    /// Initializes a new instance of <see cref="Utf8String"/> with a null-terminated UTF8 string.
    /// </summary>
    /// <param name="buffer">A null terminated UTF-8 string.</param>
    public Utf8String(byte* buffer)
        : this(buffer, buffer == null ? 0 : MemoryMarshal.CreateReadOnlySpanFromNullTerminated(buffer).Length)
    {
    }

    /// <summary>
    /// Initializes a new instance of <see cref="Utf8String"/> with a null-terminated UTF-8 string.
    /// </summary>
    /// <param name="buffer">A null terminated UTF8 string.</param>
    /// <param name="length">The lenght of UTF8 string</param>
    public Utf8String(byte* buffer, int length)
    {
        Buffer = buffer;
        Length = length;
    }

    /// <summary>
    /// Gets the pointer to the buffer.
    /// </summary>
    public readonly byte* Buffer { get; }

    /// <summary>
    /// Gets the number of bytes in the current <see cref="Utf8String"/>.
    /// </summary>
    public readonly int Length { get; }

    /// <summary>
    /// Gets whether this string is null.
    /// </summary>
    public bool IsNull => Buffer == null;

    /// <summary>
    /// Gets the <see cref="Utf8String"/> as a span of bytes.
    /// </summary>
    public ReadOnlySpan<byte> Span => new(Buffer, Length);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.AddBytes(Span);
        return hash.ToHashCode();
    }

    /// <inheritdoc />
    public override string? ToString() => IsNull ? null : Encoding.UTF8.GetString(Span);

    public static implicit operator Utf8String(ReadOnlySpan<byte> span) => new((byte*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(span)), span.Length);

    /// <summary>
    /// Converts a <see cref="Utf8String"/> to a <see cref="Utf8ReadOnlyString"/>.
    /// </summary>
    public static implicit operator Utf8ReadOnlyString(Utf8String memory) => memory.Span;

    /// <summary>
    /// Converts a <see cref="Utf8String"/> to a <see cref="ReadOnlySpan{byte}"/>.
    /// </summary>
    public static implicit operator ReadOnlySpan<byte>(Utf8String memory) => memory.Span;

    public static implicit operator byte*(Utf8String memory) => memory.Buffer;

    public bool Equals(Utf8String other)
    {
        return Span.SequenceEqual(other.Span);
    }

    public override bool Equals([NotNullWhen(true)] object? obj)
    {
        return obj is Utf8String other && Equals(other);
    }

    public static bool operator ==(Utf8String left, Utf8String right) => left.Equals(right);

    public static bool operator !=(Utf8String left, Utf8String right) => !left.Equals(right);

    public static bool operator ==(Utf8String left, ReadOnlySpan<byte> right) => left.Span.SequenceEqual(right);
    public static bool operator !=(Utf8String left, ReadOnlySpan<byte> right) => !left.Equals(right);
}
