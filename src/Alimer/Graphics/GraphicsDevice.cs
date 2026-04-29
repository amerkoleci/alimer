// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Alimer.Utilities;

namespace Alimer.Graphics;

/// <summary>
/// Defines a GPU logical device using given <see cref="GraphicsAdapter"/>.
/// </summary>
public abstract unsafe class GraphicsDevice : GraphicsBaseObject
{
    protected uint _frameIndex = 0;
    protected ulong _frameCount = 0;
    protected readonly ConcurrentQueue<Tuple<GraphicsObject, ulong>> _deferredDestroyObjects = new();
    protected bool _shuttingDown;
    protected readonly GPULinearAllocator[] _frameAllocators;

    public GraphicsDevice(GraphicsBackend backend, in GraphicsDeviceDescription description)
        : base(description.Label)
    {
        Backend = backend;
        MaxFramesInFlight = Math.Min(Math.Max(description.MaxFramesInFlight, Constants.DefaultMaxFramesInFlight), 3);
        _frameAllocators = new GPULinearAllocator[MaxFramesInFlight];
        for (int i = 0; i < MaxFramesInFlight; i++)
        {
            _frameAllocators[i] = new GPULinearAllocator();
        }
    }

    /// <summary>
    /// Get the type of the graphics backend.
    /// </summary>
    public GraphicsBackend Backend { get; }

    /// <summary>
    /// Get the <see cref="GraphicsAdapter"/> object that created this object.
    /// </summary>
    public abstract GraphicsAdapter Adapter { get; }

    /// <summary>
    /// Gets the maximum number of frames that can be processed concurrently.
    /// </summary>
    public int MaxFramesInFlight { get; }

    /// <summary>
    /// Get the device limits.
    /// </summary>
    public abstract GraphicsDeviceLimits Limits { get; }

    /// <summary>
    /// Gets the graphics command queue used to submit rendering commands to the GPU.
    /// </summary>
    [NotNull]
    public CommandQueue GraphicsQueue
    {
        get => field ??= GetCommandQueue(CommandQueueType.Graphics)!;
    }

    /// <summary>
    /// Gets the command queue used for compute operations on the device.
    /// </summary>
    [NotNull]
    public CommandQueue ComputeQueue
    {
        get => field ??= GetCommandQueue(CommandQueueType.Compute)!;
    }

    /// <summary>
    /// Gets the command queue used for copy operations.
    /// </summary>
    [NotNull]
    public CommandQueue CopyQueue
    {
        get => field ??= GetCommandQueue(CommandQueueType.Copy)!;
    }

    /// <summary>
    /// Gets the command queue used for video decode operations, if available.
    /// </summary>
    public CommandQueue? VideoDecodeQueue
    {
        get => field ??= GetCommandQueue(CommandQueueType.VideoDecode);
    }

    /// <summary>
    /// Get the timestamp frequency.
    /// </summary>
    public abstract ulong TimestampFrequency { get; }

    /// <summary>
    /// Gets the number of frame being executed.
    /// </summary>
    public ulong FrameCount => _frameCount;

    /// <summary>
    /// Gets the current frame index.
    /// </summary>
    public uint FrameIndex => _frameIndex;

    /// <summary>
    /// Gets the frame allocator used for GPU memory management in the current rendering frame.
    /// </summary>
    /// <remarks>The frame allocator provides efficient, per-frame memory allocation for GPU resources.
    /// Accessing this property allows developers to allocate temporary GPU memory that is automatically reset each
    /// frame, which is useful for transient resource management during rendering operations.</remarks>
    public GPULinearAllocator FrameAllocator => _frameAllocators[_frameIndex];

    public abstract bool QueryFeatureSupport(Feature feature);

    public abstract PixelFormatSupport QueryPixelFormatSupport(PixelFormat format);
    public abstract bool QueryVertexFormatSupport(VertexAttributeFormat format);

    /// <summary>
    /// Get command queue of the specified type.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    public abstract CommandQueue? GetCommandQueue(CommandQueueType type);

    /// <summary>
    /// Wait for device to finish pending GPU operations.
    /// </summary>
    public abstract void WaitIdle();

    /// <summary>
    /// Commit the current frame and advance to next one.
    /// </summary>
    /// <returns></returns>
    public abstract ulong CommitFrame();

