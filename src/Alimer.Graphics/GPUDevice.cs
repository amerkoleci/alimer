// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;

namespace Alimer.Graphics
{
    [DebuggerDisplay("{{DebuggerDisplay,nq}}")]
    public partial struct GPUDevice : IEquatable<GPUDevice>
    {
        public readonly IntPtr Handle;
        public GPUDevice(IntPtr handle) { Handle = handle; }
        public static GPUDevice Null => new GPUDevice(IntPtr.Zero);
        public static implicit operator GPUDevice(IntPtr handle) => new GPUDevice(handle);
        public static bool operator ==(GPUDevice left, GPUDevice right) => left.Handle == right.Handle;
        public static bool operator !=(GPUDevice left, GPUDevice right) => left.Handle != right.Handle;
        public static bool operator ==(GPUDevice left, IntPtr right) => left.Handle == right;
        public static bool operator !=(GPUDevice left, IntPtr right) => left.Handle != right;
        public bool Equals(ref GPUDevice other) => Handle == other.Handle;
        public bool Equals(GPUDevice other) => Handle == other.Handle;
        public override bool Equals(object? obj) => obj is GPUDevice handle && Equals(ref handle);
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => string.Format($"{nameof(GPUDevice)} [0x{0}]", Handle.ToString("X"));
    }
}
