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
    public enum GPUPowerPreference
    {
        Undefined = 0,
        LowPower = 1,
        HighPerformance = 2,
    }
    #endregion

    #region Structs
    public struct GPUFactoryDesc
    {
        public GraphicsBackendType preferredBackend;
        public GraphicsValidationMode validationMode;
    }

    public struct GPURequestAdapterOptions
    {
        public GPUSurface compatibleSurface;
        public GPUPowerPreference powerPreference;
    }

    public struct GPUAdapterInfo
    {
        public byte* deviceName;
        public fixed ushort driverVersion[4];
        public byte* driverDescription;
        public GraphicsAdapterType adapterType;
        public /*GPUAdapterVendor*/uint vendor;
        public uint vendorID;
        public uint deviceID;
    }

    public struct GPULimits
    {
        public uint maxTextureDimension1D;
        public uint maxTextureDimension2D;
        public uint maxTextureDimension3D;
        public uint maxTextureDimensionCube;
        public uint maxTextureArrayLayers;
        public uint maxBindGroups;
        public uint maxConstantBufferBindingSize;
        public uint maxStorageBufferBindingSize;
        public uint minConstantBufferOffsetAlignment;
        public uint minStorageBufferOffsetAlignment;
        public uint maxPushConstantsSize;
        public ulong maxBufferSize;
        public uint maxColorAttachments;
        public uint maxViewports;
        public float viewportBoundsMin;
        public float viewportBoundsMax;

        public uint maxComputeWorkgroupStorageSize;
        public uint maxComputeInvocationsPerWorkgroup;
        public uint maxComputeWorkgroupSizeX;
        public uint maxComputeWorkgroupSizeY;
        public uint maxComputeWorkgroupSizeZ;
        public uint maxComputeWorkgroupsPerDimension;

        /* Highest supported shader model */
        public ShaderModel shaderModel;

        /* Only if GPUFeature_ConservativeRasterization is supported */
        public ConservativeRasterizationTier conservativeRasterizationTier;

        /* Only if GPUFeature_VariableRateShading is supported */
        public VariableRateShadingTier variableShadingRateTier;
        public uint variableShadingRateImageTileSize;
        public Bool32 isAdditionalVariableShadingRatesSupported;
    }

    public struct GPUDeviceDesc
    {
        public byte* label;
        public uint maxFramesInFlight/* = 2*/;
    }
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

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUAdapter(nint handle) : IEquatable<GPUAdapter>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUAdapter Null => new(0);
        public static implicit operator GPUAdapter(nint handle) => new(handle);
        public static implicit operator nint(GPUAdapter handle) => handle.Handle;

        public static bool operator ==(GPUAdapter left, GPUAdapter right) => left.Handle == right.Handle;
        public static bool operator !=(GPUAdapter left, GPUAdapter right) => left.Handle != right.Handle;
        public static bool operator ==(GPUAdapter left, nint right) => left.Handle == right;
        public static bool operator !=(GPUAdapter left, nint right) => left.Handle != right;
        public bool Equals(GPUAdapter other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUAdapter handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUAdapter)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUSurfaceHandle(nint handle) : IEquatable<GPUSurfaceHandle>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUSurfaceHandle Null => new(0);
        public static implicit operator GPUSurfaceHandle(nint handle) => new(handle);
        public static implicit operator nint(GPUSurfaceHandle handle) => handle.Handle;

        public static bool operator ==(GPUSurfaceHandle left, GPUSurfaceHandle right) => left.Handle == right.Handle;
        public static bool operator !=(GPUSurfaceHandle left, GPUSurfaceHandle right) => left.Handle != right.Handle;
        public static bool operator ==(GPUSurfaceHandle left, nint right) => left.Handle == right;
        public static bool operator !=(GPUSurfaceHandle left, nint right) => left.Handle != right;
        public bool Equals(GPUSurfaceHandle other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUSurfaceHandle handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUSurfaceHandle)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUSurface(nint handle) : IEquatable<GPUSurface>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUSurface Null => new(0);
        public static implicit operator GPUSurface(nint handle) => new(handle);
        public static implicit operator nint(GPUSurface handle) => handle.Handle;

        public static bool operator ==(GPUSurface left, GPUSurface right) => left.Handle == right.Handle;
        public static bool operator !=(GPUSurface left, GPUSurface right) => left.Handle != right.Handle;
        public static bool operator ==(GPUSurface left, nint right) => left.Handle == right;
        public static bool operator !=(GPUSurface left, nint right) => left.Handle != right;
        public bool Equals(GPUSurface other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUSurface handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUSurface)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUDevice(nint handle) : IEquatable<GPUDevice>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUDevice Null => new(0);
        public static implicit operator GPUDevice(nint handle) => new(handle);
        public static implicit operator nint(GPUDevice handle) => handle.Handle;

        public static bool operator ==(GPUDevice left, GPUDevice right) => left.Handle == right.Handle;
        public static bool operator !=(GPUDevice left, GPUDevice right) => left.Handle != right.Handle;
        public static bool operator ==(GPUDevice left, nint right) => left.Handle == right;
        public static bool operator !=(GPUDevice left, nint right) => left.Handle != right;
        public bool Equals(GPUDevice other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUDevice handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUDevice)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuIsBackendSupport(GraphicsBackendType backendType);

    [LibraryImport(LibraryName)]
    public static partial GPUFactory agpuCreateFactory(GPUFactoryDesc* desc = default);
    [LibraryImport(LibraryName)]
    public static partial void agpuFactoryDestroy(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial GraphicsBackendType agpuFactoryGetBackend(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial GPUAdapter agpuFactoryRequestAdapter(GPUFactory factory, GPURequestAdapterOptions* options);

    #region GPUAdapter Methods
    [LibraryImport(LibraryName)]
    public static partial void agpuAdapterGetInfo(GPUAdapter adapter, out GPUAdapterInfo info);
    [LibraryImport(LibraryName)]
    public static partial void agpuAdapterGetLimits(GPUAdapter adapter, out GPULimits limits);

    //[LibraryImport(LibraryName)]
    //[return: MarshalAs(UnmanagedType.U1)]
    //public static partial bool agpuAdapterHasFeature(GPUAdapter* adapter, GPUFeature feature);
    #endregion

    /* SurfaceHandle */
    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceHandle agpuSurfaceHandleCreateFromWin32(nint hinstance, nint hwnd);

    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceHandle agpuSurfaceHandleCreateFromAndroid(nint window);


    [LibraryImport(LibraryName)]
    public static partial void agpuSurfaceHandleDestroy(GPUSurfaceHandle surfaceHandle);

    #region GPUSurface Methods
    [LibraryImport(LibraryName)]
    public static partial GPUSurface agpuCreateSurface(GPUFactory factory, GPUSurfaceHandle surfaceHandle);
    //[LibraryImport(LibraryName)]
    //public static partial void agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities);
    //[LibraryImport(LibraryName)]
    //public static partial bool agpuSurfaceConfigure(GPUSurface* surface, const GPUSurfaceConfig* config);
    //[LibraryImport(LibraryName)]
    //public static partial void agpuSurfaceUnconfigure(GPUSurface surface);
    [LibraryImport(LibraryName)]
    public static partial uint agpuSurfaceAddRef(GPUSurface surface);
    [LibraryImport(LibraryName)]
    public static partial uint agpuSurfaceRelease(GPUSurface surface);
    #endregion

    #region GPUDevice Methods
    /* Device */
    [LibraryImport(LibraryName)]
    public static partial GPUDevice agpuCreateDevice(GPUAdapter adapter, GPUDeviceDesc* desc);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial void agpuDeviceSetLabel(GPUDevice device, string label);
    [LibraryImport(LibraryName)]
    public static partial uint agpuDeviceAddRef(GPUDevice device);
    [LibraryImport(LibraryName)]
    public static partial uint agpuDeviceRelease(GPUDevice device);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuDeviceHasFeature(GPUDevice device, Feature feature);
    //[LibraryImport(LibraryName)]
    //public static partial GPUQueue agpuDeviceGetQueue(GPUDevice* device, GPUQueueType type);
    [LibraryImport(LibraryName)]
    public static partial void agpuDeviceWaitIdle(GPUDevice device);
    [LibraryImport(LibraryName)]
    public static partial ulong agpuDeviceGetTimestampFrequency(GPUDevice device);

    /// Commit the current frame and advance to next frame
    [LibraryImport(LibraryName)]
    public static partial ulong agpuDeviceCommitFrame(GPUDevice device);
    #endregion
}
