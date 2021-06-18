// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Vortice
{
    public static class Guard
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Assert([DoesNotReturnIf(false)] bool condition)
        {
            if (!condition)
            {
                Fail();
            }
        }

        /// <summary>Asserts that <paramref name="value" /> is not <c>null</c>.</summary>
        /// <typeparam name="T">The type of <paramref name="value" />.</typeparam>
        /// <param name="value">The value to assert is not <c>null</c>.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void AssertNotNull<T>([NotNull] T? value) where T : class => Assert(value is not null);

        /// <summary>Throws an <see cref="Exception" />.</summary>
        [DoesNotReturn]
        public static void Fail()
        {
            if (Debugger.IsAttached)
            {
                Debugger.Break();
            }

            throw new Exception();
        }
    }
}
