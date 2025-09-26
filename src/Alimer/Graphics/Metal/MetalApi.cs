// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Alimer.Graphics.Metal;

internal static partial class MetalApi
{
    #region Constants
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct MTLDevice(nint handle) : IDisposable, IEquatable<MTLDevice>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static MTLDevice Null => new(0);
        public static implicit operator MTLDevice(nint handle) => new(handle);
        public static implicit operator nint(MTLDevice handle) => handle.Handle;

        public void Dispose() => ObjectiveC.objc_msgSend(Handle, Selectors.Release);

        public static bool operator ==(MTLDevice left, MTLDevice right) => left.Handle == right.Handle;
        public static bool operator !=(MTLDevice left, MTLDevice right) => left.Handle != right.Handle;
        public static bool operator ==(MTLDevice left, nint right) => left.Handle == right;
        public static bool operator !=(MTLDevice left, nint right) => left.Handle != right;
        public bool Equals(MTLDevice other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is MTLDevice handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(MTLDevice)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(ObjectiveC.MetalFramework)]
    public static partial MTLDevice MTLCreateSystemDefaultDevice();

    [LibraryImport(ObjectiveC.MetalFramework)]
    public static partial NSArray MTLCopyAllDevices();
}
