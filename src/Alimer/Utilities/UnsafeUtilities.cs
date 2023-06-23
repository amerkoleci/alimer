// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Utilities;

/// <summary>Provides a set of methods to supplement or replace <see cref="Unsafe" /> and <see cref="MemoryMarshal" />.</summary>
public static unsafe class UnsafeUtilities
{
    /// <inheritdoc cref="Unsafe.As{T}(object)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [return: NotNullIfNotNull(nameof(o))]
    public static T? As<T>(this object? o)
        where T : class?
    {
        Debug.Assert(o is null or T);
        return Unsafe.As<T>(o);
    }

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

    /// <inheritdoc cref="Unsafe.AsPointer{T}(ref T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T* AsPointer<T>(in T source)
        where T : unmanaged => (T*)Unsafe.AsPointer(ref Unsafe.AsRef(in source));

    /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref readonly TTo AsReadonly<TFrom, TTo>(in TFrom source)
        => ref Unsafe.As<TFrom, TTo>(ref Unsafe.AsRef(in source));

    /// <summary>Reinterprets the given native integer as a reference.</summary>
    /// <typeparam name="T">The type of the reference.</typeparam>
    /// <param name="source">The native integer to reinterpret.</param>
    /// <returns>A reference to a value of type <typeparamref name="T" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T AsRef<T>(nint source) => ref Unsafe.AsRef<T>((void*)source);

    /// <summary>Reinterprets the given native unsigned integer as a reference.</summary>
    /// <typeparam name="T">The type of the reference.</typeparam>
    /// <param name="source">The native unsigned integer to reinterpret.</param>
    /// <returns>A reference to a value of type <typeparamref name="T" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T AsRef<T>(nuint source) => ref Unsafe.AsRef<T>((void*)source);

    /// <inheritdoc cref="Unsafe.AsRef{T}(in T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T AsRef<T>(in T source) => ref Unsafe.AsRef(in source);

    /// <inheritdoc cref="Unsafe.AsRef{T}(in T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref TTo AsRef<TFrom, TTo>(in TFrom source) => ref Unsafe.As<TFrom, TTo>(ref Unsafe.AsRef(in source));

    /// <summary>Reinterprets the readonly span as a writeable span.</summary>
    /// <typeparam name="T">The type of items in <paramref name="span" /></typeparam>
    /// <param name="span">The readonly span to reinterpret.</param>
    /// <returns>A writeable span that points to the same items as <paramref name="span" />.</returns>
    public static Span<T> AsSpan<T>(this ReadOnlySpan<T> span)
        => MemoryMarshal.CreateSpan(ref Unsafe.AsRef(in MemoryMarshal.GetReference(span)), span.Length);

    /// <inheritdoc cref="MemoryMarshal.Cast{TFrom, TTo}(Span{TFrom})" />
    public static Span<TTo> Cast<TFrom, TTo>(this Span<TFrom> span)
        where TFrom : struct
        where TTo : struct => MemoryMarshal.Cast<TFrom, TTo>(span);

    /// <inheritdoc cref="MemoryMarshal.Cast{TFrom, TTo}(ReadOnlySpan{TFrom})" />
    public static ReadOnlySpan<TTo> Cast<TFrom, TTo>(this ReadOnlySpan<TFrom> span)
        where TFrom : struct
        where TTo : struct => MemoryMarshal.Cast<TFrom, TTo>(span);

    /// <inheritdoc cref="Unsafe.CopyBlock(ref byte, ref byte, uint)" />
    public static void CopyBlock<TDestination, TSource>(ref TDestination destination, in TSource source, uint byteCount) => Unsafe.CopyBlock(ref Unsafe.As<TDestination, byte>(ref destination), ref Unsafe.As<TSource, byte>(ref Unsafe.AsRef(in source)), byteCount);

