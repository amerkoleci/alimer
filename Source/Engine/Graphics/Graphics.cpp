// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Graphics.h"
#include "Platform/Window.h"

namespace alimer
{
#if defined(ALIMER_RHI_D3D11)
    extern bool InitializeD3D11(Window* window);
#endif

    bool GraphicsInitialize(_In_ Window* window)
    {
#if defined(ALIMER_RHI_D3D11)
        return InitializeD3D11(window);
#else
        return nullptr;
#endif
    }

    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
