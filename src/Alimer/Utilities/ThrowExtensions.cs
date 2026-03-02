// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.
// Copyright (c) Amer Koleci and Contributors.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer;

public static class ThrowExtensions
{
    extension(ArgumentException)
    {
        /// <summary>
        /// Asserts that the input value must be <see langword="true"/>.
        /// </summary>
        /// <param name="value">The input <see cref="bool"/> to test.</param>
        /// <param name="name">The name of the input parameter being tested.</param>
        /// <exception cref="ArgumentException">Thrown if <paramref name="value"/> is <see langword="false"/>.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfFalse([DoesNotReturnIf(false)] bool value, [CallerArgumentExpression(nameof(value))] string name = "")
        {
            if (value)
            {
                return;
            }

            ThrowHelper.ThrowArgumentExceptionForIsTrue(name);
        }


        /// <summary>
        /// Asserts that the input value must be <see langword="true"/>.
        /// </summary>
        /// <param name="value">The input <see cref="bool"/> to test.</param>
        /// <param name="name">The name of the input parameter being tested.</param>
        /// <param name="message">A message to display if <paramref name="value"/> is <see langword="false"/>.</param>
        /// <exception cref="ArgumentException">Thrown if <paramref name="value"/> is <see langword="false"/>.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfFalse([DoesNotReturnIf(false)] bool value, string name, string message)
        {
            if (value)
            {
                return;
            }

            ThrowHelper.ThrowArgumentExceptionForIsTrue(name, message);
        }

        /// <summary>
        /// Asserts that the input value must be <see langword="false"/>.
        /// </summary>
        /// <param name="value">The input <see cref="bool"/> to test.</param>
        /// <param name="name">The name of the input parameter being tested.</param>
        /// <exception cref="ArgumentException">Thrown if <paramref name="value"/> is <see langword="true"/>.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfTrue([DoesNotReturnIf(true)] bool value, [CallerArgumentExpression(nameof(value))] string name = "")
        {
            if (!value)
            {
                return;
            }

            ThrowHelper.ThrowArgumentExceptionForIsFalse(name);
        }

        /// <summary>
        /// Asserts that the input value must be <see langword="false"/>.
        /// </summary>
        /// <param name="value">The input <see cref="bool"/> to test.</param>
        /// <param name="name">The name of the input parameter being tested.</param>
        /// <param name="message">A message to display if <paramref name="value"/> is <see langword="true"/>.</param>
        /// <exception cref="ArgumentException">Thrown if <paramref name="value"/> is <see langword="true"/>.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfTrue([DoesNotReturnIf(true)] bool value, string name, string message)
        {
            if (!value)
            {
                return;
            }

            ThrowHelper.ThrowArgumentExceptionForIsFalse(name, message);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void ThrowIfEmpty<T>(Span<T> span, [CallerArgumentExpression(nameof(span))] string name = "")
        {
            if (span.Length != 0)
            {
                return;
            }

            ThrowHelper.ThrowArgumentExceptionForIsNotEmptyWithSpan<T>(name);
        }
    }

    [StackTraceHidden]
    private static partial class ThrowHelper
    {
        /// <summary>
        /// Returns a formatted representation of the input value.
        /// </summary>
        /// <param name="obj">The input <see cref="object"/> to format.</param>
        /// <returns>A formatted representation of <paramref name="obj"/> to display in error messages.</returns>
        private static string AssertString(object? obj)
        {
            return obj switch
            {
                string _ => $"\"{obj}\"",
                null => "null",
                _ => $"<{obj}>"
            };
        }


        /// <summary>
        /// Throws an <see cref="ArgumentException"/> when <see cref="IsTrue(bool,string)"/> fails.
        /// </summary>
        [DoesNotReturn]
        public static void ThrowArgumentExceptionForIsTrue(string name) =>
            throw new ArgumentException($"Parameter {AssertString(name)} must be true, was false.", name);

        /// <summary>
        /// Throws an <see cref="ArgumentException"/> when <see cref="IsTrue(bool,string,string)"/> fails.
        /// </summary>
        [DoesNotReturn]
        public static void ThrowArgumentExceptionForIsTrue(string name, string message) =>
            throw new ArgumentException($"Parameter {AssertString(name)} must be true, was false: {AssertString(message)}.", name);

        /// <summary>
        /// Throws an <see cref="ArgumentException"/> when <see cref="IsFalse(bool,string)"/> fails.
        /// </summary>
        [DoesNotReturn]
        public static void ThrowArgumentExceptionForIsFalse(string name) =>
            throw new ArgumentException($"Parameter {AssertString(name)} must be false, was true.", name);

        /// <summary>
        /// Throws an <see cref="ArgumentException"/> when <see cref="IsFalse(bool,string,string)"/> fails.
        /// </summary>
        [DoesNotReturn]
        public static void ThrowArgumentExceptionForIsFalse(string name, string message) =>
            throw new ArgumentException($"Parameter {AssertString(name)} must be false, was true: {AssertString(message)}.", name);

        /// <summary>
        /// Throws an <see cref="ArgumentException"/> when <see cref="IsNotEmpty{T}(Span{T},string)"/> fails.
        /// </summary>
        /// <typeparam name="T">The item of items in the input <see cref="Span{T}"/> instance.</typeparam>
        /// <remarks>This method is needed because <see cref="Span{T}"/> can't be used as a generic type parameter.</remarks>
        [DoesNotReturn]
        public static void ThrowArgumentExceptionForIsNotEmptyWithSpan<T>(string name) =>
            throw new ArgumentException($"Parameter {AssertString(name)} ({typeof(Span<T>).Name}) must not be empty.", name);
    }
}
