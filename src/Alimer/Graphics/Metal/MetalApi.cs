// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using NSUInteger = System.UInt64;
using static Alimer.Platforms.Apple.ObjectiveC;
using Alimer.Platforms.Apple;

#pragma warning disable CS0649

namespace Alimer.Graphics.Metal;

internal static partial class MetalApi
{
    #region Constants
    #endregion

    #region Enums
    public enum MTLIOCompressionMethod : NSUInteger
    {
        Zlib = 0,
        LZFSE = 1,
        LZ4 = 2,
        LZMA = 3,
        LZBitmap = 4,
    }

    public enum MTLFeatureSet : NSUInteger
    {
        iOS_GPUFamily1_v1 = 0,
        iOS_GPUFamily2_v1 = 1,
        iOS_GPUFamily1_v2 = 2,
        iOS_GPUFamily2_v2 = 3,
        iOS_GPUFamily3_v1 = 4,
        iOS_GPUFamily1_v3 = 5,
        iOS_GPUFamily2_v3 = 6,
        iOS_GPUFamily3_v2 = 7,
        iOS_GPUFamily1_v4 = 8,
        iOS_GPUFamily2_v4 = 9,
        iOS_GPUFamily3_v3 = 10,
        iOS_GPUFamily4_v1 = 11,
        iOS_GPUFamily1_v5 = 12,
        iOS_GPUFamily2_v5 = 13,
        iOS_GPUFamily3_v4 = 14,
        iOS_GPUFamily4_v2 = 15,
        iOS_GPUFamily5_v1 = 16,
        macOS_GPUFamily1_v1 = 10000,
        macOS_GPUFamily1_v2 = 10001,
        macOS_ReadWriteTextureTier2 = 10002,
        macOS_GPUFamily1_v3 = 10003,
        macOS_GPUFamily1_v4 = 10004,
        macOS_GPUFamily2_v1 = 10005,
        watchOS_GPUFamily1_v1 = 20000,
        watchOS_GPUFamily2_v1 = 20001,
        tvOS_GPUFamily1_v1 = 30000,
        tvOS_GPUFamily1_v2 = 30001,
        tvOS_GPUFamily1_v3 = 30002,
        tvOS_GPUFamily2_v1 = 30003,
        tvOS_GPUFamily1_v4 = 30004,
        tvOS_GPUFamily2_v2 = 30005,
    }

    public enum MTLGPUFamily : NSUInteger
    {
        Apple1 = 1001,
        Apple2 = 1002,
        Apple3 = 1003,
        Apple4 = 1004,
        Apple5 = 1005,
        Apple6 = 1006,
        Apple7 = 1007,
        Apple8 = 1008,
        Apple9 = 1009,
        Mac1 = 2001,
        Mac2 = 2002,
        Common1 = 3001,
        Common2 = 3002,
        Common3 = 3003,
        MacCatalyst1 = 4001,
        MacCatalyst2 = 4002,
        Metal3 = 5001,
        Metal4 = 5002,
    }

    public enum MTLDeviceLocation : NSUInteger
    {
        BuiltIn = 0,
        Slot = 1,
        External = 2,
        Unspecified = NSUInteger.MaxValue,
    }
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

    #region MTLDevice
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct MTLDevice(nint handle) : IDisposable, IEquatable<MTLDevice>
    {
        #region Selectors
        private static Selector sel_supportsFamily => "supportsFamily:";
        private static Selector sel_isDepth24Stencil8PixelFormatSupported => "isDepth24Stencil8PixelFormatSupported";
        private static Selector sel_isHeadless => "isHeadless";
        private static Selector sel_isLowPower => "isLowPower";
        private static Selector sel_isRemovable => "isRemovable";
        private static Selector sel_hasUnifiedMemory => "hasUnifiedMemory";
        private static Selector sel_registryID => "registryID";
        private static Selector sel_name => "name";
        private static Selector sel_supportsRaytracing => "supportsRaytracing";
        private static Selector sel_supportsRaytracingFromRender => "supportsRaytracingFromRender";
        private static Selector sel_maxBufferLength => "maxBufferLength";
        private static Selector sel_queryTimestampFrequency => "queryTimestampFrequency";
        private static Selector s_sel_newMTL4CommandQueue => "newMTL4CommandQueue";
        private static Selector s_sel_newMTL4CommandQueueWithDescriptor => "newMTL4CommandQueueWithDescriptor:error:";
        #endregion

        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static MTLDevice Null => new(0);
        public static implicit operator MTLDevice(nint handle) => new(handle);
        public static implicit operator nint(MTLDevice handle) => handle.Handle;

        public void Dispose() => objc_msgSend(Handle, Selectors.Release);

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

        public bool SupportsFamily(MTLGPUFamily gpuFamily)
        {
            return bool_objc_msgSend(Handle, sel_supportsFamily, (NSUInteger)gpuFamily);
        }

        public bool IsDepth24Stencil8PixelFormatSupported => bool_objc_msgSend(Handle, sel_isDepth24Stencil8PixelFormatSupported);
        public bool IsHeadless => bool_objc_msgSend(Handle, sel_isHeadless);
        public bool IsLowPower => bool_objc_msgSend(Handle, sel_isLowPower);
        public bool IsRemovable => bool_objc_msgSend(Handle, sel_isRemovable);
        public bool HasUnifiedMemory => bool_objc_msgSend(Handle, sel_hasUnifiedMemory);
        public bool SupportsRaytracing => bool_objc_msgSend(Handle, sel_supportsRaytracing);
        public bool SupportsRaytracingFromRender => bool_objc_msgSend(Handle, sel_supportsRaytracingFromRender);

        public ulong RegistryId => ulong_objc_msgSend(Handle, sel_registryID);

        public NSString Name => new(IntPtr_objc_msgSend(Handle, sel_name));

        public ulong MaxBufferLength => ulong_objc_msgSend(Handle, sel_maxBufferLength);

        public ulong QueryTimestampFrequency() => ulong_objc_msgSend(Handle, sel_queryTimestampFrequency);

        public MTL4CommandQueue newMTL4CommandQueue() => objc_msgSend<MTL4CommandQueue>(Handle, s_sel_newMTL4CommandQueue);
    }
    #endregion

    #region MTL4CommandQueue
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct MTL4CommandQueue(nint handle) : IDisposable, IEquatable<MTL4CommandQueue>
    {
        #region Selectors
        #endregion

        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static MTL4CommandQueue Null => new(0);
        public static implicit operator MTL4CommandQueue(nint handle) => new(handle);
        public static implicit operator nint(MTL4CommandQueue handle) => handle.Handle;

        public void Dispose() => objc_msgSend(Handle, Selectors.Release);

        public static bool operator ==(MTL4CommandQueue left, MTL4CommandQueue right) => left.Handle == right.Handle;
        public static bool operator !=(MTL4CommandQueue left, MTL4CommandQueue right) => left.Handle != right.Handle;
        public static bool operator ==(MTL4CommandQueue left, nint right) => left.Handle == right;
        public static bool operator !=(MTL4CommandQueue left, nint right) => left.Handle != right;
        public bool Equals(MTL4CommandQueue other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is MTL4CommandQueue handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(MTL4CommandQueue)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(MetalFramework)]
    public static partial MTLDevice MTLCreateSystemDefaultDevice();

    [LibraryImport(MetalFramework)]
    public static partial NSArray MTLCopyAllDevices();
}
