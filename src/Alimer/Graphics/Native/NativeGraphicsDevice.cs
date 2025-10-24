// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsDevice : GraphicsDevice
{
    private readonly NativeGraphicsAdapter _adapter;

    public NativeGraphicsDevice(NativeGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(description)
    {
        _adapter = adapter;
        Handle = agpuCreateDevice(adapter.Handle, null);
        TimestampFrequency = agpuDeviceGetTimestampFrequency(Handle);
    }

    public GPUDevice Handle { get; }

    public override GraphicsAdapter Adapter => _adapter;

    public override ulong TimestampFrequency { get; }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _ = agpuDeviceRelease(Handle);
        }
    }

    public override RenderContext BeginRenderContext(Utf8ReadOnlyString label = default) => throw new NotImplementedException();

    public override ulong CommitFrame()
    {
        ulong frameCount = agpuDeviceCommitFrame(Handle);
        return frameCount;
    }

    public override CommandQueue GetCommandQueue(CommandQueueType type) => throw new NotImplementedException();

    public override void WaitIdle()
    {
        agpuDeviceWaitIdle(Handle);
    }

    protected override BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description) => throw new NotImplementedException();
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description) => throw new NotImplementedException();
    protected override unsafe GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData) => throw new NotImplementedException();
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description) => throw new NotImplementedException();
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description) => throw new NotImplementedException();
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription descriptor) => throw new NotImplementedException();
    protected override Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description) => throw new NotImplementedException();
    protected override Sampler CreateSamplerCore(in SamplerDescription description) => throw new NotImplementedException();
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description) => new NativeSwapChain(this, surface, description);
    protected override unsafe Texture CreateTextureCore(in TextureDescription description, TextureData* initialData) => new NativeTexture(this, description, initialData);

}
