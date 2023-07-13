// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.DirectX.D3D12_GPU_BASED_VALIDATION_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_DRED_ENABLEMENT;
using static TerraFX.Interop.DirectX.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.DirectX.D3D12_MESSAGE_ID;
using static TerraFX.Interop.DirectX.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.DXGI_GPU_PREFERENCE;
using static TerraFX.Interop.DirectX.D3D_SHADER_MODEL;
using static TerraFX.Interop.DirectX.D3D12_INDIRECT_ARGUMENT_TYPE;
using static TerraFX.Interop.DirectX.DXGI_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DirectX.DXGI_DEBUG_RLO_FLAGS;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_FORMAT_SUPPORT1;
using static TerraFX.Interop.DirectX.D3D12_TILED_RESOURCES_TIER;
using static TerraFX.Interop.DirectX.D3D12_VARIABLE_SHADING_RATE_TIER;
using static TerraFX.Interop.DirectX.D3D12_RAYTRACING_TIER;
using static TerraFX.Interop.DirectX.D3D12_MESH_SHADER_TIER;
using static TerraFX.Interop.DirectX.D3D12MemAlloc;
using static TerraFX.Interop.DirectX.D3D12MA_ALLOCATOR_FLAGS;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly ComPtr<IDXGIFactory6> _factory;
    private readonly ComPtr<IDXGIAdapter1> _adapter;
    private readonly ComPtr<ID3D12Device5> _handle = default;
    private readonly ComPtr<ID3D12VideoDevice> _videoDevice;
    private readonly ComPtr<D3D12MA_Allocator> _memoryAllocator;

    private readonly ComPtr<ID3D12Fence> _deviceRemovedFence = default;
    private readonly GCHandle _deviceHandle;
    private readonly HANDLE _deviceRemovedEvent = HANDLE.NULL;
    private readonly HANDLE _deviceRemovedWaitHandle = HANDLE.NULL;

    private readonly D3D12Features _features = default;
    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;

    private readonly D3D12CommandQueue[] _queues = new D3D12CommandQueue[(int)QueueType.Count];
    private readonly D3D12DescriptorAllocator[] _descriptorAllocators = new D3D12DescriptorAllocator[(int)D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    private readonly ComPtr<ID3D12DescriptorHeap> _shaderVisibleResourceHeap;
    private readonly D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandleShaderVisibleResource = default;
    private readonly D3D12_GPU_DESCRIPTOR_HANDLE _startGpuHandleShaderVisibleResource = default;
    private readonly ComPtr<ID3D12DescriptorHeap> _shaderVisibleSamplerHeap;
    private readonly D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandleShaderVisibleSampler = default;
    private readonly D3D12_GPU_DESCRIPTOR_HANDLE _startGpuHandleShaderVisibleSampler = default;

    private readonly D3D12CopyAllocator _copyAllocator;


    private readonly ComPtr<ID3D12CommandSignature> _dispatchIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _drawIndexedIndirectCommandSignature = default;
    private readonly ComPtr<ID3D12CommandSignature> _dispatchMeshIndirectCommandSignature = default;

    public static bool IsSupported() => s_isSupported.Value;

    public D3D12GraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.D3D12, description)
    {
        Guard.IsTrue(IsSupported(), nameof(D3D12GraphicsDevice), "Direct3D12 is not supported");

        uint dxgiFactoryFlags = 0u;

        if (ValidationMode != ValidationMode.Disabled)
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            using ComPtr<ID3D12Debug> d3d12Debug = default;
            if (D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), d3d12Debug.GetVoidAddressOf()).SUCCEEDED)
            {
                d3d12Debug.Get()->EnableDebugLayer();

                if (ValidationMode == ValidationMode.GPU)
                {
                    using ComPtr<ID3D12Debug1> d3d12Debug1 = default;
                    using ComPtr<ID3D12Debug2> d3d12Debug2 = default;

                    if (d3d12Debug.CopyTo(d3d12Debug1.GetAddressOf()).SUCCEEDED)
                    {
                        d3d12Debug1.Get()->SetEnableGPUBasedValidation(true);
                        d3d12Debug1.Get()->SetEnableSynchronizedCommandQueueValidation(true);
                    }

                    if (d3d12Debug.CopyTo(d3d12Debug2.GetAddressOf()).SUCCEEDED)
                    {
                        d3d12Debug2.Get()->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                    }
                }
            }
            else
            {
                Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
            }

            // DRED
            {
                using ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> pDredSettings = default;
                if (D3D12GetDebugInterface(__uuidof<ID3D12DeviceRemovedExtendedDataSettings1>(), pDredSettings.GetVoidAddressOf()).SUCCEEDED)
                {
                    // Turn on auto - breadcrumbs and page fault reporting.
                    pDredSettings.Get()->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
            }

#if DEBUG
            using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf()).SUCCEEDED)
            {
                dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                int* hide = stackalloc int[1]
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };

                DXGI_INFO_QUEUE_FILTER filter = new()
                {
                    DenyList = new()
                    {
                        NumIDs = 1,
                        pIDList = hide
                    }
                };

                dxgiInfoQueue.Get()->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
            }
