// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics.CodeAnalysis;

namespace Vortice.Graphics
{
    internal static class ThrowHelper
    {
        /// <summary>
        /// Throws a new <see cref="ArgumentException"/>.
        /// </summary>
        /// <typeparam name="T">The type of expected result.</typeparam>
        /// <exception cref="ArgumentException">Thrown with no parameters.</exception>
        /// <returns>This method always throws, so it actually never returns a value.</returns>
        [DoesNotReturn]
        public static T ThrowArgumentException<T>()
        {
            throw new ArgumentException();
        }

        /// <summary>
        /// Throws a new <see cref="ArgumentException"/>.
        /// </summary>
        /// <typeparam name="T">The type of expected result.</typeparam>
        /// <param name="message">The message to include in the exception.</param>
        /// <exception cref="ArgumentException">Thrown with the specified parameter.</exception>
        /// <returns>This method always throws, so it actually never returns a value.</returns>
        [DoesNotReturn]
        public static T ThrowArgumentException<T>(string? message)
        {
            throw new ArgumentException(message);
        }
    }
}
