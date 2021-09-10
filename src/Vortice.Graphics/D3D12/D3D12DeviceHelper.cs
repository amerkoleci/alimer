// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Diagnostics;
using Vortice.Direct3D;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;
using static Vortice.Direct3D12.D3D12;
using Vortice.DXGI.Debug;
using InfoQueueFilter = Vortice.DXGI.Debug.InfoQueueFilter;
using Vortice.Direct3D12;

namespace Vortice.Graphics.D3D12
{
    internal static class D3D12DeviceHelper
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        public static readonly IDXGIFactory4 DXGIFactory = CreateFactory();

        public static bool IsTearingSupported { get; private set; }

        public static readonly Lazy<GraphicsDeviceD3D12> DefaultDevice = new(GetDefaultDevice);

        private static bool CheckIsSupported() => IsSupported(FeatureLevel.Level_12_0);

        private static IDXGIFactory4 CreateFactory()
        {
            if (GraphicsDevice.ValidationMode != ValidationMode.Disabled)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug? d3d12Debug).Success)
                {
                    d3d12Debug!.EnableDebugLayer();

                    if (GraphicsDevice.ValidationMode == ValidationMode.GPU)
                    {
                        ID3D12Debug1? d3d12Debug1 = d3d12Debug!.QueryInterfaceOrNull<ID3D12Debug1>();

                        if (d3d12Debug1 != null)
                        {
                            d3d12Debug1.SetEnableGPUBasedValidation(true);
                            d3d12Debug1.SetEnableSynchronizedCommandQueueValidation(true);
                            d3d12Debug1.Dispose();
                        }

                        ID3D12Debug2? d3d12Debug2 = d3d12Debug!.QueryInterfaceOrNull<ID3D12Debug2>();
                        if (d3d12Debug2 != null)
                        {
                            d3d12Debug2.SetGPUBasedValidationFlags(GpuBasedValidationFlags.None);
                            d3d12Debug2.Dispose();
                        }
                    }

                    d3d12Debug.Dispose();
                }
                else
                {
                    Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                }

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIInfoQueue? dxgiInfoQueue).Success)
                {
                    dxgiInfoQueue!.SetBreakOnSeverity(DebugAll, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue!.SetBreakOnSeverity(DebugAll, InfoQueueMessageSeverity.Corruption, true);

                    int[] hide = new int[]
                    {
                        80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                    };

                    InfoQueueFilter filter = new()
                    {
                        DenyList = new()
                        {
                            Ids = hide
                        }
                    };

                    dxgiInfoQueue.AddStorageFilterEntries(DebugDxgi, filter);
                    dxgiInfoQueue.Dispose();
                }
#endif
            }

            CreateDXGIFactory2(GraphicsDevice.ValidationMode != ValidationMode.Disabled, out IDXGIFactory4? dxgiFactory4).CheckError();

            IDXGIFactory5? dxgiFactory5 = dxgiFactory4!.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;
                dxgiFactory5.Dispose();
            }

            return dxgiFactory4;
        }

        private static GraphicsDeviceD3D12 GetDefaultDevice()
        {
            IDXGIFactory6? dxgiFactory6 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory6>();
            if (dxgiFactory6 != null)
            {
                for (int adapterIndex = 0; dxgiFactory6.EnumAdapterByGpuPreference(adapterIndex, GpuPreference.HighPerformance, out IDXGIAdapter1? adapter).Success; adapterIndex++)
                {
                    AdapterDescription1 desc = adapter!.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter.Dispose();
                        continue;
                    }

                    if (D3D12CreateDevice(adapter, FeatureLevel.Level_12_0, out ID3D12Device2? device).Success)
                    {
                        return new GraphicsDeviceD3D12(device!, adapter);
                    }
                }

                dxgiFactory6.Dispose();
            }

            for (int adapterIndex = 0; DXGIFactory.EnumAdapters1(adapterIndex, out IDXGIAdapter1 adapter).Success; adapterIndex++)
            {
                AdapterDescription1 desc = adapter.Description1;

                if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                {
                    // Don't select the Basic Render Driver adapter.
                    adapter.Dispose();
                    continue;
                }

                if (D3D12CreateDevice(adapter, FeatureLevel.Level_12_0, out ID3D12Device2? device).Success)
                {
                    return new GraphicsDeviceD3D12(device!, adapter);
                }
            }

            throw new GraphicsException("No Direct3D 12 device found");
        }
    }
}