    /// <inheritdoc cref="Unsafe.CopyBlockUnaligned(ref byte, ref byte, uint)" />
    public static void CopyBlockUnaligned<TDestination, TSource>(ref TDestination destination, in TSource source, uint byteCount) => Unsafe.CopyBlockUnaligned(ref Unsafe.As<TDestination, byte>(ref destination), ref Unsafe.As<TSource, byte>(ref Unsafe.AsRef(in source)), byteCount);

    /// <inheritdoc cref="MemoryMarshal.CreateSpan{T}(ref T, int)" />
    public static Span<T> CreateSpan<T>(scoped ref T reference, int length) => MemoryMarshal.CreateSpan(ref reference, length);

    /// <inheritdoc cref="MemoryMarshal.CreateReadOnlySpan{T}(ref T, int)" />
    public static ReadOnlySpan<T> CreateReadOnlySpan<T>(scoped in T reference, int length) => MemoryMarshal.CreateReadOnlySpan(ref Unsafe.AsRef(in reference), length);

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

    /// <summary>Determines if a given reference to a value of type <typeparamref name="T" /> is not a null reference.</summary>
    /// <typeparam name="T">The type of the reference</typeparam>
    /// <param name="source">The reference to check.</param>
    /// <returns><c>true</c> if <paramref name="source" /> is not a null reference; otherwise, <c>false</c>.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsNotNullRef<T>(in T source) => !Unsafe.IsNullRef(ref Unsafe.AsRef(in source));

    /// <inheritdoc cref="Unsafe.IsNullRef{T}(ref T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsNullRef<T>(in T source) => Unsafe.IsNullRef(ref Unsafe.AsRef(in source));

    /// <inheritdoc cref="Unsafe.NullRef{T}" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T NullRef<T>() => ref Unsafe.NullRef<T>();

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

#if TODO
    /// <summary>Converts the span to an unmanaged array with the same length and contents.</summary>
    /// <typeparam name="T">The type of items in the span.</typeparam>
    /// <param name="span">The span that contains the items to copy.</param>
    /// <param name="alignment">The alignment, in bytes, of the items in the array or <c>zero</c> to use the system default.</param>
    /// <returns>The allocated unmanaged array containing the items from <paramref name="span" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static UnmanagedArray<T> ToUnmanagedArray<T>(this Span<T> span, nuint alignment = 0)
        where T : unmanaged
    {
        var length = (uint)span.Length;

        if (length == 0)
        {
            return UnmanagedArray<T>.Empty;
        }

        var destination = new UnmanagedArray<T>(length, alignment);
        CopyBlock(ref destination.GetReferenceUnsafe(0), in span.GetReferenceUnsafe(), SizeOf<T>() * length);
        return destination;
    }

    /// <summary>Converts the span to an unmanaged array with the same length and contents.</summary>
    /// <typeparam name="T">The type of items in the span.</typeparam>
    /// <param name="span">The span that contains the items to copy.</param>
    /// <param name="alignment">The alignment, in bytes, of the items in the array or <c>zero</c> to use the system default.</param>
    /// <returns>The allocated unmanaged array containing the items from <paramref name="span" />.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static UnmanagedArray<T> ToUnmanagedArray<T>(this ReadOnlySpan<T> span, nuint alignment = 0)
        where T : unmanaged
    {
        var length = (uint)span.Length;

        if (length == 0)
        {
            return UnmanagedArray<T>.Empty;
        }

        var destination = new UnmanagedArray<T>(length, alignment);
        CopyBlock(ref destination.GetReferenceUnsafe(0), in span.GetReferenceUnsafe(), SizeOf<T>() * length);
        return destination;
    } 
#endif

    /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void WriteUnaligned<T>(void* source, T value)
        where T : unmanaged => Unsafe.WriteUnaligned(source, value);

    /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void WriteUnaligned<T>(void* source, nuint offset, T value)
        where T : unmanaged => Unsafe.WriteUnaligned((void*)((nuint)source + offset), value);
}
