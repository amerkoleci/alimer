// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Vortice.Graphics
{
    internal static unsafe class Utilities
    {
        /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ref TTo As<TFrom, TTo>(ref TFrom source) => ref Unsafe.As<TFrom, TTo>(ref source);

        /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Span<TTo> As<TFrom, TTo>(this Span<TFrom> span)
            where TFrom : unmanaged
            where TTo : unmanaged
        {
            //Assert(AssertionsEnabled && (SizeOf<TFrom>() == SizeOf<TTo>()));
            return CreateSpan(ref As<TFrom, TTo>(ref span.GetReference()), span.Length);
        }

        /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ReadOnlySpan<TTo> As<TFrom, TTo>(this ReadOnlySpan<TFrom> span)
            where TFrom : unmanaged
            where TTo : unmanaged
        {
            //Assert(AssertionsEnabled && (SizeOf<TFrom>() == SizeOf<TTo>()));
            return CreateReadOnlySpan(in AsReadonly<TFrom, TTo>(in span.GetReference()), span.Length);
        }

        /// <inheritdoc cref="Unsafe.AsPointer{T}(ref T)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T* AsPointer<T>(ref T source)
            where T : unmanaged => (T*)Unsafe.AsPointer(ref source);

        /// <inheritdoc cref="Unsafe.As{TFrom, TTo}(ref TFrom)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ref readonly TTo AsReadonly<TFrom, TTo>(in TFrom source)
            => ref Unsafe.As<TFrom, TTo>(ref AsRef(in source));

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

        /// <inheritdoc cref="Unsafe.AsRef{T}(void*)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ref T AsRef<T>(void* source) => ref Unsafe.AsRef<T>(source);

        /// <summary>Reinterprets the readonly span as a writeable span.</summary>
        /// <typeparam name="T">The type of items in <paramref name="span" /></typeparam>
        /// <param name="span">The readonly span to reinterpret.</param>
        /// <returns>A writeable span that points to the same items as <paramref name="span" />.</returns>
        public static Span<T> AsSpan<T>(this ReadOnlySpan<T> span)
            => MemoryMarshal.CreateSpan(ref AsRef(in span.GetReference()), span.Length);

        /// <inheritdoc cref="MemoryMarshal.Cast{TFrom, TTo}(Span{TFrom})" />
        public static Span<TTo> Cast<TFrom, TTo>(this Span<TFrom> span)
            where TFrom : struct
            where TTo : struct
        {
            return MemoryMarshal.Cast<TFrom, TTo>(span);
        }

        /// <inheritdoc cref="MemoryMarshal.Cast{TFrom, TTo}(ReadOnlySpan{TFrom})" />
        public static ReadOnlySpan<TTo> Cast<TFrom, TTo>(this ReadOnlySpan<TFrom> span)
            where TFrom : struct
            where TTo : struct
        {
            return MemoryMarshal.Cast<TFrom, TTo>(span);
        }

        /// <inheritdoc cref="MemoryMarshal.CreateSpan{T}(ref T, int)" />
        public static Span<T> CreateSpan<T>(ref T reference, int length) => MemoryMarshal.CreateSpan(ref reference, length);

        /// <inheritdoc cref="MemoryMarshal.CreateReadOnlySpan{T}(ref T, int)" />
        public static ReadOnlySpan<T> CreateReadOnlySpan<T>(in T reference, int length) => MemoryMarshal.CreateReadOnlySpan(ref AsRef(in reference), length);

        /// <summary>Returns a pointer to the element of the span at index zero.</summary>
        /// <typeparam name="T">The type of items in <paramref name="span" />.</typeparam>
        /// <param name="span">The span from which the pointer is retrieved.</param>
        /// <returns>A pointer to the item at index zero of <paramref name="span" />.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T* GetPointer<T>(this Span<T> span)
            where T : unmanaged => AsPointer(ref span.GetReference());

        /// <summary>Returns a pointer to the element of the span at index zero.</summary>
        /// <typeparam name="T">The type of items in <paramref name="span" />.</typeparam>
        /// <param name="span">The span from which the pointer is retrieved.</param>
        /// <returns>A pointer to the item at index zero of <paramref name="span" />.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T* GetPointer<T>(this ReadOnlySpan<T> span)
            where T : unmanaged => AsPointer(ref AsRef(in span.GetReference()));

        /// <inheritdoc cref="MemoryMarshal.GetReference{T}(Span{T})" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ref T GetReference<T>(this Span<T> span) => ref MemoryMarshal.GetReference(span);

        /// <inheritdoc cref="MemoryMarshal.GetReference{T}(ReadOnlySpan{T})" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ref readonly T GetReference<T>(this ReadOnlySpan<T> span) => ref MemoryMarshal.GetReference(span);

        /// <inheritdoc cref="Unsafe.IsNullRef{T}(ref T)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool IsNullRef<T>(in T source) => Unsafe.IsNullRef(ref AsRef(in source));

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

        /// <inheritdoc cref="Unsafe.SizeOf{T}" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint SizeOf<T>() => unchecked((uint)Unsafe.SizeOf<T>());

        /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void WriteUnaligned<T>(void* source, T value)
            where T : unmanaged => Unsafe.WriteUnaligned<T>(source, value);

        /// <inheritdoc cref="Unsafe.WriteUnaligned{T}(void*, T)" />
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void WriteUnaligned<T>(void* source, nuint offset, T value)
            where T : unmanaged => Unsafe.WriteUnaligned((void*)((nuint)source + offset), value);

        /// <summary>
        /// Gets a string for a given span.
        /// </summary>
        /// <param name="span">The span for which to create the string.</param>
        /// <returns>A string created from <paramref name="span" />.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static string? GetString(this ReadOnlySpan<sbyte> span)
        {
            string? result;

            if (span.GetPointer() != null)
            {
                result = Encoding.UTF8.GetString(span.As<sbyte, byte>());
            }
            else
            {
                result = null;
            }

            return result;
        }

        /// <summary>Gets a string for a given span.</summary>
        /// <param name="span">The span for which to create the string.</param>
        /// <returns>A string created from <paramref name="span" />.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static string? GetString(this ReadOnlySpan<ushort> span)
        {
            string? result;

            if (span.GetPointer() != null)
            {
                result = new string(span.As<ushort, char>());
            }
            else
            {
                result = null;
            }

            return result;
        }

        /// <summary>
        /// Marshals a null-terminated UTF16 string to a <see cref="ReadOnlySpan{UInt16}" />.
        /// </summary>
        /// <param name="source">The reference to a null-terminated UTF16 string.</param>
        /// <param name="maxLength">The maxmimum length of <paramref name="source" /> or <c>-1</c> if the maximum length is unknown.</param>
        /// <returns>A <see cref="ReadOnlySpan{UInt16}" /> that starts at <paramref name="source" /> and extends to <paramref name="maxLength" /> or the first null character, whichever comes first.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ReadOnlySpan<ushort> GetUtf16Span(in ushort source, int maxLength = -1)
        {
            ReadOnlySpan<ushort> result;

            if (!IsNullRef(in source))
            {
                if (maxLength < 0)
                {
                    maxLength = int.MaxValue;
                }

                result = CreateReadOnlySpan(in source, maxLength);
                int length = result.IndexOf('\0');

                if (length != -1)
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
}
