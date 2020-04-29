// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.DXGI;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.D3D11
{
    public static class D3D11Instance
    {
        private static IDXGIFactory2 s_factory;

        public static IDXGIFactory2 Factory { get => s_factory ??= InitFactory(); set => s_factory = value; }
        public static bool IsTearingSupported { get; private set; }

        private static IDXGIFactory2 InitFactory()
        {
            IDXGIFactory2 dxgiFactory = null;

            var debugDXGI = false;
            if (GraphicsDevice.EnableValidation
                && DXGIGetDebugInterface1<IDXGIInfoQueue>(out var dxgiInfoQueue).Success)
            {
                debugDXGI = true;

                CreateDXGIFactory2(true, out dxgiFactory).CheckError();

                dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                dxgiInfoQueue.AddStorageFilterEntries(Dxgi, new InfoQueueFilter
                {
                    DenyList = new InfoQueueFilterDescription
                    {
                        Ids = new[]
                        {
                            80 // IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides
                        }
                    }
                });
            }

            if (!debugDXGI)
            {
                CreateDXGIFactory1(out dxgiFactory).CheckError();
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
