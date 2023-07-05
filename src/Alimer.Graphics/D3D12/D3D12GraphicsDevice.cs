// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using static Win32.Apis;
using static Win32.Graphics.Dxgi.Apis;
using static Win32.Graphics.Direct3D12.Apis;
using Win32.Graphics.Direct3D12;
using Win32.Graphics.Dxgi;
using Win32;
using MessageId = Win32.Graphics.Direct3D12.MessageId;
using InfoQueueFilter = Win32.Graphics.Direct3D12.InfoQueueFilter;
using DxgiInfoQueueFilter = Win32.Graphics.Dxgi.InfoQueueFilter;
using D3DShadingRate = Win32.Graphics.Direct3D12.ShadingRate;
using System.Diagnostics;
using Win32.Graphics.Direct3D;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly ComPtr<IDXGIFactory6> _factory;
    private readonly ComPtr<IDXGIAdapter1> _adapter;
    private readonly ComPtr<ID3D12Device5> _handle;
    //private readonly ComPtr<ID3D12VideoDevice> _videoDevice;

    private readonly D3D12Features _features = default;
    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;

    private readonly D3D12CommandQueue[] _queues = new D3D12CommandQueue[(int)QueueType.Count];
    private readonly D3D12DescriptorAllocator[] _descriptorAllocators = new D3D12DescriptorAllocator[(int)DescriptorHeapType.NumTypes];
    private readonly D3D12CopyAllocator _copyAllocator;

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
            if (D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), d3d12Debug.GetVoidAddressOf()).Success)
            {
                d3d12Debug.Get()->EnableDebugLayer();

                if (ValidationMode == ValidationMode.GPU)
                {
                    using ComPtr<ID3D12Debug1> d3d12Debug1 = default;
                    using ComPtr<ID3D12Debug2> d3d12Debug2 = default;

                    if (d3d12Debug.CopyTo(d3d12Debug1.GetAddressOf()).Success)
                    {
                        d3d12Debug1.Get()->SetEnableGPUBasedValidation(true);
                        d3d12Debug1.Get()->SetEnableSynchronizedCommandQueueValidation(true);
                    }

                    if (d3d12Debug.CopyTo(d3d12Debug2.GetAddressOf()).Success)
                    {
                        d3d12Debug2.Get()->SetGPUBasedValidationFlags(GpuBasedValidationFlags.None);
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
                if (D3D12GetDebugInterface(__uuidof<ID3D12DeviceRemovedExtendedDataSettings1>(), pDredSettings.GetVoidAddressOf()).Success)
                {
                    // Turn on auto - breadcrumbs and page fault reporting.
                    pDredSettings.Get()->SetAutoBreadcrumbsEnablement(DredEnablement.ForcedOn);
                    pDredSettings.Get()->SetPageFaultEnablement(DredEnablement.ForcedOn);
                    pDredSettings.Get()->SetBreadcrumbContextEnablement(DredEnablement.ForcedOn);
                }
            }

#if DEBUG
            using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf()).Success)
            {
                dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, InfoQueueMessageSeverity.Error, true);
                dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, InfoQueueMessageSeverity.Corruption, true);

                int* hide = stackalloc int[1]
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };

                DxgiInfoQueueFilter filter = new()
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

        Bool32 tearingSupported = true;
        if (_factory.Get()->CheckFeatureSupport(Win32.Graphics.Dxgi.Feature.PresentAllowTearing, &tearingSupported, sizeof(Bool32)).Failure)
        {
            tearingSupported = false;
        }
        TearingSupported = tearingSupported;

        GpuPreference gpuPreference = (description.PowerPreference == GpuPowerPreference.LowPower) ? GpuPreference.MinimumPower : GpuPreference.HighPerformance;

        for (uint i = 0;
            _factory.Get()->EnumAdapterByGpuPreference(i, gpuPreference, __uuidof<IDXGIAdapter1>(), (void**)_adapter.ReleaseAndGetAddressOf()).Success;
            ++i)
        {
            AdapterDescription1 adapterDesc;
            ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));

            // Don't select the Basic Render Driver adapter.
            if ((adapterDesc.Flags & AdapterFlags.Software) != 0)
            {
                continue;
            }

            if (D3D12CreateDevice((IUnknown*)_adapter.Get(), FeatureLevel.Level_12_0, __uuidof<ID3D12Device5>(), _handle.GetVoidAddressOf()).Success)
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
            if (_handle.CopyTo(infoQueue.GetAddressOf()).Success)
            {
                infoQueue.Get()->SetBreakOnSeverity(MessageSeverity.Corruption, true);
                infoQueue.Get()->SetBreakOnSeverity(MessageSeverity.Error, true);

                // These severities should be seen all the time
                uint enabledSeveritiesCount = (ValidationMode == ValidationMode.Verbose) ? 5u : 4u;
                MessageSeverity* enabledSeverities = stackalloc MessageSeverity[5]
                {
                    MessageSeverity.Corruption,
                    MessageSeverity.Error,
                    MessageSeverity.Warning,
                    MessageSeverity.Message,
                    MessageSeverity.Info
                };

                const int disabledMessagesCount = 12;
                MessageId* disabledMessages = stackalloc MessageId[disabledMessagesCount]
                {
                    MessageId.SetPrivateDataChangingParams,
                    MessageId.ClearRenderTargetViewMismatchingClearValue,
                    MessageId.ClearDepthStencilViewMismatchingClearValue,
                    MessageId.MapInvalidNullRange,
                    MessageId.UnmapInvalidNullRange,
                    MessageId.ExecuteCommandListsWrongSwapchainBufferReference,
                    MessageId.ResourceBarrierMismatchingCommandListType,
                    MessageId.ExecuteCommandListsGpuWrittenReadbackResourceMapped,
                    MessageId.CreatePipelinelibraryDriverVersionMismatch,
                    MessageId.CreatePipelinelibraryAdapterVersionMismatch,
                    MessageId.LoadPipelineNameNotFound,
                    MessageId.StorePipelineDuplicateName
                };


                InfoQueueFilter filter = new();
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
#if TODO //WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        {
            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&deviceRemovedFence)));

            HANDLE deviceRemovedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
            ThrowIfFailed(deviceRemovedFence->SetEventOnCompletion(UINT64_MAX, deviceRemovedEvent));

            RegisterWaitForSingleObject(
                &deviceRemovedWaitHandle,
                deviceRemovedEvent,
                HandleDeviceRemoved,
                this, // Pass the device as our context
                INFINITE, // No timeout
                0 // No flags
            );
        }
