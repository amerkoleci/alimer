// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Core/Log.h"
#include "Window.h"

#if defined(ALIMER_GRAPHICS_D3D11)
#include "Graphics/D3D11/D3D11Graphics.h"
#endif

#if defined(ALIMER_GRAPHICS_GL)
#include "Graphics/GL/GLGraphics.h"
#endif

namespace Alimer
{
    Graphics::Graphics(Window& window)
        : window{ window }
    {
    }

    bool Graphics::Initialize(Window& window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_GRAPHICS_GL)
        gGraphics().Start(new GLGraphics(window, presentationParameters));
#endif

        return gGraphics().IsInitialized();
    }


    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
