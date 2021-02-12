// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;
using System;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12SwapChain : SwapChain
    {
        public D3D12SwapChain(D3D12GraphicsDevice device, IntPtr windowHandle, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            using ComPtr<IDXGISwapChain1> swapChain = null;

            var swapChainDesc = new DXGI_SWAP_CHAIN_DESC1
            {
                Width = (uint)descriptor.Width,
                Height = (uint)descriptor.Height,
                Format = ToDXGIFormat(descriptor.ColorFormat),
                BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                BufferCount = 2,
                SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                SampleDesc = new DXGI_SAMPLE_DESC(count: 1, quality: 0),
            };

            ThrowIfFailed(D3D12GraphicsDevice.DxgiFactory->CreateSwapChainForHwnd(
                (IUnknown*)device.DirectQueue,
                windowHandle,
                &swapChainDesc,
                pFullscreenDesc: null,
                pRestrictToOutput: null,
                swapChain.GetAddressOf()
            ));
        }

        protected override void Dispose(bool disposing)
        {

        }
    }
}
