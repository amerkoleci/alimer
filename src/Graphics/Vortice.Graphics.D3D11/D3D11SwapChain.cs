// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.DXGI;
using static Vortice.Graphics.D3DUtils;

namespace Vortice.Graphics;

internal class D3D11SwapChain : SwapChain
{
    private readonly int _syncInterval = 1;
    private readonly PresentFlags _presentFlags = PresentFlags.None;

    public D3D11SwapChain(D3D11GraphicsDevice device, in SwapChainSource source, in SwapChainDescriptor descriptor)
        : base(device, descriptor)
    {
        SwapChainDescription1 swapChainDesc = new()
        {
            Width = descriptor.Size.Width,
            Height = descriptor.Size.Height,
            Format = ToDXGISwapChainFormat(descriptor.ColorFormat),
            Stereo = false,
            SampleDescription = new(1, 0),
            BufferUsage = Usage.RenderTargetOutput,
            BufferCount = PresentModeToBufferCount(descriptor.PresentMode),
            Scaling = Scaling.Stretch,
            SwapEffect = SwapEffect.FlipDiscard,
            AlphaMode = AlphaMode.Ignore,
            Flags = device.IsTearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
        };

        switch (source.Type)
        {
            case SwapChainSourceType.Win32:
                Win32SwapChainSource win32Source = (Win32SwapChainSource)source;
                SwapChainFullscreenDescription fsSwapChainDesc = new()
                {
                    Windowed = !descriptor.IsFullscreen
                };

                Handle = device.DXGIFactory.CreateSwapChainForHwnd(
                    device.NativeDevice,
                    win32Source.Hwnd,
                    swapChainDesc,
                    fsSwapChainDesc,
                    null);


                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                device.DXGIFactory.MakeWindowAssociation(win32Source.Hwnd, WindowAssociationFlags.IgnoreAltEnter).CheckError();
                break;

            default:
                throw new GraphicsException("Surface not supported");

        }

        _syncInterval = PresentModeToSwapInterval(descriptor.PresentMode);
        if (!descriptor.IsFullscreen
            && _syncInterval == 0
            && device.IsTearingSupported)
        {
            _presentFlags |= PresentFlags.AllowTearing;
        }

        AfterReset();
    }

    public IDXGISwapChain1 Handle { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            //for (uint i = 0; i < 3; i++)
            //{
            //    _renderTargets[i].Reset();
            //}

            Handle.Dispose();
        }
    }

    /// <inheritdoc />
    public override void Present()
    {
        Handle.Present(_syncInterval, _presentFlags).CheckError();
    }

    private void AfterReset()
    {
        // Handle color space settings for HDR
        UpdateColorSpace();

        SwapChainDescription1 swapChainDesc = Handle.Description1;

        // Obtain the back buffers for this window which will be the final render targets
        // and create render target views for each of them.
        for (uint n = 0; n < swapChainDesc.BufferCount; n++)
        {
            //_handle.Get()->GetBuffer(n, __uuidof<ID3D12Resource>(), (void**)_renderTargets[n].ReleaseAndGetAddressOf()).Assert();
        }
    }

    private void UpdateColorSpace()
    {

    }
}
