// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer.Utilities;

public readonly ref struct Utf8ReadOnlyString(ReadOnlySpan<byte> span)
{
    private readonly ReadOnlySpan<byte> _span = span;

    /// <inheritdoc />
    public override string? ToString() => Unsafe.IsNullRef(ref MemoryMarshal.GetReference(_span)) ? null : Encoding.UTF8.GetString(_span);

    public static implicit operator Utf8ReadOnlyString(ReadOnlySpan<byte> span) => new(span);
    public static implicit operator Utf8ReadOnlyString(byte[] data) => new(data);

    public static implicit operator ReadOnlySpan<byte>(Utf8ReadOnlyString span) => span._span;

    public static unsafe implicit operator byte*(Utf8ReadOnlyString span) => (byte*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(span._span));

    internal ref readonly byte GetPinnableReference() => ref _span.GetPinnableReference();

    public static bool operator ==(Utf8ReadOnlyString left, Utf8ReadOnlyString right) => left._span.SequenceEqual(right._span);

    public static bool operator !=(Utf8ReadOnlyString left, Utf8ReadOnlyString right) => !left._span.SequenceEqual(right._span);
}
