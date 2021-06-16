// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;
using static Vortice.Direct3D12.D3D12;
using Vortice.Direct3D12.Debug;
using System.Diagnostics;

namespace Vortice.Graphics.D3D12
{
    public static unsafe class D3D12DeviceHelper
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        public static readonly Lazy<IDXGIFactory4> DXGIFactory = new(CreateFactory);

        public static bool IsTearingSupported { get; private set; }

        public static readonly Lazy<D3D12GraphicsDevice> DefaultDevice = new(GetDefaultDevice);

        private static bool CheckIsSupported()
        {
            return Vortice.Direct3D12.D3D12.IsSupported(FeatureLevel.Level_11_0);
        }

        private static IDXGIFactory4 CreateFactory()
        {
            bool debug = false;
            if (debug)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug? debugController).Success)
                {
                    debugController!.EnableDebugLayer();
                    debugController!.Dispose();
                }
                else
                {
                    Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                }

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIInfoQueue? dxgiInfoQueue).Success)
                {
                    dxgiInfoQueue!.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue!.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                    DXGI.InfoQueueFilter filter = new DXGI.InfoQueueFilter()
                    {
                        DenyList = new DXGI.InfoQueueFilterDescription()
                        {
                            Ids = new int[]
                            {
                                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                            }
                        }
                    };

                    dxgiInfoQueue!.AddStorageFilterEntries(Dxgi, filter);
                    dxgiInfoQueue!.Dispose();
                }
                else
                {
                    debug = false;
                }
#endif
            }

            CreateDXGIFactory2(debug, out IDXGIFactory4? factory).CheckError();

            using (IDXGIFactory5? factory5 = factory!.QueryInterfaceOrNull<IDXGIFactory5>())
            {
                if (factory5 != null)
                {
                    IsTearingSupported = factory5.PresentAllowTearing;
                }
            }

            return factory!;
        }

        private static D3D12GraphicsDevice GetDefaultDevice()
        {
            for (int adapterIndex = 0; DXGIFactory.Value.EnumAdapters1(adapterIndex, out IDXGIAdapter1 adapter).Success; adapterIndex++)
            {
                AdapterDescription1 desc = adapter.Description1;

                if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                {
                    // Don't select the Basic Render Driver adapter.
                    adapter.Dispose();
                    continue;
                }

                if (D3D12CreateDevice(adapter, FeatureLevel.Level_11_0, out ID3D12Device2? device).Success)
                {
                    return new D3D12GraphicsDevice(device!, adapter);
                }
            }

            throw new GraphicsException("No Direct3D 12 device found");
        }
    }
}
