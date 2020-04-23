// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Alimer.Graphics.Direct3D12
{
    public sealed class D3D12GraphicsAdapter : GraphicsAdapter
    {
        private readonly IDXGIAdapter1 _dxgiAdapter;

        internal D3D12GraphicsAdapter(D3D12GraphicsProvider provider, IDXGIAdapter1 dxgiAdapter)
            : base(provider)
        {
            Guard.AssertNotNull(dxgiAdapter, nameof(dxgiAdapter));

            _dxgiAdapter = dxgiAdapter;

            var adapterDescription = dxgiAdapter.Description1;
            VendorId = adapterDescription.VendorId;
            DeviceId = adapterDescription.DeviceId;
            Name = adapterDescription.Description;
        }

        public override int VendorId { get; }

        public override int DeviceId { get; }

        public override string Name { get; }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _dxgiAdapter.Dispose();
            }
        }
    }
}
