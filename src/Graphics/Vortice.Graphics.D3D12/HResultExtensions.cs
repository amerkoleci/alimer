// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop.Windows;

namespace Vortice.Graphics;

/// <summary>
/// Helper methods to efficiently throw exceptions.
/// </summary>
[DebuggerStepThrough]
internal static class HResultExtensions
{
    /// <summary>
    /// Throws a <see cref="Win32Exception"/> if <paramref name="hr"/> represents an error.
    /// </summary>
    /// <param name="hr">The input <see cref="HRESULT"/> to check.</param>
    /// <exception cref="Win32Exception">Thrown if <paramref name="hr"/> represents an error.</exception>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Assert(this HRESULT hr)
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
        if (hr.FAILED)
        {
            ThrowHelper.ThrowWin32Exception(hr);
        }
#endif
    }
}
