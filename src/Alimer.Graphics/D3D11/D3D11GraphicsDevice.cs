// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using static Win32.Apis;
using static Win32.Graphics.Dxgi.Apis;
using static Win32.Graphics.Direct3D11.Apis;
using Win32.Graphics.Direct3D11;
using Win32.Graphics.Dxgi;
using Win32;
using MessageId = Win32.Graphics.Direct3D11.MessageId;
using InfoQueueFilter = Win32.Graphics.Direct3D11.InfoQueueFilter;
using DxgiInfoQueueFilter = Win32.Graphics.Dxgi.InfoQueueFilter;
using System.Diagnostics;
using Win32.Graphics.Direct3D;
using static Alimer.Graphics.D3D11.D3D11Utils;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11GraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly ComPtr<IDXGIFactory2> _factory;
    private readonly ComPtr<IDXGIAdapter1> _adapter;
    private readonly ComPtr<ID3D11Device1> _handle;
    private readonly ComPtr<ID3D11DeviceContext1> _context;

    private readonly D3D11Features _features = default;
    private readonly GraphicsAdapterInfo _adapterInfo;
    private readonly GraphicsDeviceLimits _limits;

    public static bool IsSupported() => s_isSupported.Value;

    public D3D11GraphicsDevice(in GraphicsDeviceDescriptor descriptor)
        : base(GraphicsBackendType.D3D11, descriptor)
    {
        Guard.IsTrue(IsSupported(), nameof(D3D11GraphicsDevice), "Direct3D11 is not supported");

        uint dxgiFactoryFlags = 0u;

        if (ValidationMode != ValidationMode.Disabled)
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
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

        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, __uuidof<IDXGIFactory2>(), _factory.GetVoidAddressOf()));

        {
            using ComPtr<IDXGIFactory5> dxgiFactory5 = default;
            if (_factory.CopyTo(dxgiFactory5.GetAddressOf()).Success)
            {
                Bool32 tearingSupported = true;
                if (dxgiFactory5.Get()->CheckFeatureSupport(Win32.Graphics.Dxgi.Feature.PresentAllowTearing, &tearingSupported, sizeof(Bool32)).Failure)
                {
                    tearingSupported = false;
                }
                TearingSupported = tearingSupported;
            }
        }

        // Enumerate adapter and create D3D device
        {
            GpuPreference gpuPreference = (descriptor.PowerPreference == GpuPowerPreference.LowPower) ? GpuPreference.MinimumPower : GpuPreference.HighPerformance;

            using ComPtr<IDXGIFactory6> dxgiFactory6 = default;
            bool queryByPreference = _factory.CopyTo(dxgiFactory6.GetAddressOf()).Success;

            AdapterDescription1 adapterDesc = default;

            for (uint i = 0;
                NextAdapter(i, _adapter.ReleaseAndGetAddressOf());
                ++i)
            {
                ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));

                // Don't select the Basic Render Driver adapter.
                if ((adapterDesc.Flags & AdapterFlags.Software) != 0)
                {
                    continue;
                }

                break;
            }

            if (_adapter.Get() is null)
            {
                Log.Error("D3D11: No capable adapter found!");
                return;
            }

            ReadOnlySpan<FeatureLevel> featureLevels = stackalloc FeatureLevel[2]
            {
                FeatureLevel.Level_11_1,
                FeatureLevel.Level_11_0,
            };

            ReadOnlySpan<FeatureLevel> featureLevelsFallback = stackalloc FeatureLevel[1]
            {
                FeatureLevel.Level_11_0
            };

            // Set debug and Direct2D compatibility flags.
            CreateDeviceFlags creationFlags = CreateDeviceFlags.BgraSupport;

            if (ValidationMode != ValidationMode.Disabled)
            {
                if (SdkLayersAvailable())
                {
                    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                    creationFlags |= CreateDeviceFlags.Debug;
                }
                else
                {
                    Log.Warn("Direct3D11 Debug Device is not available");
                }
            }

            // Create the Direct3D 11 API device object and a corresponding context.
            {
                using ComPtr<ID3D11Device> tempDevice = default;
                using ComPtr<ID3D11DeviceContext> tempContext = default;
                FeatureLevel featureLevel = default;

                HResult hr = HResult.Fail;
                hr = D3D11CreateDevice(
                    (IDXGIAdapter*)_adapter.Get(),
                    DriverType.Unknown,
                    creationFlags,
                    featureLevels,
                    tempDevice.GetAddressOf(),
                    &featureLevel,
                    tempContext.GetAddressOf()
                );

                if (hr.Failure)
                {
                    Log.Warn("Creating device with feature level 11_1 failed. Lowering feature level.");
                    hr = D3D11CreateDevice(
                        (IDXGIAdapter*)_adapter.Get(),
                        DriverType.Unknown,
                        creationFlags,
                        featureLevelsFallback,
                        tempDevice.GetAddressOf(),
                        &featureLevel,
                        tempContext.GetAddressOf()
                    );
                }

                if (ValidationMode != ValidationMode.Disabled)
                {
                    // Configure debug device (if active).
                    using ComPtr<ID3D11Debug> d3dDebug = default;
                    if (tempDevice.CopyTo(d3dDebug.GetAddressOf()).Success)
                    {
                        using ComPtr<ID3D11InfoQueue> infoQueue = default;
                        if (d3dDebug.CopyTo(infoQueue.GetAddressOf()).Success)
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

                            const int disabledMessagesCount = 1;
                            MessageId* disabledMessages = stackalloc MessageId[disabledMessagesCount]
                            {
                                MessageId.SetPrivateDataChangingParams,
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
                }

                ThrowIfFailed(tempDevice.CopyTo(_handle.ReleaseAndGetAddressOf()));
                ThrowIfFailed(tempContext.CopyTo(_context.ReleaseAndGetAddressOf()));
            }

            ThrowIfFailed(_adapter.Get()->GetDesc1(&adapterDesc));
            _features = new D3D11Features((ID3D11Device*)_handle.Get());

            // Convert the adapter's D3D11 driver version to a readable string like "24.21.13.9793".
            string driverDescription = string.Empty;
            long umdVersion;
            if (_adapter.Get()->CheckInterfaceSupport(__uuidof<IDXGIDevice>(), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
            {
                driverDescription = "D3D11 driver version ";

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
                adapterType = _features.UnifiedMemoryArchitecture ? GpuAdapterType.IntegratedGpu : GpuAdapterType.DiscreteGpu;
            }

            _adapterInfo = new GraphicsAdapterInfo
            {
                VendorId = adapterDesc.VendorId,
                DeviceId = adapterDesc.DeviceId,
                AdapterName = new((char*)adapterDesc.Description),
                AdapterType = adapterType,
                DriverDescription = driverDescription
            };

            bool NextAdapter(uint index, IDXGIAdapter1** ppAdapter)
            {
                if (queryByPreference)
                    return dxgiFactory6.Get()->EnumAdapterByGpuPreference(index, gpuPreference, __uuidof<IDXGIAdapter1>(), (void**)ppAdapter).Success;
                else
                    return _factory.Get()->EnumAdapters1(index, ppAdapter).Success;
            };
        }

        // Limits
        {
            QueryDescription queryDesc = new(Win32.Graphics.Direct3D11.QueryType.TimestampDisjoint);

            using ComPtr<ID3D11Query> query = default;
            HResult hr = _handle.Get()->CreateQuery(&queryDesc, query.GetAddressOf());
            if (hr.Success)
            {
                _context.Get()->Begin((ID3D11Asynchronous*)query.Get());
                _context.Get()->End((ID3D11Asynchronous*)query.Get());

                QueryDataTimestampDisjoint data = default;
                while (_context.Get()->GetData((ID3D11Asynchronous*)query.Get(), &data, (uint)sizeof(QueryDataTimestampDisjoint), 0) == HResult.False)
                {
                }

                TimestampFrequency = data.Frequency;
            }

            _limits = new GraphicsDeviceLimits
            {
                MaxTextureDimension1D = D3D11_REQ_TEXTURE1D_U_DIMENSION,
                MaxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION,
                MaxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
                MaxTextureDimensionCube = D3D11_REQ_TEXTURECUBE_DIMENSION,
                MaxTextureArrayLayers = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
                MaxTexelBufferDimension2D = (1 << (int)D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1u,
            };

            //limits.uniformBufferMaxRange = D3D11_REQ_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            //limits.storageBufferMaxRange = (1 << D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1;
            //limits.minUniformBufferOffsetAlignment = 256; // D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
            //limits.minStorageBufferOffsetAlignment = D3D11_RAW_UAV_SRV_BYTE_ALIGNMENT;
            //limits.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
            //limits.maxSamplerAnisotropy = D3D11_MAX_MAXANISOTROPY;
        }
    }

    public IDXGIFactory2* Factory => _factory;
    public bool TearingSupported { get; }
    public IDXGIAdapter1* Adapter => _adapter;
    public ID3D11Device1* Handle => _handle;
    public ID3D11DeviceContext1* Context => _context;
    public FeatureLevel FeatureLevel { get; }

    /// <inheritdoc />
    public override GraphicsAdapterInfo AdapterInfo => _adapterInfo;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    /// <summaryD3D11GraphicsDevice
    /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
    /// </summary>
    ~D3D11GraphicsDevice() => Dispose(disposing: false);

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

            _context.Dispose();
#if DEBUG
            uint refCount = _handle.Get()->Release();
            if (refCount > 0)
            {
                Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D11Debug> debugDevice = default;

                if (_handle.CopyTo(debugDevice.GetAddressOf()).Success)
                {
                    debugDevice.Get()->ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Summary | ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
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
        _context.Get()->Flush();
    }

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescriptor descriptor, void* initialData)
    {
        return new D3D11Buffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, void* initialData)
    {
        return new D3D11Texture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor)
    {
        return new D3D11QueryHeap(this, descriptor);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(SwapChainSurface surface, in SwapChainDescriptor descriptor)
    {
        return new D3D11SwapChain(this, surface, descriptor);
    }

    private static bool CheckIsSupported()
    {
        try
        {
            using ComPtr<IDXGIFactory2> dxgiFactory = default;
            using ComPtr<IDXGIAdapter1> dxgiAdapter = default;

            ThrowIfFailed(CreateDXGIFactory1(__uuidof<IDXGIFactory2>(), dxgiFactory.GetVoidAddressOf()));

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

                foundCompatibleDevice = true;
                break;
            }

            if (!foundCompatibleDevice)
            {
                return false;
            }

            FeatureLevel* featureLevels = stackalloc FeatureLevel[2]
            {
                FeatureLevel.Level_11_1,
                FeatureLevel.Level_11_0,
            };

            HResult hr = D3D11CreateDevice(
                null,
                DriverType.Hardware,
                0,
                CreateDeviceFlags.BgraSupport,
                featureLevels,
                2,
                D3D11_SDK_VERSION,
                null,
                null,
                null
            );

            if (hr.Failure)
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
