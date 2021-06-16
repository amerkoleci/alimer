// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    public sealed unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        internal D3D12GraphicsDevice(ID3D12Device2 device, IDXGIAdapter1 adapter)
        {
            NativeDevice = device;
            Adapter = adapter;
        }

        public ID3D12Device2 NativeDevice { get; }
        public IDXGIAdapter1 Adapter { get; }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
        /// </summary>
        ~D3D12GraphicsDevice() => Dispose(isDisposing: false);


        /// <inheritdoc />
        protected override void Dispose(bool isDisposing)
        {
        }
    }
}
