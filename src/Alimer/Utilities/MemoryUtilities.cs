// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Utilities;

/// <summary>Provides a set of methods for interacting with unmanaged memory.</summary>
public static unsafe class MemoryUtilities
{
    /// <summary>Allocates a chunk of unmanaged memory.</summary>
    /// <param name="count">The count of elements contained in the allocation.</param>
    /// <param name="size">The size, in bytes, of the elements in the allocation.</param>
    /// <param name="zero"><c>true</c> if the allocated memory should be zeroed; otherwise, <c>false</c>.</param>
    /// <returns>The address to an allocated chunk of memory that is at least <paramref name="size" /> bytes in length.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void* AllocateArray(nuint count, nuint size, bool zero = false)
    {
        void* result = TryAllocateArray(count, size, zero);

        if (result == null)
        {
            ThrowOutOfMemoryException(count, size);
        }
        return result;
    }

    /// <summary>Allocates a chunk of unmanaged memory.</summary>
    /// <typeparam name="T">The type used to compute the size, in bytes, of the elements in the allocation.</typeparam>
    /// <param name="count">The count of elements contained in the allocation.</param>
    /// <param name="zero"><c>true</c> if the allocated memory should be zeroed; otherwise, <c>false</c>.</param>
    /// <returns>The address to an allocated chunk of memory that is at least <c>sizeof(<typeparamref name="T" />)</c> bytes in length.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static T* AllocateArray<T>(nuint count, bool zero = false)
        where T : unmanaged
    {
        T* result = (T*)TryAllocateArray(count, (nuint)sizeof(T), zero);

        if (result == null)
        {
            ThrowOutOfMemoryException(count, SizeOf<T>());
        }
        return result;
    }

    /// <summary>Frees an allocated chunk of unmanaged memory.</summary>
    /// <param name="address">The address to an allocated chunk of memory to free</param>
    public static void Free(void* address) => NativeMemory.Free(address);

    /// <summary>Tries to allocate a chunk of unmanaged memory.</summary>
    /// <param name="count">The count of elements contained in the allocation.</param>
    /// <param name="size">The size, in bytes, of the elements in the allocation.</param>
    /// <param name="zero"><c>true</c> if the allocated memory should be zeroed; otherwise, <c>false</c>.</param>
    /// <returns>The address to an allocated chunk of memory that is at least <paramref name="size" /> bytes in length or <c>null</c> if the allocation failed.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void* TryAllocateArray(nuint count, nuint size, bool zero = false)
    {
        return zero ? NativeMemory.AllocZeroed(count, size) : NativeMemory.Alloc(count, size);
    }

    [DoesNotReturn]
    private static void ThrowOutOfMemoryException(ulong size)
    {
        throw new OutOfMemoryException($"The allocation of '{size}' bytes failed");
    }

    [DoesNotReturn]
    private static void ThrowOutOfMemoryException(ulong count, ulong size)
    {
        throw new OutOfMemoryException($"The allocation of '{count}x{size}' bytes failed");
    }
}
