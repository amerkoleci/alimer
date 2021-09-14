// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "D3D11Backend.h"

namespace Alimer
{
    class D3D11CommandContext;

    class D3D11GraphicsDevice final : public Graphics
    {
    public:
        D3D11GraphicsDevice(Window& window, const GraphicsCreateInfo& createInfo);
        ~D3D11GraphicsDevice() override;

        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        auto GetD3DDevice() const noexcept { return d3dDevice; }
        auto GetDXGIFactory() const noexcept { return dxgiFactory.Get(); }
        CommandContext* GetImmediateContext() const override;
        ID3D11RenderTargetView* GetBackBufferView() const noexcept { return backBufferView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return depthStencilView.Get(); }

    private:
        void CreateFactory();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void AfterReset();
        void UpdateColorSpace();
        void HandleDeviceLost();

        bool CreateBuffer(const BufferDesc* desc, const void* initialData, GPUBuffer* pBuffer) const override;
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

        std::unique_ptr<D3D11CommandContext> mainContext;
    };
}

