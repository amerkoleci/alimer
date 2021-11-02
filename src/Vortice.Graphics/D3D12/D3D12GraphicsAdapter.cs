// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using TerraFX.Interop;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12GraphicsAdapter : GraphicsAdapter, IDisposable
    {
        private readonly ComPtr<IDXGIAdapter1> _adapter;
        private readonly ComPtr<ID3D12Device2> _device;

        public D3D12GraphicsAdapter(D3D12GraphicsDeviceFactory factory, ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device2> device)
        {
            Factory = factory;
            _adapter = adapter;
            _device = device;

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
                D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1 = _device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_ARCHITECTURE1>(D3D12_FEATURE_ARCHITECTURE1);

                adapterType = architecture1.UMA == TRUE ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = architecture1.CacheCoherentUMA == TRUE;
            }

            VendorId = (VendorId)adapterDesc.VendorId;
            AdapterId = adapterDesc.DeviceId;
            AdapterType = adapterType;
            AdapterName = new string((char*)adapterDesc.Description);
        }

        public D3D12GraphicsDeviceFactory Factory { get; }

        public IDXGIAdapter1* Adapter => _adapter;

        public ComPtr<ID3D12Device2> NativeDevice => _device;

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

        /// <inheritdoc />
        public void Dispose()
        {
#if DEBUG
            uint refCount = _device.Get()->Release();
            if (refCount > 0)
            {
                System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D12DebugDevice> d3d12DebugDevice = default;
                if (SUCCEEDED(_device.CopyTo(d3d12DebugDevice.GetAddressOf())))
                {
                    d3d12DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                }
            }
#else
            _device.Dispose();
#endif
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