#endif

        // Create command queue's
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            QueueType queue = (QueueType)i;
            if (queue >= QueueType.VideoDecode)
                continue;

            _queues[i] = new D3D12CommandQueue(this, queue);
        }

        // Init CPU descriptor allocators
        _descriptorAllocators[(int)DescriptorHeapType.CbvSrvUav] = new(this, DescriptorHeapType.CbvSrvUav, 4096);
        _descriptorAllocators[(int)DescriptorHeapType.Sampler] = new(this, DescriptorHeapType.Sampler, 256);
        _descriptorAllocators[(int)DescriptorHeapType.Rtv] = new(this, DescriptorHeapType.Rtv, 512);
        _descriptorAllocators[(int)DescriptorHeapType.Dsv] = new(this, DescriptorHeapType.Dsv, 128);

        // Init CopyAllocator
        _copyAllocator = new D3D12CopyAllocator(this);

        // Init adapter info, caps and limits
        {
            AdapterDescription1 adapterDesc;
            ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));

            _features = new D3D12Features((ID3D12Device*)_handle.Get());

            // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
            string driverDescription = string.Empty;
            long umdVersion;
            if (_adapter.Get()->CheckInterfaceSupport(__uuidof<IDXGIDevice>(), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
            {
                driverDescription = "D3D12 driver version ";

                for (int i = 0; i < 4; ++i)
                {
                    ushort driverVersion = (ushort)((umdVersion >> (48 - 16 * i)) & 0xFFFF);
                    driverDescription += $"{driverVersion}.";
                }
            }

            // Detect adapter type.
            GpuAdapterType adapterType = GpuAdapterType.Other;
            if ((adapterDesc.Flags & AdapterFlags.Software) != AdapterFlags.None)
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
                AdapterName = new((char*)adapterDesc.Description),
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
                MaxTexelBufferDimension2D = (1u << (int)D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1,
            };

            ulong timestampFrequency = 0;
            ThrowIfFailed(D3D12GraphicsQueue->GetTimestampFrequency(&timestampFrequency));
            TimestampFrequency = timestampFrequency;
        }
    }

    public IDXGIFactory6* Factory => _factory;
    public bool TearingSupported { get; }
    public IDXGIAdapter1* Adapter => _adapter;
    public ID3D12Device5* Handle => _handle;

    public ID3D12CommandQueue* D3D12GraphicsQueue => _queues[(int)QueueType.Graphics].Handle;
    public D3D12CommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public D3D12CommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public D3D12CommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public D3D12CommandQueue? VideDecodeQueue => _queues[(int)QueueType.Copy];

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterProperties;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

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

            for (int i = 0; i < (int)DescriptorHeapType.NumTypes; i++)
            {
                _descriptorAllocators[i].Dispose();
            }

