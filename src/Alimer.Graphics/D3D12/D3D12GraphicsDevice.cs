// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;
using Vortice.Direct3D;
using Vortice.Direct3D12;

namespace Alimer.Graphics.D3D12
{
    internal class D3D12GraphicsDevice : GraphicsDevice
    {
        public readonly IDXGIFactory4 DXGIFactory;
        public readonly bool IsTearingSupported;
        public readonly ID3D12Device NativeDevice;

        public static bool IsSupported()
        {
            try
            {
                if (CreateDXGIFactory1(out IDXGIFactory4 dxgiFactory).Failure)
                {
                    return false;
                }

                dxgiFactory.Dispose();

#if DEBUG
                if (DXGIGetDebugInterface1<IDXGIDebug1>(out IDXGIDebug1? dxgiDebug).Success)
                {
                    dxgiDebug.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                    dxgiDebug.Release();
                }
#endif

                return true;
            }
            catch
            {
                return false;
            }
        }

        public D3D12GraphicsDevice(GraphicsAdapterPreference adapterPreference, FeatureLevel minFeatureLevel = FeatureLevel.Level_11_1)
            : base()
        {
            bool debugFactory = false;

            if (EnableValidation || EnableGPUBasedValidation)
            {
                if (D3D12GetDebugInterface<ID3D12Debug>(out ID3D12Debug? debugController).Success)
                {
                    // Enable the D3D12 debug layer.
                    debugController.EnableDebugLayer();

                    if (EnableGPUBasedValidation)
                    {
                        ID3D12Debug1? debugController1 = debugController.QueryInterfaceOrNull<ID3D12Debug1>();
                        if (debugController1 != null)
                        {
                            debugController1.SetEnableGPUBasedValidation(true);
                            debugController1.SetEnableSynchronizedCommandQueueValidation(true);
                            debugController1.Dispose();
                        }
                    }

                    debugController.Dispose();
                }

                if (DXGIGetDebugInterface1<IDXGIInfoQueue>(out IDXGIInfoQueue? dxgiInfoQueue).Success)
                {
                    debugFactory = true;

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

            if (CreateDXGIFactory2(debugFactory, out DXGIFactory).Failure)
            {
                throw new NotSupportedException("IDXGIFactory4 creation failed");
            }

            // Check for tearing support.
            IDXGIFactory5? dxgiFactory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;

                dxgiFactory5.Dispose();
            }

            // Find adapter now.
            IDXGIAdapter1? adapter = D3D12Utils.GetAdapter(DXGIFactory, minFeatureLevel, adapterPreference);
            if (adapter == null)
            {
                throw new NotSupportedException("Direct3D12: Cannot find suitable adapter with Direct3D12 support.");
            }

            D3D12CreateDevice(adapter, minFeatureLevel, out NativeDevice).CheckError();

            // Init capabilites.
            {
                AdapterDescription1 adapterDescription = adapter.Description1;

                // Detect adapter type.
                GraphicsAdapterType adapterType = GraphicsAdapterType.Unknown;
                if ((adapterDescription.Flags & AdapterFlags.Software) != AdapterFlags.None)
                {
                    adapterType = GraphicsAdapterType.CPU;
                }
                else
                {
                    adapterType = NativeDevice.Architecture.Uma ? GraphicsAdapterType.IntegratedGPU : GraphicsAdapterType.DiscreteGPU;
                }

                Capabilities = new GraphicsDeviceCaps
                {
                    BackendType = BackendType.Direct3D12,
                    DeviceId = adapterDescription.DeviceId,
                    VendorId = adapterDescription.VendorId,
                    AdapterName = adapterDescription.Description,
                    AdapterType = adapterType
                };

                adapter.Release();
            }
        }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities { get; }

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
                DXGIFactory.Dispose();

#if DEBUG
                if (DXGIGetDebugInterface1<IDXGIDebug1>(out IDXGIDebug1? dxgiDebug).Success)
                {
                    dxgiDebug.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                    dxgiDebug.Release();
                }
#endif
            }
        }
    }
}
