// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.DXGI_FORMAT;
using static TerraFX.Interop.DXGI_SCALING;
using static TerraFX.Interop.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.DXGI_ALPHA_MODE;
using static TerraFX.Interop.DXGI_SWAP_CHAIN_FLAG;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.Utils;

namespace Vortice.Graphics.D3D12
{
    public unsafe class SwapChainD3D12 : SwapChain
    {
        private ComPtr<IDXGISwapChain3> _handle;

        public SwapChainD3D12(GraphicsDeviceD3D12 device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
            : base(device, surface, descriptor)
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = new DXGI_SWAP_CHAIN_DESC1
            {
                Width = (uint)descriptor.Size.Width,
                Height = (uint)descriptor.Size.Height,
                Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                SampleDesc = new DXGI_SAMPLE_DESC(count: 1, quality: 0),
                BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                BufferCount = PresentModeToBufferCount(descriptor.PresentMode),
                Scaling = DXGI_SCALING_STRETCH,
                SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                AlphaMode = DXGI_ALPHA_MODE_IGNORE,
                Flags = D3D12DeviceHelper.IsTearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
            };

            if (surface is SwapChainSurfaceWin32 surfaceWin32)
            {
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
        public override void Present() => throw new System.NotImplementedException();
    }
}
