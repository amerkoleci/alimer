// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12SwapChain.h"
#include "D3D12Texture.h"
#include "D3D12Graphics.h"

namespace Alimer
{
    D3D12SwapChain::D3D12SwapChain(D3D12Graphics& device_, void* windowHandle, const SwapChainCreateInfo& info)
        : SwapChain(info)
        , device(device_)
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        this->window = (HWND)windowHandle;
        ALIMER_ASSERT(IsWindow(this->window));
#else
        this->window = windowHandle;
#endif

        if (!verticalSync)
        {
            syncInterval = 0;
            if (device.IsTearingSupported()) // TODO: not supported in fullscreen mode.
            {
                flags = DXGI_PRESENT_ALLOW_TEARING;
            }
        }

        ResizeBackBuffer(width, height);
    }

    D3D12SwapChain::~D3D12SwapChain()
    {
        Destroy();
    }

    void D3D12SwapChain::Destroy()
    {
        device.DeferDestroy(handle);
    }

    HRESULT D3D12SwapChain::Present()
    {
        return handle->Present(syncInterval, flags);
    }

    void D3D12SwapChain::ResizeBackBuffer(uint32_t width, uint32_t height)
    {
        // Release resources that are tied to the swap chain.
        colorTextures.clear();

        // If the swap chain already exists, resize it, otherwise create one.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

        if (handle != nullptr)
        {
            // If the swap chain already exists, resize it.
            HRESULT hr = handle->ResizeBuffers(
                kMaxFramesInFlight,
                width,
                height,
                ToDXGISwapChainFormat(colorFormat),
                device.IsTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
            );

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
                // If the device was removed for any reason, a new device and swap chain will need to be created.
                device.HandleDeviceLost(hr);

                return;
            }
            else
            {
                ThrowIfFailed(hr);
            }
        }
        else
        {
            // Create a descriptor for the swap chain.
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = ToDXGISwapChainFormat(colorFormat);
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = kMaxFramesInFlight;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = device.IsTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

            IDXGISwapChain1* tempSwapChain;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
            fsSwapChainDesc.Windowed = TRUE;

            // Create a swap chain for the window.
            ThrowIfFailed(device.GetDXGIFactory()->CreateSwapChainForHwnd(
                device.GetCommandQueue(),
                window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                &tempSwapChain
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(device.GetDXGIFactory()->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));
#else
            swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;

            ThrowIfFailed(device.GetDXGIFactory()->CreateSwapChainForCoreWindow(
                device.GetCommandQueue(),
                window->window,
                &swapChainDesc,
                nullptr,
                &tempSwapChain
            ));
#endif

            ThrowIfFailed(tempSwapChain->QueryInterface(IID_PPV_ARGS(&handle)));
            SafeRelease(tempSwapChain);
        }

        // Handle color space settings for HDR
        UpdateColorSpace();

        ThrowIfFailed(handle->GetDesc1(&swapChainDesc));
        width = swapChainDesc.Width;
        height = swapChainDesc.Height;

        TextureCreateInfo textureInfo{};
        textureInfo.width = swapChainDesc.Width;
        textureInfo.height = swapChainDesc.Height;
        textureInfo.format = colorFormat;
        textureInfo.usage = TextureUsage::RenderTarget;

        colorTextures.resize(swapChainDesc.BufferCount);
        for (uint32_t i = 0; i < swapChainDesc.BufferCount; i++)
        {
            ID3D12Resource* backBufferTexture;
            ThrowIfFailed(handle->GetBuffer(i, IID_PPV_ARGS(&backBufferTexture)));
            auto texture = new D3D12Texture(device, textureInfo, nullptr, backBufferTexture, colorFormat);
            texture->SetSwapChain(this);

            colorTextures[i] = RefCountPtr<D3D12Texture>::Create(texture);
            backBufferTexture->Release();
        }
    }

    void D3D12SwapChain::UpdateColorSpace()
    {
        colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

        bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
        if (handle)
        {
            IDXGIOutput* output;
            if (SUCCEEDED(handle->GetContainingOutput(&output)))
            {
                IDXGIOutput6* output6;
                if (SUCCEEDED(output->QueryInterface(&output6)))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    ThrowIfFailed(output6->GetDesc1(&desc));

                    if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                    {
                        // Display output is HDR10.
                        isDisplayHDR10 = true;
                    }
                    output6->Release();
                }

                output->Release();
            }
        }
#endif

        if (isDisplayHDR10)
        {
            switch (colorFormat)
            {
                case PixelFormat::RGB10A2Unorm:
                    // The application creates the HDR10 signal.
                    colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                    break;

                case PixelFormat::RGBA16Float:
                    // The system creates the HDR10 signal; application uses linear values.
                    colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                    break;

                default:
                    break;
            }
        }

        UINT colorSpaceSupport = 0;
        if (SUCCEEDED(handle->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
            && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
        {
            ThrowIfFailed(handle->SetColorSpace1(colorSpace));
        }
    }

    TextureView* D3D12SwapChain::GetCurrentTextureView() const
    {
        uint32_t backbufferIndex = handle->GetCurrentBackBufferIndex();
        return colorTextures[backbufferIndex]->GetDefault();
    }
}
