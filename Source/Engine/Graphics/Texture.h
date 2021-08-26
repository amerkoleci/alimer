// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Core/RefCount.h"
#include "Graphics/Types.h"

namespace alimer
{
    enum class TextureDimension : uint32_t
    {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    enum class TextureUsage : uint32_t
    {
        None,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        RenderTarget = 1 << 2,
        ShadingRate = 1 << 3,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(TextureUsage);

    struct TextureDesc
    {
        TextureDimension dimension = TextureDimension::Texture2D;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArraySize = 1;
        Format format = Format::RGBA8UNorm;
        uint32_t mipLevels = 1;
        TextureUsage usage = TextureUsage::ShaderRead;
        uint32_t sampleCount = 1;
    };

    struct TextureData
    {
        const void* pData = nullptr;
        uint32_t    rowPitch = 0;
        uint32_t    slicePitch = 0;
    };

    class Texture;
    using TextureRef = RefCountPtr<Texture>;

    class ALIMER_API Texture : public RefCounted
    {
    public:
        static TextureRef Create2D(uint32_t width, uint32_t height, Format format = Format::RGBA8UNorm, uint32_t arraySize = 1, uint32_t mipLevels = 0, const TextureData* initialData = nullptr, TextureUsage usage = TextureUsage::ShaderRead);
        static TextureRef Create(const TextureDesc& desc, const TextureData* initialData = nullptr);
    };
}

