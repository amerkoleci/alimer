// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.Windows;

namespace Alimer.Shaders;

internal static class ComPtrExtensions
{
    /// <summary>
    /// Moves the current <see cref="ComPtr{T}"/> instance and resets it without releasing the reference.
    /// </summary>
    /// <typeparam name="T">The type to wrap in the current <see cref="ComPtr{T}"/> instance.</typeparam>
    /// <param name="ptr">The input <see cref="ComPtr{T}"/> instance to move.</param>
    /// <returns>The moved <see cref="ComPtr{T}"/> instance.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static unsafe ComPtr<T> Move<T>(this in ComPtr<T> ptr)
        where T : unmanaged
    {
        ComPtr<T> copy = default;

        Unsafe.AsRef(in ptr).Swap(ref copy);

        return copy;
    }
}
