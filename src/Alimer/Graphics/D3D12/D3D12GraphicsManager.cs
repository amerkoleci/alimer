// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Vortice.Win32;
using Vortice.Win32.Graphics;
using static Vortice.Win32.Apis;
using static Vortice.Win32.Graphics.D3D;
using static Vortice.Win32.Graphics.D3D12;
using static Vortice.Win32.Graphics.DXGI;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsManager : GraphicsManager
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private static ComPtr<ID3D12DeviceFactory> s_deviceFactory = default;
    private readonly ComPtr<IDXGIFactory6> _handle = default;
    private readonly D3D12GraphicsAdapter[] _adapters;

    /// <summary>
    /// Gets value indicating whether D3D12 is supported on this platform.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public static bool EnableAgilitySDK { get; set; } = true;

    public D3D12GraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        bool dxgiFactoryFlags = false;

        if (options.ValidationMode != GraphicsValidationMode.Disabled)
        {
            dxgiFactoryFlags = true;

            using ComPtr<ID3D12Debug> d3d12Debug = default;
            HRESULT hr;
            if (DeviceFactory is not null)
            {
                hr = DeviceFactory->GetConfigurationInterface(CLSID_D3D12Debug, d3d12Debug.GetAddressOf());
            }
            else
            {
                hr = D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), (void**)d3d12Debug.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                d3d12Debug.Get()->EnableDebugLayer();

                if (options.ValidationMode == GraphicsValidationMode.Gpu)
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
                if (DeviceFactory is not null)
                {
                    hr = DeviceFactory->GetConfigurationInterface(CLSID_D3D12DeviceRemovedExtendedData, pDredSettings.GetAddressOf());
                }
                else
                {
                    hr = D3D12GetDebugInterface(__uuidof<ID3D12DeviceRemovedExtendedDataSettings1>(), (void**)pDredSettings.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    // Turn on auto - breadcrumbs and page fault reporting.
                    pDredSettings.Get()->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
            }

#if DEBUG
            using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIInfoQueue>(), (void**)dxgiInfoQueue.GetAddressOf()).Success)
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

        CreateDXGIFactory2(dxgiFactoryFlags, __uuidof<IDXGIFactory6>(), (void**)_handle.GetAddressOf()).ThrowIfFailed();
        TearingSupported = _handle.Get()->IsTearingSupported();

        DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        List<D3D12GraphicsAdapter> adapters = [];
        using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

        for (uint i = 0;
            _handle.Get()->EnumAdapterByGpuPreference(i, gpuPreference, __uuidof<IDXGIAdapter1>(), (void**)dxgiAdapter1.ReleaseAndGetAddressOf()).Success;
            ++i)
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            dxgiAdapter1.Get()->GetDesc1(&adapterDesc).ThrowIfFailed();

            // Don't select the Basic Render Driver adapter.
            if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                continue;
            }

            ID3D12Device* device = default;
            if (s_deviceFactory.Get() is not null)
            {
                if (s_deviceFactory.Get()->CreateDevice((IUnknown*)dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device>(), (void**)&device).Failure)
                {
                    continue;
                }
            }
            else
            {
                if (D3D12CreateDevice((IUnknown*)dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device>(), (void**)&device).Failure)
                {
                    continue;
                }
            }

            D3D12GraphicsAdapter adapter = new(this, dxgiAdapter1.Move(), device);
            adapters.Add(adapter);
            _ = device->Release();
        }

        _adapters = [.. adapters];
    }

    public static ID3D12DeviceFactory* DeviceFactory => s_deviceFactory;
    public IDXGIFactory6* Handle => _handle;
    public bool TearingSupported { get; }

    /// <inheritdoc/>
    protected override void DisposeManagedResources()
    {
        for (int i = 0; i < _adapters.Length; i++)
        {
            _adapters[i].Dispose();
        }

        _handle.Dispose();

#if DEBUG
        using ComPtr<IDXGIDebug1> dxgiDebug = default;
        if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIDebug1>(), (void**)dxgiDebug.GetAddressOf()).Success)
        {
            dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
        }
#endif
    }

    protected override Surface CreateSurfaceCore(in SurfaceDescriptor descriptor) => new D3D12Surface(this, descriptor);

    private static bool CheckIsSupported()
    {
        try
        {
            if (!OperatingSystem.IsWindowsVersionAtLeast(10, 0, 20348))
            {
                return false;
            }


            if (EnableAgilitySDK)
            {
                using ComPtr<ID3D12SDKConfiguration1> sdkConfig = default;
                Guid guid = CLSID_D3D12SDKConfiguration;
                HRESULT hr = D3D12GetInterface(&guid, __uuidof<ID3D12SDKConfiguration1>(), (void**)sdkConfig.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    uint agilitySdkVersion = D3D12_SDK_VERSION;
                    string agilitySdkPath = ".\\D3D12\\"; // D3D12SDKPath;

                    hr = sdkConfig.Get()->CreateDeviceFactory(agilitySdkVersion, agilitySdkPath, __uuidof<ID3D12DeviceFactory>(), (void**)s_deviceFactory.GetAddressOf());
                    if (FAILED(hr))
                    {
                        agilitySdkPath = ".\\";
                        hr = sdkConfig.Get()->CreateDeviceFactory(agilitySdkVersion, agilitySdkPath, __uuidof<ID3D12DeviceFactory>(), (void**)s_deviceFactory.GetAddressOf());
                    }
                }
            }

            using ComPtr<IDXGIFactory4> dxgiFactory = default;
            using ComPtr<IDXGIAdapter1> dxgiAdapter = default;

            CreateDXGIFactory2(0u, __uuidof<IDXGIFactory4>(), (void**)dxgiFactory.GetAddressOf()).ThrowIfFailed();

            bool foundCompatibleDevice = false;
            for (uint adapterIndex = 0;
                dxgiFactory.Get()->EnumAdapters1(adapterIndex, dxgiAdapter.ReleaseAndGetAddressOf()).Success;
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                dxgiAdapter.Get()->GetDesc1(&adapterDesc).ThrowIfFailed();

                if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
                if (s_deviceFactory.Get() is not null)
                {
                    using ComPtr<ID3D12Device> d3d12Device = default;
                    if (s_deviceFactory.Get()->CreateDevice((IUnknown*)dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device>(), (void**)d3d12Device.GetAddressOf()).Success)
                    {
                        foundCompatibleDevice = true;
                        break;
                    }

                }
                else
                {
                    if (D3D12CreateDevice((IUnknown*)dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device>(), null).Success)
                    {
                        foundCompatibleDevice = true;
                        break;
                    }
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
