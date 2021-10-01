// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using Vortice.Direct3D;
using Vortice.DXGI;
using Vortice.DXGI.Debug;
using static Vortice.Direct3D11.D3D11;
using static Vortice.DXGI.DXGI;
using InfoQueueFilter = Vortice.DXGI.Debug.InfoQueueFilter;

namespace Vortice.Graphics
{
    internal class D3D11GraphicsDeviceFactory : GraphicsDeviceFactory
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        private static bool CheckIsSupported() => IsSupportedFeatureLevel(FeatureLevel.Level_11_0);

        public D3D11GraphicsDeviceFactory(ValidationMode validationMode)
            : base(validationMode)
        {
            if (OperatingSystem.IsWindowsVersionAtLeast(8, 1))
            {
                if (validationMode != ValidationMode.Disabled)
                {
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
                }
#endif
                DXGIFactory = CreateDXGIFactory2<IDXGIFactory2>(validationMode != ValidationMode.Disabled);
            }
            else
            {
                DXGIFactory = CreateDXGIFactory1<IDXGIFactory2>();
            }

            IDXGIFactory5? dxgiFactory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;
                dxgiFactory5.Dispose();
            }

            // Enumerate physical devices
            List<D3D11.D3D11PhysicalDevice> physicalDevices = new();
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

                    physicalDevices.Add(new D3D11.D3D11PhysicalDevice(this, adapter));
                }

                dxgiFactory6.Dispose();
            }
            else
            {
                for (int adapterIndex = 0; DXGIFactory.EnumAdapters1(adapterIndex, out IDXGIAdapter1 adapter).Success; adapterIndex++)
                {
                    AdapterDescription1 desc = adapter.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter.Dispose();
                        continue;
                    }

                    physicalDevices.Add(new D3D11.D3D11PhysicalDevice(this, adapter));
                }
            }

            PhysicalDevices = physicalDevices.AsReadOnly();
        }

        public IDXGIFactory2 DXGIFactory { get; }
        public bool IsTearingSupported { get; private set; }

        /// <inheritdoc />
        public override IReadOnlyList<PhysicalDevice> PhysicalDevices { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                foreach (D3D11.D3D11PhysicalDevice physicalDevice in PhysicalDevices)
                {
                    physicalDevice.Dispose();
                }

                DXGIFactory.Dispose();

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIDebug1? dxgiDebug).Success)
                {
                    dxgiDebug.ReportLiveObjects(DebugAll, ReportLiveObjectFlags.Summary | ReportLiveObjectFlags.IgnoreInternal);
                    dxgiDebug.Dispose();
                }
#endif
            }
        }
    }
}
