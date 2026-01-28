// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsDevice : GraphicsDevice
{
    private readonly MetalGraphicsAdapter _adapter;
    private readonly GraphicsDeviceLimits _limits;

    public MetalGraphicsDevice(MetalGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(GraphicsBackend.Metal, in description)
    {
        _adapter = adapter;
        Device = adapter.Device;
    }

    public MTLDevice Device { get; }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.TextureComponentSwizzle:
                return Device.SupportsFamily(MTLGPUFamily.Mac2) || Device.SupportsFamily(MTLGPUFamily.Apple2);

            default:
                return false;
        }
    }
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format)
    {
        PixelFormatSupport result = PixelFormatSupport.None;
        return result;
    }

    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        return true;
    }

    public override CommandQueue? GetCommandQueue(CommandQueueType type) => throw new NotImplementedException();
    public override void WaitIdle() => throw new NotImplementedException();
    public override ulong CommitFrame() => throw new NotImplementedException();
    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default) => throw new NotImplementedException();
    protected override unsafe GraphicsBuffer CreateBufferCore(in BufferDescriptor descriptor, void* initialData) => throw new NotImplementedException();
    protected override unsafe Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData) => throw new NotImplementedException();
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor) => throw new NotImplementedException();
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescriptor descridescriptorption) => throw new NotImplementedException();
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescriptor descriptor) => throw new NotImplementedException();
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor) => throw new NotImplementedException();
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor) => throw new NotImplementedException();
    protected override SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor) => throw new NotImplementedException();
    protected override void Dispose(bool disposing) => throw new NotImplementedException();
}