#if DEBUG
            uint refCount = _handle.Get()->Release();
            if (refCount > 0)
            {
                Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D12DebugDevice> debugDevice = default;

                if (_handle.CopyTo(debugDevice.GetAddressOf()).Success)
                {
                    debugDevice.Get()->ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                }
            }
#else
            _handle.Dispose();
#endif

            _adapter.Dispose();
            _factory.Dispose();

#if DEBUG
            using ComPtr<IDXGIDebug1> dxgiDebug = default;
            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIDebug1>(), dxgiDebug.GetVoidAddressOf()).Success)
            {
                dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, ReportLiveObjectFlags.Summary | ReportLiveObjectFlags.IgnoreInternal);
            }
#endif
        }
    }

    public void OnDeviceRemoved()
    {

    }

    public CpuDescriptorHandle AllocateDescriptor(DescriptorHeapType type)
    {
        return _descriptorAllocators[(int)type].Allocate();
    }

    public void FreeDescriptor(DescriptorHeapType type, in CpuDescriptorHandle handle)
    {
        if (handle.ptr == 0)
            return;

        _descriptorAllocators[(int)type].Free(in handle);
    }

    public uint GetDescriptorHandleIncrementSize(DescriptorHeapType type)
    {
        return _descriptorAllocators[(int)type].DescriptorSize;
    }

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
            case Feature.GeometryShader:
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
                return _features.HighestShaderModel >= ShaderModel.SM_6_2 && _features.Native16BitShaderOpsSupported;

            case Feature.RG11B10UfloatRenderable:
                return true;

            case Feature.BGRA8UnormStorage:
                {
                    FeatureDataFormatSupport bgra8unormFormatInfo = default;
                    bgra8unormFormatInfo.Format = Win32.Graphics.Dxgi.Common.Format.B8G8R8A8Unorm;
                    HResult hr = _handle.Get()->CheckFeatureSupport(Win32.Graphics.Direct3D12.Feature.FormatSupport, &bgra8unormFormatInfo, sizeof(FeatureDataFormatSupport));
                    if (hr.Success &&
                        (bgra8unormFormatInfo.Support1 & FormatSupport1.TypedUnorderedAccessView) != 0)
                    {
                        return true;
                    }
                    return false;
                }

            case Feature.DepthBoundsTest:
                return _features.DepthBoundsTestSupported;

            case Feature.SamplerMinMax:
                if (_features.TiledResourcesTier >= TiledResourcesTier.Tier2)
                {
                    // Tier 2 for tiled resources
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                }

                return (_features.MaxSupportedFeatureLevel >= FeatureLevel.Level_11_1);

            case Feature.VariableRateShading:
                return (_features.VariableShadingRateTier >= VariableShadingRateTier.Tier1);

            case Feature.VariableRateShadingTier2:
                return (_features.VariableShadingRateTier >= VariableShadingRateTier.Tier2);

            case Feature.RayTracing:
                return (_features.RaytracingTier >= RaytracingTier.Tier1_0);

            case Feature.RayTracingTier2:
                return (_features.RaytracingTier >= RaytracingTier.Tier1_1);

            case Feature.MeshShader:
                return (_features.MeshShaderTier >= MeshShaderTier.Tier1);

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
            d3dRate = Math.Min(d3dRate, (byte)D3DShadingRate.Rate2x2);
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
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description)
    {
        throw new NotImplementedException();
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
                dxgiFactory.Get()->EnumAdapters1(adapterIndex, dxgiAdapter.ReleaseAndGetAddressOf()).Success;
                adapterIndex++)
            {
                AdapterDescription1 adapterDesc;
                ThrowIfFailed(dxgiAdapter.Get()->GetDesc1(&adapterDesc));

                if ((adapterDesc.Flags & AdapterFlags.Software) != 0)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
                if (D3D12CreateDevice((IUnknown*)dxgiAdapter.Get(), Win32.Graphics.Direct3D.FeatureLevel.Level_12_0,
                     __uuidof<ID3D12Device>(), null).Success)
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
