// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.DXGI;
using static Vortice.Graphics.D3DUtils;

namespace Vortice.Graphics.D3D12
{
    internal class D3D12SwapChain : SwapChain
    {
        private readonly int _syncInterval = 1;
        private readonly PresentFlags _presentFlags = PresentFlags.None;

        public D3D12SwapChain(D3D12GraphicsDevice device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            SwapChainDescription1 swapChainDesc = new()
            {
                Width = descriptor.Size.Width,
                Height = descriptor.Size.Height,
                Format = ToDXGISwapChainFormat(descriptor.ColorFormat),
                Stereo = false,
                SampleDescription = new(1, 0),
                Usage = Usage.RenderTargetOutput,
                BufferCount = PresentModeToBufferCount(descriptor.PresentMode),
                Scaling = Scaling.Stretch,
                SwapEffect = SwapEffect.FlipDiscard,
                AlphaMode = AlphaMode.Ignore,
                Flags = device.Factory.IsTearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
            };

            if (surface is SwapChainSurfaceWin32 surfaceWin32)
            {
                SwapChainFullscreenDescription fsSwapChainDesc = new SwapChainFullscreenDescription()
                {
                    Windowed = !descriptor.IsFullscreen
                };

                using IDXGISwapChain1 tempSwapChain = device.Factory.DXGIFactory.CreateSwapChainForHwnd(
                    device.GetQueue().Handle,
                    surfaceWin32.Hwnd,
                    swapChainDesc,
                    fsSwapChainDesc
                    );

                Handle = tempSwapChain.QueryInterface<IDXGISwapChain3>();
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
                _presentFlags |= PresentFlags.AllowTearing;
            }
        }

        public IDXGISwapChain3 Handle { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Handle.Dispose();
            }
        }

        /// <inheritdoc />
        public override void Present()
        {
            Handle.Present(_syncInterval, _presentFlags).CheckError();
        }
    }
}
