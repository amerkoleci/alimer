// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"
#include "Private/GraphicsBackend.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Core/Log.h"
#include "Window.h"

namespace Alimer
{
#if defined(ALIMER_RHI_D3D12)
    extern bool InitializeD3D12Backend(Window* window, const PresentationParameters& presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
    extern bool InitializeVulkanBackend(Window* window, const PresentationParameters& presentationParameters);
#endif

    Graphics::Graphics()
        : impl(std::make_unique<GraphicsImpl>())
    {
    }

    bool Graphics::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_RHI_D3D12)
        return InitializeD3D12Backend(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
        //return InitializeVulkanBackend(window, presentationParameters);
#endif

        return false;
    }


    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
