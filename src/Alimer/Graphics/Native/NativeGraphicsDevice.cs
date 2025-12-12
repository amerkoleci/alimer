// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsDevice : GraphicsDevice
{
    private readonly NativeGraphicsAdapter _adapter;
    private readonly NativeCommandQueue?[] _queues = new NativeCommandQueue[(int)CommandQueueType.Count];

    public NativeGraphicsDevice(NativeGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(description)
    {
        _adapter = adapter;
        Handle = agpuCreateDevice(adapter.Handle, null);
        TimestampFrequency = agpuDeviceGetTimestampFrequency(Handle);

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

    public override ulong TimestampFrequency { get; }
    public NativeGraphicsAdapter NativeAdapter => _adapter;
    public GPUFactory Factory => _adapter.Factory.Handle;

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _ = agpuDeviceRelease(Handle);
        }
    }


    public override ulong CommitFrame()
    {
        ulong frameCount = agpuDeviceCommitFrame(Handle);
        return frameCount;
    }

    public override CommandQueue? GetCommandQueue(CommandQueueType type) => _queues[(int)type];

    public override void WaitIdle()
    {
        agpuDeviceWaitIdle(Handle);
    }

    /// <inheritdoc />
    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default)
    {
        return _queues[(int)queue].AcquireCommandBuffer(label);
    }

    protected override BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description) => throw new NotImplementedException();
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description) => throw new NotImplementedException();
    protected override unsafe GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData) => throw new NotImplementedException();
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor description) => throw new NotImplementedException();
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description) => throw new NotImplementedException();
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription descriptor) => throw new NotImplementedException();
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor) => throw new NotImplementedException();
    protected override Sampler CreateSamplerCore(in SamplerDescription description) => throw new NotImplementedException();
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description) => new NativeSwapChain(this, surface, description);
    protected override unsafe Texture CreateTextureCore(in TextureDescription description, TextureData* initialData) => new NativeTexture(this, description, initialData);
    
}
