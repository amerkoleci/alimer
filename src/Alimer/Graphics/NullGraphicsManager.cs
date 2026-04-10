// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

internal class NullGraphicsManager : GraphicsManager
{
    private readonly NullGraphicsAdapter[] _adapters;

    public NullGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        _adapters = [new NullGraphicsAdapter(this)];
    }

    public override ReadOnlySpan<GPUAdapter> Adapters => _adapters;
}

internal class NullGraphicsAdapter : GPUAdapter
{
    public NullGraphicsAdapter(GraphicsManager manager)
        : base(manager)
    {
        DeviceName = "Null Graphics Adapter";
        VendorId = 0;
        DeviceId = 0;
        Type = GraphicsAdapterType.Cpu;
    }

    public override string DeviceName { get; }

    public override uint VendorId { get; }

    public override uint DeviceId { get; }

    public override GraphicsAdapterType Type { get; }

    protected override GPUDevice CreateDeviceCore(in GraphicsDeviceDescription description)
    {
        return new NullGraphicsDevice(this, description);
    }
}

internal class NullGraphicsDevice : GPUDevice
{
    private readonly GPUDeviceLimits _limits;

    public NullGraphicsDevice(NullGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(GraphicsBackend.Null, in description)
    {
        Adapter = adapter;
        _limits = new();
    }

    public override GPUAdapter Adapter { get; }

    public override GPUDeviceLimits Limits => _limits;

    public override ulong TimestampFrequency => 0;

    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default) => throw new NotImplementedException();
    public override ulong CommitFrame()
    {
        AdvanceFrame();
        ProcessDeletionQueue(false);
        return _frameCount;
    }

    public override CommandQueue? GetCommandQueue(CommandQueueType type) => throw new NotImplementedException();
    public override bool QueryFeatureSupport(Feature feature) => throw new NotImplementedException();
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) => throw new NotImplementedException();
    public override bool QueryVertexFormatSupport(VertexAttributeFormat format) => throw new NotImplementedException();
    public override void WaitIdle() => throw new NotImplementedException();
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescriptor descriptor) => throw new NotImplementedException();
    protected override unsafe GPUBuffer CreateBufferCore(in GPUBufferDescriptor descriptor, void* initialData) => throw new NotImplementedException();
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescriptor descriptor) => throw new NotImplementedException();
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor) => throw new NotImplementedException();
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor) => throw new NotImplementedException();
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor) => throw new NotImplementedException();
    protected override SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor) => throw new NotImplementedException();
    protected override unsafe Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData) => throw new NotImplementedException();
}
