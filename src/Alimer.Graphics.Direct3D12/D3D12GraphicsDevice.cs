// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;
using SharpGen.Runtime;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.Direct3D12
{
    public sealed class D3D12GraphicsDevice : GraphicsDevice
    {
        private const FeatureLevel MinFeatureLevel = FeatureLevel.Level_11_0;
        private static IDXGIFactory4? s_factory;

        public D3D12GraphicsDevice(GPUPowerPreference powerPreference = GPUPowerPreference.HighPerformance)
        {
            ID3D12Device d3dDevice = null;

            // Detect adapter to use.
            IDXGIFactory6 factory6 = Factory.QueryInterfaceOrNull<IDXGIFactory6>();
            if (factory6 != null)
            {
                GpuPreference gpuPreference = GpuPreference.HighPerformance;
                if (powerPreference == GPUPowerPreference.LowPower)
                {
                    gpuPreference = GpuPreference.MinimumPower;
                }


                var adapters = factory6.EnumAdaptersByGpuPreference<IDXGIAdapter1>(gpuPreference);
                foreach (var dXGIAdapter in adapters)
                {
                    if ((dXGIAdapter.Description1.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (D3D12CreateDevice(dXGIAdapter, MinFeatureLevel, out d3dDevice).Success)
                    {
                        Adapter = new D3D12GraphicsAdapter(dXGIAdapter);

                        break;
                    }
                }

                Utilities.ReleaseIfNotDefault(adapters);
                factory6.Dispose();
            }

            if (d3dDevice == null)
            {
                var dxgiAdapters = Factory.EnumAdapters1();
                foreach (var dxgiAdapter in dxgiAdapters)
                {
                    if ((dxgiAdapter.Description1.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (D3D12CreateDevice(dxgiAdapter, MinFeatureLevel, out d3dDevice).Success)
                    {
                        Adapter = new D3D12GraphicsAdapter(dxgiAdapter);
                        break;
                    }
                }

                Utilities.ReleaseIfNotDefault(dxgiAdapters);
            }

#if DEBUG
            if (d3dDevice == null)
            {
                // Try WARP12 instead
                var dxgiAdapter = Factory.GetWarpAdapter<IDXGIAdapter1>();
                if (dxgiAdapter == null ||
                    D3D12CreateDevice(dxgiAdapter, MinFeatureLevel, out d3dDevice).Success)
                {
                    throw new NotSupportedException("WARP12 not available. Enable the 'Graphics Tools' optional feature");
                }

                Adapter = new D3D12GraphicsAdapter(dxgiAdapter);
                Debug.WriteLine("Direct3D Adapter - WARP12");
                dxgiAdapter.Release();
            }
#endif

            if (d3dDevice == null)
            {
                throw new NotSupportedException("Direct3D12 device creation failed");
            }

            if (DXGIGetDebugInterface1<IDXGIDebug1>(out var dxgiDebug).Success)
            {
                dxgiDebug.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                dxgiDebug.Release();
            }

            NativeDevice = d3dDevice;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
        /// </summary>
        ~D3D12GraphicsDevice()
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

        public static IDXGIFactory4 Factory { get => s_factory ??= InitFactory(); set => s_factory = value; }
        public static bool IsTearingSupported { get; private set; }

        internal ID3D12Device NativeDevice { get; }

        private static IDXGIFactory4 InitFactory()
        {
            var validation = false;
            if (EnableValidation
                && D3D12GetDebugInterface<ID3D12Debug>(out var debugController).Success)
            {
                // Enable the D3D12 debug layer.
                debugController.EnableDebugLayer();

                var debugController1 = debugController.QueryInterfaceOrNull<ID3D12Debug1>();
                if (debugController1 != null)
                {
                    if (EnableGPUBasedValidation)
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
    }
}
