// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using static Vortice.Graphics.D3D12.D3D12Utils;
using System;
using Vortice.DXGI;
using Vortice.Mathematics;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12SwapChain : SwapChain
    {
        private PresentFlags _noSyncPresentFlags = PresentFlags.None;

        public D3D12SwapChain(D3D12GraphicsDevice device, IntPtr windowHandle, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            SwapChainDescription1 swapChainDesc = new()
            {
                Width = descriptor.Width,
                Height = descriptor.Height,
                Format = ToDXGISwapChainFormat(descriptor.ColorFormat),
                Usage = Usage.RenderTargetOutput,
                BufferCount = 2,
                SwapEffect = SwapEffect.FlipDiscard,
                SampleDescription = new SampleDescription(1, 0),
                Flags = device.IsTearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
            };

            if (device.IsTearingSupported)
            {
                _noSyncPresentFlags = PresentFlags.AllowTearing;
            }

            SwapChainFullscreenDescription fullscreenDesc = new()
            {
                Windowed = !descriptor.IsFullscreen
            };

            using (IDXGISwapChain1 tempSwapChain = device.DXGIFactory.CreateSwapChainForHwnd(
                device.DirectQueue,
                windowHandle,
                swapChainDesc,
                fullscreenDesc))
            {
                Handle = tempSwapChain.QueryInterface<IDXGISwapChain3>();
            }

            AfterReset();
        }

        public IDXGISwapChain3 Handle { get; }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Handle?.Dispose();
            }
        }

        public override void Present()
        {
            if (EnableVerticalSync)
            {
                Handle.Present(1, PresentFlags.None);
            }
            else
            {
                Handle.Present(0, _noSyncPresentFlags);
            }
        }

        private void AfterReset()
        {
            SwapChainDescription1 swapChainDesc = Handle.Description1;

            Size = new Size(swapChainDesc.Width, swapChainDesc.Height);
        }
    }
}
