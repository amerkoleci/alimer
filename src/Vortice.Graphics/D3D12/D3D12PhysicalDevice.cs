// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    internal class D3D12PhysicalDevice : PhysicalDevice, IDisposable
    {
        public D3D12PhysicalDevice(D3D12GraphicsDeviceFactory factory, IDXGIAdapter1 adapter, in FeatureDataArchitecture1 featureDataArchitecture)
        {
            Factory = factory;
            Adapter = adapter;

            AdapterDescription1 adapterDesc = adapter.Description1;

            // Init capabilites.
            GPUAdapterType adapterType;
            if ((adapterDesc.Flags & AdapterFlags.Software) != 0)
            {
                adapterType = GPUAdapterType.CPU;
            }
            else
            {
                adapterType = featureDataArchitecture.Uma ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA;
            }

            VendorId = (VendorId)adapterDesc.VendorId;
            AdapterId = (uint)adapterDesc.DeviceId;
            AdapterType = adapterType;
            AdapterName = adapterDesc.Description;
        }

        public D3D12GraphicsDeviceFactory Factory { get; }

        public IDXGIAdapter1 Adapter { get; }

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

        /// <inheritdoc />
        public void Dispose()
        {
            Adapter.Dispose();
        }

        /// <inheritdoc />
        public override VendorId VendorId { get; }

        /// <inheritdoc />
        public override uint AdapterId { get; }

        /// <inheritdoc />
        public override GPUAdapterType AdapterType { get; }

        /// <inheritdoc />
        public override string AdapterName { get; }

        /// <inheritdoc />
        public override GraphicsDevice CreateDevice(string? name = null) => new D3D12GraphicsDevice(this, name);
    }
}
