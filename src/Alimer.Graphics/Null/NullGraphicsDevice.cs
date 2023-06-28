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
    public override bool QueryFeature(Feature feature)
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
    protected override unsafe GraphicsBuffer CreateBufferCore(in BufferDescription descriptor, void* initialData)
    {
        return new NullBuffer(this, descriptor);
    }

    /// <inheritdoc />
    protected override unsafe Texture CreateTextureCore(in TextureDescriptor descriptor, void* initialData)
    {
        return new NullTexture(this, descriptor);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
    {
        return new NullQueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(SwapChainSurface surface, in SwapChainDescriptor descriptor)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    public override CommandBuffer BeginCommandBuffer(CommandQueue queue, string? label = null)
    {
        throw new NotImplementedException();
    }
}
