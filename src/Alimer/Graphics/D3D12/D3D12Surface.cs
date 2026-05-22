// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Graphics.D3D;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.DXGI_SCALING;
using static TerraFX.Interop.DirectX.DXGI_SWAP_EFFECT;
using static TerraFX.Interop.DirectX.DXGI_ALPHA_MODE;
using static TerraFX.Interop.DirectX.DXGI_SWAP_CHAIN_FLAG;
using static TerraFX.Interop.DirectX.DXGI_COLOR_SPACE_TYPE;
using static TerraFX.Interop.DirectX.DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_FORMAT_SUPPORT1;

#if WINDOWS
using WinRT;
#endif

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Surface : Surface
{
    private static readonly Guid IID_ISwapChainPanelNativeWinUI = new(0x63AAD0B8, 0x7C24, 0x40FF, 0x85, 0xA8, 0x64, 0x0D, 0x94, 0x4C, 0xC3, 0x25);
    private readonly ComPtr<IDXGISwapChain3> _handle;
    private ComPtr<ISwapChainPanelNative> _swapChainPanelNative;
    private D3D12Texture[]? _backbufferTextures;
    private readonly uint _syncInterval = 1;
    private readonly uint _presentFlags = 0;
    private bool _configured;

    public DXGI_FORMAT BackBufferFormat { get; private set; }
    public uint BackBufferCount { get; private set; }
    public IDXGISwapChain3* Handle => _handle;
    public D3D12Texture CurrentTexture => _backbufferTextures![CurrentBackBufferIndex];
    public uint CurrentBackBufferIndex { get; private set; }

    public D3D12Surface(D3D12GraphicsManager manager, in SurfaceDescriptor descriptor)
        : base(descriptor)
    {
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Surface" /> class.
    /// </summary>
    ~D3D12Surface() => Dispose(disposing: false);

    protected override void Dispose(bool disposing)
    {
        _configured = false;
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
    protected override void OnLabelChanged(string? newLabel)
    {
        _handle.Get()->SetDxgiName(newLabel);
    }

    protected override void ConfigureCore()
    {
        if (_configured)
            return;

        D3D12GraphicsDevice backendDevice = (D3D12GraphicsDevice)Device;
        BackBufferFormat = Format.ToDxgiSwapChainFormat();
        BackBufferCount = PresentMode.PresentModeToBufferCount();

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = new()
        {
            Width = (uint)Width,
            Height = (uint)Height,
            Format = BackBufferFormat,
            Stereo = false,
            SampleDesc = new(1, 0),
            BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT,
            BufferCount = BackBufferCount,
            Scaling = DXGI_SCALING_STRETCH,
            SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            AlphaMode = DXGI_ALPHA_MODE_IGNORE,
            Flags = backendDevice.DxAdapter.DxManager.TearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        };

        using ComPtr<IDXGISwapChain1> tempSwapChain = default;
        switch (Source)
        {
            case Win32SwapChainSurface win32Surface:
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = new()
                {
                    Windowed = true
                };

                ThrowIfFailed(backendDevice.DxAdapter.DxManager.Handle->CreateSwapChainForHwnd(
                    (IUnknown*)backendDevice.D3D12GraphicsQueue.Handle,
                    (HWND)win32Surface.Hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf())
                    );

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                ThrowIfFailed(backendDevice.DxAdapter.DxManager.Handle->MakeWindowAssociation((HWND)win32Surface.Hwnd, DXGI_MWA_NO_ALT_ENTER));
                ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                break;

            case SwapChainPanelChainSurface swapChainPanelSurface:
            {
                ThrowIfFailed(backendDevice.DxAdapter.DxManager.Handle->CreateSwapChainForComposition(
                    (IUnknown*)backendDevice.D3D12GraphicsQueue.Handle,
                    &swapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf()
                    ));

                fixed (ISwapChainPanelNative** swapChainPanelNative = _swapChainPanelNative)
                {
                    using ComPtr<IUnknown> swapChainPanel = default;
                    //swapChainPanel.Attach((IUnknown*)((IWinRTObject)swapChainPanelSurface.Panel).NativeObject.GetRef());
                    swapChainPanel.Attach((IUnknown*)swapChainPanelSurface.SwapChainPanel);


                    ThrowIfFailed(swapChainPanel.CopyTo(
                        (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(in IID_ISwapChainPanelNativeWinUI)),
                        (void**)swapChainPanelNative)
                        );
                }

                ThrowIfFailed(tempSwapChain.CopyTo(_handle.GetAddressOf()));
                ThrowIfFailed(_swapChainPanelNative.Get()->SetSwapChain((IDXGISwapChain*)tempSwapChain.Get()));
                //DXGI_MATRIX_3X2_F transformMatrix = new()
                //{
                //    _11 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleX,
                //    _22 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleY
                //};
                //ThrowIfFailed(_handle.Get()->SetMatrixTransform(&transformMatrix));
            }
            break;

            default:
                throw new GraphicsException("Surface not supported");
        }


        if (!string.IsNullOrEmpty(Label))
        {
            OnLabelChanged(Label!);
        }

        UpdateColorSpace();
        AfterReset();
        _configured = true;
    }

    protected override void UnconfigureCore()
    {
        if (!_configured)
            return;

        //DestroySwapchain(false);
        _configured = false;
    }

    /// <inheritdoc />
    protected override void ResizeCore(int newWidth, int newHeight)
    {
        D3D12GraphicsDevice backendDevice = (D3D12GraphicsDevice)Device;
        if (_backbufferTextures?.Length > 0)
        {
            backendDevice.WaitIdle();

            for (int i = 0; i < _backbufferTextures.Length; ++i)
            {
                _backbufferTextures[i].Destroy();
            }
        }

        // Determine the render target size in pixels.
        uint backBufferWidth = Math.Max((uint)newWidth, 1u);
        uint backBufferHeight = Math.Max((uint)newHeight, 1u);

        // If the swap chain already exists, resize it.
        HRESULT hr = _handle.Get()->ResizeBuffers(
            BackBufferCount,
            backBufferWidth,
            backBufferHeight,
            BackBufferFormat,
            backendDevice.DxAdapter.DxManager.TearingSupported ? (uint)DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
            );

        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
#if DEBUG
            HRESULT logHR = (hr == DXGI_ERROR_DEVICE_REMOVED) ? backendDevice.Device->GetDeviceRemovedReason() : hr;
            System.Diagnostics.Debug.WriteLine($"Device Lost on ResizeBuffers: Reason code {logHR}");
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            backendDevice.OnDeviceRemoved();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }

        UpdateColorSpace();
        AfterReset();
    }

    /// <inheritdoc />
    public override Texture? AcquireNextTexture()
    {
        return _backbufferTextures![CurrentBackBufferIndex];
    }

    private void AfterReset()
    {
        D3D12GraphicsDevice backendDevice = (D3D12GraphicsDevice)Device;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&swapChainDesc));

        _backbufferTextures = new D3D12Texture[swapChainDesc.BufferCount];
        for (uint i = 0; i < swapChainDesc.BufferCount; ++i)
        {
            TextureDescriptor description = TextureDescriptor.Texture2D(
                Format,
                swapChainDesc.Width,
                swapChainDesc.Height,
                usage: TextureUsage.RenderTarget,
                label: $"BackBuffer texture {i}"
            );

            using ComPtr<ID3D12Resource> backbufferTexture = default;
            ThrowIfFailed(_handle.Get()->GetBuffer(i, __uuidof<ID3D12Resource>(), (void**)backbufferTexture.GetAddressOf()));
            _backbufferTextures[i] = new D3D12Texture(backendDevice, backbufferTexture.Get(), description, TextureLayout.Present);
        }

        // Reset the index to the current back buffer.
        CurrentBackBufferIndex = _handle.Get()->GetCurrentBackBufferIndex();
    }

    public bool Present()
    {
        HRESULT hr = _handle.Get()->Present(_syncInterval, _presentFlags);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            return false;
        }

        // Update the back buffer index.
        CurrentBackBufferIndex = _handle.Get()->GetCurrentBackBufferIndex();

        return true;
    }

    public void UpdateColorSpace()
    {
        D3D12GraphicsDevice backendDevice = (D3D12GraphicsDevice)Device;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = default;
        formatSupport.Format = BackBufferFormat;
        HRESULT hr = backendDevice.Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, (uint)sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
        if (hr.SUCCEEDED &&
            (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY) == 0)
        {
            return;
        }

        bool isDisplayHDR10 = false;

        // Detect output
        // HDR display query: https://docs.microsoft.com/en-us/windows/win32/direct3darticles/high-dynamic-range
        using ComPtr<IDXGIOutput> dxgiOutput = default;
        if (SUCCEEDED(_handle.Get()->GetContainingOutput(dxgiOutput.GetAddressOf())))
        {
            ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(dxgiOutput.As(&output6)))
            {
                DXGI_OUTPUT_DESC1 desc1;
                if (SUCCEEDED(output6.Get()->GetDesc1(&desc1)))
                {
                    if (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                    {
                        isDisplayHDR10 = true;
                    }
                }
            }
        }
        else
        {
            // Get the retangle bounds of the app window.
            RECT windowBounds;
            if (Source is Win32SwapChainSurface win32Surface)
            {
                if (!GetWindowRect((HWND)win32Surface.Hwnd, &windowBounds))
                    throw new InvalidOperationException($"GetWindowRect failed with {GetLastError()}");
            }
            else
            {
                // TODO: 
                windowBounds = default;
            }

            int ax1 = windowBounds.left;
            int ay1 = windowBounds.top;
            int ax2 = windowBounds.right;
            int ay2 = windowBounds.bottom;

            using ComPtr<IDXGIOutput> bestOutput = default;
            long bestIntersectArea = -1;

            // Search for an HDR display among the outputs (e.g. in case of a virtual output from a remote desktop session).
            using ComPtr<IDXGIAdapter> adapter = default;
            for (uint adapterIndex = 0;
                SUCCEEDED(backendDevice.DxAdapter.DxManager.Handle->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
                ++adapterIndex)
            {
                ComPtr<IDXGIOutput> output = default;
                for (uint outputIndex = 0;
                    SUCCEEDED(adapter.Get()->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
                    ++outputIndex)
                {
                    // Get the rectangle bounds of current output.
                    DXGI_OUTPUT_DESC desc;
                    ThrowIfFailed(output.Get()->GetDesc(&desc));
                    RECT desktopCoordinates = desc.DesktopCoordinates;

                    // Compute the intersection
                    int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, desc.DesktopCoordinates);
                    if (intersectArea > bestIntersectArea)
                    {
                        bestOutput.Swap(ref output);
                        bestIntersectArea = intersectArea;
                    }
                }
            }

            if (bestOutput.Get() is not null)
            {
                using ComPtr<IDXGIOutput6> output6 = default;
                if (SUCCEEDED(bestOutput.CopyTo(output6.GetAddressOf())))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    ThrowIfFailed(output6.Get()->GetDesc1(&desc));

                    if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                    {
                        // Display output is HDR10.
                        isDisplayHDR10 = true;
                    }
                }
            }
        }

        // The application creates the HDR10 signal.
        DXGI_COLOR_SPACE_TYPE colorSpace = BackBufferFormat switch
        {
            DXGI_FORMAT_R10G10B10A2_UNORM => isDisplayHDR10 ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,// The application creates the HDR10 signal.
            DXGI_FORMAT_R16G16B16A16_FLOAT => DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,// The system creates the HDR10 signal; application uses linear values.
            _ => DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,// Anything else will be SDR (SRGB):
        };

        switch (colorSpace)
        {
            default:
            case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
                ColorSpace = ColorSpace.SRGB;
                break;
            case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
                ColorSpace = ColorSpace.HDR_LINEAR;
                break;
            case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
                ColorSpace = ColorSpace.HDR10_ST2084;
                break;
        }

        uint colorSpaceSupport = 0;
        if (_handle.Get()->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport).SUCCEEDED
            && (colorSpaceSupport & (uint)DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) != 0)
        {
            ThrowIfFailed(_handle.Get()->SetColorSpace1(colorSpace));
        }
    }

    private static int ComputeIntersectionArea(
        int ax1, int ay1, int ax2, int ay2,
        in RECT b)
    {
        return Math.Max(0, Math.Min(ax2, b.right) - Math.Max(ax1, b.left)) * Math.Max(0, Math.Min(ay2, b.bottom) - Math.Max(ay1, b.top));
    }
}
