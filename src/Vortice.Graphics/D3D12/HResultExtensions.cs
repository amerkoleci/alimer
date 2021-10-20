// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using HRESULT = System.Int32;

namespace Vortice.Graphics.D3D12
{
    /// <summary>
    /// Helper methods to efficiently throw exceptions.
    /// </summary>
    [DebuggerStepThrough]
    internal static class HResultExtensions
    {
        /// <summary>
        /// Throws a <see cref="Win32Exception"/> if <paramref name="result"/> represents an error.
        /// </summary>
        /// <param name="result">The input <see cref="HRESULT"/> to check.</param>
        /// <exception cref="Win32Exception">Thrown if <paramref name="result"/> represents an error.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Assert(this HRESULT result)
        {
#if DEBUG && TODO
            bool hasErrorsOrWarnings = DeviceHelper.FlushAllID3D12InfoQueueMessagesAndCheckForErrorsOrWarnings();

            if (result < 0)
            {
                ThrowHelper.ThrowWin32Exception(result);
            }

            if (hasErrorsOrWarnings)
            {
                ThrowHelper.ThrowWin32Exception("Warning or error detected by ID3D12InfoQueue");
            }
#else
            if (result < 0)
            {
                ThrowHelper.ThrowWin32Exception(result);
            }
#endif
        }
    }
}
