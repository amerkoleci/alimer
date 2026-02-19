// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Utilities;

/// <summary>Provides a set of methods to supplement or replace <see cref="Unsafe" /> and <see cref="MemoryMarshal" />.</summary>
public static unsafe class UnsafeUtilities
{
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

    /// <inheritdoc cref="Unsafe.AsPointer{T}(ref T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T* AsPointer<T>(in T source)
        where T : unmanaged => (T*)Unsafe.AsPointer(ref Unsafe.AsRef(in source));

    /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref readonly TTo AsReadonly<TFrom, TTo>(in TFrom source)
        => ref Unsafe.As<TFrom, TTo>(ref Unsafe.AsRef(in source));

    /// <summary>Reinterprets the readonly span as a writeable span.</summary>
    /// <typeparam name="T">The type of items in <paramref name="span" /></typeparam>
    /// <param name="span">The readonly span to reinterpret.</param>
    /// <returns>A writeable span that points to the same items as <paramref name="span" />.</returns>
    public static Span<T> AsSpan<T>(this ReadOnlySpan<T> span)
        => MemoryMarshal.CreateSpan(ref Unsafe.AsRef(in MemoryMarshal.GetReference(span)), span.Length);

    /// <inheritdoc cref="MemoryMarshal.CreateSpan{T}(ref T, int)" />
    public static Span<T> CreateSpan<T>(scoped ref T reference, int length) => MemoryMarshal.CreateSpan(ref reference, length);

    /// <summary>Returns a pointer to the element of the span at index zero.</summary>
    /// <typeparam name="T">The type of items in <paramref name="span" />.</typeparam>
    /// <param name="span">The span from which the pointer is retrieved.</param>
    /// <returns>A pointer to the item at index zero of <paramref name="span" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T* GetPointerUnsafe<T>(this Span<T> span)
        where T : unmanaged => (T*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(span));

    /// <summary>Returns a pointer to the element of the span at index zero.</summary>
    /// <typeparam name="T">The type of items in <paramref name="span" />.</typeparam>
    /// <param name="span">The span from which the pointer is retrieved.</param>
    /// <returns>A pointer to the item at index zero of <paramref name="span" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T* GetPointerUnsafe<T>(this ReadOnlySpan<T> span)
        where T : unmanaged => (T*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(span));

    /// <inheritdoc cref="MemoryMarshal.GetArrayDataReference{T}(T[])" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this T[] array) => ref MemoryMarshal.GetArrayDataReference(array);

    /// <inheritdoc cref="MemoryMarshal.GetArrayDataReference{T}(T[])" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this T[] array, int index) => ref Unsafe.Add(ref MemoryMarshal.GetArrayDataReference(array), index);

    /// <inheritdoc cref="MemoryMarshal.GetArrayDataReference{T}(T[])" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this T[] array, nuint index) => ref Unsafe.Add(ref MemoryMarshal.GetArrayDataReference(array), index);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(Span{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this Span<T> span) => ref MemoryMarshal.GetReference(span);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(Span{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this Span<T> span, int index) => ref Unsafe.Add(ref MemoryMarshal.GetReference(span), index);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(Span{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T GetReferenceUnsafe<T>(this Span<T> span, nuint index) => ref Unsafe.Add(ref MemoryMarshal.GetReference(span), index);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(ReadOnlySpan{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref readonly T GetReferenceUnsafe<T>(this ReadOnlySpan<T> span) => ref MemoryMarshal.GetReference(span);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(ReadOnlySpan{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref readonly T GetReferenceUnsafe<T>(this ReadOnlySpan<T> span, int index) => ref Unsafe.Add(ref MemoryMarshal.GetReference(span), index);

    /// <inheritdoc cref="MemoryMarshal.GetReference{T}(ReadOnlySpan{T})" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref readonly T GetReferenceUnsafe<T>(this ReadOnlySpan<T> span, nuint index) => ref Unsafe.Add(ref MemoryMarshal.GetReference(span), index);

    /// <inheritdoc cref="Unsafe.ReadUnaligned{T}(void*)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T ReadUnaligned<T>(void* source)
        where T : unmanaged => Unsafe.ReadUnaligned<T>(source);

    /// <inheritdoc cref="Unsafe.ReadUnaligned{T}(void*)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T ReadUnaligned<T>(void* source, nuint offset)
        where T : unmanaged => Unsafe.ReadUnaligned<T>((void*)((nuint)source + offset));

#pragma warning disable CS8500
    /// <inheritdoc cref="Unsafe.SizeOf{T}" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint SizeOf<T>() => unchecked((uint)sizeof(T));
#pragma warning restore CS8500

    /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void WriteUnaligned<T>(void* source, T value)
        where T : unmanaged => Unsafe.WriteUnaligned(source, value);

    /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void WriteUnaligned<T>(void* source, nuint offset, T value)
        where T : unmanaged => Unsafe.WriteUnaligned((void*)((nuint)source + offset), value);
}