    protected void AdvanceFrame()
    {
        // Begin new frame
        _frameAllocators[_frameIndex].Reset();
        _frameCount++;
        _frameIndex = (uint)(_frameCount % (uint)MaxFramesInFlight);
    }

    protected virtual void ProcessDeletionQueue(bool force)
    {
        while (!_deferredDestroyObjects.IsEmpty)
        {
            if (_deferredDestroyObjects.TryPeek(out Tuple<GraphicsObject, ulong>? item)
                && (force || item.Item2 + (uint)MaxFramesInFlight < _frameCount))
            {
                if (_deferredDestroyObjects.TryDequeue(out item))
                {
                    item.Item1.Destroy();
                }
            }
            else
            {
                break;
            }
        }
    }

    internal void QueueDestroy(GraphicsObject @object)
    {
        if (_shuttingDown)
        {
            @object.Destroy();
            return;
        }

        _deferredDestroyObjects.Enqueue(Tuple.Create(@object, _frameCount));
    }

    public virtual unsafe void WriteShadingRateValue(ShadingRate rate, void* dest)
    {

    }

    public GPUBuffer CreateBuffer(in GPUBufferDescriptor descriptor)
    {
        return CreateBuffer(descriptor, null);
    }

    public GPUBuffer CreateBuffer(ulong size,
        GPUBufferUsage usage = GPUBufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
    {
        return CreateBuffer(new GPUBufferDescriptor(size, usage, memoryType, label), (void*)null);
    }

    public GPUBuffer CreateBuffer(in GPUBufferDescriptor descriptor, IntPtr initialData)
    {
        return CreateBuffer(descriptor, initialData.ToPointer());
    }

    public GPUBuffer CreateBuffer(in GPUBufferDescriptor descriptor, void* initialData)
    {
        ArgumentOutOfRangeException.ThrowIfLessThan(descriptor.Size, 4u, nameof(GPUBufferDescriptor.Size));

#if VALIDATE_USAGE
        if ((descriptor.Usage & GPUBufferUsage.Predication) != 0 &&
            !QueryFeatureSupport(Feature.Predication))
        {
            throw new GraphicsException($"Buffer cannot be created with {GPUBufferUsage.Predication} usage as adapter doesn't support it");
        }

        if ((descriptor.Usage & GPUBufferUsage.RayTracing) != 0 &&
            Limits.RayTracingTier == RayTracingTier.NotSupported)
        {
            throw new GraphicsException($"Buffer cannot be created with {GPUBufferUsage.RayTracing} usage as adapter doesn't support it");
        }
#endif
        // ID3D12Device::CreateCommittedResource3: A buffer cannot be created on a D3D12_HEAP_TYPE_UPLOAD or D3D12_HEAP_TYPE_READBACK heap when either D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET or D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS is used.
        if ((descriptor.Usage & GPUBufferUsage.ShaderWrite) == GPUBufferUsage.ShaderWrite)
        {
            if (descriptor.MemoryType == MemoryType.Readback)
            {
                throw new GraphicsException($"Buffer cannot be created with {GPUBufferUsage.ShaderWrite} usage and {MemoryType.Readback} memory type");
            }
            else if (descriptor.MemoryType == MemoryType.Upload)
            {
                throw new GraphicsException($"Buffer cannot be created with {GPUBufferUsage.ShaderWrite} usage and {MemoryType.Upload} memory type");
            }
        }

        return CreateBufferCore(descriptor, initialData);
    }

    public GPUBuffer CreateBuffer<T>(in GPUBufferDescriptor description, ref T initialData) where T : unmanaged
    {
        ArgumentOutOfRangeException.ThrowIfLessThan(description.Size, 4u, nameof(GPUBufferDescriptor.Size));

        fixed (void* initialDataPtr = &initialData)
        {
            return CreateBuffer(description, initialDataPtr);
        }
    }

    public GPUBuffer CreateBuffer<T>(Span<T> initialData,
        GPUBufferUsage usage = GPUBufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
        where T : unmanaged
    {
        int typeSize = sizeof(T);
        ArgumentException.ThrowIfFalse(initialData.Length > 0, nameof(initialData));

        GPUBufferDescriptor description = new((uint)(initialData.Length * typeSize), usage, memoryType, label);
        return CreateBuffer(description, ref MemoryMarshal.GetReference(initialData));
    }

    public GPUBuffer CreateBuffer<T>(ReadOnlySpan<T> initialData,
        GPUBufferUsage usage = GPUBufferUsage.ShaderReadWrite,
        MemoryType memoryType = MemoryType.Private,
        string? label = default)
        where T : unmanaged
    {
        int typeSize = sizeof(T);
        ArgumentException.ThrowIfFalse(initialData.Length > 0, nameof(initialData));

        GPUBufferDescriptor description = new((uint)(initialData.Length * typeSize), usage, memoryType, label);
        return CreateBuffer(description, ref MemoryMarshal.GetReference(initialData));
    }

    public Texture CreateTexture2D<T>(ReadOnlySpan<T> initialData,
        PixelFormat format,
        uint width,
        uint height,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead
        )
        where T : unmanaged
    {
        ArgumentException.ThrowIfFalse(format != PixelFormat.Undefined, nameof(TextureDescriptor.Format));
        ArgumentOutOfRangeException.ThrowIfLessThan(width, 1u, nameof(TextureDescriptor.Width));
        ArgumentOutOfRangeException.ThrowIfLessThan(height, 1u, nameof(TextureDescriptor.Height));
        ArgumentOutOfRangeException.ThrowIfLessThan(arrayLayers, 1u, nameof(TextureDescriptor.DepthOrArrayLayers));

        fixed (T* initialDataPtr = initialData)
        {
            PixelFormatUtils.GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch);
            TextureData initData = new(new IntPtr(initialDataPtr), rowPitch, slicePitch);

            return CreateTextureCore(TextureDescriptor.Texture2D(format, width, height, mipLevelCount, arrayLayers, usage), &initData);
        }
    }

    public Texture CreateTexture(in TextureDescriptor descriptor) => CreateTexture(descriptor, []);

    public Texture CreateTexture(in TextureDescriptor descriptor, ReadOnlySpan<TextureData> initialData)
    {
        ArgumentException.ThrowIfFalse(descriptor.Format != PixelFormat.Undefined, nameof(TextureDescriptor.Format));
        ArgumentOutOfRangeException.ThrowIfLessThan(descriptor.Width, 1u, nameof(TextureDescriptor.Width));
        ArgumentOutOfRangeException.ThrowIfLessThan(descriptor.Width, 1u, nameof(TextureDescriptor.Width));
        ArgumentOutOfRangeException.ThrowIfLessThan(descriptor.Height, 1u, nameof(TextureDescriptor.Height));
        ArgumentOutOfRangeException.ThrowIfLessThan(descriptor.DepthOrArrayLayers, 1u, nameof(TextureDescriptor.DepthOrArrayLayers));

        if (descriptor.Dimension == TextureDimension.TextureCube)
        {
            ArgumentException.ThrowIfFalse(descriptor.Width == descriptor.Height, nameof(TextureDescriptor.Width), nameof(TextureDescriptor.Height));
            ArgumentException.ThrowIfFalse(descriptor.DepthOrArrayLayers == 1 || descriptor.DepthOrArrayLayers % 6 == 0, nameof(TextureDescriptor.DepthOrArrayLayers));
        }

        if (initialData.IsEmpty)
        {
            return CreateTextureCore(descriptor, default);
        }
        else
        {
            fixed (TextureData* initialDataPtr = initialData)
            {
                return CreateTextureCore(in descriptor, initialDataPtr);
            }
        }
    }

    public Sampler CreateSampler(in SamplerDescriptor descriptor)
    {
        if (descriptor.ReductionType == SamplerReductionType.Minimum
            || descriptor.ReductionType == SamplerReductionType.Maximum)
        {
            if (QueryFeatureSupport(Feature.SamplerMinMax))
            {
                throw new GraphicsException($"{nameof(Feature.SamplerMinMax)} feature is not supported");
            }
        }

        return CreateSamplerCore(descriptor);
    }

    public BindGroupLayout CreateBindGroupLayout(in BindGroupLayoutDescriptor descriptor)
    {
        return CreateBindGroupLayoutCore(in descriptor);
    }

    public BindGroupLayout CreateBindGroupLayout(string label, params Span<BindGroupLayoutEntry> entries)
    {
        return CreateBindGroupLayoutCore(new BindGroupLayoutDescriptor(entries, label));
    }

    public PipelineLayout CreatePipelineLayout(in PipelineLayoutDescriptor description)
    {
        return CreatePipelineLayoutCore(in description);
    }

    public PipelineLayout CreatePipelineLayout(params Span<BindGroupLayout> bindGroupLayouts)
    {
        return CreatePipelineLayout(new PipelineLayoutDescriptor(bindGroupLayouts));
    }

    public ShaderModule CreateShaderModule(in ShaderModuleDescriptor descriptor)
    {
        ArgumentException.ThrowIfFalse(descriptor.Stage != ShaderStages.None, nameof(ShaderModuleDescriptor.Stage));
        ArgumentException.ThrowIfTrue(descriptor.ByteCode.IsEmpty, nameof(ShaderModuleDescriptor.ByteCode));

        return CreateShaderModuleCore(in descriptor);
    }

    public RenderPipeline CreateRenderPipeline(in RenderPipelineDescriptor descriptor)
    {
        // Vertex shader is necessary when not using mesh shaders
        if (descriptor.VertexShader is null)
        {
            if (descriptor.MeshShader == null)
            {
                throw new GraphicsException($"{nameof(RenderPipelineDescriptor.MeshShader)} is required when creating mesh pipeline");
            }

            ArgumentException.ThrowIfFalse(descriptor.MeshShader.Stage == ShaderStages.Mesh, nameof(RenderPipelineDescriptor.MeshShader));

            if (descriptor.AmplificationShader is not null)
            {
                ArgumentException.ThrowIfFalse(descriptor.AmplificationShader.Stage == ShaderStages.Amplification, nameof(RenderPipelineDescriptor.AmplificationShader));
            }
        }
        else
        {
            ArgumentException.ThrowIfFalse(descriptor.VertexShader.Stage == ShaderStages.Vertex, nameof(RenderPipelineDescriptor.VertexShader));
        }

        if (descriptor.FragmentShader is not null)
        {
            ArgumentException.ThrowIfFalse(descriptor.FragmentShader.Stage == ShaderStages.Fragment, nameof(RenderPipelineDescriptor.FragmentShader));
        }

        return CreateRenderPipelineCore(in descriptor);
    }

    public ComputePipeline CreateComputePipeline(in ComputePipelineDescriptor descriptor)
    {
        ArgumentNullException.ThrowIfNull(descriptor.ComputeShader, nameof(ComputePipelineDescriptor.ComputeShader));
        ArgumentException.ThrowIfFalse(descriptor.ComputeShader.Stage == ShaderStages.Compute, nameof(ComputePipelineDescriptor.ComputeShader));

        return CreateComputePipelineCore(in descriptor);
    }

    public QueryHeap CreateQueryHeap(in QueryHeapDescriptor descriptor)
    {
        ArgumentException.ThrowIfFalse(descriptor.Count > 0 && descriptor.Count < Constants.QuerySetMaxQueries);

        return CreateQueryHeapCore(descriptor);
    }

    public SwapChain CreateSwapChain(in SwapChainDescriptor descriptor)
    {
        ArgumentNullException.ThrowIfNull(descriptor.Surface, nameof(SwapChainDescriptor.Surface));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(descriptor.Width, nameof(SwapChainDescriptor.Width));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(descriptor.Height, nameof(SwapChainDescriptor.Height));
        ArgumentException.ThrowIfFalse(descriptor.Format != PixelFormat.Undefined, nameof(SwapChainDescriptor.Format));

        return CreateSwapChainCore(descriptor);
    }

    /// <summary>
    /// Acquire command buffer ready for recording.
    /// </summary>
    /// <param name="queue"></param>
    /// <param name="label"></param>
    /// <returns></returns>
    public abstract CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default);

    protected abstract GPUBuffer CreateBufferCore(in GPUBufferDescriptor descriptor, void* initialData);
    protected abstract Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData);
    protected abstract Sampler CreateSamplerCore(in SamplerDescriptor descriptor);
    protected abstract BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescriptor descriptor);
    protected abstract PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescriptor descriptor);
    protected abstract ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor);
    protected abstract RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor);
    protected abstract ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor);
    protected abstract QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor);
    protected abstract SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor);
}
