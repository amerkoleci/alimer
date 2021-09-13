// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "GLGraphics.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "Core/Log.h"
#include "Window.h"

namespace Alimer
{
    GLGraphics::GLGraphics(Window& window, const PresentationParameters& presentationParameters)
        : Graphics(window)
    {
    }

    GLGraphics::~GLGraphics()
    {

    }

    void GLGraphics::WaitIdle()
    {

    }

    bool GLGraphics::BeginFrame()
    {
        return true;
    }

    void GLGraphics::EndFrame()
    {

    }

    void GLGraphics::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    TextureRef GLGraphics::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef GLGraphics::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef GLGraphics::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef GLGraphics::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
