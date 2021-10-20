// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Diagnostics;
using System.Collections.Generic;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_GPU_BASED_VALIDATION_FLAGS;
using static TerraFX.Interop.DXGI_FEATURE;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DXGI_GPU_PREFERENCE;
#if DEBUG
using static TerraFX.Interop.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.DXGI_DEBUG_RLO_FLAGS;
#endif

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12GraphicsDeviceFactory : GraphicsDeviceFactory
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        private static bool CheckIsSupported()
        {
            try
            {
                return SUCCEEDED(D3D12CreateDevice(null, D3D_FEATURE_LEVEL_12_0, null, null));
            }
            catch (DllNotFoundException)
            {
                return false;
            }
        }

        private ComPtr<IDXGIFactory4> _dxgiFactory4;

        public D3D12GraphicsDeviceFactory(ValidationMode validationMode)
            : base(validationMode)
        {
            uint factoryFlags = 0;

            if (validationMode != ValidationMode.Disabled)
            {
                factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                using ComPtr<ID3D12Debug> d3d12Debug = default;

                if (SUCCEEDED(D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), d3d12Debug.GetVoidAddressOf())))
                {
                    d3d12Debug.Get()->EnableDebugLayer();

                    if (validationMode == ValidationMode.GPU)
                    {
                        using ComPtr<ID3D12Debug1> d3d12Debug1 = default;

                        if (SUCCEEDED(d3d12Debug.CopyTo(d3d12Debug1.GetAddressOf())))
                        {
                            d3d12Debug1.Get()->SetEnableGPUBasedValidation(TRUE);
                            d3d12Debug1.Get()->SetEnableSynchronizedCommandQueueValidation(TRUE);
                        }

                        using ComPtr<ID3D12Debug2> d3d12Debug2 = default;
                        if (SUCCEEDED(d3d12Debug.CopyTo(d3d12Debug2.GetAddressOf())))
                        {
                            d3d12Debug2.Get()->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                        }
                    }
                }
                else
                {
                    Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                }

#if DEBUG
                using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

                if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf())))
                {
                    dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
                    dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);

                    ReadOnlySpan<int> hide = stackalloc int[]
                    {
                        80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                    };

                    fixed (int* pIDList = hide)
                    {
                        DXGI_INFO_QUEUE_FILTER filter = new()
                        {
                            DenyList = new()
                            {
                                NumIDs = (uint)hide.Length,
                                pIDList = pIDList
                            }
                        };

                        dxgiInfoQueue.Get()->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
                    }
                }
#endif
            }

            CreateDXGIFactory2(factoryFlags, __uuidof<IDXGIFactory4>(), _dxgiFactory4.GetVoidAddressOf()).Assert();

            // Check tearing support
            {
                using ComPtr<IDXGIFactory5> dxgiFactory5 = default;
                if (SUCCEEDED(_dxgiFactory4.CopyTo(dxgiFactory5.GetAddressOf())))
                {
                    int allowTearing = FALSE;
                    if (FAILED(dxgiFactory5.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, (uint)sizeof(int)))
                        && allowTearing == FALSE)
                    {
                        IsTearingSupported = false;
                    }
                    else
                    {
                        IsTearingSupported = true;
                    }
                }
            }

            // Enumerate physical devices
            List<D3D12.D3D12PhysicalDevice> physicalDevices = new();

            using ComPtr<IDXGIFactory6> dxgiFactory6 = default;
            if (SUCCEEDED(_dxgiFactory4.CopyTo(dxgiFactory6.GetAddressOf())))
            {
                using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

                for (uint adapterIndex = 0;
                    dxgiFactory6.Get()->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                        __uuidof<IDXGIAdapter1>(),
                        dxgiAdapter1.GetVoidAddressOf()) != DXGI_ERROR_NOT_FOUND;
                    adapterIndex++)
                {
                    DXGI_ADAPTER_DESC1 adapterDesc;
                    dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

                    if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    using ComPtr<ID3D12Device2> tempDevice = default;

                    if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.AsIUnknown().Get(),
                        D3D_FEATURE_LEVEL_12_0,
                        __uuidof<ID3D12Device2>(),
                        tempDevice.GetVoidAddressOf())))
                    {
                        D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1 = tempDevice.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_ARCHITECTURE1>(D3D12_FEATURE_ARCHITECTURE1);
                        physicalDevices.Add(new D3D12PhysicalDevice(this, dxgiAdapter1, architecture1));
                    }
                }
            }
            else
            {
                using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

                for (uint adapterIndex = 0; _dxgiFactory4.Get()->EnumAdapters1(adapterIndex, dxgiAdapter1.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; adapterIndex++)
                {
                    DXGI_ADAPTER_DESC1 adapterDesc;
                    dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

                    if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    //if (D3D12CreateDevice(adapter, FeatureLevel.Level_12_0, out ID3D12Device2? device).Success)
                    //{
                    //    physicalDevices.Add(new D3D12.D3D12PhysicalDevice(this, dxgiAdapter1.Get(), device.Architecture1));
                    //    device.Dispose();
                    //}
                }
            }

            PhysicalDevices = physicalDevices.AsReadOnly();
        }

        public IDXGIFactory4* DXGIFactory => _dxgiFactory4.Get();
        public bool IsTearingSupported { get; private set; }

        /// <inheritdoc />
        public override IReadOnlyList<PhysicalDevice> PhysicalDevices { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                foreach (D3D12.D3D12PhysicalDevice physicalDevice in PhysicalDevices)
                {
                    physicalDevice.Dispose();
                }

                _dxgiFactory4.Dispose();

#if DEBUG
                using ComPtr<IDXGIDebug1> dxgiDebug = default;

                if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof<IDXGIDebug1>(), dxgiDebug.GetVoidAddressOf())))
                {
                    dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
                }
#endif
            }
        }
    }
}
