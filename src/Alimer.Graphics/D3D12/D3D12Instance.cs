// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.D3D12
{
    public static class D3D12Instance
    {
        private static IDXGIFactory4? s_factory;

        public static IDXGIFactory4 Factory { get => s_factory ??= InitFactory(); set => s_factory = value; }
        public static bool IsTearingSupported { get; private set; }

        private static IDXGIFactory4 InitFactory()
        {
            var validation = false;
            if (GraphicsDevice.EnableValidation
                && D3D12GetDebugInterface<ID3D12Debug>(out var debugController).Success)
            {
                // Enable the D3D12 debug layer.
                debugController.EnableDebugLayer();

                var debugController1 = debugController.QueryInterfaceOrNull<ID3D12Debug1>();
                if (debugController1 != null)
                {
                    if (GraphicsDevice.EnableGPUBasedValidation)
                    {
                        debugController1.SetEnableGPUBasedValidation(true);
                        debugController1.SetEnableSynchronizedCommandQueueValidation(true);
                    }
                    else
                    {
                        debugController1.SetEnableGPUBasedValidation(false);
                    }
                    debugController1.Dispose();
                }

                debugController.Dispose();

                if (DXGIGetDebugInterface1<IDXGIInfoQueue>(out var dxgiInfoQueue).Success)
                {
                    validation = true;

                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                    dxgiInfoQueue.AddStorageFilterEntries(Dxgi, new Vortice.DXGI.InfoQueueFilter
                    {
                        DenyList = new Vortice.DXGI.InfoQueueFilterDescription
                        {
                            Ids = new[]
                            {
                                80 // IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides
                            }
                        }
                    });

                    dxgiInfoQueue.Dispose();
                }
            }

            if (CreateDXGIFactory2(validation, out IDXGIFactory4 dxgiFactory).Failure)
            {
                throw new NotSupportedException("IDXGIFactory4 creation failed");
            }

            // Check for tearing support.
            var dxgiFactory5 = dxgiFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;

                dxgiFactory5.Dispose();
            }

            return dxgiFactory;
        }
        
        public static void Shutdown()
        {
            s_factory?.Dispose();

#if DEBUG
            if (DXGIGetDebugInterface1<IDXGIDebug1>(out var dxgiDebug).Success)
            {
                dxgiDebug.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                dxgiDebug.Release();
            }
#endif
        }


        public static bool IsSupported()
        {
            try
            {
                return Factory != null;
            }
            catch
            {
                return false;
            }
        }
    }
}
