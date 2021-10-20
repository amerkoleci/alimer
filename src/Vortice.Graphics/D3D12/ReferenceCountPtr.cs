// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if ENABLE_D3D12MA
using System;
using System.ComponentModel;
using System.Diagnostics.Contracts;
using System.Runtime.CompilerServices;
using System.Threading;
using TerraFX.Interop;

namespace Vortice.Graphics.D3D12
{
    /// <summary>
    /// A <see cref="ComPtr{T}"/>-equivalent type to safely work with pointers to public <see cref="D3D12MemAlloc"/> APIs.
    /// This special type internally keeps track of a reference count for the wrapped native object.
    /// This works around the lack of internal reference counting for objects from <see cref="D3D12MemAlloc"/>.
    /// </summary>
    /// <typeparam name="T">Must be either <see cref="D3D12MA_Allocator"/>, <see cref="D3D12MA_Allocation"/>, <see cref="D3D12MA_Pool"/> or <see cref="D3D12MA_VirtualBlock"/>.</typeparam>
    /// <remarks>
    /// This type is not marked as <see langword="ref"/> so that it can also be used in fields, so make sure to
    /// avoid copying it and disposing it multiple times, as that would lead to a access violation exceptions.
    /// </remarks>
    internal unsafe struct ReferenceCountPtr<T> : IDisposable where T : unmanaged
    {
        /// <summary>
        /// The raw pointer to an <typeparamref name="T"/> object, if existing.
        /// </summary>
        private T* _pointer;

        /// <summary>
        /// The current reference count - 1 for <see cref="pointer"/>.
        /// </summary>
        private volatile int _count;

        /// <summary>
        /// Creates a new <see cref="ReferenceCountPtr{T}"/> instance from a raw pointer.
        /// </summary>
        /// <param name="other">The raw pointer to wrap.</param>
        public ReferenceCountPtr(T* other)
        {
            _pointer = other;
            _count = 0;
        }

        /// <summary>
        /// Unwraps a <see cref="ReferenceCountPtr{T}"/> instance and returns the internal raw pointer.
        /// </summary>
        /// <param name="other">The <see cref="ReferenceCountPtr{T}"/> instance to unwrap.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static implicit operator T*(ReferenceCountPtr<T> other)
        {
            return other.Get();
        }

        /// <summary>
        /// Increments the internal reference count.
        /// </summary>
        public void AddRef()
        {
            Interlocked.Increment(ref _count);
        }

        /// <inheritdoc/>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Dispose()
        {
            if (Interlocked.Decrement(ref _count) < 0)
            {
                T* pointer = _pointer;

                if (pointer != null)
                {
                    _pointer = null;

                    if (typeof(T) == typeof(D3D12MA_Allocator)) ((D3D12MA_Allocator*)pointer)->Release();
                    else if (typeof(T) == typeof(D3D12MA_Allocation)) ((D3D12MA_Allocation*)pointer)->Release();
                    else if (typeof(T) == typeof(D3D12MA_Pool)) ((D3D12MA_Pool*)pointer)->Release();
                    else if (typeof(T) == typeof(D3D12MA_VirtualBlock)) ((D3D12MA_VirtualBlock*)pointer)->Release();
                    else throw new ArgumentException("Invalid pointer type");
                }
            }
        }

        /// <summary>
        /// Gets the currently wrapped raw pointer to an <typeparamref name="T"/> object.
        /// </summary>
        /// <returns>The raw pointer wrapped by the current <typeparamref name="T"/> instance.</returns>
        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly T* Get() => _pointer;

        /// <summary>
        /// Gets the address of the current <see cref="ReferenceCountPtr{T}"/> instance as a raw <typeparamref name="T"/> double pointer.
        /// </summary>
        /// <returns>The raw pointer to the current <see cref="ReferenceCountPtr{T}"/> instance.</returns>
        /// <remarks>This method is only valid when the current <see cref="ReferenceCountPtr{T}"/> instance is on the stack or pinned.</remarks>
        [Pure]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly T** GetAddressOf()
        {
            return (T**)Unsafe.AsPointer(ref Unsafe.AsRef(in this));
        }

        /// <summary>
        /// Gets the address of the current <see cref="ReferenceCountPtr{T}"/> instance as a managed <typeparamref name="T"/> reference to pointer.
        /// </summary>
        /// <returns>The reference to the current <see cref="ReferenceCountPtr{T}"/> instance.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public readonly ref T* GetPinnableReference()
        {
            fixed (T** ptr = &_pointer)
            {
                return ref *ptr;
            }
        }

        /// <summary>
        /// Moves the current <see cref="ReferenceCountPtr{T}"/> instance and resets it without releasing the reference.
        /// </summary>
        /// <returns>The moved <see cref="ReferenceCountPtr{T}"/> instance.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public ReferenceCountPtr<T> Move()
        {
            ReferenceCountPtr<T> copy = this;

            this = default;

            return copy;
        }
    }
}
#endif // ENABLE_D3D12MA 
