// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using TerraFX.Interop;
using FX = TerraFX.Interop.Windows;
using System.Diagnostics;
using static TerraFX.Interop.DXGI_FEATURE;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;

namespace Vortice.Graphics.D3D12
{
    internal static unsafe class D3D12DeviceHelper
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        public static readonly ComPtr<IDXGIFactory4> DXGIFactory = CreateFactory();

        public static bool IsTearingSupported { get; private set; }

        public static readonly Lazy<GraphicsDeviceD3D12> DefaultDevice = new(GetDefaultDevice);

        private static bool CheckIsSupported()
        {
            try
            {
                HRESULT result = FX.D3D12CreateDevice(null, D3D_FEATURE_LEVEL_11_0, FX.__uuidof<ID3D12Device>(), null);
                return FX.SUCCEEDED(result);
            }
            catch
            {
                // On pre Windows 10 d3d12.dll is not present and therefore not supported.
                return false;
            }
        }

        private static ComPtr<IDXGIFactory4> CreateFactory()
        {
            uint factoryFlags = 0;
            if (GraphicsDevice.EnableValidationLayers)
            {
                using ComPtr<ID3D12Debug> d3d12Debug = default;
                using ComPtr<ID3D12Debug1> d3d12Debug1 = default;

                if (FX.SUCCEEDED(FX.D3D12GetDebugInterface(FX.__uuidof<ID3D12Debug>(), d3d12Debug.GetVoidAddressOf())))
                {
                    d3d12Debug.Get()->EnableDebugLayer();

                    if (FX.SUCCEEDED(d3d12Debug.CopyTo(d3d12Debug1.GetAddressOf())))
                    {
                        d3d12Debug1.Get()->SetEnableGPUBasedValidation(GraphicsDevice.EnableGpuBasedValidation ? FX.TRUE : FX.FALSE);
                    }
                }
                else
                {
                    Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                }

#if DEBUG
                using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;
                if (FX.SUCCEEDED(FX.DXGIGetDebugInterface1(0, FX.__uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf())))
                {
                    factoryFlags = FX.DXGI_CREATE_FACTORY_DEBUG;

                    dxgiInfoQueue.Get()->SetBreakOnSeverity(FX.DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, FX.TRUE);
                    dxgiInfoQueue.Get()->SetBreakOnSeverity(FX.DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, FX.TRUE);

                    Span<int> hide = stackalloc int[]
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

                        dxgiInfoQueue.Get()->AddStorageFilterEntries(FX.DXGI_DEBUG_DXGI, &filter);
                    }
                }
#endif
            }

            ComPtr<IDXGIFactory4> dxgiFactory4 = default;
            FX.CreateDXGIFactory2(factoryFlags, FX.__uuidof<IDXGIFactory4>(), dxgiFactory4.GetVoidAddressOf()).Assert();

            using ComPtr<IDXGIFactory5> dxgiFactory5 = default;
            if (FX.SUCCEEDED(dxgiFactory4.CopyTo(dxgiFactory5.GetAddressOf())))
            {
                int allowTearing = FX.FALSE;
                if (FX.FAILED(dxgiFactory5.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, (uint)sizeof(int)))
                    && allowTearing == FX.FALSE)
                {
                    IsTearingSupported = false;
                }
                else
                {
                    IsTearingSupported = true;
                }
            }


            return dxgiFactory4;
        }

        private static GraphicsDeviceD3D12 GetDefaultDevice()
        {
            uint i = 0;

            while (true)
            {
                using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;
                HRESULT result = DXGIFactory.Get()->EnumAdapters1(i++, dxgiAdapter1.GetAddressOf());

                if (result == FX.DXGI_ERROR_NOT_FOUND)
                {
                    break;
                }

                result.Assert();

                DXGI_ADAPTER_DESC1 adapterDesc;
                dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

                if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                HRESULT createDeviceResult = FX.D3D12CreateDevice(
                    dxgiAdapter1.AsIUnknown().Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    FX.__uuidof<ID3D12Device>(),
                    null);

                if (FX.SUCCEEDED(createDeviceResult))
                {
                    return new GraphicsDeviceD3D12(dxgiAdapter1);
                }
            }

            throw new GraphicsException("No Direct3D 12 device found");
        }
    }
}
