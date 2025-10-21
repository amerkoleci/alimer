// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Audio;
using Alimer.Graphics;

namespace Alimer;

unsafe partial class AlimerApi
{
    #region Enums
    
    #endregion

    #region Structs
    
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUFactory(nint handle) : IEquatable<GPUFactory>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUFactory Null => new(0);
        public static implicit operator GPUFactory(nint handle) => new(handle);
        public static implicit operator nint(GPUFactory handle) => handle.Handle;

        public static bool operator ==(GPUFactory left, GPUFactory right) => left.Handle == right.Handle;
        public static bool operator !=(GPUFactory left, GPUFactory right) => left.Handle != right.Handle;
        public static bool operator ==(GPUFactory left, nint right) => left.Handle == right;
        public static bool operator !=(GPUFactory left, nint right) => left.Handle != right;
        public bool Equals(GPUFactory other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUFactory handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUFactory)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuIsBackendSupport(GraphicsBackendType backendType);
}
