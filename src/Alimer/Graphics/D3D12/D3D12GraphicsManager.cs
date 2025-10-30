// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.DirectX.D3D12_DRED_ENABLEMENT;
using static TerraFX.Interop.DirectX.D3D12_GPU_BASED_VALIDATION_FLAGS;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DirectX.DXGI_FEATURE;
using static TerraFX.Interop.DirectX.DXGI_GPU_PREFERENCE;
using static TerraFX.Interop.Windows.Windows;
#if DEBUG
using static TerraFX.Interop.DirectX.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.DirectX.DXGI_DEBUG_RLO_FLAGS;
#endif

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12GraphicsManager : GraphicsManager
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly ComPtr<IDXGIFactory6> _handle;
    private readonly D3D12GraphicsAdapter[] _adapters;

    /// <summary>
    /// Gets value indicating whether D3D12 is supported on this platform.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    /// <inheritdoc/>
    public override GraphicsBackendType BackendType => GraphicsBackendType.D3D12;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public D3D12GraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        uint dxgiFactoryFlags = 0u;

        if (options.ValidationMode != GraphicsValidationMode.Disabled)
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            using ComPtr<ID3D12Debug> d3d12Debug = default;
            if (D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), (void**)d3d12Debug.GetAddressOf()).SUCCEEDED)
            {
                d3d12Debug.Get()->EnableDebugLayer();

                if (options.ValidationMode == GraphicsValidationMode.Gpu)
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
                if (D3D12GetDebugInterface(__uuidof<ID3D12DeviceRemovedExtendedDataSettings1>(), (void**)pDredSettings.GetAddressOf()).SUCCEEDED)
                {
                    // Turn on auto - breadcrumbs and page fault reporting.
                    pDredSettings.Get()->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings.Get()->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
            }

#if DEBUG
            using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

            if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIInfoQueue>(), (void**)dxgiInfoQueue.GetAddressOf()).SUCCEEDED)
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

        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, __uuidof<IDXGIFactory6>(), (void**)_handle.GetAddressOf()));

        BOOL tearingSupported = true;
        if (_handle.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, (uint)sizeof(BOOL)).FAILED)
        {
            tearingSupported = false;
        }
        TearingSupported = tearingSupported;

        DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        List<D3D12GraphicsAdapter> adapters = [];
        using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

        for (uint i = 0;
            _handle.Get()->EnumAdapterByGpuPreference(i, gpuPreference, __uuidof<IDXGIAdapter1>(), (void**)dxgiAdapter1.ReleaseAndGetAddressOf()).SUCCEEDED;
            ++i)
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(dxgiAdapter1.Get()->GetDesc1(&adapterDesc));

            // Don't select the Basic Render Driver adapter.
            if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                continue;
            }

            ID3D12Device* device = default;
            if (D3D12CreateDevice((IUnknown*)dxgiAdapter1.Get(),
                D3D_FEATURE_LEVEL_12_0,
                __uuidof<ID3D12Device>(),
                (void**)&device).FAILED)
            {
                continue;
            }

            D3D12GraphicsAdapter adapter = new(this, dxgiAdapter1.Move(), device);
            adapters.Add(adapter);
            _ = device->Release();
        }

        _adapters = [.. adapters];
    }

    public IDXGIFactory6* Handle => _handle;
    public bool TearingSupported { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        for (int i = 0; i < _adapters.Length; i++)
        {
            _adapters[i].Dispose();
        }

#if DEBUG
        using ComPtr<IDXGIDebug1> dxgiDebug = default;
        if (DXGIGetDebugInterface1(0u, __uuidof<IDXGIDebug1>(), (void**)dxgiDebug.GetAddressOf()).SUCCEEDED)
        {
            dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
        }
#endif
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

            ThrowIfFailed(CreateDXGIFactory1(__uuidof<IDXGIFactory4>(), (void**)dxgiFactory.GetAddressOf()));

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
                if (D3D12CreateDevice((IUnknown*)dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device>(), null).SUCCEEDED)
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
