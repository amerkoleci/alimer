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
        , depthOrArrayLayers(info.depthOrArrayLayers)
        , mipLevels(info.mipLevels)
        , sampleCount(info.sampleCount)
        , format(info.format)
        , usage(info.usage)
    {
    }

    void Texture::DestroyViews()
    {
        views.clear();
    }

    bool Texture::VerifyInfo(const TextureDesc& info)
    {
        ALIMER_ASSERT(info.width >= 1);
        ALIMER_ASSERT(info.height >= 1);
        ALIMER_ASSERT(info.depthOrArrayLayers >= 1);
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

    TextureView* Texture::GetView(const TextureViewDesc& desc)
    {
        auto it = views.find(desc);
        if (it == views.end())
        {
            std::unique_ptr<TextureView> newView = CreateView(desc);
            views[desc] = std::move(newView);

            it = views.find(desc);
        }

        return it->second.get();
    }

    /* TextureView */
    TextureView::TextureView(_In_ Texture* texture_, const TextureViewDesc& desc_)
        : texture(texture_)
        , desc(desc_)
    {
        if (desc.format == PixelFormat::Undefined)
        {
            desc.format = texture->GetFormat();
        }

        if (desc.dimension == TextureViewDimension::Undefined)
        {
            switch (texture->GetDimension())
            {
                case TextureDimension::Texture1D:
                    desc.dimension = TextureViewDimension::View1D;
                    break;
                case TextureDimension::Texture2D:
                    desc.dimension = TextureViewDimension::View2D;
                    break;
                case TextureDimension::Texture3D:
                    desc.dimension = TextureViewDimension::View3D;
                    break;
            }
        }

        if (desc.arrayLayerCount == 0)
        {
            switch (desc.dimension)
            {
                case TextureViewDimension::View1D:
                case TextureViewDimension::View2D:
                case TextureViewDimension::View3D:
                    desc.arrayLayerCount = 1;
                    break;

                case TextureViewDimension::ViewCube:
                    desc.arrayLayerCount = 6;
                    break;
                case TextureViewDimension::View1DArray:
                case TextureViewDimension::View2DArray:
                case TextureViewDimension::ViewCubeArray:
                    desc.arrayLayerCount = texture->GetArrayLayers() - desc.baseArrayLayer;
                    break;
                default:
                    // We don't put UNREACHABLE() here because we validate enums only after this
                    // function sets default values. Otherwise, the UNREACHABLE() will be hit.
                    break;
            }
        }

        if (desc.mipLevelCount == 0) {
            desc.mipLevelCount = texture->GetMipLevels() - desc.baseMipLevel;
        }
    }
}
