// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Assets/Asset.h"
#include "Graphics/GPUResource.h"
#include <unordered_map>

namespace Alimer
{
    enum class TextureType : uint32_t
    {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    enum class TextureUsage : uint32_t
    {
        None,
        Sampled = 1 << 0,
        Storage = 1 << 1,
        RenderTarget = 1 << 2,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(TextureUsage);

    struct TextureCreateInfo
    {
        const char* label = nullptr;
        TextureType type = TextureType::Texture2D;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArraySize = 1;
        PixelFormat format = PixelFormat::RGBA8Unorm;
        uint32_t mipLevels = 1;
        TextureUsage usage = TextureUsage::Sampled;
        SampleCount sampleCount = SampleCount::Count1;

        static inline TextureCreateInfo Tex1D(
            PixelFormat format,
            uint32_t width,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureCreateInfo info;
            info.type = TextureType::Texture1D;
            info.width = width;
            info.height = 1;
            info.depthOrArraySize = arraySize;
            info.mipLevels = mipLevels;
            info.format = format;
            info.sampleCount = SampleCount::Count1;
            info.usage = usage;
            return info;
        }

        static inline TextureCreateInfo Tex2D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::Sampled,
            SampleCount sampleCount = SampleCount::Count1) noexcept
        {
            TextureCreateInfo info;
            info.type = TextureType::Texture2D;
            info.width = width;
            info.height = height;
            info.depthOrArraySize = arraySize;
            info.mipLevels = mipLevels;
            info.format = format;
            info.sampleCount = sampleCount;
            info.usage = usage;
            return info;
        }

        static inline TextureCreateInfo Tex3D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureCreateInfo info;
            info.type = TextureType::Texture3D;
            info.width = width;
            info.height = height;
            info.depthOrArraySize = depth;
            info.mipLevels = mipLevels;
            info.format = format;
            info.sampleCount = SampleCount::Count1;
            info.usage = usage;
            return info;
        }

        static inline TextureCreateInfo TexCube(
            PixelFormat format,
            uint32_t size,
            uint32_t mipLevels = 1,
            uint32_t arraySize = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureCreateInfo info;
            info.type = TextureType::TextureCube;
            info.width = size;
            info.height = size;
            info.depthOrArraySize = arraySize;
            info.mipLevels = mipLevels;
            info.format = format;
            info.sampleCount = SampleCount::Count1;
            info.usage = usage;
            return info;
        }
    };

    struct TextureViewCreateInfo {
        PixelFormat format = PixelFormat::Undefined;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = 0;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerCount = 0;
    };

	struct TextureData
	{
		const void* data = nullptr;
		uint32_t rowPitch = 0;
		uint32_t slicePitch = 0;
	};

	class Stream;
	class Texture;

	class ALIMER_API Texture : public GPUResource, public Asset
	{
		friend class TextureView;

		ALIMER_OBJECT(Texture, Asset);

	public:
        [[nodiscard]] static TextureRef Create(const TextureCreateInfo& info, const void* initialData = nullptr);
        [[nodiscard]] static TextureRef Create2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize = 1, uint32_t mipLevels = 1, TextureUsage usage = TextureUsage::Sampled, const void* initialData = nullptr);

        [[nodiscard]] static TextureRef FromFile(const std::string& path);
        [[nodiscard]] static TextureRef FromStream(Stream& stream);

        TextureView* GetView(uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 0, uint32_t baseArrayLayer = 0, uint32_t arrayLayerCount = 0);
        TextureView* GetDefault() const { return defaultView; }

		TextureType GetTextureType() const noexcept { return type; }
        uint32_t GetWidth(uint32_t mipLevel = 0) const noexcept { return Max(1u, width >> mipLevel); }
        uint32_t GetHeight(uint32_t mipLevel = 0) const noexcept { return Max(1u, height >> mipLevel); }
        uint32_t GetDepth(uint32_t mipLevel = 0) const noexcept { return type == TextureType::Texture3D ? Max(1u, depthOrArraySize >> mipLevel) : 1; }
        uint32_t GetArraySize() const noexcept { return type != TextureType::Texture3D ? depthOrArraySize : 1; }
        uint32_t GetMipLevels() const noexcept { return mipLevels; }
		PixelFormat GetFormat() const noexcept { return format; }
		TextureUsage GetUsage() const noexcept { return usage; }
        SampleCount GetSampleCount() const noexcept { return sampleCount; }

        /// Get the array index of a subresource.
        uint32_t GetSubresourceArraySlice(uint32_t subresource) const { return subresource / mipLevels; }

        /// Get the mip-level of a subresource.
        uint32_t GetSubresourceMipLevel(uint32_t subresource) const { return subresource % mipLevels; }

        uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arraySlice, uint32_t placeSlice = 0) const
        {
            return mipLevel + arraySlice * GetMipLevels() + placeSlice * GetMipLevels() * GetArraySize();
        }

	protected:
		/// Constructor.
		Texture(const TextureCreateInfo& info);

        void DestroyViews();
        virtual TextureView* CreateView(const TextureViewCreateInfo& createInfo) = 0;

		TextureType type;
        uint32_t width;
        uint32_t height;
        uint32_t depthOrArraySize;
		PixelFormat format;
        uint32_t mipLevels;
		TextureUsage usage;
        SampleCount sampleCount;

        std::unordered_map<size_t, TextureView*> views;
        TextureView* defaultView{ nullptr };
	};

    class ALIMER_API TextureView
    {
    public:
        virtual ~TextureView() = default;

        TextureView(TextureView&) = delete;
        TextureView(TextureView&&) = delete;
        TextureView& operator=(const TextureView&) = delete;
        TextureView& operator=(TextureView&&) = delete;

        const Texture* GetTexture() const;
        Texture* GetTexture();
        PixelFormat GetFormat() const noexcept { return format; }
        uint32_t GetBaseMipLevel() const noexcept { return baseMipLevel; }
        uint32_t GetLevelCount() const noexcept { return mipLevelCount; }
        uint32_t GetBaseArrayLayer() const noexcept { return baseArrayLayer; }
        uint32_t GetLayerCount() const noexcept { return arrayLayerCount; }

        uint32_t GetBindlessSRV() const noexcept { return bindless_srv; }
        uint32_t GetBindlessUAV() const noexcept { return bindless_uav; }

    protected:
        TextureView(_In_ Texture* texture, const TextureViewCreateInfo& info);

        Texture* texture;
        PixelFormat format;
        uint32_t baseMipLevel;
        uint32_t mipLevelCount;
        uint32_t baseArrayLayer;
        uint32_t arrayLayerCount;
        size_t hash{ 0 };

        mutable uint32_t bindless_srv = kInvalidBindlessIndex;
        mutable uint32_t bindless_uav = kInvalidBindlessIndex;
    };
}

namespace std
{
    template <> class hash<Alimer::TextureViewCreateInfo>
    {
    public:
        size_t operator()(const Alimer::TextureViewCreateInfo& info) const
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, (uint32_t)info.format);
            Alimer::HashCombine(hash, info.baseMipLevel);
            Alimer::HashCombine(hash, info.mipLevelCount);
            Alimer::HashCombine(hash, info.baseArrayLayer);
            Alimer::HashCombine(hash, info.arrayLayerCount);
            return hash;
        }
    };
}
