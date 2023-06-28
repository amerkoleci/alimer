// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D11;
using Win32.Graphics.Dxgi.Common;
using DxgiUsage = Win32.Graphics.Dxgi.Usage;
using static Alimer.Graphics.D3D.D3DUtils;
using static Win32.Graphics.Dxgi.Apis;
using Win32.Graphics.Dxgi;
using static Win32.Apis;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11SwapChain : SwapChain
{
    private readonly ComPtr<IDXGISwapChain1> _handle;
    private D3D11Texture? _backbufferTexture;

    public D3D11SwapChain(D3D11GraphicsDevice device, SwapChainSurface surface, in SwapChainDescriptor descriptor)
        : base(device, surface, descriptor)
    {
        SwapChainDescription1 swapChainDesc = new()
        {
            Width = (uint)descriptor.Width,
            Height = (uint)descriptor.Height,
            Format = descriptor.Format.ToDxgiSwapChainFormat(),
            Stereo = false,
            SampleDesc = new(1, 0),
            BufferUsage = DxgiUsage.RenderTargetOutput,
            BufferCount = PresentModeToBufferCount(descriptor.PresentMode),
            Scaling = Scaling.Stretch,
            SwapEffect = SwapEffect.FlipDiscard,
            AlphaMode = AlphaMode.Ignore,
            Flags = device.TearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
        };

        switch (surface)
        {
            case Win32SwapChainSurface win32Source:
                SwapChainFullscreenDescription fsSwapChainDesc = new()
                {
                    Windowed = !descriptor.IsFullscreen
                };

                ThrowIfFailed(device.Factory->CreateSwapChainForHwnd(
                    (IUnknown*)device.Handle,
                    win32Source.Hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    _handle.GetAddressOf())
                    );

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                ThrowIfFailed(device.Factory->MakeWindowAssociation(win32Source.Hwnd, WindowAssociationFlags.NoAltEnter));
                break;

#if WINDOWS && TODO
            case CoreWindowSwapChainSurface coreWindowSurface:
                swapChainDesc.Scaling = Scaling.Stretch;

                ThrowIfFailed(device.Factory->CreateSwapChainForCoreWindow(
                    (IUnknown*)device.Handle,
                    win32Source.Hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    _handle.GetAddressOf())
                    );

                using (ComObject comObject = new(coreWindowSurface.CoreWindow))
                {
                    using IDXGISwapChain1 tempSwapChain = factory.CreateSwapChainForCoreWindow(
                        deviceOrCommandQueue,
                        comObject,
                        swapChainDesc);

                    Handle = tempSwapChain.QueryInterface<IDXGISwapChain3>();
                }

                break;

            case SwapChainPanelSwapChainSurface swapChainPanelSurface:
                using (ComObject comObject = new ComObject(swapChainPanelSurface.Panel))
                {
                    using ISwapChainPanelNative nativePanel = comObject.QueryInterface<ISwapChainPanelNative>();
                    using IDXGISwapChain1 tempSwapChain = factory.CreateSwapChainForComposition(deviceOrCommandQueue, swapChainDesc);

                    Handle = tempSwapChain.QueryInterface<IDXGISwapChain3>();
                    nativePanel.SetSwapChain(Handle);
                    Handle.MatrixTransform = new System.Numerics.Matrix3x2
                    {
                        M11 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleX,
                        M22 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleY
                    };
                }

                break;
#endif

            default:
                throw new GraphicsException("Surface not supported");
        }

        AfterReset();
    }

    public Format DxgiFormat { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D11SwapChain" /> class.
    /// </summary>
    ~D3D11SwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        SwapChainDescription1 swapChainDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&swapChainDesc));

        DxgiFormat dxgiFormat = (DxgiFormat)swapChainDesc.Format;
        TextureDescriptor descriptor = TextureDescriptor.Texture2D(
            dxgiFormat.FromDxgiFormat(),
            swapChainDesc.Width,
            swapChainDesc.Height,
            usage: TextureUsage.RenderTarget
            );

        using ComPtr<ID3D11Texture2D> backbufferTexture = default;
        ThrowIfFailed(_handle.Get()->GetBuffer(0, __uuidof<ID3D11Texture2D>(), backbufferTexture.GetVoidAddressOf()));
        _backbufferTexture = new D3D11Texture(Device, (ID3D11Resource*)backbufferTexture.Get(), descriptor);
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _backbufferTexture!.Dispose();
        }

        base.Dispose(disposing);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        //_handle.Get()->SetDebugName(newLabel);
    }

    protected override void ResizeBackBuffer(int width, int height)
    {

    }
}
