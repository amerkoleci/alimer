// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.DXGI;
using static Vortice.Graphics.D3D12.Utils;

namespace Vortice.Graphics.D3D12
{
    public class SwapChainD3D12 : SwapChain
    {
        public SwapChainD3D12(GraphicsDeviceD3D12 device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
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
                Flags = D3D12DeviceHelper.IsTearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
            };

            if (surface is SwapChainSurfaceWin32 surfaceWin32)
            {
                SwapChainFullscreenDescription fsSwapChainDesc = new SwapChainFullscreenDescription()
                {
                    Windowed = true
                };

                using IDXGISwapChain1 tempSwapChain = D3D12DeviceHelper.DXGIFactory.CreateSwapChainForHwnd(
                    device.GetQueue().Handle,
                    surfaceWin32.Hwnd,
                    swapChainDesc,
                    fsSwapChainDesc
                    );

                Handle = tempSwapChain.QueryInterface<IDXGISwapChain3>();
            }
        }

        public IDXGISwapChain3? Handle { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Handle?.Dispose();
            }
        }

        /// <inheritdoc />
        public override void Present()
        {
            Handle!.Present(1, 0);
        }
    }
}
