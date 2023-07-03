// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.CompilerServices;
using Alimer.Graphics.D3D;
using Win32;
using Win32.Graphics.Direct3D12;
using Win32.Graphics.Dxgi;
using Win32.Graphics.Dxgi.Common;
using static Alimer.Graphics.D3D.D3DUtils;
using static Win32.Apis;
using static Win32.Graphics.Dxgi.Apis;
using DxgiUsage = Win32.Graphics.Dxgi.Usage;
#if WINDOWS
using WinRT;
#endif

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12SwapChain : SwapChain
{
    private readonly ComPtr<IDXGISwapChain3> _handle;
    private ComPtr<ISwapChainPanelNative> _swapChainPanelNative;
    private D3D12Texture[]? _backbufferTextures;
    private uint _syncInterval = 1;
    private PresentFlags _presentFlags = PresentFlags.None;

    public D3D12SwapChain(D3D12GraphicsDevice device, ISwapChainSurface surface, in SwapChainDescription description)
        : base(device, surface, description)
    {
        SwapChainDescription1 swapChainDesc = new()
        {
            Width = (uint)surface.PixelWidth,
            Height = (uint)surface.PixelHeight,
            Format = description.Format.ToDxgiSwapChainFormat(),
            Stereo = false,
            SampleDesc = new(1, 0),
            BufferUsage = DxgiUsage.RenderTargetOutput,
            BufferCount = PresentModeToBufferCount(description.PresentMode),
            Scaling = Scaling.Stretch,
            SwapEffect = SwapEffect.FlipDiscard,
            AlphaMode = AlphaMode.Ignore,
            Flags = device.TearingSupported ? SwapChainFlags.AllowTearing : SwapChainFlags.None
        };

        using ComPtr<IDXGISwapChain1> tempSwapChain = default;
        switch (surface.Kind)
        {
            case SwapChainSurfaceType.Win32:
                SwapChainFullscreenDescription fsSwapChainDesc = new()
                {
                    Windowed = !description.IsFullscreen
                };

                ThrowIfFailed(device.Factory->CreateSwapChainForHwnd(
                    (IUnknown*)device.D3D12GraphicsQueue,
                    surface.Handle,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf())
                    );

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                ThrowIfFailed(device.Factory->MakeWindowAssociation(surface.Handle, WindowAssociationFlags.NoAltEnter));
                ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                break;

            case SwapChainSurfaceType.CoreWindow:
                {
                    swapChainDesc.Scaling = Scaling.Stretch;

                    using ComPtr<IUnknown> coreWindow = default;
                    //coreWindow.Attach((IUnknown*)((IWinRTObject)coreWindowSurface.CoreWindow).NativeObject.GetRef());
                    coreWindow.Attach((IUnknown*)surface.Handle);

                    ThrowIfFailed(device.Factory->CreateSwapChainForCoreWindow(
                        (IUnknown*)device.D3D12GraphicsQueue,
                        coreWindow.Get(),
                        &swapChainDesc,
                        null,
                        tempSwapChain.GetAddressOf())
                        );
                    ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                }
                break;

            case SwapChainSurfaceType.SwapChainPanel:
                {
                    ThrowIfFailed(device.Factory->CreateSwapChainForComposition(
                        (IUnknown*)device.D3D12GraphicsQueue,
                        &swapChainDesc,
                        null,
                        tempSwapChain.GetAddressOf()
                        ));


                    fixed (ISwapChainPanelNative** swapChainPanelNative = _swapChainPanelNative)
                    {
                        using ComPtr<IUnknown> swapChainPanel = default;
                        //swapChainPanel.Attach((IUnknown*)((IWinRTObject)swapChainPanelSurface.Panel).NativeObject.GetRef());
                        swapChainPanel.Attach((IUnknown*)surface.Handle);

                        ThrowIfFailed(swapChainPanel.CopyTo(
                            (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(new Guid(0x63AAD0B8, 0x7C24, 0x40FF, 0x85, 0xA8, 0x64, 0x0D, 0x94, 0x4C, 0xC3, 0x25))),
                            (void**)swapChainPanelNative)
                            );
                    }

                    ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                    ThrowIfFailed(_swapChainPanelNative.Get()->SetSwapChain((IDXGISwapChain*)tempSwapChain.Get()));
                    //Matrix3x2 transformMatrix = new()
                    //{
                    //    M11 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleX,
                    //    M22 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleY
                    //};
                    //ThrowIfFailed(_handle.Get()->SetMatrixTransform(&transformMatrix));
                }
                break;

            default:
                throw new GraphicsException("Surface not supported");
        }

        AfterReset();
    }

    public Format DxgiFormat { get; }
    public IDXGISwapChain3* Handle => _handle;
    public uint CurrentBackBufferIndex => _handle.Get()->GetCurrentBackBufferIndex();
    public D3D12Texture CurrentBackBufferTexture => _backbufferTextures[_handle.Get()->GetCurrentBackBufferIndex()]!;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12SwapChain" /> class.
    /// </summary>
    ~D3D12SwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        SwapChainDescription1 swapChainDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&swapChainDesc));

        DxgiFormat dxgiFormat = (DxgiFormat)swapChainDesc.Format;

        _backbufferTextures = new D3D12Texture[swapChainDesc.BufferCount];
        for (uint i = 0; i < swapChainDesc.BufferCount; ++i)
        {
            TextureDescription descriptor = TextureDescription.Texture2D(
                dxgiFormat.FromDxgiFormat(),
                swapChainDesc.Width,
                swapChainDesc.Height,
                usage: TextureUsage.RenderTarget,
                label: $"BackBuffer texture {i}"
            );

            using ComPtr<ID3D12Resource> backbufferTexture = default;
            ThrowIfFailed(_handle.Get()->GetBuffer(i, __uuidof<ID3D12Resource>(), backbufferTexture.GetVoidAddressOf()));
            _backbufferTextures[i] = new D3D12Texture(Device, backbufferTexture.Get(), descriptor);
        }

    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            for (int i = 0; i < _backbufferTextures!.Length; ++i)
            {
                _backbufferTextures[i].Dispose();
            }
        }

        base.Dispose(disposing);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (_swapChainPanelNative.Get() is not null)
        {
            _swapChainPanelNative.Dispose();
        }

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

    public bool Present()
    {
        HResult hr = _handle.Get()->Present(_syncInterval, _presentFlags);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            return false;
        }

        return true;
    }
}
