// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"
//#include "Assets/TextureLoader.h"
#include "Core/Assert.h"
#include "Core/Log.h"

namespace Alimer
{
    Texture::Texture(const TextureCreateInfo& info)
        : GPUResource(Type::Texture)
        , type(info.type)
        , format(info.format)
        , usage(info.usage)
        , width(info.width)
        , height(info.height)
        , depthOrArraySize(info.depthOrArraySize)
        , mipLevels(info.mipLevels)
        , sampleCount(info.sampleCount)
    {
        if (mipLevels == 0)
        {
            mipLevels = Log2(Max(Max(width, height), depthOrArraySize)) + 1;
        }
    }

    void Texture::DestroyViews()
    {
        for (auto it : views)
        {
            //SafeDelete(it.second);
        }
        views.clear();
    }

    TextureRef Texture::Create(const TextureCreateInfo& info, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return gGraphics().CreateTexture(info, initialData);
    }

    TextureRef Texture::Create2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize, uint32_t mipLevels, TextureUsage usage, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());
        ALIMER_ASSERT(width >= 1);
        ALIMER_ASSERT(height >= 1);
        ALIMER_ASSERT(format != PixelFormat::Undefined);
        ALIMER_ASSERT(arraySize >= 1);
        ALIMER_ASSERT(mipLevels >= 0);

        TextureCreateInfo info{};
        info.type = TextureType::Texture2D;
        info.width = width;
        info.height = height;
        info.depthOrArraySize = arraySize;
        info.mipLevels = mipLevels;
        info.format = format;
        info.sampleCount = SampleCount::Count1;
        info.usage = usage;
        return gGraphics().CreateTexture(info, initialData);
    }

    TextureRef Texture::FromFile(const std::string& path)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return nullptr;
        //return TextureLoader::Load(path);
    }

    TextureRef Texture::FromStream(Stream& stream)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return nullptr;
        //return TextureLoader::Load(stream);
    }

    TextureView* Texture::GetView(uint32_t baseMipLevel, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t arrayLayerCount)
    {
        TextureViewCreateInfo info;
        info.format = format;
        info.baseMipLevel = baseMipLevel;
        info.mipLevelCount = mipLevelCount;
        info.baseArrayLayer = baseArrayLayer;
        info.arrayLayerCount = arrayLayerCount;

        const size_t hash = Hash(info);
        auto it = views.find(hash);
        if (it == views.end())
        {
            auto textureView = CreateView(info);
            views[hash] = textureView;
            return textureView;
        }

        return it->second;
    }

    TextureView::TextureView(Texture* texture, const TextureViewCreateInfo& info)
        : texture{ texture }
    {
        ALIMER_ASSERT(texture != nullptr);

        if (info.format == PixelFormat::Undefined)
        {
            format = texture->GetFormat();
        }
        else
        {
            format = info.format;
        }

        baseMipLevel = info.baseMipLevel;
        mipLevelCount = info.mipLevelCount;
        baseArrayLayer = info.baseArrayLayer;
        arrayLayerCount = info.arrayLayerCount;

        if (mipLevelCount == 0) {
            mipLevelCount = texture->GetMipLevels() - baseMipLevel;
        }

        if (arrayLayerCount == 0) {
            arrayLayerCount = texture->GetArraySize() - baseArrayLayer;
        }

        HashCombine(hash, (uint32_t)format, baseMipLevel, mipLevelCount, baseArrayLayer, arrayLayerCount);
    }

    const Texture* TextureView::GetTexture() const
    {
        return texture;
    }

    Texture* TextureView::GetTexture()
    {
        return texture;
    }
}
