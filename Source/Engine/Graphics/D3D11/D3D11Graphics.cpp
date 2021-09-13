// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D11Graphics.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "Core/Log.h"
#include "Window.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    D3D11Graphics::D3D11Graphics(Window& window, const PresentationParameters& presentationParameters)
        : Graphics(window)
    {
    }

    D3D11Graphics::~D3D11Graphics()
    {

    }

    void D3D11Graphics::WaitIdle()
    {
        immediateContext->Flush();
    }

    bool D3D11Graphics::BeginFrame()
    {
        if (deviceLost)
            return false;

        return true;
    }

    void D3D11Graphics::EndFrame()
    {

    }

    void D3D11Graphics::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    TextureRef D3D11Graphics::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef D3D11Graphics::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef D3D11Graphics::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef D3D11Graphics::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
