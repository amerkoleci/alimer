// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "D3D11Backend.h"

namespace Alimer
{
    class D3D11Graphics final : public Graphics
    {
    public:
        D3D11Graphics(Window& window, const GraphicsCreateInfo& createInfo);
        ~D3D11Graphics() override;

        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        auto GetD3DDevice() const noexcept { return d3dDevice; }
        auto GetD3DDeviceContext() const noexcept { return d3dContext.Get(); }
        auto GetDXGIFactory() const noexcept { return dxgiFactory.Get(); }

        Texture* GetCurrentBackBuffer() const override
        {
            return 0;
        }

        Texture* GetBackBuffer(uint32_t index) const override
        {
            return nullptr;
        }

        uint32_t GetCurrentBackBufferIndex() const override
        {
            return 0;
        }

        uint32_t GetBackBufferCount() const override
        {
            return 1;
        }

        Texture* GetBackBufferDepthStencilTexture() const
        {
            return nullptr;
        }

    private:
        void CreateFactory();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void AfterReset();
        void UpdateColorSpace();
        void HandleDeviceLost();

        TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        SamplerRef CreateSampler(const SamplerDesc& desc)  override;
        ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)  override;
        PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc)  override;

        ValidationMode validationMode;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        HMODULE dxgiDLL{ nullptr };
        HMODULE d3d11DLL{ nullptr };
#endif

        ComPtr<IDXGIFactory2> dxgiFactory;
        bool tearingSupported{ false };

        ID3D11Device1* d3dDevice = nullptr;
        ComPtr<ID3D11DeviceContext1> d3dContext;
        D3D_FEATURE_LEVEL featureLevel{};
        bool deviceLost{ false };

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
#endif

        DXGI_FORMAT backBufferFormat{ DXGI_FORMAT_B8G8R8A8_UNORM };
        DXGI_FORMAT backBufferRTVFormat{ DXGI_FORMAT_UNKNOWN };
        DXGI_FORMAT depthBufferFormat{ DXGI_FORMAT_UNKNOWN };

        // HDR Support
        DXGI_COLOR_SPACE_TYPE colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };
        ComPtr<IDXGISwapChain1> swapChain;
        ComPtr<ID3D11Texture2D> backBuffer;
        ComPtr<ID3D11RenderTargetView> backBufferView;
        ComPtr<ID3D11Texture2D> depthStencilTexture;
        ComPtr<ID3D11DepthStencilView> depthStencilView;
    };
}

