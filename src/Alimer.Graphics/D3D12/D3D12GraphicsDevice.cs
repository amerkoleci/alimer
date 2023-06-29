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

using System.Diagnostics;
using Win32.Graphics.Direct3D;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly ComPtr<IDXGIFactory6> _factory;
    private readonly ComPtr<IDXGIAdapter1> _adapter;
    private readonly ComPtr<ID3D12Device5> _handle;

    private readonly D3D12CommandQueue[] _queues = new D3D12CommandQueue[(int)QueueType.Count];

    private readonly D3D12Features _features = default;
    private readonly GraphicsAdapterProperties _adapterInfo;
    private readonly GraphicsDeviceLimits _limits;

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
            Log.Error("D3D12: No capable adapter found!");
            return;
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

                const int disabledMessagesCount = 9;
                MessageId* disabledMessages = stackalloc MessageId[disabledMessagesCount]
                {
                    MessageId.ClearRenderTargetViewMismatchingClearValue,
                    MessageId.ClearDepthStencilViewMismatchingClearValue,
                    MessageId.MapInvalidNullRange,
                    MessageId.UnmapInvalidNullRange,
                    MessageId.ExecuteCommandListsWrongSwapchainBufferReference,
                    MessageId.ResourceBarrierMismatchingCommandListType,
                    MessageId.ExecuteCommandListsGpuWrittenReadbackResourceMapped,
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
            _queues[i] = new D3D12CommandQueue(this, queue);
        }

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

            _adapterInfo = new GraphicsAdapterProperties
            {
                VendorId = adapterDesc.VendorId,
                DeviceId = adapterDesc.DeviceId,
                AdapterName = new((char*)adapterDesc.Description),
                AdapterType = adapterType,
                DriverDescription = driverDescription
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

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterInfo;

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
                _queues[i].Dispose();
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

    /// <inheritdoc />
    public override bool QueryFeature(Feature feature)
    {
        return false;
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        using ComPtr<ID3D12Fence> fence = default;
        ThrowIfFailed(_handle.Get()->CreateFence(0, FenceFlags.None, __uuidof<ID3D12Fence>(), fence.GetVoidAddressOf()));

        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            ThrowIfFailed(_queues[i].Handle->Signal(fence.Get(), 1));

            if (fence.Get()->GetCompletedValue() < 1)
            {
                ThrowIfFailed(fence.Get()->SetEventOnCompletion(1, Win32.Handle.Null));
            }

            ThrowIfFailed(fence.Get()->Signal(0));
        }
    }

    /// <inheritdoc />
    public override void FinishFrame()
    {
        AdvanceFrame();

        ProcessDeletionQueue();
    }

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescription descriptor, void* initialData)
    {
        return new D3D12Buffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, void* initialData)
    {
        return new D3D12Texture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
    {
        return new D3D12QueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(SwapChainSurface surface, in SwapChainDescriptor descriptor)
    {
        return new D3D12SwapChain(this, surface, descriptor);
    }

    /// <inheritdoc />
    public override CommandBuffer BeginCommandBuffer(QueueType queue, string? label = null)
    {
        throw new NotImplementedException();
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
