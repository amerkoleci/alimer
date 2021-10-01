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
using System.Collections.Generic;

namespace Vortice.Graphics
{
    internal class D3D12GraphicsDeviceFactory : GraphicsDeviceFactory
    {
        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        private static bool CheckIsSupported() => IsSupported(FeatureLevel.Level_12_0);

        public D3D12GraphicsDeviceFactory(ValidationMode validationMode)
            : base(validationMode)
        {
            if (validationMode != ValidationMode.Disabled)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug? d3d12Debug).Success)
                {
                    d3d12Debug!.EnableDebugLayer();

                    if (validationMode == ValidationMode.GPU)
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
            }
#endif
            DXGIFactory = CreateDXGIFactory2<IDXGIFactory4>(validationMode != ValidationMode.Disabled);

            IDXGIFactory5? dxgiFactory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;
                dxgiFactory5.Dispose();
            }

            // Enumerate physical devices
            List<D3D12.D3D12PhysicalDevice> physicalDevices = new();
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
                        physicalDevices.Add(new D3D12.D3D12PhysicalDevice(this, adapter, device.Architecture1));
                        device.Dispose();
                    }
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

                    if (D3D12CreateDevice(adapter, FeatureLevel.Level_12_0, out ID3D12Device2? device).Success)
                    {
                        physicalDevices.Add(new D3D12.D3D12PhysicalDevice(this, adapter, device.Architecture1));
                        device.Dispose();
                    }
                }
            }

            PhysicalDevices = physicalDevices.AsReadOnly();
        }

        public IDXGIFactory4 DXGIFactory { get; }
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
