// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "GLBackend.h"

namespace Alimer
{
    class GLCommandContext;

    class GLGraphicsDevice final : public Graphics
    {
    public:
        GLGraphicsDevice(Window& window, const GraphicsCreateInfo& createInfo);
        ~GLGraphicsDevice() override;

        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;
        CommandContext* GetImmediateContext() const override;

    private:
        TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        SamplerRef CreateSampler(const SamplerDesc& desc)  override;
        ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)  override;
        PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc)  override;


        std::unique_ptr<GLCommandContext> mainContext;
    };
}

