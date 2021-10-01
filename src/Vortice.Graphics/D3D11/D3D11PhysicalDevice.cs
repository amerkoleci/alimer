// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D11
{
    internal class D3D11PhysicalDevice : PhysicalDevice, IDisposable
    {
        public D3D11PhysicalDevice(D3D11GraphicsDeviceFactory factory, IDXGIAdapter1 adapter)
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
                adapterType = GPUAdapterType.DiscreteGPU;
            }

            VendorId = (VendorId)adapterDesc.VendorId;
            AdapterId = (uint)adapterDesc.DeviceId;
            AdapterType = adapterType;
            AdapterName = adapterDesc.Description;
        }

        public D3D11GraphicsDeviceFactory Factory { get; }

        public IDXGIAdapter1 Adapter { get; }


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
        public override GraphicsDevice CreateDevice(string? name = null) => new D3D11GraphicsDevice(this, name);
    }
}
