// https://github.com/Sergio0694/ComputeSharp/blob/main/LICENSE

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop;

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
        public static void Assert(this int result)
        {
            Assert(new HRESULT(result));
        }

        /// <summary>
        /// Throws a <see cref="Win32Exception"/> if <paramref name="result"/> represents an error.
        /// </summary>
        /// <param name="result">The input <see cref="HRESULT"/> to check.</param>
        /// <exception cref="Win32Exception">Thrown if <paramref name="result"/> represents an error.</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Assert(this HRESULT result)
        {
#if DEBUG && TODO
            bool hasErrorsOrWarnings = D3D12DeviceHelper.FlushAllID3D12InfoQueueMessagesAndCheckForErrorsOrWarnings();

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
