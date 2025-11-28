// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using Alimer.Utilities;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract unsafe class GraphicsDevice : GraphicsObjectBase
{
    protected uint _frameIndex = 0;
    protected ulong _frameCount = 0;
    protected readonly ConcurrentQueue<Tuple<GraphicsObject, ulong>> _deferredDestroyObjects = new();
    protected bool _shuttingDown;

    public GraphicsDevice(in GraphicsDeviceDescription description)
        : base(description.Label)
    {
        MaxFramesInFlight = Math.Min(Math.Max(description.MaxFramesInFlight, Constants.DefaultMaxFramesInFlight), 3u);
    }

    /// <summary>
    /// Get the <see cref="GraphicsAdapter"/> object that created this object.
    /// </summary>
    public abstract GraphicsAdapter Adapter { get; }

    /// <summary>
    /// Gets the maximum number of frames that can be processed concurrently.
    /// </summary>
    public uint MaxFramesInFlight { get; }

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
    /// Get command queue of the specified type.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    public abstract CommandQueue GetCommandQueue(CommandQueueType type);

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
        _frameCount++;
        _frameIndex = (uint)(_frameCount % MaxFramesInFlight);
    }

    protected void ProcessDeletionQueue(bool force)
    {
        while (!_deferredDestroyObjects.IsEmpty)
        {
            if (_deferredDestroyObjects.TryPeek(out Tuple<GraphicsObject, ulong>? item)
                && (force || item.Item2 + MaxFramesInFlight < _frameCount))
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

    public GraphicsBuffer CreateBuffer(in BufferDescription description)
    {
        return CreateBuffer(description, null);
    }

    public GraphicsBuffer CreateBuffer(ulong size,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode cpuAccess = CpuAccessMode.None,
        string? label = default)
    {
        return CreateBuffer(new BufferDescription(size, usage, cpuAccess, label), (void*)null);
    }

    public GraphicsBuffer CreateBuffer(in BufferDescription description, IntPtr initialData)
    {
        return CreateBuffer(description, initialData.ToPointer());
    }

    public GraphicsBuffer CreateBuffer(in BufferDescription description, void* initialData)
    {
        Guard.IsGreaterThanOrEqualTo(description.Size, 4, nameof(BufferDescription.Size));

#if VALIDATE_USAGE
        if ((description.Usage & BufferUsage.Predication) != 0 &&
            !Adapter.QueryFeatureSupport(Feature.Predication))
        {
            throw new GraphicsException($"Buffer cannot be created with {BufferUsage.Predication} usage as adapter doesn't support it");
        }

        if ((description.Usage & BufferUsage.RayTracing) != 0 &&
            !Adapter.QueryFeatureSupport(Feature.RayTracing))
        {
            throw new GraphicsException($"Buffer cannot be created with {BufferUsage.RayTracing} usage as adapter doesn't support it");
        }
#endif

        return CreateBufferCore(description, initialData);
    }

    public GraphicsBuffer CreateBuffer<T>(in BufferDescription description, ref T initialData) where T : unmanaged
    {
        Guard.IsGreaterThanOrEqualTo(description.Size, 4, nameof(BufferDescription.Size));

        fixed (void* initialDataPtr = &initialData)
        {
            return CreateBuffer(description, initialDataPtr);
        }
    }

    public GraphicsBuffer CreateBuffer<T>(Span<T> initialData,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode cpuAccess = CpuAccessMode.None,
        string? label = default)
        where T : unmanaged
    {
        int typeSize = sizeof(T);
        Guard.IsTrue(initialData.Length > 0, nameof(initialData));

        BufferDescription description = new((uint)(initialData.Length * typeSize), usage, cpuAccess, label);
        return CreateBuffer(description, ref MemoryMarshal.GetReference(initialData));
    }

    public GraphicsBuffer CreateBuffer<T>(ReadOnlySpan<T> initialData,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode cpuAccess = CpuAccessMode.None,
        string? label = default)
        where T : unmanaged
    {
        int typeSize = sizeof(T);
        Guard.IsTrue(initialData.Length > 0, nameof(initialData));

        BufferDescription description = new((uint)(initialData.Length * typeSize), usage, cpuAccess, label);
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
        Guard.IsTrue(format != PixelFormat.Undefined, nameof(TextureDescription.Format));
        Guard.IsGreaterThanOrEqualTo(width, 1, nameof(TextureDescription.Width));
        Guard.IsGreaterThanOrEqualTo(height, 1, nameof(TextureDescription.Height));
        Guard.IsGreaterThanOrEqualTo(arrayLayers, 1, nameof(TextureDescription.DepthOrArrayLayers));

        fixed (T* initialDataPtr = initialData)
        {
            PixelFormatUtils.GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch);
            TextureData initData = new(initialDataPtr, rowPitch, slicePitch);

            return CreateTextureCore(TextureDescription.Texture2D(format, width, height, mipLevelCount, arrayLayers, usage), &initData);
        }
    }

    public Texture CreateTexture(in TextureDescription description)
    {
        Guard.IsTrue(description.Format != PixelFormat.Undefined, nameof(TextureDescription.Format));
        Guard.IsGreaterThanOrEqualTo(description.Width, 1, nameof(TextureDescription.Width));
        Guard.IsGreaterThanOrEqualTo(description.Width, 1, nameof(TextureDescription.Width));
        Guard.IsGreaterThanOrEqualTo(description.Height, 1, nameof(TextureDescription.Height));
        Guard.IsGreaterThanOrEqualTo(description.DepthOrArrayLayers, 1, nameof(TextureDescription.DepthOrArrayLayers));

        return CreateTextureCore(description, default);
    }

    public Sampler CreateSampler(in SamplerDescription description)
    {
        if (description.ReductionType == SamplerReductionType.Minimum ||
            description.ReductionType == SamplerReductionType.Maximum)
        {
            if (Adapter.QueryFeatureSupport(Feature.SamplerMinMax))
            {
                throw new GraphicsException($"{nameof(Feature.SamplerMinMax)} feature is not supported");
            }
        }

        return CreateSamplerCore(description);
    }

    public BindGroupLayout CreateBindGroupLayout(in BindGroupLayoutDescription description)
    {
        return CreateBindGroupLayoutCore(in description);
    }

    public BindGroupLayout CreateBindGroupLayout(params BindGroupLayoutEntry[] entries)
    {
        return CreateBindGroupLayoutCore(new BindGroupLayoutDescription(entries));
    }

    public BindGroup CreateBindGroup(BindGroupLayout layout, in BindGroupDescription description)
    {
        Guard.IsNotNull(layout, nameof(layout));
        Guard.IsNotNull(description.Entries, nameof(BindGroupDescription.Entries));
        Guard.IsGreaterThan(description.Entries.Length, 0, nameof(BindGroupDescription.Entries));

        return CreateBindGroupCore(layout, in description);
    }

    public BindGroup CreateBindGroup(BindGroupLayout layout, params BindGroupEntry[] entries)
    {
        Guard.IsNotNull(layout, nameof(layout));
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        return CreateBindGroupCore(layout, new BindGroupDescription(entries));
    }

    public PipelineLayout CreatePipelineLayout(in PipelineLayoutDescription description)
    {
        return CreatePipelineLayoutCore(in description);
    }

    public PipelineLayout CreatePipelineLayout(params BindGroupLayout[] bindGroupLayouts)
    {
        return CreatePipelineLayout(new PipelineLayoutDescription(bindGroupLayouts));
    }

    public RenderPipeline CreateRenderPipeline(in RenderPipelineDescriptor descriptor)
    {
        Guard.IsGreaterThanOrEqualTo(descriptor.ShaderStages.Length, 1, nameof(RenderPipelineDescriptor.ShaderStages));

        return CreateRenderPipelineCore(in descriptor);
    }

    public ComputePipeline CreateComputePipeline(in ComputePipelineDescriptor descriptor)
    {
        Guard.IsTrue(descriptor.ComputeShader.Stage == ShaderStages.Compute, nameof(ComputePipelineDescriptor.ComputeShader));
        Guard.IsNotNull(descriptor.ComputeShader.ByteCode != null, nameof(ComputePipelineDescriptor.ComputeShader.ByteCode));
        Guard.IsGreaterThan(descriptor.ComputeShader.ByteCode!.Length, 0);

        return CreateComputePipelineCore(in descriptor);
    }

    public QueryHeap CreateQueryHeap(in QueryHeapDescription descriptor)
    {
        return CreateQueryHeapCore(descriptor);
    }

    public SwapChain CreateSwapChain(ISwapChainSurface surface, in SwapChainDescription description)
    {
        Guard.IsNotNull(surface, nameof(surface));

        return CreateSwapChainCore(surface, description);
    }

    /// <summary>
    /// Acquire command buffer ready for recording.
    /// </summary>
    /// <param name="queue"></param>
    /// <param name="label"></param>
    /// <returns></returns>
    public abstract CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default);

    protected abstract unsafe GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData);
    protected abstract unsafe Texture CreateTextureCore(in TextureDescription description, TextureData* initialData);
    protected abstract Sampler CreateSamplerCore(in SamplerDescription description);
    protected abstract BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description);
    protected abstract BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description);
    protected abstract PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description);
    protected abstract RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor);
    protected abstract ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor);
    protected abstract QueryHeap CreateQueryHeapCore(in QueryHeapDescription descriptor);
    protected abstract SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description);
}
