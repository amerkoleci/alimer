// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

#pragma warning disable CS0649

namespace Alimer.Graphics.Native;

internal static unsafe partial class AlimerGPUApi
{
#if DEBUG
    //public const string LibraryName = "alimer_gpu_d";
    public const string LibraryName = "alimer_gpu";
#else
    public const string LibraryName = "alimer_gpu";
#endif
    public const int GPU_MAX_ADAPTER_NAME_SIZE = 256;
    public const int GPU_MAX_COLOR_ATTACHMENTS = 8;
    public const int GPU_MAX_VERTEX_BUFFER_BINDINGS = 8;
    public const ulong GPU_WHOLE_SIZE = ulong.MaxValue;
    public const float GPU_LOD_CLAMP_NONE = 1000.0f;

    #region Enums

    public enum GPULogLevel
    {
        Off = 0,
        Error,
        Warn,
        Info,
        Debug,
        Trace,

        Count
    }

    public enum GPUMemoryType
    {
        /// <summary>
        /// CPU no access, GPU read/write
        /// </summary>
        Private,
        /// <summary>
        /// CPU write, GPU read
        /// </summary>
        Upload,
        /// <summary>
        /// CPU read, GPU write
        /// </summary>
        Readback,

        Count,
    }

    [Flags]
    public enum GPUBufferUsage
    {

        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        /// <summary>
        /// Supports Constant buffer access.
        /// </summary>
        Constant = 1 << 2,
        ShaderRead = 1 << 3,
        ShaderWrite = 1 << 4,
        /// <summary>
        /// Supports indirect buffer access for indirect draw/dispatch.
        /// </summary>
        Indirect = 1 << 5,
        /// <summary>
        /// Supports predication access for conditional rendering.
        /// </summary>
        Predication = (1 << 6),
        /// <summary>
        /// Supports ray tracing acceleration structure usage.
        /// </summary>
        RayTracing = 1 << 7,
        Count
    }

    public enum GPUTextureDimension
    {
        /// Undefined - default to 2D texture.
        Undefined = 0,
        /// One-dimensional Texture.
        Texture1D = 1,
        /// Two-dimensional Texture.
        Texture2D = 2,
        /// Three-dimensional Texture.
        Texture3D = 3,
        /// Cubemap Texture.
        TextureCube = 4,
    }
    #endregion

    #region Structs
    public struct GPUFactoryDesc
    {
        public GraphicsBackend preferredBackend;
        public GraphicsValidationMode validationMode;
    }

    public struct GPUAdapterInfo
    {
        public fixed byte deviceName[GPU_MAX_ADAPTER_NAME_SIZE];
        public fixed ushort driverVersion[4];
        public byte* driverDescription;
        public GraphicsAdapterType adapterType;
        public GraphicsAdapterVendor vendor;
        public uint vendorID;
        public uint deviceID;
    }

    public struct GPUDeviceLimits
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

        /* ConservativeRasterization tier */
        public ConservativeRasterizationTier conservativeRasterizationTier;

        /* VariableRateShading tier */
        public VariableShadingRateTier variableShadingRateTier;
        public uint variableShadingRateImageTileSize;
        public Bool32 isAdditionalVariableShadingRatesSupported;

        /* Ray tracing */
        public RayTracingTier rayTracingTier;
        public uint rayTracingShaderGroupIdentifierSize;
        public uint rayTracingShaderTableAlignment;
        public uint rayTracingShaderTableMaxStride;
        public uint rayTracingShaderRecursionMaxDepth;
        public uint rayTracingMaxGeometryCount;
        public uint rayTracingScratchAlignment;

