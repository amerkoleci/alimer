// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics.VGPU;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.VGPU.VGPU;
using static Alimer.Graphics.VGPU.VGPUUtils;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics;

public sealed unsafe class GraphicsDevice : GraphicsObjectBase
{
    private static VGPULogCallback _logCallback;
    private readonly HashSet<WeakReference<GraphicsObject?>> _resources = new();

    static GraphicsDevice()
    {
        _logCallback = OnLogNative;
        vgpuSetLogCallback(_logCallback, 0);
    }

    public GraphicsDevice(in GraphicsDeviceDescription description)
    {
        VGPUDeviceDescriptor deviceDesc = new()
        {
            //deviceDesc.label = "test device";
            preferredBackend = description.PreferredBackend.ToVGPU(),
            validationMode = description.ValidationMode.ToVGPU(),
            powerPreference = description.PowerPreference.ToVGPU(),
        };

        Handle = vgpuCreateDevice(&deviceDesc);
        Guard.IsTrue(Handle.IsNotNull);

        Backend = vgpuDeviceGetBackend(Handle).FromVGPU();

        VGPUAdapterProperties adapterProps;
        vgpuDeviceGetAdapterProperties(Handle, &adapterProps);
        AdapterProperties = GraphicsAdapterProperties.FromVGPU(in adapterProps);

        VGPULimits limits;
        vgpuDeviceGetLimits(Handle, &limits);
    }

    internal VGPUDevice Handle { get; }

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
    public GraphicsAdapterProperties AdapterProperties { get; }

    /// <summary>
    /// Get the device limits.
    /// </summary>
    //public GraphicsDeviceLimits Limits { get; }

    /// <summary>
    /// Get the timestamp frequency.
    /// </summary>
    public ulong TimestampFrequency { get; }

    /// <summary>
    /// Gets the number of frame being executed.
    /// </summary>
    public ulong FrameCount => vgpuDeviceGetFrameCount(Handle);

    /// <summary>
    /// Gets the current frame inde.
    /// </summary>
    public uint FrameIndex => vgpuDeviceGetFrameIndex(Handle);

    /// <summary>
    /// Finalizes an instance of the <see cref="GraphicsDevice" /> class.
    /// </summary>
    ~GraphicsDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            WaitIdle();

            lock (_resources)
            {
                foreach (WeakReference<GraphicsObject?> weakReference in _resources)
                {
                    if (weakReference.TryGetTarget(out GraphicsObject? target))
                    {
                        target.Dispose();
                    }
                }
                _resources.Clear();
            }


            _ = vgpuDeviceRelease(Handle);
        }
    }

    /// <summary>
    /// Wait for device to finish pending GPU operations.
    /// </summary>
    public void WaitIdle() => vgpuDeviceWaitIdle(Handle);

    public bool QueryFeatureSupport(Feature feature)
    {
        return vgpuDeviceQueryFeatureSupport(Handle, (VGPUFeature)feature);
    }

    /// <summary>
    /// Begin new <see cref="CommandBuffer"/> in recording state.
    /// </summary>
    /// <param name="label">Optional label.</param>
    /// <returns></returns>
    public CommandBuffer BeginCommandBuffer(CommandQueue queueType = CommandQueue.Graphics, string? label = default)
    {
        fixed (sbyte* pLabel = label.GetUtf8Span())
        {
            VGPUCommandQueue nativeQueueType = (VGPUCommandQueue)queueType;
            VGPUCommandBuffer handle = vgpuBeginCommandBuffer(Handle, nativeQueueType, pLabel);
            return new CommandBuffer(this, handle);
        }
    }

    public void Submit(CommandBuffer commandBuffer)
    {
        VGPUCommandBuffer cmdBufferHandle = commandBuffer.Handle;
        vgpuDeviceSubmit(Handle, &cmdBufferHandle, 1u);
    }

    public void Submit(CommandBuffer[] commandBuffers)
    {
        VGPUCommandBuffer* commandBufferPtrs = stackalloc VGPUCommandBuffer[commandBuffers.Length];

        for (int i = 0; i < commandBuffers.Length; i += 1)
        {
            commandBufferPtrs[i] = commandBuffers[i].Handle;
        }

        vgpuDeviceSubmit(Handle, commandBufferPtrs, (uint)commandBuffers.Length);
    }

    internal void AddResourceReference(WeakReference<GraphicsObject?> @object)
    {
        lock (_resources)
        {
            _resources.Add(@object);
        }
    }

    internal void RemoveResourceReference(WeakReference<GraphicsObject?> @object)
    {
        lock (_resources)
        {
            _resources.Remove(@object);
        }
    }

    public static bool IsBackendSupport(GraphicsBackendType backend)
    {
        Guard.IsTrue(backend != GraphicsBackendType.Count, nameof(backend), "Invalid backend");

        return vgpuIsBackendSupported(backend.ToVGPU());
    }

#if NET6_0_OR_GREATER
    //[UnmanagedCallersOnly]
#else
    [MonoPInvokeCallback(typeof(VGPULogCallbackDelegate))]
#endif
    private static void OnLogNative(VGPULogLevel level, sbyte* pMessage, nint userData)
    {
        string message = GetUtf8Span(pMessage).GetString()!;

        switch (level)
        {
            case VGPULogLevel.Error:
                Log.Error(message);
                break;
            case VGPULogLevel.Warning:
                Log.Warn(message);
                break;
            case VGPULogLevel.Info:
                Log.Info(message);
                break;
        }
    }
}
