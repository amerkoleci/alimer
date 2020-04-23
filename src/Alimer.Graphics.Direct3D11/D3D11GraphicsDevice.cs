// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.Direct3D11;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.Direct3D11
{
    public sealed class D3D11GraphicsDevice : GraphicsDevice
    {
        private static IDXGIFactory2? s_factory;

        public D3D11GraphicsDevice(GPUPowerPreference powerPreference = GPUPowerPreference.HighPerformance)
        {
            // Detect adapter to use.
            IDXGIAdapter1 dxgiAdapter = null;

            var factory6 = Factory.QueryInterfaceOrNull<IDXGIFactory6>();
            if (factory6 != null)
            {
                var gpuPreference = GpuPreference.HighPerformance;
                if (powerPreference == GPUPowerPreference.LowPower)
                {
                    gpuPreference = GpuPreference.MinimumPower;
                }

                var adapters = factory6.EnumAdaptersByGpuPreference<IDXGIAdapter1>(gpuPreference);
                foreach (var adapter in adapters)
                {
                    if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    dxgiAdapter = adapter;
                    dxgiAdapter.AddRef();
                    break;
                }

                Utilities.ReleaseIfNotDefault(adapters);
                factory6.Dispose();
            }

            if (dxgiAdapter == null)
            {
                var dxgiAdapters = Factory.EnumAdapters1();
                foreach (var adapter in dxgiAdapters)
                {
                    if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    dxgiAdapter = adapter;
                    dxgiAdapter.AddRef();
                    break;
                }
                Utilities.ReleaseIfNotDefault(dxgiAdapters);
            }

            Adapter = new D3D11GraphicsAdapter(dxgiAdapter);
            dxgiAdapter.Release();
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D11GraphicsDevice" /> class.
        /// </summary>
        ~D3D11GraphicsDevice()
        {
            Dispose(disposing: false);
        }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                NativeDevice.Dispose();
            }
        }

        public static IDXGIFactory2 Factory { get => s_factory ??= InitFactory(); set => s_factory = value; }
        public static bool IsTearingSupported { get; private set; }

        internal ID3D11Device1 NativeDevice { get; }

        private static IDXGIFactory2 InitFactory()
        {
            // Just try to enable debug layer.
            IDXGIFactory2 dxgiFactory = null;

            var debugDXGI = false;
            if (EnableValidation
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
    }
}
