// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.DirectX.D3D_SHADER_MODEL;
using static TerraFX.Interop.DirectX.D3D12_FORMAT_SUPPORT1;
using static TerraFX.Interop.DirectX.D3D12_FORMAT_SUPPORT2;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BINDING_TIER;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_TIER;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_INDIRECT_ARGUMENT_TYPE;
using static TerraFX.Interop.DirectX.D3D12_TILED_RESOURCES_TIER;
using static TerraFX.Interop.DirectX.D3D12_MESH_SHADER_TIER;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_ID;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.DirectX.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_RAYTRACING_TIER;
using static TerraFX.Interop.DirectX.D3D12_VARIABLE_SHADING_RATE_TIER;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_CALLBACK_FLAGS;
using static TerraFX.Interop.Windows.Windows;
using static Alimer.Graphics.D3D12.D3D12MA.ALLOCATOR_FLAGS;
using Alimer.Utilities;
namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    private readonly D3D12GraphicsAdapter _adapter;
    private readonly ComPtr<ID3D12Device5> _device = default;
    private readonly ComPtr<ID3D12Device8> _device8 = default;
    private readonly ComPtr<ID3D12VideoDevice> _videoDevice;
    private readonly nint _memoryAllocator;
    private readonly GraphicsDeviceLimits _limits;

    private readonly ComPtr<ID3D12Fence> _deviceRemovedFence = default;
    private readonly GCHandle _deviceHandle;
    private readonly HANDLE _deviceRemovedEvent = HANDLE.NULL;
    private readonly HANDLE _deviceRemovedWaitHandle = HANDLE.NULL;
    private uint _callbackCookie;

    private readonly D3D12CommandQueue[] _queues = new D3D12CommandQueue[(int)CommandQueueType.Count];
    private readonly D3D12CopyAllocator _copyAllocator;
    private readonly ComPtr<ID3D12CommandSignature> _dispatchIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndexedIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _dispatchMeshIndirectCommandSignature = default;

    private readonly nint _winPixEventRuntimeDLL;

    public D3D12GraphicsDevice(D3D12GraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.D3D12, in description)
    {
        _adapter = adapter;

        HRESULT hr = D3D12CreateDevice(
            (IUnknown*)adapter.Handle,
            D3D_FEATURE_LEVEL_12_0,
            __uuidof<ID3D12Device5>(),
            (void**)_device.GetAddressOf()
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

            using ComPtr<ID3D12InfoQueue1> infoQueue1 = default;
            if (_device.CopyTo(infoQueue1.GetAddressOf()).SUCCEEDED)
            {
                uint callbackCookie = default;
                ThrowIfFailed(infoQueue1.Get()->RegisterMessageCallback(
                    &DebugMessageCallback,
                    D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                    null,
                    &callbackCookie
                    ));
                _callbackCookie = callbackCookie;
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

            if (FAILED(D3D12MA.CreateAllocator(&allocatorDesc, out _memoryAllocator)))
            {
                throw new GraphicsException("D3D12: Failed to create memory allocator");
            }
        }

        // Create command queue's
        CommandQueueType supportedQueueCount = supportVideoDevice ? CommandQueueType.Count : CommandQueueType.VideoDecode;
        for (int i = 0; i < (int)supportedQueueCount; i++)
        {
            CommandQueueType queue = (CommandQueueType)i;
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

        // Try to load PIX (WinPixEventRuntime.dll)
        if (NativeLibrary.TryLoad("WinPixEventRuntime.dll", out _winPixEventRuntimeDLL))
        {
            //PIXBeginEventOnCommandList = (PFN_PIXBeginEventOnCommandList)NativeLibrary.GetExport(_winPixEventRuntimeDLL, "PIXBeginEventOnCommandList");
            //PIXEndEventOnCommandList = (PFN_PIXEndEventOnCommandList)NativeLibrary.GetExport(_winPixEventRuntimeDLL, "PIXEndEventOnCommandList");
            //PIXSetMarkerOnCommandList = (PFN_PIXSetMarkerOnCommandList)NativeLibrary.GetExport(_winPixEventRuntimeDLL, "PIXSetMarkerOnCommandList");
        }



        // https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
        // In DWORDS. Descriptor tables cost 1, Root constants cost 1, Root descriptors cost 2.
        const uint kMaxRootSignatureSize = 64u;

        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION,
            MaxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
            MaxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
            MaxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION,
            MaxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
            MaxBindGroups = kMaxRootSignatureSize,
            //MaxTexelBufferDimension2D = (1u << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1,
            //UploadBufferTextureRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT,
            //UploadBufferTextureSliceAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,
            MinConstantBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
            MaxConstantBufferBindingSize = D3D12_REQ_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT * 16,
            MinStorageBufferOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT,
            MaxStorageBufferBindingSize = (1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1,

            TextureRowPitchAlignment = _adapter.Features.UnrestrictedBufferTextureCopyPitchSupported ? 1u : D3D12_TEXTURE_DATA_PITCH_ALIGNMENT,
            TextureDepthPitchAlignment = _adapter.Features.UnrestrictedBufferTextureCopyPitchSupported ? 1u : D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,

            MaxBufferSize = D3D12_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM * 1024ul * 1024ul,
            MaxPushConstantsSize = sizeof(uint) * kMaxRootSignatureSize / 1,

            MaxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT,
            MaxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,

            // Slot values can be 0-15, inclusive:
            // https://docs.microsoft.com/en-ca/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
            MaxVertexBuffers = 16,
            MaxVertexAttributes = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
            MaxVertexBufferArrayStride = D3D12_SO_BUFFER_MAX_STRIDE_IN_BYTES,

            // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
            // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
            // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
            MaxComputeWorkgroupStorageSize = 32768,

            MaxComputeInvocationsPerWorkGroup = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP,

            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
            MaxComputeWorkGroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X,
            MaxComputeWorkGroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y,
            MaxComputeWorkGroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z,
            // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
            MaxComputeWorkGroupsPerDimension = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
        };

        if (_adapter.Features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2)
        {
            _limits.VariableRateShadingTileSize = _adapter.Features.ShadingRateImageTileSize;
        }

        if (_adapter.Features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0)
        {
            _limits.RayTracingShaderGroupIdentifierSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
            _limits.RayTracingShaderTableAligment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            _limits.RayTracingShaderTableMaxStride = ulong.MaxValue;
            _limits.RayTracingShaderRecursionMaxDepth = D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH;
            _limits.RayTracingMaxGeometryCount = (1 << 24) - 1;
        }

        uint MaxNonSamplerDescriptors = 0;
        uint MaxSamplerDescriptors = 0;
        D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19 = default;
        if (FAILED(_device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, ref options19)))
        {
            if (_adapter.Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            }
            else if (_adapter.Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_2)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            else
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            MaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        }
        else
        {
            MaxNonSamplerDescriptors = options19.MaxViewDescriptorHeapSize;
            MaxSamplerDescriptors = options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers;
        }

        ulong timestampFrequency;
        ThrowIfFailed(D3D12GraphicsQueue.Handle->GetTimestampFrequency(&timestampFrequency));
        TimestampFrequency = timestampFrequency;
    }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public ID3D12Device5* Device => _device;
    public ID3D12Device8* Device8 => _device8;
    public nint MemoryAllocator => _memoryAllocator;
    public D3D12GraphicsAdapter DxAdapter => _adapter;
    public bool EnhancedBarriersSupported => _adapter.Features.EnhancedBarriersSupported;

    //public ID3D12CommandQueue* D3D12GraphicsQueue => _queues[(int)CommandQueueType.Graphics].Handle;
    public D3D12CommandQueue D3D12GraphicsQueue => _queues[(int)CommandQueueType.Graphics];
    public D3D12CommandQueue D3D12ComputeQueue => _queues[(int)CommandQueueType.Compute];
    public D3D12CommandQueue D3D12CopyQueue => _queues[(int)CommandQueueType.Copy];
    public D3D12CommandQueue? D3D12VideoDecodeQueue => _queues[(int)CommandQueueType.VideoDecode];
    //public D3D12CommandQueue? VideoEncode => _queues[(int)CommandQueueType.VideoEncode];

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
            for (int i = 0; i < (int)CommandQueueType.Count; i++)
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
            if (UnregisterWait(_deviceRemovedWaitHandle) == S.S_OK &&
                _deviceHandle.IsAllocated)
            {
                _deviceHandle.Free();
            }

            CloseHandle(_deviceRemovedEvent);
            _deviceRemovedFence.Dispose();

            if (_callbackCookie != 0)
            {
                using ComPtr<ID3D12InfoQueue1> infoQueue1 = default;
                ThrowIfFailed(_device.CopyTo(infoQueue1.GetAddressOf()));
                infoQueue1.Get()->UnregisterMessageCallback(_callbackCookie);
                _callbackCookie = 0;
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

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            // Always supported features
            case Feature.Depth32FloatStencil8:
            case Feature.TimestampQuery:
            case Feature.PipelineStatisticsQuery:
            case Feature.TextureCompressionBC:
            case Feature.IndirectFirstInstance:
            case Feature.SamplerClampToBorder:
            case Feature.SamplerMirrorClampToEdge:
            case Feature.DepthResolveMinMax:
            case Feature.StencilResolveMinMax:
            case Feature.Predication:
                return true;

            // Always unsupported features
            case Feature.TextureCompressionETC2:
            case Feature.TextureCompressionASTC:
            case Feature.TextureCompressionASTC_HDR:
                return false;

            case Feature.ShaderFloat16:
                //const bool supportsDP4a = d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_4;
                return _adapter.Features.HighestShaderModel >= D3D_SHADER_MODEL_6_2 && _adapter.Features.Native16BitShaderOpsSupported;

            case Feature.RG11B10UfloatRenderable:
                return true;

            case Feature.BGRA8UnormStorage:
                D3D12_FEATURE_DATA_FORMAT_SUPPORT bgra8unormFormatInfo = default;
                bgra8unormFormatInfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                HRESULT hr = _device.Get()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &bgra8unormFormatInfo, (uint)sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
                if (hr.SUCCEEDED &&
                    (bgra8unormFormatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0)
                {
                    return true;
                }
                return false;

            case Feature.DepthBoundsTest:
                return _adapter.Features.DepthBoundsTestSupported;

            case Feature.SamplerMinMax:
                if (_adapter.Features.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
                {
                    // Tier 2 for tiled resources
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                }

                return (_adapter.Features.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_1);

            case Feature.ConservativeRasterization:
                return _adapter.Features.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;

            case Feature.CacheCoherentUMA:
                return _adapter.Features.CacheCoherentUMA();

            case Feature.DescriptorIndexing:
                return true;

            case Feature.VariableRateShading:
                return _adapter.Features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1;

            case Feature.VariableRateShadingTier2:
                return _adapter.Features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2;

            case Feature.RayTracing:
                return _adapter.Features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;

            case Feature.RayTracingTier2:
                return _adapter.Features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1;

            case Feature.MeshShader:
                return _adapter.Features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;

            default:
                return false;
        }
    }

    /// <inheritdoc />
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format)
    {
        // TODO:
        PixelFormatSupport result = PixelFormatSupport.None;
        DXGI_FORMAT dxgiFormat = (DXGI_FORMAT)format.ToDxgiFormat();
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return result;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData = new()
        {
            Format = dxgiFormat
        };
        HRESULT hr = _device.Get()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, ref featureData);
        if (FAILED(hr))
            return result;

        if (featureData.Support1.HasFlag(D3D12_FORMAT_SUPPORT1_TEXTURE1D | D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_TEXTURE3D | D3D12_FORMAT_SUPPORT1_TEXTURECUBE))
            result |= PixelFormatSupport.Texture;
        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) != 0)
            result |= PixelFormatSupport.DepthStencil;
        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) != 0)
            result |= PixelFormatSupport.RenderTarget;
        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE) != 0)
            result |= PixelFormatSupport.Blendable;

        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) != 0)
            result |= PixelFormatSupport.ShaderLoad;
        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0)
            result |= PixelFormatSupport.ShaderSample;
        if ((featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD) != 0)
            result |= PixelFormatSupport.ShaderAtomic;
        if ((featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            result |= PixelFormatSupport.ShaderUavLoad;
        if ((featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0)
            result |= PixelFormatSupport.ShaderUavStore;

        TextureSampleCount supportedSampleCount = TextureSampleCount.Count1;
        for (uint sampleCount = 1; sampleCount <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount *= 2)
        {
            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS featureDataQualityLevels = new()
            {
                Format = dxgiFormat,
                SampleCount = sampleCount
            };

            hr = _device.Get()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, ref featureDataQualityLevels);
            if (SUCCEEDED(hr) && featureDataQualityLevels.NumQualityLevels > 0)
                supportedSampleCount |= (TextureSampleCount)sampleCount;
        }

        return result;
    }

    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        DXGI_FORMAT dxgiFormat = format.ToDxgiFormat();
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return false;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData = new()
        {
            Format = dxgiFormat
        };
        HRESULT hr = _device.Get()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, ref featureData);
        if (FAILED(hr))
            return false;

        if ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER) != 0)
            return true;

        return false;
    }

    public D3D12UploadContext Allocate(ulong size) => _copyAllocator.Allocate(size);
    public void Submit(in D3D12UploadContext context) => _copyAllocator.Submit(in context);

    /// <inheritdoc />
    public override CommandQueue GetCommandQueue(CommandQueueType type) => _queues[(int)type];

    /// <inheritdoc />
    public override void WaitIdle()
    {
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
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
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit();
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= MaxFramesInFlight)
        {
            for (int i = 0; i < (int)CommandQueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].WaitIdle();
            }
        }

        for (int i = 0; i < (int)CommandQueueType.Count; i++)
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
    protected override GraphicsBuffer CreateBufferCore(in BufferDescriptor descriptor, void* initialData)
    {
        return new D3D12Buffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData)
    {
        return new D3D12Texture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescriptor description)
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
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor)
    {
        return new D3D12ShaderModule(this, descriptor);
    }

    /// <inheritdoc />
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor)
    {
        return new D3D12RenderPipeline(this, descriptor);
    }

    /// <inheritdoc />
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor descriptor)
    {
        return new D3D12ComputePipeline(this, descriptor);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor)
    {
        return new D3D12QueryHeap(this, descriptor);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor)
    {
        return new D3D12SwapChain(this, descriptor);
    }

    /// <inheritdoc />
    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default)
    {
        return _queues[(int)queue].AcquireCommandBuffer(label);
    }

    [UnmanagedCallersOnly]
    static void DebugMessageCallback(
        D3D12_MESSAGE_CATEGORY Category,
        D3D12_MESSAGE_SEVERITY Severity,
        D3D12_MESSAGE_ID ID,
        sbyte* pDescription,
        void* pContext)
    {
        string message = MarshalUtilities.GetUtf8Span(pDescription).GetString()!;
        if (Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION
            || Severity == D3D12_MESSAGE_SEVERITY_ERROR)
        {
            Log.Error($"[D3D12]: {message}");
        }
        else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
        {
            Log.Warn($"[D3D12]: {message}");
        }
        else
        {
            Log.Info($"[D3D12]: {message}");
        }
    }
}
