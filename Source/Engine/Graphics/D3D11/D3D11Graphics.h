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

        auto GetD3DDevice() const noexcept { return d3dDevice.Get(); }
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
        void HandleDeviceLost();

        TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        SamplerRef CreateSampler(const SamplerDesc& desc)  override;
        ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)  override;
        PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc)  override;

        ValidationMode validationMode;

        ComPtr<IDXGIFactory2> dxgiFactory;
        bool tearingSupported{ false };

        ComPtr<ID3D11Device1> d3dDevice;
        ComPtr<ID3D11DeviceContext1> d3dContext;
        D3D_FEATURE_LEVEL featureLevel{};
        bool deviceLost{ false };

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
#endif
        ComPtr<IDXGISwapChain1> swapChain;
    };
}

