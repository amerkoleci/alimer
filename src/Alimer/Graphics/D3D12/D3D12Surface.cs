// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Vortice.Win32;
using static Vortice.Win32.Apis;
using static Vortice.Win32.Graphics.DXGICommon;
using static Vortice.Win32.Graphics.DXGI;
using static Vortice.Win32.Graphics.D3D12;
using Vortice.Win32.Graphics;
using static Alimer.Graphics.D3D12.Win32Native;
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Surface : Surface
{
    private readonly ComPtr<IDXGISwapChain3> _handle;
    private ComPtr<ISwapChainPanelNative> _swapChainPanelNative;
    private D3D12Texture[]? _backbufferTextures;
    private readonly uint _syncInterval = 1;
    private readonly DXGI_PRESENT _presentFlags = 0;
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

    /// <inheritdoc/>
    protected override void DisposeManagedResources()
    {
        _configured = false;

        for (int i = 0; i < _backbufferTextures!.Length; ++i)
        {
            _backbufferTextures[i].Dispose();
        }

        base.DisposeManagedResources();
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
            Flags = backendDevice.DxAdapter.DxManager.TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        };

        using ComPtr<IDXGISwapChain1> tempSwapChain = default;
        switch (Source)
        {
            case Win32SwapChainSurface win32Surface:
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = new()
                {
                    Windowed = true
                };

                backendDevice.DxAdapter.DxManager.Handle->CreateSwapChainForHwnd(
                    (IUnknown*)backendDevice.D3D12GraphicsQueue.Handle,
                    win32Surface.Hwnd,
                    &swapChainDesc,
                    &fsSwapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf()).ThrowIfFailed();

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                backendDevice.DxAdapter.DxManager.Handle->MakeWindowAssociation(win32Surface.Hwnd, DXGI_MWA_NO_ALT_ENTER).ThrowIfFailed();
                tempSwapChain.CopyTo(_handle.GetAddressOf()).ThrowIfFailed();
                break;

#if WINDOWS
            case SwapChainPanelChainSurface swapChainPanelSurface:
            {
                backendDevice.DxAdapter.DxManager.Handle->CreateSwapChainForComposition(
                    (IUnknown*)backendDevice.D3D12GraphicsQueue.Handle,
                    &swapChainDesc,
                    null,
                    tempSwapChain.GetAddressOf()
                    ).ThrowIfFailed();

                fixed (ISwapChainPanelNative** swapChainPanelNative = _swapChainPanelNative)
                {
                    // Unwrap and get the 'ISwapChainPanelNative' interface with the right IID depending on the UI framework
                    int result = ((WinRT.IWinRTObject)swapChainPanelSurface.SwapChainPanel).NativeObject.TryAs(
                        iid: IID.IID_ISwapChainPanelNativeWinUI,
                        ppv: out *(nint*)swapChainPanelNative);

                    if (result < 0)
                    {
                        throw new Win32Exception(result);
                    }
                }

                tempSwapChain.CopyTo(_handle.GetAddressOf()).ThrowIfFailed();
                _swapChainPanelNative.Get()->SetSwapChain((IDXGISwapChain*)tempSwapChain.Get()).ThrowIfFailed();
                //DXGI_MATRIX_3X2_F transformMatrix = new()
                //{
                //    _11 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleX,
                //    _22 = 1.0f / swapChainPanelSurface.Panel.CompositionScaleY
                //};
                //ThrowIfFailed(_handle.Get()->SetMatrixTransform(&transformMatrix));
            }
            break;
#endif

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
            backendDevice.DxAdapter.DxManager.TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
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
            hr.ThrowIfFailed();
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
        _handle.Get()->GetDesc1(&swapChainDesc).ThrowIfFailed();

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
            _handle.Get()->GetBuffer(i, __uuidof<ID3D12Resource>(), (void**)backbufferTexture.GetAddressOf()).ThrowIfFailed();
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
        HRESULT hr = backendDevice.Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, ref formatSupport);
        if (hr.Success &&
            (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY) == 0)
        {
            return;
        }

        bool isDisplayHDR10 = false;

        // Detect output
        // HDR display query: https://docs.microsoft.com/en-us/windows/win32/direct3darticles/high-dynamic-range
        using ComPtr<IDXGIOutput> dxgiOutput = default;
        if (_handle.Get()->GetContainingOutput(dxgiOutput.GetAddressOf()).Success)
        {
            ComPtr<IDXGIOutput6> output6;
            if (dxgiOutput.As(&output6).Success)
            {
                DXGI_OUTPUT_DESC1 desc1;
                if (output6.Get()->GetDesc1(&desc1).Success)
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
            Vortice.Win32.Numerics.Rect windowBounds;
            if (Source is Win32SwapChainSurface win32Surface)
            {
                if (!GetWindowRect(win32Surface.Hwnd, &windowBounds))
                    throw new InvalidOperationException($"GetWindowRect failed with {GetLastError()}");
            }
            else
            {
                // TODO: 
                windowBounds = default;
            }

            using ComPtr<IDXGIOutput> bestOutput = default;
            long bestIntersectArea = -1;

            // Search for an HDR display among the outputs (e.g. in case of a virtual output from a remote desktop session).
            using ComPtr<IDXGIAdapter> adapter = default;
            for (uint adapterIndex = 0;
                backendDevice.DxAdapter.DxManager.Handle->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()).Success;
                ++adapterIndex)
            {
                ComPtr<IDXGIOutput> output = default;
                for (uint outputIndex = 0;
                    adapter.Get()->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()).Success;
                    ++outputIndex)
                {
                    // Get the rectangle bounds of current output.
                    DXGI_OUTPUT_DESC desc;
                    output.Get()->GetDesc(&desc).ThrowIfFailed();
                    Vortice.Win32.Numerics.Rect desktopCoordinates = desc.DesktopCoordinates;

                    // Compute the intersection
                    int intersectArea = ComputeIntersectionArea(windowBounds, desc.DesktopCoordinates);
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
                if (bestOutput.CopyTo(output6.GetAddressOf()).Success)
                {
                    DXGI_OUTPUT_DESC1 desc;
                    output6.Get()->GetDesc1(&desc).ThrowIfFailed();

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

        ColorSpace = colorSpace switch
        {
            DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 => ColorSpace.HDR_LINEAR,
            DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 => ColorSpace.HDR10_ST2084,
            _ => ColorSpace.SRGB,
        };

        uint colorSpaceSupport = 0;
        if (_handle.Get()->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport).Success
            && (colorSpaceSupport & (uint)DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) != 0)
        {
            _handle.Get()->SetColorSpace1(colorSpace).ThrowIfFailed();
        }
    }

    private static int ComputeIntersectionArea(in Vortice.Win32.Numerics.Rect a, in Vortice.Win32.Numerics.Rect b)
    {
        return Math.Max(0, Math.Min(a.Right, b.Right) - Math.Max(a.Left, b.Left)) * Math.Max(0, Math.Min(a.Bottom, b.Bottom) - Math.Max(a.Top, b.Top));
    }
}
