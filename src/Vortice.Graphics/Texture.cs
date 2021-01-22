// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;

namespace Vortice.Graphics
{
    [DebuggerDisplay("{{DebuggerDisplay,nq}}")]
    public partial struct GPUTexture : IEquatable<GPUTexture>
    {
        public readonly IntPtr Handle;
        public GPUTexture(IntPtr handle) { Handle = handle; }
        public static GPUTexture Null => new GPUTexture(IntPtr.Zero);
        public static implicit operator GPUTexture(IntPtr handle) => new GPUTexture(handle);
        public static bool operator ==(GPUTexture left, GPUTexture right) => left.Handle == right.Handle;
        public static bool operator !=(GPUTexture left, GPUTexture right) => left.Handle != right.Handle;
        public static bool operator ==(GPUTexture left, IntPtr right) => left.Handle == right;
        public static bool operator !=(GPUTexture left, IntPtr right) => left.Handle != right;
        public bool Equals(ref GPUTexture other) => Handle == other.Handle;
        public bool Equals(GPUTexture other) => Handle == other.Handle;
        public override bool Equals(object? obj) => obj is GPUTexture handle && Equals(ref handle);
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => string.Format($"{nameof(GPUTexture)} [0x{0}]", Handle.ToString("X"));
    }
}
