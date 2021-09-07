// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"

namespace Alimer
{
    enum class TextureDimension : uint32_t
    {
        Texture1D,
        Texture2D,
        Texture3D
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
        uint32_t mipLevels = 1;
        uint32_t sampleCount = 1;
        PixelFormat format = PixelFormat::RGBA8UNorm;
        TextureUsage usage = TextureUsage::ShaderRead;
        ResourceStates initialState = ResourceStates::Unknown;

        static inline TextureDesc Tex1D(
            PixelFormat format,
            uint32_t width,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::ShaderRead) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture1D;
            desc.width = width;
            desc.height = 1;
            desc.depthOrArraySize = arraySize;
            desc.mipLevels = mipLevels;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex2D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            uint32_t sampleCount = 1) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArraySize = arraySize;
            desc.mipLevels = mipLevels;
            desc.format = format;
            desc.sampleCount = sampleCount;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex3D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::ShaderRead) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture3D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArraySize = depth;
            desc.mipLevels = mipLevels;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc TexCube(
            PixelFormat format,
            uint32_t size,
            uint32_t mipLevels = 1,
            uint32_t arraySize = 1,
            TextureUsage usage = TextureUsage::ShaderRead) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = size;
            desc.height = size;
            desc.depthOrArraySize = 6 * arraySize;
            desc.mipLevels = mipLevels;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }
    };

    struct TextureData
    {
        const void* pData = nullptr;
        uint32_t    rowPitch = 0;
        uint32_t    slicePitch = 0;
    };

    struct TextureSubresourceSet
    {
        uint32_t baseMipLevel = 0;
        uint32_t numMipLevels = 1;
        uint32_t baseArraySlice = 0;
        uint32_t numArraySlices = 1;

        TextureSubresourceSet() = default;

        TextureSubresourceSet(uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseArraySlice, uint32_t numArraySlices)
            : baseMipLevel{ baseMipLevel }
            , numMipLevels{ numMipLevels }
            , baseArraySlice{ baseArraySlice }
            , numArraySlices{ numArraySlices }
        {
        }

        bool operator ==(const TextureSubresourceSet& other) const
        {
            return baseMipLevel == other.baseMipLevel &&
                numMipLevels == other.numMipLevels &&
                baseArraySlice == other.baseArraySlice &&
                numArraySlices == other.numArraySlices;
        }
        bool operator !=(const TextureSubresourceSet& other) const { return !(*this == other); }
    };
    static const TextureSubresourceSet AllSubresources = TextureSubresourceSet(0, kAllMipLevels, 0, kAllArraySlices);

    class ALIMER_API Texture : public GraphicsResource
    {
    public:
        /// Create new texture.
        [[nodiscard]] static TextureRef Create(const TextureDesc& info, const TextureData* initialData = nullptr);

        /// Create new texture from external handle.
        [[nodiscard]] static TextureRef CreateExternal(void* nativeHandle, const TextureDesc& info);

        [[nodiscard]] TextureDimension GetDimension() const noexcept { return dimension; }
        [[nodiscard]] uint32_t GetWidth(uint32_t mipLevel = 0) const noexcept { return Max(1u, width >> mipLevel); }
        [[nodiscard]] uint32_t GetHeight(uint32_t mipLevel = 0) const noexcept { return Max(1u, height >> mipLevel); }
        [[nodiscard]] uint32_t GetDepth(uint32_t mipLevel = 0) const noexcept { return dimension == TextureDimension::Texture3D ? Max(1u, depthOrArraySize >> mipLevel) : 1; }
        [[nodiscard]] uint32_t GetArraySize() const noexcept { return dimension != TextureDimension::Texture3D ? depthOrArraySize : 1; }
        [[nodiscard]] uint32_t GetMipLevels() const noexcept { return mipLevels; }
        [[nodiscard]] PixelFormat GetFormat() const noexcept { return format; }
        [[nodiscard]] TextureUsage GetUsage() const noexcept { return usage; }
        [[nodiscard]] uint32_t GetSampleCount() const noexcept { return sampleCount; }

    private:
        [[nodiscard]] static bool VerifyInfo(const TextureDesc& info);

    protected:
        /// Constructor.
        Texture(const TextureDesc& info);

        TextureDimension dimension;
        uint32_t width;
        uint32_t height;
        uint32_t depthOrArraySize;
        uint32_t mipLevels;
        uint32_t sampleCount;
        PixelFormat format;
        TextureUsage usage;
    };
}

namespace std
{
    template<> struct hash<Alimer::TextureSubresourceSet>
    {
        std::size_t operator()(const Alimer::TextureSubresourceSet& set) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, set.baseMipLevel);
            Alimer::HashCombine(hash, set.numMipLevels);
            Alimer::HashCombine(hash, set.baseArraySlice);
            Alimer::HashCombine(hash, set.numArraySlices);
            return hash;
        }
    };
}
