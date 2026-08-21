// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.Utilities.MarshalUtilities;
using static Alimer.Graphics.Native.AlimerGPUApi;

namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsDevice : GraphicsDevice
{
    private readonly NativeGraphicsAdapter _adapter;
    private readonly GraphicsDeviceLimits _limits;
    private readonly NativeCommandQueue[] _queues = new NativeCommandQueue[(int)CommandQueueType.Count];

    public NativeGraphicsDevice(NativeGraphicsAdapter adapter, GraphicsBackend backend, in GraphicsDeviceDescription description)
        : base(backend, in description)
    {
        _adapter = adapter;
        Handle = agpuAdapterCreateDevice(_adapter.Handle, null);
        agpuDeviceGetLimits(Handle, out GPUDeviceLimits limits);
        TimestampFrequency = agpuDeviceGetTimestampFrequency(Handle);

        // TODO: Align GraphicsDeviceLimits with GPULimits
        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = limits.maxTextureDimension1D,
            MaxTextureDimension2D = limits.maxTextureDimension2D,
            MaxTextureDimension3D = limits.maxTextureDimension3D,
            MaxTextureDimensionCube = limits.maxTextureDimensionCube,
            MaxTextureArrayLayers = limits.maxTextureArrayLayers,
            MinConstantBufferOffsetAlignment = limits.minConstantBufferOffsetAlignment,
        };

        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            CommandQueueType queueType = (CommandQueueType)i;
            GPUCommandQueue queueHandle = agpuDeviceGetCommandQueue(Handle, queueType);
            if (queueHandle.IsNull)
            {
                continue;
            }

            _queues[i] = new NativeCommandQueue(this, queueType, queueHandle);
        }
    }

    public GPUDevice Handle { get; }
    public override GraphicsAdapter Adapter => _adapter;

    public override GraphicsDeviceLimits Limits => _limits;

    public override ulong TimestampFrequency { get; }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _ = agpuDeviceRelease(Handle);
        }
    }


    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default)
    {
        return _queues[(int)queue].AcquireCommandBuffer(label);
    }

    public override ulong CommitFrame()
    {
        return agpuDeviceCommitFrame(Handle);
    }

    public override CommandQueue? GetCommandQueue(CommandQueueType type) => throw new NotImplementedException();
    public override GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type) => throw new NotImplementedException();
    public override bool QueryFeatureSupport(Feature feature) => throw new NotImplementedException();
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) => throw new NotImplementedException();
    public override bool QueryVertexFormatSupport(VertexAttributeFormat format) => throw new NotImplementedException();
    public override void WaitIdle()
    {
        agpuDeviceWaitIdle(Handle);
    }

    protected override GraphicsBuffer CreateBufferCore(in GraphicsBufferDescriptor descriptor, void* initialData) => throw new NotImplementedException();
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor) => throw new NotImplementedException();
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor) => throw new NotImplementedException();
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor) => throw new NotImplementedException();
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData) => new NativeTexture(this, in descriptor, initialData);
}
