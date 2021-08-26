// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"

namespace alimer
{
    TextureRef Texture::Create(u32 width, u32 height)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return gGraphics().CreateTexture(width, height);
    }
}
