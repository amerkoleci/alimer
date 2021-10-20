// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using TerraFX.Interop;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.Windows;
using static Vortice.MarshalUtilities;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12PhysicalDevice : PhysicalDevice, IDisposable
    {
        private readonly ComPtr<IDXGIAdapter1> _adapter;

        public D3D12PhysicalDevice(D3D12GraphicsDeviceFactory factory, ComPtr<IDXGIAdapter1> adapter, in D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1)
        {
            Factory = factory;
            _adapter = adapter;

            DXGI_ADAPTER_DESC1 adapterDesc;
            adapter.Get()->GetDesc1(&adapterDesc).Assert();

            // Init capabilites.
            GPUAdapterType adapterType;
            if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                adapterType = GPUAdapterType.CPU;
            }
            else
            {
                adapterType = architecture1.UMA == TRUE ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = architecture1.CacheCoherentUMA == TRUE;
            }

            VendorId = (VendorId)adapterDesc.VendorId;
            AdapterId = adapterDesc.DeviceId;
            AdapterType = adapterType;
            AdapterName = GetUtf16Span(in adapterDesc.Description[0], 128).GetString();
        }

        public D3D12GraphicsDeviceFactory Factory { get; }

        public IDXGIAdapter1* Adapter => _adapter;

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

        /// <inheritdoc />
        public void Dispose()
        {
            _adapter.Dispose();
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
