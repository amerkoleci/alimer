// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"

namespace alimer
{
    TextureRef Texture::Create2D(uint32_t width, uint32_t height, Format format, uint32_t arraySize, uint32_t mipLevels, const TextureData* initialData, TextureUsage usage)
    {
        TextureDesc desc;
        desc.dimension = TextureDimension::Texture2D;
        desc.width = width;
        desc.height = height;
        desc.depthOrArraySize = arraySize;
        desc.format = format;
        desc.mipLevels = mipLevels;
        desc.usage = usage;
        return Create(desc, initialData);
    }

    TextureRef Texture::Create(const TextureDesc& desc, const TextureData* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return gGraphics().CreateTexture(desc, initialData);
    }
}
