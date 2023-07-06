// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract unsafe class GraphicsDevice : GraphicsObjectBase
{
    protected uint _frameIndex = 0;
    protected ulong _frameCount = 0;
    protected readonly ConcurrentQueue<Tuple<GraphicsObject, ulong>> _deferredDestroyObjects = new();
    protected bool _shuttingDown;

    public GraphicsDevice(GraphicsBackendType backend, in GraphicsDeviceDescription description)
        : base(description.Label)
    {
        Backend = backend;
        ValidationMode = description.ValidationMode;
    }

    /// <summary>
    /// Get the device backend type.
    /// </summary>
    public GraphicsBackendType Backend { get; }

    /// <summary>
    /// Gets the device validation mode.
    /// </summary>
    public ValidationMode ValidationMode { get; }

    /// <summary>
    /// Get the adapter info.
    /// </summary>
    public abstract GraphicsAdapterProperties AdapterInfo { get; }

    /// <summary>
    /// Get the device limits.
    /// </summary>
    public abstract GraphicsDeviceLimits Limits { get; }

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

    public static bool IsBackendSupport(GraphicsBackendType backend)
    {
        Guard.IsTrue(backend != GraphicsBackendType.Count, nameof(backend), "Invalid backend");

        switch (backend)
        {
            case GraphicsBackendType.Null:
                return true;
#if !EXCLUDE_VULKAN_BACKEND
            case GraphicsBackendType.Vulkan:
                return Vulkan.VulkanGraphicsDevice.IsSupported();
#endif

#if !EXCLUDE_D3D12_BACKEND
            case GraphicsBackendType.D3D12:
                return D3D12.D3D12GraphicsDevice.IsSupported();
#endif

#if !EXCLUDE_METAL_BACKEND
            case GraphicsBackendType.Metal:
                return false;
#endif

            default:
                return false;
        }
    }

    public static GraphicsDevice CreateDefault(in GraphicsDeviceDescription description)
    {
        GraphicsBackendType backend = description.PreferredBackend;
        if (backend == GraphicsBackendType.Count)
        {
            if (IsBackendSupport(GraphicsBackendType.D3D12))
            {
                backend = GraphicsBackendType.D3D12;
            }
            else if (IsBackendSupport(GraphicsBackendType.Metal))
            {
                backend = GraphicsBackendType.Metal;
            }
            else if (IsBackendSupport(GraphicsBackendType.Vulkan))
            {
                backend = GraphicsBackendType.Vulkan;
            }
        }

        GraphicsDevice? device = default;
        switch (backend)
        {
#if !EXCLUDE_VULKAN_BACKEND
            case GraphicsBackendType.Vulkan:
                if (Vulkan.VulkanGraphicsDevice.IsSupported())
                {
                    device = new Vulkan.VulkanGraphicsDevice(in description);
                }
                break;
#endif

#if !EXCLUDE_D3D12_BACKEND 
            case GraphicsBackendType.D3D12:
                if (D3D12.D3D12GraphicsDevice.IsSupported())
                {
                    device = new D3D12.D3D12GraphicsDevice(in description);
                }
                break;
#endif

#if !EXCLUDE_METAL_BACKEND
            case GraphicsBackendType.Metal:
                break;
#endif

            default:
            case GraphicsBackendType.Null:
                return new Null.NullGraphicsDevice(in description);
        }

        if (device == null)
        {
            throw new GraphicsException($"{backend} is not supported");
        }

        return device!;
    }

    /// <summary>
    /// Wait for device to finish pending GPU operations.
    /// </summary>
    public abstract void WaitIdle();

    public abstract void FinishFrame();

    protected void AdvanceFrame()
    {
        // Begin new frame
        _frameCount++;
        _frameIndex = (uint)(_frameCount % Constants.MaxFramesInFlight);
    }

    protected void ProcessDeletionQueue()
    {
        while (!_deferredDestroyObjects.IsEmpty)
        {
            if (_deferredDestroyObjects.TryPeek(out Tuple<GraphicsObject, ulong>? item) &&
                item.Item2 + Constants.MaxFramesInFlight < _frameCount)
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

    public abstract bool QueryFeatureSupport(Feature feature);

    public virtual void WriteShadingRateValue(ShadingRate rate, void* dest)
    {

    }

    public GraphicsBuffer CreateBuffer(in BufferDescription description)
    {
        return CreateBuffer(description, null);
    }

    public GraphicsBuffer CreateBuffer(in BufferDescription description, IntPtr initialData)
    {
        return CreateBuffer(description, initialData.ToPointer());
    }

    public GraphicsBuffer CreateBuffer(in BufferDescription description, void* initialData)
    {
        Guard.IsGreaterThanOrEqualTo(description.Size, 4, nameof(BufferDescription.Size));

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

    public GraphicsBuffer CreateBuffer<T>(T[] initialData,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode cpuAccess = CpuAccessMode.None)
        where T : unmanaged
    {
        ReadOnlySpan<T> dataSpan = initialData.AsSpan();

        return CreateBuffer(dataSpan, usage, cpuAccess);
    }

    public GraphicsBuffer CreateBuffer<T>(ReadOnlySpan<T> initialData,
        BufferUsage usage = BufferUsage.ShaderReadWrite,
        CpuAccessMode cpuAccess = CpuAccessMode.None,
        string? label = default)
        where T : unmanaged
    {
        var typeSize = sizeof(T);
        Guard.IsTrue(initialData.Length > 0, nameof(initialData));

        BufferDescription description = new((uint)(initialData.Length * typeSize), usage, cpuAccess, label);
        return CreateBuffer(description, ref MemoryMarshal.GetReference(initialData));
    }

    public Texture CreateTexture(in TextureDescription description)
    {
        Guard.IsGreaterThanOrEqualTo(description.Width, 1, nameof(TextureDescription.Width));
        Guard.IsGreaterThanOrEqualTo(description.Height, 1, nameof(TextureDescription.Height));
        Guard.IsGreaterThanOrEqualTo(description.DepthOrArrayLayers, 1, nameof(TextureDescription.DepthOrArrayLayers));

        return CreateTextureCore(description, default);
    }

    public Sampler CreateSampler(in SamplerDescription description)
    {
        return CreateSamplerCore(description);
    }

    public PipelineLayout CreatePipelineLayout(in PipelineLayoutDescription description)
    {
        return CreatePipelineLayoutCore(in description);
    }

    public Pipeline CreateRenderPipeline(in RenderPipelineDescription description)
    {
        Guard.IsGreaterThanOrEqualTo(description.ShaderStages.Length, 1, nameof(RenderPipelineDescription.ShaderStages));

        return CreateRenderPipelineCore(in description);
    }

    public Pipeline CreateComputePipeline(in ComputePipelineDescription description)
    {
        Guard.IsTrue(description.ComputeShader.Stage == ShaderStages.Compute, nameof(ComputePipelineDescription.ComputeShader));
        Guard.IsNotNull(description.ComputeShader.ByteCode != null, nameof(ComputePipelineDescription.ComputeShader.ByteCode));
        Guard.IsGreaterThan(description.ComputeShader.ByteCode!.Length, 0);

        return CreateComputePipelineCore(in description);
    }

    public QueryHeap CreateQueryHeap(in QueryHeapDescription description)
    {
        return CreateQueryHeapCore(description);
    }

    public SwapChain CreateSwapChain(ISwapChainSurface surface, in SwapChainDescription description)
    {
        Guard.IsNotNull(surface, nameof(surface));

        return CreateSwapChainCore(surface, description);
    }

    /// <summary>
    /// Begin new <see cref="RenderContext"/> in recording state.
    /// </summary>
    /// <param name="label">Optional label.</param>
    /// <returns></returns>
    public abstract RenderContext BeginRenderContext(string? label = default);

    protected abstract GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData);
    protected abstract Texture CreateTextureCore(in TextureDescription description, void* initialData);
    protected abstract Sampler CreateSamplerCore(in SamplerDescription description);
    protected abstract PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description);
    protected abstract Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description);
    protected abstract Pipeline CreateComputePipelineCore(in ComputePipelineDescription description);
    protected abstract QueryHeap CreateQueryHeapCore(in QueryHeapDescription description);
    protected abstract SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description);
}
