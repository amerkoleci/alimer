// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Core/Log.h"
#include "Window.h"

namespace Alimer
{
    Graphics::Graphics()
    {
    }

    bool Graphics::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        return false;
    }


    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
