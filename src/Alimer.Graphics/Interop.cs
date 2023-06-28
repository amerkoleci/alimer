// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

internal unsafe static class Interop
{

#pragma warning disable CS8500
    /// <inheritdoc cref="Unsafe.SizeOf{T}" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint SizeOf<T>() => unchecked((uint)sizeof(T));
#pragma warning restore CS8500

    /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref TTo As<TFrom, TTo>(ref TFrom source)
        => ref Unsafe.As<TFrom, TTo>(ref source);

    /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Span<TTo> As<TFrom, TTo>(this Span<TFrom> span)
        where TFrom : unmanaged
        where TTo : unmanaged
    {
        Debug.Assert(SizeOf<TFrom>() == SizeOf<TTo>());
        return MemoryMarshal.CreateSpan(ref Unsafe.As<TFrom, TTo>(ref MemoryMarshal.GetReference(span)), span.Length);
    }

    /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ReadOnlySpan<TTo> As<TFrom, TTo>(this ReadOnlySpan<TFrom> span)
        where TFrom : unmanaged
        where TTo : unmanaged
    {
        Debug.Assert(SizeOf<TFrom>() == SizeOf<TTo>());
        return MemoryMarshal.CreateReadOnlySpan(ref Unsafe.As<TFrom, TTo>(ref MemoryMarshal.GetReference(span)), span.Length);

    }

    /// <inheritdoc cref="Unsafe.IsNullRef{T}(ref T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsNullRef<T>(in T source) => Unsafe.IsNullRef(ref Unsafe.AsRef(in source));

    /// <inheritdoc cref="MemoryMarshal.CreateSpan{T}(ref T, int)" />
    public static Span<T> CreateSpan<T>(scoped ref T reference, int length) => MemoryMarshal.CreateSpan(ref reference, length);

    /// <inheritdoc cref="MemoryMarshal.CreateReadOnlySpan{T}(ref T, int)" />
    public static ReadOnlySpan<T> CreateReadOnlySpan<T>(scoped in T reference, int length) => MemoryMarshal.CreateReadOnlySpan(ref Unsafe.AsRef(in reference), length);

    /// <summary>Gets a null-terminated sequence of UTF8 characters for a string.</summary>
    /// <param name="source">The string for which to get the null-terminated UTF8 character sequence.</param>
    /// <returns>A null-terminated UTF8 character sequence created from <paramref name="source" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ReadOnlySpan<sbyte> GetUtf8Span(this string? source)
    {
        ReadOnlySpan<byte> result;

        if (source is not null)
        {
            int maxLength = Encoding.UTF8.GetMaxByteCount(source.Length);
            var bytes = new byte[maxLength + 1];

            int length = Encoding.UTF8.GetBytes(source, bytes);
            result = bytes.AsSpan(0, length);
        }
        else
        {
            result = null;
        }

        return result.As<byte, sbyte>();
    }

    /// <summary>Gets a span for a null-terminated UTF8 character sequence.</summary>
    /// <param name="source">The pointer to a null-terminated UTF8 character sequence.</param>
    /// <param name="maxLength">The maximum length of <paramref name="source" /> or <c>-1</c> if the maximum length is unknown.</param>
    /// <returns>A span that starts at <paramref name="source" /> and extends to <paramref name="maxLength" /> or the first null character, whichever comes first.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ReadOnlySpan<sbyte> GetUtf8Span(sbyte* source, int maxLength = -1)
        => (source != null) ? GetUtf8Span(in source[0], maxLength) : null;

    /// <summary>Gets a span for a null-terminated UTF8 character sequence.</summary>
    /// <param name="source">The reference to a null-terminated UTF8 character sequence.</param>
    /// <param name="maxLength">The maximum length of <paramref name="source" /> or <c>-1</c> if the maximum length is unknown.</param>
    /// <returns>A span that starts at <paramref name="source" /> and extends to <paramref name="maxLength" /> or the first null character, whichever comes first.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ReadOnlySpan<sbyte> GetUtf8Span(in sbyte source, int maxLength = -1)
    {
        ReadOnlySpan<sbyte> result;

        if (!IsNullRef(in source))
        {
            if (maxLength < 0)
            {
                maxLength = int.MaxValue;
            }

            result = CreateReadOnlySpan(in source, maxLength);
            var length = result.IndexOf((sbyte)'\0');

            if (length >= 0)
            {
                result = result.Slice(0, length);
            }
        }
        else
        {
            result = null;
        }

        return result;
    }

}