#endif
        }

        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, __uuidof<IDXGIFactory6>(), _factory.GetVoidAddressOf()));

        BOOL tearingSupported = true;
        if (_factory.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, (uint)sizeof(BOOL)).FAILED)
        {
            tearingSupported = false;
        }
        TearingSupported = tearingSupported;

        DXGI_GPU_PREFERENCE gpuPreference = (description.PowerPreference == GpuPowerPreference.LowPower) ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;

        for (uint i = 0;
            _factory.Get()->EnumAdapterByGpuPreference(i, gpuPreference, __uuidof<IDXGIAdapter1>(), (void**)_adapter.ReleaseAndGetAddressOf()).SUCCEEDED;
            ++i)
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));

            // Don't select the Basic Render Driver adapter.
            if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                continue;
            }

            if (D3D12CreateDevice((IUnknown*)_adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device5>(), _handle.GetVoidAddressOf()).SUCCEEDED)
            {
                break;
            }
        }

        if (_adapter.Get() is null)
        {
            throw new GraphicsException("D3D12: No capable adapter found!");
        }

        if (ValidationMode != ValidationMode.Disabled)
        {
            // Configure debug device (if active).
            using ComPtr<ID3D12InfoQueue> infoQueue = default;
            if (_handle.CopyTo(infoQueue.GetAddressOf()).SUCCEEDED)
            {
                infoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                infoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

                // These severities should be seen all the time
                uint enabledSeveritiesCount = (ValidationMode == ValidationMode.Verbose) ? 5u : 4u;
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

        // Create fence to detect device removal
        {
            _deviceRemovedFence = _handle.Get()->CreateFence();

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
            D3D12MA_ALLOCATOR_DESC allocatorDesc = new()
            {
                pDevice = (ID3D12Device*)_handle.Get(),
                pAdapter = (IDXGIAdapter*)_adapter.Get()
            };
            //allocatorDesc.PreferredBlockSize = 256 * 1024 * 1024;
            //allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_ALWAYS_COMMITTED;
            allocatorDesc.Flags |= D3D12MA_ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
            allocatorDesc.Flags |= D3D12MA_ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;

            if (FAILED(D3D12MA_CreateAllocator(&allocatorDesc, _memoryAllocator.GetAddressOf())))
            {
                return;
            }
        }

        // Init features
        _features = new D3D12Features((ID3D12Device*)_handle.Get());

        // Create command queue's
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            QueueType queue = (QueueType)i;
            if (queue >= QueueType.VideoDecode)
                continue;

            _queues[i] = new D3D12CommandQueue(this, queue);
        }

        // Init CPU descriptor allocators
        _descriptorAllocators[(int)D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
        _descriptorAllocators[(int)D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 256);
        _descriptorAllocators[(int)D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 512);
        _descriptorAllocators[(int)D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = new(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);

        // Shader visible descriptor heaps
        {
            // Resource
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = new()
            {
                Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                NumDescriptors = 1000000, // Tier1 limit
                Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                NodeMask = 0,
            };
            ThrowIfFailed(_handle.Get()->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), _shaderVisibleResourceHeap.GetVoidAddressOf()));
            _startCpuHandleShaderVisibleResource = _shaderVisibleResourceHeap.Get()->GetCPUDescriptorHandleForHeapStart();
            _startGpuHandleShaderVisibleResource = _shaderVisibleResourceHeap.Get()->GetGPUDescriptorHandleForHeapStart();

            // Sampler
            heapDesc = new()
            {
                Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                NumDescriptors = 2048, // Tier1 limit
                Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                NodeMask = 0,
            };
            ThrowIfFailed(_handle.Get()->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), _shaderVisibleSamplerHeap.GetVoidAddressOf()));
            _startCpuHandleShaderVisibleSampler = _shaderVisibleSamplerHeap.Get()->GetCPUDescriptorHandleForHeapStart();
            _startGpuHandleShaderVisibleSampler = _shaderVisibleSamplerHeap.Get()->GetGPUDescriptorHandleForHeapStart();
        }

        // Init CopyAllocator
        _copyAllocator = new D3D12CopyAllocator(this);

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

            _dispatchIndirectCommandSignature = _handle.Get()->CreateCommandSignature(&cmdSignatureDesc);

            // DrawIndirectCommand
            D3D12_INDIRECT_ARGUMENT_DESC drawInstancedArg = new()
            {
                Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW
            };

            cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DRAW_ARGUMENTS);
            cmdSignatureDesc.NumArgumentDescs = 1;
            cmdSignatureDesc.pArgumentDescs = &drawInstancedArg;
            _drawIndirectCommandSignature = _handle.Get()->CreateCommandSignature(&cmdSignatureDesc);

            // DrawIndexedIndirectCommand
            D3D12_INDIRECT_ARGUMENT_DESC drawIndexedInstancedArg = new()
            {
                Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED
            };

            cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
            cmdSignatureDesc.NumArgumentDescs = 1;
            cmdSignatureDesc.pArgumentDescs = &drawIndexedInstancedArg;
            _drawIndexedIndirectCommandSignature = _handle.Get()->CreateCommandSignature(&cmdSignatureDesc);

            if (_features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1)
            {
                D3D12_INDIRECT_ARGUMENT_DESC dispatchMeshArg = new();
                dispatchMeshArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

                cmdSignatureDesc.ByteStride = (uint)sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
                cmdSignatureDesc.NumArgumentDescs = 1;
                cmdSignatureDesc.pArgumentDescs = &dispatchMeshArg;
                _dispatchMeshIndirectCommandSignature = _handle.Get()->CreateCommandSignature(&cmdSignatureDesc);
            }
        }

        // Init adapter info, caps and limits
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));


            // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
            string driverDescription = string.Empty;
            LARGE_INTEGER umdVersion;
            if (_adapter.Get()->CheckInterfaceSupport(__uuidof<IDXGIDevice>(), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
            {
                driverDescription = "D3D12 driver version ";

                long encodedVersion = umdVersion.QuadPart;
                for (int i = 0; i < 4; ++i)
                {
                    ushort driverVersion = (ushort)((encodedVersion >> (48 - 16 * i)) & 0xFFFF);
                    driverDescription += $"{driverVersion}.";
                }
            }

            // Detect adapter type.
            GpuAdapterType adapterType = GpuAdapterType.Other;
            if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
            {
                adapterType = GpuAdapterType.Cpu;
            }
            else
            {
                adapterType = _features.UMA() ? GpuAdapterType.IntegratedGpu : GpuAdapterType.DiscreteGpu;
            }

            _adapterProperties = new GraphicsAdapterProperties
            {
                VendorId = adapterDesc.VendorId,
                DeviceId = adapterDesc.DeviceId,
                AdapterName = GetUtf16Span(adapterDesc.Description, 128).GetString() ?? string.Empty,
                AdapterType = adapterType,
                DriverDescription = driverDescription
            };

            _limits = new GraphicsDeviceLimits
            {
                MaxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION,
                MaxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
                MaxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
                MaxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION,
                MaxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
                MaxTexelBufferDimension2D = (1u << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1,
                UploadBufferTextureRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT,
                UploadBufferTextureSliceAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,
                ConstantBufferMinOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                ConstantBufferMaxRange = D3D12_REQ_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT * 16,
                StorageBufferMinOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT,
                StorageBufferMaxRange = (1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1,

                MaxBufferSize = D3D12_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM * 1024ul * 1024ul,
                MaxPushConstantsSize = Constants.MaxPushConstantsSize, // D3D12_REQ_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT * 16,

                // Slot values can be 0-15, inclusive:
                // https://docs.microsoft.com/en-ca/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
                MaxVertexBuffers = 16,
                MaxVertexAttributes = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
                MaxVertexBufferArrayStride = D3D12_SO_BUFFER_MAX_STRIDE_IN_BYTES,

                MaxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
                MaxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT,

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

                SamplerMaxAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY,
                SamplerMinLodBias = D3D12_MIP_LOD_BIAS_MIN,
                SamplerMaxLodBias = D3D12_MIP_LOD_BIAS_MAX,
            };

            if (_features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2)
            {
                _limits.VariableRateShadingTileSize = _features.ShadingRateImageTileSize;
            }

            if (_features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0)
            {
                _limits.RayTracingShaderGroupIdentifierSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
                _limits.RayTracingShaderTableAligment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
                _limits.RayTracingShaderTableMaxStride = ulong.MaxValue;
                _limits.RayTracingShaderRecursionMaxDepth = D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH;
                _limits.RayTracingMaxGeometryCount = (1 << 24) - 1;
            }

            //if (_features.IndependentFrontAndBackStencilRefMaskSupported() == TRUE)
            //{
            //    LOGD("D3D12: IndependentFrontAndBackStencilRefMaskSupported supported");
            //}
            //
            //if (_features.DynamicDepthBiasSupported() == TRUE)
            //{
            //    LOGD("D3D12: DynamicDepthBiasSupported supported");
            //}
            //
            //if (d3dFeatures.GPUUploadHeapSupported() == TRUE)
            //{
            //    LOGD("D3D12: GPUUploadHeapSupported supported");
            //}

            ulong timestampFrequency;
            ThrowIfFailed(D3D12GraphicsQueue->GetTimestampFrequency(&timestampFrequency));
            TimestampFrequency = timestampFrequency;
        }
    }

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterProperties;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public IDXGIFactory6* Factory => _factory;
    public bool TearingSupported { get; }
    public IDXGIAdapter1* Adapter => _adapter;
    public ID3D12Device5* Handle => _handle;
    public D3D12MA_Allocator* MemoryAllocator => _memoryAllocator;
    public D3D12Features D3D12Features => _features;

    public ID3D12CommandQueue* D3D12GraphicsQueue => _queues[(int)QueueType.Graphics].Handle;
    public D3D12CommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public D3D12CommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public D3D12CommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public D3D12CommandQueue? VideDecodeQueue => _queues[(int)QueueType.Copy];

    public ID3D12DescriptorHeap* ShaderVisibleResourceHeap => _shaderVisibleResourceHeap;
    public ID3D12DescriptorHeap* ShaderVisibleSamplerHeap => _shaderVisibleSamplerHeap;

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

            _frameCount = ulong.MaxValue;
            ProcessDeletionQueue();
            _frameCount = 0;
            _frameIndex = 0;

            // Destroy CommandQueue's
            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] == null)
                    continue;

                _queues[i].Dispose();
            }

            for (int i = 0; i < (int)D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
            {
                _descriptorAllocators[i].Dispose();
            }
            _shaderVisibleResourceHeap.Dispose();
            _shaderVisibleSamplerHeap.Dispose();

            _dispatchIndirectCommandSignature.Dispose();
            _drawIndirectCommandSignature.Dispose();
            _drawIndexedIndirectCommandSignature.Dispose();
            _dispatchMeshIndirectCommandSignature.Dispose();

            // Allocator.
            if (_memoryAllocator.Get() is not null)
            {
                D3D12MA_TotalStatistics stats;
                _memoryAllocator.Get()->CalculateStatistics(&stats);

                if (stats.Total.Stats.AllocationBytes > 0)
                {
                    Log.Info($"Total device memory leaked: {stats.Total.Stats.AllocationBytes} bytes.");
                }

                _memoryAllocator.Dispose();
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

#if DEBUG
            uint refCount = _handle.Get()->Release();
            if (refCount > 0)
            {
                Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D12DebugDevice> debugDevice = default;

                if (_handle.CopyTo(debugDevice.GetAddressOf()).SUCCEEDED)
                {
                    debugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                }
            }
#else
            _handle.Dispose();
#endif

            _adapter.Dispose();
            _factory.Dispose();

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

    public D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        return _descriptorAllocators[(int)type].Allocate();
    }

    public void FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, in D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        if (handle.ptr == 0)
            return;

        _descriptorAllocators[(int)type].Free(in handle);
    }

    public uint GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        return _descriptorAllocators[(int)type].DescriptorSize;
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetResourceHeapCpuHandle(uint index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = _startCpuHandleShaderVisibleResource;
        handle.ptr += index * GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        return handle;
    }

    public D3D12_GPU_DESCRIPTOR_HANDLE GetResourceHeapGpuHandle(uint index)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = _startGpuHandleShaderVisibleResource;
        handle.ptr += index * GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        return handle;
    }

    public D3D12UploadContext Allocate(ulong size) => _copyAllocator.Allocate(size);
    public void Submit(in D3D12UploadContext context) => _copyAllocator.Submit(in context);

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)  // NOLINT(clang-diagnostic-switch-enum)
        {
            // Always supported features
            case Feature.DepthClipControl:
            case Feature.Depth32FloatStencil8:
            case Feature.TimestampQuery:
            case Feature.PipelineStatisticsQuery:
            case Feature.TextureCompressionBC:
            case Feature.IndirectFirstInstance:
            case Feature.TessellationShader:
            case Feature.SamplerAnisotropy:
            case Feature.DepthResolveMinMax:
            case Feature.StencilResolveMinMax:
            case Feature.Predication:
                return true;

            // Always unsupported features
            case Feature.TextureCompressionETC2:
            case Feature.TextureCompressionASTC:
                return false;

            case Feature.ShaderFloat16:
                //const bool supportsDP4a = d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_4;
                return _features.HighestShaderModel >= D3D_SHADER_MODEL_6_2 && _features.Native16BitShaderOpsSupported;

            case Feature.RG11B10UfloatRenderable:
                return true;

            case Feature.BGRA8UnormStorage:
                {
                    D3D12_FEATURE_DATA_FORMAT_SUPPORT bgra8unormFormatInfo = default;
                    bgra8unormFormatInfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    HRESULT hr = _handle.Get()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &bgra8unormFormatInfo, (uint)sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
                    if (hr.SUCCEEDED &&
                        (bgra8unormFormatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0)
                    {
                        return true;
                    }
                    return false;
                }

            case Feature.DepthBoundsTest:
                return _features.DepthBoundsTestSupported;

            case Feature.SamplerMinMax:
                if (_features.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
                {
                    // Tier 2 for tiled resources
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                }

                return (_features.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_1);

            case Feature.DescriptorIndexing:
                return true;

            case Feature.VariableRateShading:
                return (_features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1);

            case Feature.VariableRateShadingTier2:
                return (_features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2);

            case Feature.RayTracing:
                return (_features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

            case Feature.RayTracingTier2:
                return (_features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1);

            case Feature.MeshShader:
                return (_features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);

            default:
                return false;
        }
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] == null)
                continue;

            _queues[i].WaitIdle();
        }
    }

    /// <inheritdoc />
    public override void FinishFrame()
    {
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit();
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= Constants.MaxFramesInFlight)
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

        ProcessDeletionQueue();
    }

    public override void WriteShadingRateValue(ShadingRate rate, void* dest)
    {
        byte d3dRate = (byte)rate.ToD3D12();
        if (!_features.AdditionalShadingRatesSupported)
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
    protected override Texture CreateTextureCore(in TextureDescription description, void* initialData)
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
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
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

    private static bool CheckIsSupported()
    {
        try
        {
            if (!OperatingSystem.IsWindowsVersionAtLeast(10, 0, 19041))
            {
                return false;
            }

            using ComPtr<IDXGIFactory4> dxgiFactory = default;
            using ComPtr<IDXGIAdapter1> dxgiAdapter = default;

            ThrowIfFailed(CreateDXGIFactory1(__uuidof<IDXGIFactory4>(), dxgiFactory.GetVoidAddressOf()));

            bool foundCompatibleDevice = false;
            for (uint adapterIndex = 0;
                dxgiFactory.Get()->EnumAdapters1(adapterIndex, dxgiAdapter.ReleaseAndGetAddressOf()).SUCCEEDED;
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                ThrowIfFailed(dxgiAdapter.Get()->GetDesc1(&adapterDesc));

                if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
                if (D3D12CreateDevice((IUnknown*)dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
                     __uuidof<ID3D12Device>(), null).SUCCEEDED)
                {
                    foundCompatibleDevice = true;
                    break;
                }
            }

            if (!foundCompatibleDevice)
            {
                return false;
            }

            return true;
        }
        catch
        {
            return false;
        }
    }
}
