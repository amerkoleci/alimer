// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static Alimer.Graphics.D3D12.D3D12MA.ALLOCATOR_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_INDIRECT_ARGUMENT_TYPE;
using static TerraFX.Interop.DirectX.D3D12_MESH_SHADER_TIER;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_ID;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.DirectX.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_DEBUG_RLO_FLAGS;
using static TerraFX.Interop.Windows.Windows;
namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    private readonly D3D12GraphicsAdapter _adapter;
    private readonly ComPtr<ID3D12Device5> _device = default;
    private readonly ComPtr<ID3D12Device8> _device8 = default;
    private readonly ComPtr<ID3D12VideoDevice> _videoDevice;
    private readonly nint _memoryAllocator;

    private readonly ComPtr<ID3D12Fence> _deviceRemovedFence = default;
    private readonly GCHandle _deviceHandle;
    private readonly HANDLE _deviceRemovedEvent = HANDLE.NULL;
    private readonly HANDLE _deviceRemovedWaitHandle = HANDLE.NULL;

    private readonly D3D12CommandQueue[] _queues = new D3D12CommandQueue[(int)QueueType.Count];
    private readonly D3D12CopyAllocator _copyAllocator;
    private readonly ComPtr<ID3D12CommandSignature> _dispatchIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndexedIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _dispatchMeshIndirectCommandSignature = default;

    public D3D12GraphicsDevice(D3D12GraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(description)
    {
        _adapter = adapter;

        HRESULT hr = D3D12CreateDevice(
            (IUnknown*)adapter.Handle,
            D3D_FEATURE_LEVEL_12_0,
            __uuidof<ID3D12Device5>(),
            _device.GetVoidAddressOf()
            );

        if (hr.FAILED)
        {
            throw new GraphicsException("D3D12: Failed to create device");
        }

        _device.CopyTo(_device8.GetAddressOf());

        if (adapter.Manager.ValidationMode != GraphicsValidationMode.Disabled)
        {
            // Configure debug device (if active).
            using ComPtr<ID3D12InfoQueue> infoQueue = default;
            if (_device.CopyTo(infoQueue.GetAddressOf()).SUCCEEDED)
            {
                infoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                infoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

                // These severities should be seen all the time
                uint enabledSeveritiesCount = (adapter.Manager.ValidationMode == GraphicsValidationMode.Verbose) ? 5u : 4u;
                D3D12_MESSAGE_SEVERITY* enabledSeverities = stackalloc D3D12_MESSAGE_SEVERITY[5]
                {
                    D3D12_MESSAGE_SEVERITY_CORRUPTION,
                    D3D12_MESSAGE_SEVERITY_ERROR,
                    D3D12_MESSAGE_SEVERITY_WARNING,
                    D3D12_MESSAGE_SEVERITY_MESSAGE,
                    D3D12_MESSAGE_SEVERITY_INFO
                };

                const int disabledMessagesCount = 12;
                D3D12_MESSAGE_ID* disabledMessages = stackalloc D3D12_MESSAGE_ID[disabledMessagesCount]
                {
                    D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
                    D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
                    D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED,
                    D3D12_MESSAGE_ID_CREATEPIPELINELIBRARY_DRIVERVERSIONMISMATCH,
                    D3D12_MESSAGE_ID_CREATEPIPELINELIBRARY_ADAPTERVERSIONMISMATCH,
                    D3D12_MESSAGE_ID_LOADPIPELINE_NAMENOTFOUND,
                    D3D12_MESSAGE_ID_STOREPIPELINE_DUPLICATENAME
                };

                D3D12_INFO_QUEUE_FILTER filter = new();
                filter.AllowList.NumSeverities = enabledSeveritiesCount;
                filter.AllowList.pSeverityList = enabledSeverities;
                filter.DenyList.NumIDs = disabledMessagesCount;
                filter.DenyList.pIDList = disabledMessages;

                // Clear out the existing filters since we're taking full control of them
                infoQueue.Get()->PushEmptyStorageFilter();

                ThrowIfFailed(infoQueue.Get()->AddStorageFilterEntries(&filter));
            }
        }

        bool supportVideoDevice = false;
        if (_device.CopyTo(_videoDevice.GetAddressOf()).SUCCEEDED)
        {
            supportVideoDevice = true;
        }

        // Create fence to detect device removal
        {
            _deviceRemovedFence = _device.Get()->CreateFence();

            _deviceRemovedEvent = CreateEventW(null, FALSE, FALSE, null);
            ThrowIfFailed(_deviceRemovedFence.Get()->SetEventOnCompletion(UINT64_MAX, _deviceRemovedEvent));

            _deviceHandle = GCHandle.Alloc(this, GCHandleType.Weak);

            HANDLE deviceRemovedWaitHandle = default;
            RegisterWaitForSingleObject(
                &deviceRemovedWaitHandle,
                _deviceRemovedEvent,
                &HandleDeviceRemoved,
                (void*)GCHandle.ToIntPtr(_deviceHandle), // Pass the device as our context
                INFINITE, // No timeout
                0 // No flags
            );
            _deviceRemovedWaitHandle = deviceRemovedWaitHandle;
        }

        // Create memory allocator
        {
            D3D12MA.ALLOCATOR_DESC allocatorDesc = new()
            {
                pDevice = (ID3D12Device*)_device.Get(),
                pAdapter = (IDXGIAdapter*)_adapter.Handle
            };
            //allocatorDesc.PreferredBlockSize = 256 * 1024 * 1024;
            //allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_ALWAYS_COMMITTED;
            allocatorDesc.Flags |= ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
            allocatorDesc.Flags |= ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;

            if (FAILED(D3D12MA.CreateAllocator(in allocatorDesc, out _memoryAllocator)))
            {
                throw new GraphicsException("D3D12: Failed to create memory allocator");
            }
        }

        // Create command queue's
        QueueType supportedQueueCount = supportVideoDevice ? QueueType.Count : QueueType.VideoDecode;
        for (int i = 0; i < (int)supportedQueueCount; i++)
        {
            QueueType queue = (QueueType)i;
            _queues[i] = new D3D12CommandQueue(this, queue);
        }

        // Init CopyAllocator
        _copyAllocator = new D3D12CopyAllocator(this);

        // Init CPU descriptor allocators
        const uint renderTargetViewHeapSize = 1024;
        const uint depthStencilViewHeapSize = 256;

        // Maximum number of CBV/SRV/UAV descriptors in heap for Tier 1
        const uint shaderResourceViewHeapSize = 1_000_000;
        // Maximum number of samplers descriptors in heap for Tier 1
        const uint samplerHeapSize = 2048; // 2048 ->  Tier1 limit

        // CPU visible heaps
        RenderTargetViewHeap = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetViewHeapSize, false);
        DepthStencilViewHeap = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthStencilViewHeapSize, false);

        // Shader visible descriptor heaps
        ShaderResourceViewHeap = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceViewHeapSize, true);
        SamplerHeap = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerHeapSize, true);

        // Create command signatures
        {
            // DispatchIndirectCommand
            D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = new()
            {
                Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH
            };

            D3D12_COMMAND_SIGNATURE_DESC cmdSignatureDesc = new()
            {
                ByteStride = (uint)sizeof(D3D12_DISPATCH_ARGUMENTS),
                NumArgumentDescs = 1,
                pArgumentDescs = &dispatchArg
            };

            _dispatchIndirectCommandSignature = _device.Get()->CreateCommandSignature(&cmdSignatureDesc);

            // DrawIndirectCommand
            D3D12_INDIRECT_ARGUMENT_DESC drawInstancedArg = new()
            {
                Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW
            };

            cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DRAW_ARGUMENTS);
            cmdSignatureDesc.NumArgumentDescs = 1;
            cmdSignatureDesc.pArgumentDescs = &drawInstancedArg;
            _drawIndirectCommandSignature = _device.Get()->CreateCommandSignature(&cmdSignatureDesc);

            // DrawIndexedIndirectCommand
            D3D12_INDIRECT_ARGUMENT_DESC drawIndexedInstancedArg = new()
            {
                Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED
            };

            cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
            cmdSignatureDesc.NumArgumentDescs = 1;
            cmdSignatureDesc.pArgumentDescs = &drawIndexedInstancedArg;
            _drawIndexedIndirectCommandSignature = _device.Get()->CreateCommandSignature(&cmdSignatureDesc);

            if (_adapter.Features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1)
            {
                D3D12_INDIRECT_ARGUMENT_DESC dispatchMeshArg = new();
                dispatchMeshArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

                cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
                cmdSignatureDesc.NumArgumentDescs = 1;
                cmdSignatureDesc.pArgumentDescs = &dispatchMeshArg;
                _dispatchMeshIndirectCommandSignature = _device.Get()->CreateCommandSignature(&cmdSignatureDesc);
            }
        }

        ulong timestampFrequency;
        ThrowIfFailed(D3D12GraphicsQueue->GetTimestampFrequency(&timestampFrequency));
        TimestampFrequency = timestampFrequency;
    }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public ID3D12Device5* Device => _device;
    public ID3D12Device8* Device8 => _device8;
    public nint MemoryAllocator => _memoryAllocator;
    public D3D12GraphicsAdapter DxAdapter => _adapter;
    public bool EnhancedBarriersSupported => _adapter.Features.EnhancedBarriersSupported;

    public ID3D12CommandQueue* D3D12GraphicsQueue => _queues[(int)QueueType.Graphics].Handle;
    public D3D12CommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public D3D12CommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public D3D12CommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public D3D12CommandQueue? VideDecodeQueue => _queues[(int)QueueType.VideoDecode];
    public D3D12CommandQueue? VideoEncode => _queues[(int)QueueType.VideoEncode];

    public D3D12DescriptorAllocator RenderTargetViewHeap { get; }
    public D3D12DescriptorAllocator DepthStencilViewHeap { get; }
    public D3D12DescriptorAllocator ShaderResourceViewHeap { get; }
    public D3D12DescriptorAllocator SamplerHeap { get; }

    public ID3D12CommandSignature* DispatchIndirectCommandSignature => _dispatchIndirectCommandSignature;
    public ID3D12CommandSignature* DrawIndirectCommandSignature => _drawIndirectCommandSignature;
    public ID3D12CommandSignature* DrawIndexedIndirectCommandSignature => _drawIndexedIndirectCommandSignature;
    public ID3D12CommandSignature* DispatchMeshIndirectCommandSignature => _dispatchMeshIndirectCommandSignature;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
    /// </summary>
    ~D3D12GraphicsDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            WaitIdle();
            _shuttingDown = true;

            _copyAllocator.Dispose();

            ProcessDeletionQueue(true);
            _frameCount = 0;
            _frameIndex = 0;

            // Destroy CommandQueue's
            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] == null)
                    continue;

                _queues[i].Dispose();
            }

            RenderTargetViewHeap.Dispose();
            DepthStencilViewHeap.Dispose();
            ShaderResourceViewHeap.Dispose();
            SamplerHeap.Dispose();

            _dispatchIndirectCommandSignature.Dispose();
            _drawIndirectCommandSignature.Dispose();
            _drawIndexedIndirectCommandSignature.Dispose();
            _dispatchMeshIndirectCommandSignature.Dispose();

            // Allocator.
            if (_memoryAllocator != 0)
            {
                D3D12MA.TotalStatistics stats;
                D3D12MA.Allocator_CalculateStatistics(_memoryAllocator, &stats);

                if (stats.Total.Stats.AllocationBytes > 0)
                {
                    Log.Info($"Total device memory leaked: {stats.Total.Stats.AllocationBytes} bytes.");
                }

                _ = D3D12MA.Allocator_Release(_memoryAllocator);
            }

            // Device removed event
            {
                if (UnregisterWait(_deviceRemovedWaitHandle) == S.S_OK &&
                    _deviceHandle.IsAllocated)
                {
                    _deviceHandle.Free();
                }

                CloseHandle(_deviceRemovedEvent);
                _deviceRemovedFence.Dispose();
            }

            _videoDevice.Dispose();
            _device8.Dispose();