        /* Mesh shader */
        public MeshShaderTier meshShaderTier;
    }

    public struct GPUSurfaceCapabilities
    {
        public PixelFormat preferredFormat;
        public TextureUsage supportedUsage;
        public int formatCount;
        public PixelFormat* formats;
        public int presentModeCount;
        public PresentMode* presentModes;
    }

    public struct GPUSurfaceConfig
    {
        public GPUDevice device;
        public PixelFormat format;
        public uint width;
        public uint height;
        public PresentMode presentMode;
    }

    public struct GPUDeviceDesc
    {
        public byte* label;
        public uint maxFramesInFlight/* = 2*/;
    }

    public struct GPUCommandBufferDesc
    {
        public byte* label;
    }

    public struct GPUBufferDesc
    {
        public byte* label;
        public ulong size;
        public GPUBufferUsage usage;
        public GPUMemoryType memoryType;
    }

    public struct GPUTextureDesc
    {
        public byte* label;
        public GPUTextureDimension dimension;
        public PixelFormat format;
        public TextureUsage usage;
        public uint width;
        public uint height;
        public uint depthOrArrayLayers;
        public uint mipLevelCount;
        public uint sampleCount;
    }

    public struct GPUTextureData
    {
        public void* pData;
        public uint rowPitch;
        public uint slicePitch;
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
    public readonly partial struct GPUSurfaceSource(nint handle) : IEquatable<GPUSurfaceSource>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUSurfaceSource Null => new(0);
        public static implicit operator GPUSurfaceSource(nint handle) => new(handle);
        public static implicit operator nint(GPUSurfaceSource handle) => handle.Handle;

        public static bool operator ==(GPUSurfaceSource left, GPUSurfaceSource right) => left.Handle == right.Handle;
        public static bool operator !=(GPUSurfaceSource left, GPUSurfaceSource right) => left.Handle != right.Handle;
        public static bool operator ==(GPUSurfaceSource left, nint right) => left.Handle == right;
        public static bool operator !=(GPUSurfaceSource left, nint right) => left.Handle != right;
        public bool Equals(GPUSurfaceSource other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUSurfaceSource handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUSurfaceSource)} [0x{Handle:X}]";
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

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUCommandQueue(nint handle) : IEquatable<GPUCommandQueue>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUCommandQueue Null => new(0);
        public static implicit operator GPUCommandQueue(nint handle) => new(handle);
        public static implicit operator nint(GPUCommandQueue handle) => handle.Handle;

        public static bool operator ==(GPUCommandQueue left, GPUCommandQueue right) => left.Handle == right.Handle;
        public static bool operator !=(GPUCommandQueue left, GPUCommandQueue right) => left.Handle != right.Handle;
        public static bool operator ==(GPUCommandQueue left, nint right) => left.Handle == right;
        public static bool operator !=(GPUCommandQueue left, nint right) => left.Handle != right;
        public bool Equals(GPUCommandQueue other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUCommandQueue handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUCommandQueue)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUCommandBuffer(nint handle) : IEquatable<GPUCommandBuffer>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUCommandBuffer Null => new(0);
        public static implicit operator GPUCommandBuffer(nint handle) => new(handle);
        public static implicit operator nint(GPUCommandBuffer handle) => handle.Handle;

        public static bool operator ==(GPUCommandBuffer left, GPUCommandBuffer right) => left.Handle == right.Handle;
        public static bool operator !=(GPUCommandBuffer left, GPUCommandBuffer right) => left.Handle != right.Handle;
        public static bool operator ==(GPUCommandBuffer left, nint right) => left.Handle == right;
        public static bool operator !=(GPUCommandBuffer left, nint right) => left.Handle != right;
        public bool Equals(GPUCommandBuffer other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUCommandBuffer handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUCommandBuffer)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUBuffer(nint handle) : IEquatable<GPUBuffer>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUBuffer Null => new(0);
        public static implicit operator GPUBuffer(nint handle) => new(handle);
        public static implicit operator nint(GPUBuffer handle) => handle.Handle;

        public static bool operator ==(GPUBuffer left, GPUBuffer right) => left.Handle == right.Handle;
        public static bool operator !=(GPUBuffer left, GPUBuffer right) => left.Handle != right.Handle;
        public static bool operator ==(GPUBuffer left, nint right) => left.Handle == right;
        public static bool operator !=(GPUBuffer left, nint right) => left.Handle != right;
        public bool Equals(GPUBuffer other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUBuffer handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUBuffer)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUTexture(nint handle) : IEquatable<GPUTexture>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUTexture Null => new(0);
        public static implicit operator GPUTexture(nint handle) => new(handle);
        public static implicit operator nint(GPUTexture handle) => handle.Handle;

        public static bool operator ==(GPUTexture left, GPUTexture right) => left.Handle == right.Handle;
        public static bool operator !=(GPUTexture left, GPUTexture right) => left.Handle != right.Handle;
        public static bool operator ==(GPUTexture left, nint right) => left.Handle == right;
        public static bool operator !=(GPUTexture left, nint right) => left.Handle != right;
        public bool Equals(GPUTexture other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUTexture handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUTexture)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct GPUSampler(nint handle) : IEquatable<GPUSampler>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static GPUSampler Null => new(0);
        public static implicit operator GPUSampler(nint handle) => new(handle);
        public static implicit operator nint(GPUSampler handle) => handle.Handle;

        public static bool operator ==(GPUSampler left, GPUSampler right) => left.Handle == right.Handle;
        public static bool operator !=(GPUSampler left, GPUSampler right) => left.Handle != right.Handle;
        public static bool operator ==(GPUSampler left, nint right) => left.Handle == right;
        public static bool operator !=(GPUSampler left, nint right) => left.Handle != right;
        public bool Equals(GPUSampler other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is GPUSampler handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(GPUSampler)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(LibraryName)]
    public static partial GPULogLevel agpuGetLogLevel();
    [LibraryImport(LibraryName)]
    public static partial void agpuSetLogLevel(GPULogLevel level);

    [LibraryImport(LibraryName)]
    public static partial void agpuSetLogCallback(delegate* unmanaged<GPULogLevel, byte*, nint, void> callback, nint userdata);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuIsBackendSupport(GraphicsBackend backendType);

    [LibraryImport(LibraryName)]
    public static partial GPUFactory agpuCreateFactory(GPUFactoryDesc* desc = default);
    [LibraryImport(LibraryName)]
    public static partial void agpuFactoryDestroy(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial GraphicsBackend agpuFactoryGetBackend(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial int agpuFactoryGetAdapterCount(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial GPUAdapter agpuFactoryGetAdapter(GPUFactory factory, int index);
    [LibraryImport(LibraryName)]
    public static partial GPUAdapter agpuFactoryGetBestAdapter(GPUFactory factory);
    [LibraryImport(LibraryName)]
    public static partial GPUSurface agpuFactoryCreateSurface(GPUFactory factory, GPUSurfaceSource source);

    #region GPUAdapter Methods
    [LibraryImport(LibraryName)]
    public static partial void agpuAdapterGetInfo(GPUAdapter adapter, out GPUAdapterInfo info);
    /* Device */
    [LibraryImport(LibraryName)]
    public static partial GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter, GPUDeviceDesc* desc);

    //[LibraryImport(LibraryName)]
    //[return: MarshalAs(UnmanagedType.U1)]
    //public static partial bool agpuAdapterHasFeature(GPUAdapter* adapter, GPUFeature feature);
    #endregion

    /* SurfaceHandle */
    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceSource agpuSurfaceSourceCreateFromWin32(nint hwnd);

    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceSource agpuSurfaceSourceCreateFromAndroid(nint window);

    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceSource agpuSurfaceSourceCreateFromMetalLayer(nint metalLayer);

    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceSource agpuSurfaceSourceCreateFromWaylandSurface(nint display, nint surface);

    [LibraryImport(LibraryName)]
    public static partial GPUSurfaceSource agpuSurfaceSourceCreateFromXlibWindow(nint display, ulong window);


    [LibraryImport(LibraryName)]
    public static partial void agpuSurfaceSourceDestroy(GPUSurfaceSource surfaceSource);

    #region GPUSurface Methods
    [LibraryImport(LibraryName)]
    public static partial void agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter adapter, out GPUSurfaceCapabilities capabilities);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuSurfaceConfigure(GPUSurface surface, GPUSurfaceConfig* config);
    [LibraryImport(LibraryName)]
    public static partial void agpuSurfaceUnconfigure(GPUSurface surface);
    [LibraryImport(LibraryName)]
    public static partial uint agpuSurfaceAddRef(GPUSurface surface);
    [LibraryImport(LibraryName)]
    public static partial uint agpuSurfaceRelease(GPUSurface surface);
    #endregion

    #region GPUDevice Methods
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial void agpuDeviceSetLabel(GPUDevice device, string label);
    [LibraryImport(LibraryName)]
    public static partial uint agpuDeviceAddRef(GPUDevice device);
    [LibraryImport(LibraryName)]
    public static partial uint agpuDeviceRelease(GPUDevice device);
    [LibraryImport(LibraryName)]
    public static partial void agpuDeviceGetLimits(GPUDevice device, out GPUDeviceLimits limits);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool agpuDeviceHasFeature(GPUDevice device, Feature feature);
    [LibraryImport(LibraryName)]
    public static partial GPUCommandQueue agpuDeviceGetCommandQueue(GPUDevice device, CommandQueueType type);
    [LibraryImport(LibraryName)]
    public static partial void agpuDeviceWaitIdle(GPUDevice device);
    [LibraryImport(LibraryName)]
    public static partial ulong agpuDeviceGetTimestampFrequency(GPUDevice device);

    /// Commit the current frame and advance to next frame
    [LibraryImport(LibraryName)]
    public static partial ulong agpuDeviceCommitFrame(GPUDevice device);

    [LibraryImport(LibraryName)]
    public static partial GPUBuffer agpuDeviceCreateBuffer(GPUDevice device, GPUBufferDesc* desc, void* pInitialData);
    [LibraryImport(LibraryName)]
    public static partial GPUTexture agpuDeviceCreateTexture(GPUDevice device, GPUTextureDesc* desc, GPUTextureData* pInitialData);
    //[LibraryImport(LibraryName)]
    //public static partial GPUSampler agpuDeviceCreateSampler(GPUDevice device, GPUSamplerDesc* desc);
    #endregion

    #region CommandQueue
    [LibraryImport(LibraryName)]
    public static partial CommandQueueType agpuCommandQueueGetType(GPUCommandQueue queue);

    [LibraryImport(LibraryName)]
    public static partial void agpuCommandQueueWaitIdle(GPUCommandQueue queue);

    [LibraryImport(LibraryName)]
    public static partial GPUCommandBuffer agpuCommandQueueAcquireCommandBuffer(GPUCommandQueue queue, GPUCommandBufferDesc* desc);

    [LibraryImport(LibraryName)]
    public static partial void agpuCommandQueueSubmit(GPUCommandQueue queue, int numCommandBuffers, GPUCommandBuffer* commandBuffers);
    #endregion
}
