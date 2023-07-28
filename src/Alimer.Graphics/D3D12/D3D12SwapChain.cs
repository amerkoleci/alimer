// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Graphics.D3D;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.D3D.D3DUtils;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_SCALING;
using static TerraFX.Interop.DirectX.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.DirectX.DXGI_ALPHA_MODE;
using static TerraFX.Interop.DirectX.DXGI_SWAP_CHAIN_FLAG;

#if WINDOWS
using WinRT;
#endif

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12SwapChain : SwapChain
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<IDXGISwapChain3> _handle;
    private ComPtr<ISwapChainPanelNative> _swapChainPanelNative;
    private D3D12Texture[]? _backbufferTextures;
    private uint _syncInterval = 1;
    private uint _presentFlags = 0;

    public D3D12SwapChain(D3D12GraphicsDevice device, ISwapChainSurface surface, in SwapChainDescription description)
        : base(surface, description)
    {
        _device = device;
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = new()
        {
            Width = (uint)surface.PixelWidth,
            Height = (uint)surface.PixelHeight,
            Format = description.Format.ToDxgiSwapChainFormat(),
            Stereo = false,
            SampleDesc = new(1, 0),
            BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            BufferCount = PresentModeToBufferCount(description.PresentMode),
            Scaling = DXGI_SCALING_STRETCH,
            SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            AlphaMode = DXGI_ALPHA_MODE_IGNORE,
            Flags = device.TearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        };

        using ComPtr<IDXGISwapChain1> tempSwapChain = default;
        switch (surface.Kind)
        {
            case SwapChainSurfaceType.Win32:
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = new()
                {
                    Windowed = !description.IsFullscreen
                };

                ThrowIfFailed(device.Factory->CreateSwapChainForHwnd(
                    (IUnknown*)device.D3D12GraphicsQueue,
                    (HWND)surface.Handle,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf())
                    );

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                ThrowIfFailed(device.Factory->MakeWindowAssociation((HWND)surface.Handle, DXGI_MWA_NO_ALT_ENTER));
                ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                break;

            case SwapChainSurfaceType.CoreWindow:
                {
                    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

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

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public DXGI_FORMAT DxgiFormat { get; }
    public IDXGISwapChain3* Handle => _handle;
    public uint CurrentBackBufferIndex => _handle.Get()->GetCurrentBackBufferIndex();
    public D3D12Texture CurrentBackBufferTexture => _backbufferTextures![_handle.Get()->GetCurrentBackBufferIndex()];

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12SwapChain" /> class.
    /// </summary>
    ~D3D12SwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&swapChainDesc));

        _backbufferTextures = new D3D12Texture[swapChainDesc.BufferCount];
        for (uint i = 0; i < swapChainDesc.BufferCount; ++i)
        {
            TextureDescription descriptor = TextureDescription.Texture2D(
                ColorFormat,
                swapChainDesc.Width,
                swapChainDesc.Height,
                usage: TextureUsage.RenderTarget,
                label: $"BackBuffer texture {i}"
            );

            using ComPtr<ID3D12Resource> backbufferTexture = default;
            ThrowIfFailed(_handle.Get()->GetBuffer(i, __uuidof<ID3D12Resource>(), backbufferTexture.GetVoidAddressOf()));
            _backbufferTextures[i] = new D3D12Texture(_device, backbufferTexture.Get(), descriptor);
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
        _handle.Get()->SetDxgiName(newLabel);
    }

    protected override void ResizeBackBuffer()
    {
        _device.WaitIdle();
    }

    public bool Present()
    {
        HRESULT hr = _handle.Get()->Present(_syncInterval, _presentFlags);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            return false;
        }

        return true;
    }
}
