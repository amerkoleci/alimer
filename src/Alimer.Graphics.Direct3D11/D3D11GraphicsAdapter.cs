// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.DXGI;

namespace Alimer.Graphics.Direct3D11
{
    public sealed class D3D11GraphicsAdapter : GraphicsAdapter
    {
        internal D3D11GraphicsAdapter(IDXGIAdapter1 dxgiAdapter)
        {
            Guard.AssertNotNull(dxgiAdapter, nameof(dxgiAdapter));

            var adapterDescription = dxgiAdapter.Description1;
            VendorId = adapterDescription.VendorId;
            DeviceId = adapterDescription.DeviceId;
            Name = adapterDescription.Description;
        }

        public override int VendorId { get; }

        public override int DeviceId { get; }

        public override string Name { get; }
    }
}
