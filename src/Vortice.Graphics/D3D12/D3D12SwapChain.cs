// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;
using static TerraFX.Interop.DXGI_SCALING;
using static TerraFX.Interop.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.DXGI_ALPHA_MODE;
using static TerraFX.Interop.DXGI_SWAP_CHAIN_FLAG;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12SwapChain : SwapChain
    {
        private readonly ComPtr<IDXGISwapChain3> _handle;

        private readonly uint _syncInterval = 1u;
        private readonly uint _presentFlags = 0u;

        public D3D12SwapChain(D3D12GraphicsDevice device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = new()
            {
                Width = (uint)descriptor.Size.Width,
                Height = (uint)descriptor.Size.Height,
                Format = ToDXGISwapChainFormat(descriptor.ColorFormat),
                Stereo = FALSE,
                SampleDesc = new(1, 0),
                BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                BufferCount = PresentModeToBufferCount(descriptor.PresentMode),
                Scaling = DXGI_SCALING_STRETCH,
                SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                AlphaMode = DXGI_ALPHA_MODE_IGNORE,
                Flags = device.Factory.IsTearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
            };

            if (surface is SwapChainSurfaceWin32 surfaceWin32)
            {
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = new()
                {
                    Windowed = descriptor.IsFullscreen ? FALSE : TRUE
                };

                using ComPtr<IDXGISwapChain1> tempSwapChain = default;

                device.Factory.DXGIFactory->CreateSwapChainForHwnd(
                    (IUnknown*)device.GetQueue().Handle,
                    surfaceWin32.Hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf()
                    ).Assert();

                tempSwapChain.CopyTo(_handle.GetAddressOf()).Assert();
            }
            else
            {
                throw new GraphicsException("Surface not supported");
            }

            _syncInterval = PresentModeToSwapInterval(descriptor.PresentMode);
            if (!descriptor.IsFullscreen
                && _syncInterval == 0
                && device.Factory.IsTearingSupported)
            {
                _presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
            }
        }

        public IDXGISwapChain3* Handle => _handle;

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _handle.Dispose();
            }
        }

        /// <inheritdoc />
        public override void Present()
        {
            _handle.Get()->Present(_syncInterval, _presentFlags).Assert();
        }
    }
}
