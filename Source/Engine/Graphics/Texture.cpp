// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Texture::Texture(const TextureDesc& info)
        : GraphicsResource(Type::Texture)
        , dimension(info.dimension)
        , width(info.width)
        , height(info.height)
        , depthOrArraySize(info.depthOrArraySize)
        , mipLevels(info.mipLevels)
        , sampleCount(info.sampleCount)
        , format(info.format)
        , usage(info.usage)
    {
        if (mipLevels == 0)
        {
            mipLevels = CalculateMipLevels(width, height, GetDepth());
        }
    }

    bool Texture::VerifyInfo(const TextureDesc& info)
    {
        ALIMER_ASSERT(info.width >= 1);
        ALIMER_ASSERT(info.height >= 1);
        ALIMER_ASSERT(info.depthOrArraySize >= 1);
        ALIMER_ASSERT(info.usage != TextureUsage::None);

        if ((info.usage & TextureUsage::ShaderWrite) != 0)
        {
            // Check storage support
            //if (!Any(gGraphics().GetCaps().formatProperties[(uint32_t)desc.format].features, PixelFormatFeatures::Storage))
            //{
            //    LOGE("PixelFormat doesn't support shader write");
            //    return nullptr;
            //}

            if (CheckBitsAny(info.usage, TextureUsage::RenderTarget) && IsDepthStencilFormat(info.format))
            {
                LOGE("Cannot create DepthStencil texture with ShaderWrite usage");
                return false;
            }
        }

        return true;
    }

    TextureRef Texture::Create(const TextureDesc& info, const TextureData* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        if (!VerifyInfo(info))
        {
            return nullptr;
        }

        return gGraphics().CreateTexture(info, nullptr, initialData);
    }

    TextureRef Texture::CreateExternal(void* nativeHandle, const TextureDesc& info)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        if (!VerifyInfo(info))
        {
            return nullptr;
        }

        return gGraphics().CreateTexture(info, nativeHandle, nullptr);
    }
}
