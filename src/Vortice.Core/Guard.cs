// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Vortice
{
    public static class Guard
    {
        [Conditional("DEBUG")]
        public static void Assert([DoesNotReturnIf(false)] bool condition, string message)
        {
            if (!condition)
            {
                Debug.Assert(condition, message);
            }
        }

        [Conditional("DEBUG")]
        public static void Assert<T>([DoesNotReturnIf(false)] bool condition, string messageFormat, T formatArg)
        {
            if (!condition)
            {
                string message = string.Format(messageFormat, formatArg);
                Debug.Assert(condition, message);
            }
        }

        [Conditional("DEBUG")]
        public static void AssertNotNull<T>([NotNull] T? value, string paramName)
            where T : class => Assert(value != null, $"{paramName} is null.");

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfNull<T>([NotNull] T? value, string paramName)
            where T : class
        {
            if (value is null)
            {
                ThrowArgumentNullException(paramName);
            }
        }

        [DoesNotReturn]
        [MethodImpl(MethodImplOptions.NoInlining)]
        private static void ThrowArgumentNullException(string paramName)
        {
            string message = $"{paramName} is null.";
            throw new ArgumentNullException(paramName, message);
        }
    }
}
