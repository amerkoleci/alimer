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
        D3D11Graphics(Window& window, const PresentationParameters& presentationParameters);
        ~D3D11Graphics() override;

        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

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
        TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        SamplerRef CreateSampler(const SamplerDesc& desc)  override;
        ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)  override;
        PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc)  override;

        ComPtr<ID3D11Device1> device;
        ComPtr<ID3D11DeviceContext1> immediateContext;
        bool deviceLost{ false };
    };
}

