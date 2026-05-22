// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Assets/Asset.h"
#include "Alimer/RHI/RHI.h"

namespace Alimer
{
    class Image;
    using ImageRef = SharedPtr<Image>;

   /// Image file format used by <see cref="Image.Save"/>
    enum class ImageFileType
    {
        /// A BMP file.
        Bmp,
        /// A PNG file.
        Png,
        /// A JPG file.
        Jpg,
        /// A TGA File.
        Tga,
        /// A HDR file.
        Hdr,
        /// A DDS file.
        DDS,
        /// Engine format.
        Alimer,
    };

    using ImageDimension = RHITextureDimension;

    /// Description for image.
    struct ImageDesc
    {
        ImageDimension dimension = ImageDimension::Texture2D;
        PixelFormat format = PixelFormat::RGBA8Unorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        uint32_t mipLevelCount = 1;
    };

    /// Description of image mip level data.
    struct ImageLevel final
    {
        uint32_t      width = 0;
        uint32_t      height = 0;
        PixelFormat   format = PixelFormat::Undefined;
        uint32_t      rowPitch = 0;
        uint32_t      slicePitch = 0;
        uint8_t*      pixels = nullptr;
    };

    class ALIMER_API Image final : public Asset
    {
        ALIMER_OBJECT(Image, Asset);

    public:
        static void Register();

        /// Constructor.
        explicit Image(std::string_view name = kEmptyStringView);

        /// Destructor.
        ~Image() override;

        void Destroy() noexcept;
        bool Initialize1D(PixelFormat format, uint32_t width, uint32_t arrayLayers = 1, uint32_t mipLevelCount = 0) noexcept;
        bool Initialize2D(PixelFormat format, uint32_t width, uint32_t height,  uint32_t arrayLayers = 1,  uint32_t mipLevelCount = 0) noexcept;
        bool Initialize3D(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount = 1) noexcept;
        bool InitializeCube(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers = 1, uint32_t mipLevelCount = 0) noexcept;

        bool Save(Stream& dest) const override;

        /// Save image to file. Return true if successful.
        bool Save(std::string_view fileName, ImageFileType fileType) const;

        /// Save image to stream. Return true if successful.
        bool Save(Stream& stream, ImageFileType fileType) const;

        /// Save in PNG format to file. Return true if successful.
        bool SavePNG(const std::string& fileName) const;

        /// Save in PNG format to stream. Return true if successful.
        bool SavePNG(Stream& stream) const;

        /// Save in JPG format to file with specified quality. Return true if successful.
        bool SaveJPG(const std::string& fileName, int quality) const;

        /// Save in JPG format to stream with specified quality. Return true if successful.
        bool SaveJPG(Stream& stream, int quality) const;

        /// Save in DDS format to stream. Return true if successful.
        bool SaveDDS(Stream& stream) const;

        [[nodiscard]] static ImageRef FromFile(std::string_view path);
        [[nodiscard]] static ImageRef FromStream(Stream& stream);
        [[nodiscard]] static ImageRef FromMemory(const void* data, size_t size);

        const ImageDesc& GetDesc() const noexcept { return desc; }
        const ImageLevel* GetLevel(uint32_t mipLevel, uint32_t arrayOrDepthSlice = 0u) const noexcept;

        PixelFormat GetFormat() const noexcept { return desc.format; }
        uint32_t GetWidth(uint32_t mipLevel = 0) const
        {
            return (mipLevel == 0) || (mipLevel < desc.mipLevelCount) ? Alimer::Max(1u, desc.width >> mipLevel) : 1u;
        }

        uint32_t GetHeight(uint32_t mipLevel = 0) const
        {
            return (mipLevel == 0) || (mipLevel < desc.mipLevelCount) ? Alimer::Max(1u, desc.height >> mipLevel) : 1u;
        }

        uint32_t GetDepth(uint32_t mipLevel = 0) const
        {
            if (desc.dimension != ImageDimension::Texture3D)
            {
                return 1;
            }

            return (mipLevel == 0) || (mipLevel < desc.mipLevelCount) ? Alimer::Max(1u, desc.depthOrArrayLayers >> mipLevel) : 0;
        }

        uint32_t GetMipLevelCount() const noexcept { return desc.mipLevelCount; }
        const ImageLevel* GetLevels() const noexcept { return levels; }
        uint32_t GetLevelCount() const noexcept { return levelsCount; }

        uint8_t* GetPixels() const noexcept { return pixels; }
        size_t GetMemorySize() const noexcept { return _memorySize; }

    private:
        static bool IsEngine(const uint8_t* data, size_t size);
        static bool IsKTX1(const uint8_t* data, size_t size);
        static bool IsKTX2(const uint8_t* data, size_t size);
        static bool IsDDS(const uint8_t* data, size_t size);
        static bool IsQOI(const uint8_t* data, size_t size);

        bool BeginLoad(Stream& source) override;

        bool Initialize();
        bool DetermineImageArray();

        /// Dimension.
        ImageDesc desc{};
        /// Image levels count.
        uint32_t levelsCount = 0;
        /// Image mip levels.
        ImageLevel* levels = nullptr;
        /// Image pixel data.
        uint8_t* pixels = nullptr;
        /// Image pixel memory size.
        size_t _memorySize = 0;
    };

    ALIMER_API bool CalculateMipLevels(uint32_t width, uint32_t height, uint32_t& mipLevels) noexcept;
    ALIMER_API bool CalculateMipLevels3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t& mipLevels) noexcept;
}
