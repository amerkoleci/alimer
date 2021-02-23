// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Vortice
{
    /// <summary>
    /// Provides various configuration switches and values for the engine.
    /// </summary>
    public static class Configuration
    {
        /// <summary><c>true</c> if engine was built with the <c>Debug</c> configuration; otherwise, <c>false</c>.</summary>
        /// <remarks>This value is not configurable via an <see cref="AppContext" /> switch.</remarks>
#if DEBUG
        public static readonly bool IsDebug = true;
#else
        public static readonly bool IsDebug = false;
#endif

        // TODO: Read from AppContext

        public static readonly bool AssertionsEnabled = IsDebug;
        public static readonly bool BreakOnFailedAssert = IsDebug;
    }
}
