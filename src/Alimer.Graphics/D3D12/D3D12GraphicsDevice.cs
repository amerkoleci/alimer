// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Immutable;
using System.Diagnostics;
using SharpGen.Runtime;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.D3D12
{
    internal class D3D12GraphicsDevice : GraphicsDevice
    {
        private const FeatureLevel MinFeatureLevel = FeatureLevel.Level_11_0;
        private readonly GraphicsDeviceCapabilities _capabilities;

        public D3D12GraphicsDevice(in DeviceDescriptor descriptor)
        {
            ID3D12Device? d3dDevice = null;

            // Detect adapter to use.
            ImmutableArray<IDXGIAdapter1> dxgiAdapters;

            if (descriptor.PowerPreference != GPUPowerPreference.Default)
            {
                IDXGIFactory6 factory6 = D3D12Instance.Factory.QueryInterfaceOrNull<IDXGIFactory6>();
                if (factory6 != null)
                {
                    GpuPreference gpuPreference = GpuPreference.HighPerformance;
                    if (descriptor.PowerPreference == GPUPowerPreference.LowPower)
                    {
                        gpuPreference = GpuPreference.MinimumPower;
                    }


                    dxgiAdapters = factory6.EnumAdaptersByGpuPreference<IDXGIAdapter1>(gpuPreference);
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
                            var adapterDescription = dxgiAdapter.Description1;
                            _capabilities.VendorId = adapterDescription.VendorId;
                            _capabilities.DeviceId = adapterDescription.DeviceId;
                            _capabilities.AdapterName = adapterDescription.Description;
                            break;
                        }
                    }

                    factory6.Dispose();
                }
            }

            Utilities.ReleaseIfNotDefault(dxgiAdapters);

            if (d3dDevice == null)
            {
                dxgiAdapters = D3D12Instance.Factory.EnumAdapters1();
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
                        var adapterDescription = dxgiAdapter.Description1;
                        _capabilities.VendorId = adapterDescription.VendorId;
                        _capabilities.DeviceId = adapterDescription.DeviceId;
                        _capabilities.AdapterName = adapterDescription.Description;
                        break;
                    }
                }

            }

            Utilities.ReleaseIfNotDefault(dxgiAdapters);
#if DEBUG
            if (d3dDevice == null)
            {
                // Try WARP12 instead
                var dxgiAdapter = D3D12Instance.Factory.GetWarpAdapter<IDXGIAdapter1>();
                if (dxgiAdapter == null ||
                    D3D12CreateDevice(dxgiAdapter, MinFeatureLevel, out d3dDevice).Success)
                {
                    throw new NotSupportedException("WARP12 not available. Enable the 'Graphics Tools' optional feature");
                }

                Debug.WriteLine("Direct3D Adapter - WARP12");
                var adapterDescription = dxgiAdapter.Description1;
                _capabilities.VendorId = adapterDescription.VendorId;
                _capabilities.DeviceId = adapterDescription.DeviceId;
                _capabilities.AdapterName = adapterDescription.Description;
                dxgiAdapter.Release();
            }
#endif

            if (d3dDevice == null)
            {
                throw new NotSupportedException("Direct3D12 device creation failed");
            }

            NativeDevice = d3dDevice;

            // Init capabilities.
            _capabilities.Backend = BackendType.Direct3D12;
        }

        internal ID3D12Device NativeDevice { get; }
        public override GraphicsDeviceCapabilities Capabilities => _capabilities;

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
    }
}
