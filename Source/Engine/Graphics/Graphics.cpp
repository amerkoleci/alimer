// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Graphics.h"
#include "Window.h"

namespace alimer
{
#if defined(ALIMER_RHI_D3D11)
    extern bool D3D11_Initialize(Window* window, const PresentationParameters& presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
    extern bool Vulkan_IsAvailable();
    extern bool Vulkan_Initialize(Window* window, const PresentationParameters& presentationParameters);
#endif

    bool GraphicsInitialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_RHI_D3D11)
        //return D3D11_Initialize(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
        return Vulkan_Initialize(window, presentationParameters);
#endif

        return false;
    }

    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