#if DEBUG
            uint refCount = _device.Get()->Release();
            if (refCount > 0)
            {
                Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D12DebugDevice> debugDevice = default;

                if (_device.CopyTo(debugDevice.GetAddressOf()).SUCCEEDED)
                {
                    debugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                }
            }
#else
            _device.Dispose();
#endif

#if DEBUG
            using ComPtr<IDXGIDebug1> dxgiDebug = default;
            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIDebug1>(), dxgiDebug.GetVoidAddressOf()).SUCCEEDED)
            {
                dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
            }
#endif
        }
    }

    [UnmanagedCallersOnly]
    private static void HandleDeviceRemoved(void* pContext, byte timedOut)
    {
        GCHandle handle = GCHandle.FromIntPtr((IntPtr)pContext);
        D3D12GraphicsDevice? device = Unsafe.As<D3D12GraphicsDevice>(handle.Target);

        handle.Free();

        device?.OnDeviceRemoved();
    }

    public void OnDeviceRemoved()
    {

    }

    public D3D12UploadContext Allocate(ulong size) => _copyAllocator.Allocate(size);
    public void Submit(in D3D12UploadContext context) => _copyAllocator.Submit(in context);

    /// <inheritdoc />
    public override CommandQueue GetCommandQueue(QueueType type) => _queues[(int)type];

    /// <inheritdoc />
    public override void WaitIdle()
    {
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] == null)
                continue;

            _queues[i].WaitIdle();
        }

        ProcessDeletionQueue(true);
    }

    /// <inheritdoc />
    public override ulong CommitFrame()
    {
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit();
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= MaxFramesInFlight)
        {
            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].WaitIdle();
            }
        }

        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].FinishFrame();
        }

        ProcessDeletionQueue(false);
        return _frameIndex;
    }

    public override void WriteShadingRateValue(ShadingRate rate, void* dest)
    {
        byte d3dRate = (byte)rate.ToD3D12();
        if (!_adapter.Features.AdditionalShadingRatesSupported)
        {
            d3dRate = Math.Min(d3dRate, (byte)D3D12_SHADING_RATE_2X2);
        }
        *(byte*)dest = d3dRate;
    }

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescription description, void* initialData)
    {
        return new D3D12Buffer(this, description, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescription description, TextureData* initialData)
    {
        return new D3D12Texture(this, description, initialData);
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescription description)
    {
        return new D3D12Sampler(this, description);
    }

    /// <inheritdoc />
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description)
    {
        return new D3D12BindGroupLayout(this, description);
    }

    /// <inheritdoc />
    protected override BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description)
    {
        return new D3D12BindGroup(this, layout, description);
    }

    /// <inheritdoc />
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description)
    {
        return new D3D12PipelineLayout(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description)
    {
        return new D3D12Pipeline(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        return new D3D12Pipeline(this, description);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor description)
    {
        return new D3D12QueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription description)
    {
        return new D3D12SwapChain(this, surface, description);
    }

    /// <inheritdoc />
    public override RenderContext BeginRenderContext(string? label = null)
    {
        return _queues[(int)QueueType.Graphics].BeginCommandContext(label);
    }
}
