// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Null;

internal class NullGraphicsDevice : GraphicsDevice
{
    public NullGraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.Null, description)
    {
    }

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => default;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => default;

    /// <inheritdoc />
    public override ulong TimestampFrequency => 0;


    /// <summary>
    /// Finalizes an instance of the <see cref="NullGraphicsDevice" /> class.
    /// </summary>
    ~NullGraphicsDevice() => Dispose(isDisposing: false);

    protected override void Dispose(bool isDisposing)
    {
        if (isDisposing)
        {
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        return false;
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
    }

    /// <inheritdoc />
    public override void FinishFrame()
    {
        AdvanceFrame();
    }

    /// <inheritdoc />
    protected override unsafe GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData)
    {
        return new NullBuffer(this, description);
    }

    /// <inheritdoc />
    protected override unsafe Texture CreateTextureCore(in TextureDescription description, void* initialData)
    {
        return new NullTexture(this, description);
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescription description)
    {
        return new NullSampler(this, description);
    }

    /// <inheritdoc />
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
    {
        return new NullQueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    public override RenderContext BeginRenderContext(string? label = default)
    {
        throw new NotImplementedException();
    }
}
