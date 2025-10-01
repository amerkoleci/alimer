// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using NSUInteger = System.UInt64;
namespace Alimer.Graphics.Metal;

internal static partial class MetalApi
{
    #region Constants
    #endregion

    #region Types
    public struct MTLOrigin(NSUInteger x, NSUInteger y, NSUInteger z)
    {
        public NSUInteger x = x;
        public NSUInteger y = y;
        public NSUInteger z = z;
    }

    public struct MTLSize(NSUInteger width, NSUInteger height, NSUInteger depth)
    {
        public NSUInteger width = width;
        public NSUInteger height = height;
        public NSUInteger depth = depth;
    }

    public struct MTLRegion(MTLOrigin origin, MTLSize size)
    {
        public MTLOrigin origin = origin;
        public MTLSize size = size;

        public static MTLRegion Make1D(NSUInteger x, NSUInteger width)
        {
            return new MTLRegion(new MTLOrigin(x, 0, 0), new MTLSize(width, 1, 1));
        }

        public static MTLRegion Make2D(NSUInteger x, NSUInteger y, NSUInteger width, NSUInteger height)
        {
            return new MTLRegion(new MTLOrigin(x, y, 0), new MTLSize(width, height, 1));
        }

        public static MTLRegion Make3D(NSUInteger x, NSUInteger y, NSUInteger z, NSUInteger width, NSUInteger height, NSUInteger depth)
        {
            return new MTLRegion(new MTLOrigin(x, y, z), new MTLSize(width, height, depth));
        }
    }

    public struct MTLResourceID
    {
        public ulong _impl;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MTLSamplePosition(float x, float y)
    {
        public float x = x;
        public float y = y;
    }
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct MTLDevice(nint handle) : IDisposable, IEquatable<MTLDevice>
    {
        #region Selectors
        private static readonly Selector s_sel_isDepth24Stencil8PixelFormatSupported = "isDepth24Stencil8PixelFormatSupported";
        #endregion

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

        public bool Depth24Stencil8PixelFormatSupported => ObjectiveC.bool_objc_msgSend(Handle, s_sel_isDepth24Stencil8PixelFormatSupported);

    }
    #endregion

    [LibraryImport(ObjectiveC.MetalFramework)]
    public static partial MTLDevice MTLCreateSystemDefaultDevice();

    [LibraryImport(ObjectiveC.MetalFramework)]
    public static partial NSArray MTLCopyAllDevices();
}
