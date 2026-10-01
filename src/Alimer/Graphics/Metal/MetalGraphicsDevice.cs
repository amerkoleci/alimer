// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsDevice : GraphicsDevice
{
    private readonly MetalGraphicsAdapter _adapter;
    private readonly GraphicsDeviceLimits _limits;
    private readonly MetalCommandQueue _graphicsQueue;
    private readonly MetalCommandQueue _computeQueue;
    private readonly MetalCommandQueue _copyQueue;

    public MetalGraphicsDevice(MetalGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(GraphicsBackend.Metal, in description)
    {
        _adapter = adapter;
        Handle = adapter.Device;
        _limits = new();
        _graphicsQueue = new(this, CommandQueueType.Graphics);
        _computeQueue = new(this, CommandQueueType.Compute);
        _copyQueue = new(this, CommandQueueType.Copy);
    }

    public MTLDevice Handle { get; }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency => 1_000_000_000;

    protected override void DisposeManagedResources()
    {
        WaitIdle();

        if (Handle.IsNotNull)
        {
            Handle.Dispose();
        }
    }

    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.TextureComponentSwizzle:
                return Handle.SupportsFamily(MTLGPUFamily.Mac2) || Handle.SupportsFamily(MTLGPUFamily.Apple2);

            case Feature.TextureCompressionBC:
                return OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst();

            default:
                return false;
        }
    }

    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format)
    {
        PixelFormatSupport result = PixelFormatSupport.None;
        return result;
    }

    public override bool QueryVertexFormatSupport(VertexAttributeFormat format)
    {
        return true;
    }

    public override CommandQueue? GetCommandQueue(CommandQueueType type)
    {
        return type switch
        {
            CommandQueueType.Graphics => _graphicsQueue,
            CommandQueueType.Compute => _computeQueue,
            CommandQueueType.Copy => _copyQueue,
            _ => null,
        };
    }

    public override void WaitIdle()
    {
    }

    public override ulong CommitFrame()
    {
        AdvanceFrame();
        ProcessDeletionQueue(false);
        return _frameCount;
    }

    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default)
        => throw new NotSupportedException("Metal command encoding is not implemented yet.");

    public override GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type)
    {
        return type switch
        {
            GraphicsNativeHandleType.MTLDevice => new GraphicsNativeHandle(Handle),
            _ => GraphicsNativeHandle.InvalidHandle,
        };
    }

    protected override unsafe GraphicsBuffer CreateBufferCore(in GraphicsBufferDescriptor descriptor, void* initialData) => throw new NotSupportedException("Metal buffer creation is not implemented yet.");
    protected override unsafe Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData) => throw new NotSupportedException("Metal texture creation is not implemented yet.");
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor) => throw new NotSupportedException("Metal sampler creation is not implemented yet.");
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor) => throw new NotSupportedException("Metal shader module creation is not implemented yet.");
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor) => throw new NotSupportedException("Metal render pipeline creation is not implemented yet.");
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor) => throw new NotSupportedException("Metal compute pipeline creation is not implemented yet.");
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor) => throw new NotSupportedException("Metal query heap creation is not implemented yet.");
}
