// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.DXGI_SWAP_CHAIN_FLAG;
using static TerraFX.Interop.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;
using System;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12SwapChain : SwapChain
    {
        private IDXGISwapChain3* _handle;

        public D3D12SwapChain(D3D12GraphicsDevice device, IntPtr windowHandle, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            using ComPtr<IDXGISwapChain1> swapChain = null;

            var swapChainDesc = new DXGI_SWAP_CHAIN_DESC1
            {
                Width = (uint)descriptor.Width,
                Height = (uint)descriptor.Height,
                Format = ToDXGISwapChainFormat(descriptor.ColorFormat),
                BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                BufferCount = 2,
                SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                SampleDesc = new DXGI_SAMPLE_DESC(count: 1, quality: 0),
                Flags = device.IsTearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
            };

            var fullscreenDesc = new DXGI_SWAP_CHAIN_FULLSCREEN_DESC
            {
                Windowed = descriptor.IsFullscreen ? FALSE : TRUE
            };

            ThrowIfFailed(device.DXGIFactory->CreateSwapChainForHwnd(
                (IUnknown*)device.DirectQueue,
                windowHandle,
                &swapChainDesc,
                pFullscreenDesc: &fullscreenDesc,
                pRestrictToOutput: null,
                swapChain.GetAddressOf()
            ));

            IDXGISwapChain3* swapChain3;
            ThrowIfFailed(swapChain.Get()->QueryInterface(__uuidof<IDXGISwapChain3>(), (void**)&swapChain3));
            _handle = swapChain3;
            AfterReset();
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _handle->Release();
                _handle = null;
            }
        }

        public override void Present()
        {
            ThrowIfFailed(_handle->Present(SyncInterval: 1, Flags: 0));
        }

        private void AfterReset()
        {
            DXGI_SWAP_CHAIN_DESC1 desc;
            ThrowIfFailed(_handle->GetDesc1(&desc));

            Width = (int)desc.Width;
            Height = (int)desc.Height;
        }
    }
}
