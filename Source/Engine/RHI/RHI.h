// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"

namespace Alimer::rhi
{
    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 8;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxCommandLists = 32u;
    static constexpr uint32_t kMaxUniformBufferBindings = 14;
    static constexpr uint32_t kMaxLogMessageSize = 1024;

    enum class ShaderFormat : uint8_t
    {
        DXIL,
        SPIRV
    };

    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,

        Count
    };

    enum class HeapType : uint32_t
    {
        Default,
        Upload,
        Readback,
    };

    enum class ResourceStates : uint32_t
    {
        Unknown = 0,
        Common = 0x00000001,
        ConstantBuffer = 0x00000002,
        VertexBuffer = 0x00000004,
        IndexBuffer = 0x00000008,
        IndirectArgument = 0x00000010,
        ShaderResource = 0x00000020,
        UnorderedAccess = 0x00000040,
        RenderTarget = 0x00000080,
        DepthWrite = 0x00000100,
        DepthRead = 0x00000200,
        StreamOut = 0x00000400,
        CopyDest = 0x00000800,
        CopySource = 0x00001000,
        ResolveDest = 0x00002000,
        ResolveSource = 0x00004000,
        Present = 0x00008000,
        AccelerationStructureRead = 0x00010000,
        AccelerationStructureWrite = 0x00020000,
        AccelerationStructureBuildInput = 0x00040000,
        AccelerationStructureBuildBlas = 0x00080000,
        ShadingRateSurface = 0x00100000,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ResourceStates);

    /// Defines texture format.
    enum class TextureFormat : uint32_t
    {
        Undefined = 0,
        // 8-bit formats
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        // 16-bit formats
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        // Packed 16-Bit Pixel Formats
        BGRA4Unorm,
        B5G6R5Unorm,
        B5G5R5A1Unorm,
        // 32-bit formats
        R32Uint,
        R32Sint,
        R32Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-Bit formats
        RGB10A2Unorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-Bit formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth32Float,
        Depth24UnormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBFloat,
        BC6HRGBUFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        // EAC/ETC compressed formats
        ETC2RGB8Unorm,
        ETC2RGB8UnormSrgb,
        ETC2RGB8A1Unorm,
        ETC2RGB8A1UnormSrgb,
        ETC2RGBA8Unorm,
        ETC2RGBA8UnormSrgb,
        EACR11Unorm,
        EACR11Snorm,
        EACRG11Unorm,
        EACRG11Snorm,
        // ASTC compressed formats
        ASTC4x4Unorm,
        ASTC4x4UnormSrgb,
        ASTC5x4Unorm,
        ASTC5x4UnormSrgb,
        ASTC5x5Unorm,
        ASTC5x5UnormSrgb,
        ASTC6x5Unorm,
        ASTC6x5UnormSrgb,
        ASTC6x6Unorm,
        ASTC6x6UnormSrgb,
        ASTC8x5Unorm,
        ASTC8x5UnormSrgb,
        ASTC8x6Unorm,
        ASTC8x6UnormSrgb,
        ASTC8x8Unorm,
        ASTC8x8UnormSrgb,
        ASTC10x5Unorm,
        ASTC10x5UnormSrgb,
        ASTC10x6Unorm,
        ASTC10x6UnormSrgb,
        ASTC10x8Unorm,
        ASTC10x8UnormSrgb,
        ASTC10x10Unorm,
        ASTC10x10UnormSrgb,
        ASTC12x10Unorm,
        ASTC12x10UnormSrgb,
        ASTC12x12Unorm,
        ASTC12x12UnormSrgb,

        Count
    };

    enum class LoadAction : uint32_t
    {
        Clear,
        Load,
        Discard,
    };

    enum class StoreAction : uint32_t
    {
        Store,
        Discard,
    };

    enum class PresentMode : uint32_t
    {
        Immediate = 0,
        Mailbox,
        Fifo,
    };

    /* Structs */

    enum class TextureDimension : uint32_t
    {
        Texture1D,
        Texture2D,
        Texture3D,
    };

    enum class TextureUsage : uint32_t
    {
        None = 0,
        Sampled = 1 << 0,
        Storage = 1 << 1,
        RenderTarget = 1 << 2,
        ShadingRate = 1 << 3,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(TextureUsage);

    struct TextureDesc
    {
        const char* label = nullptr;
        TextureUsage usage = TextureUsage::Sampled;
        TextureDimension dimension = TextureDimension::Texture2D;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        TextureFormat format = TextureFormat::BGRA8Unorm;
        uint32_t mipLevelCount = 1;
        uint32_t sampleCount = 1;

        static inline TextureDesc Tex1D(
            TextureFormat format,
            uint32_t width,
            uint32_t arrayLayers = 1,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture1D;
            desc.width = width;
            desc.height = 1;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex2D(
            TextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t arrayLayers = 1,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled,
            uint32_t sampleCount = 1) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = sampleCount;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex3D(
            TextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture3D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = depth;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc TexCube(
            TextureFormat format,
            uint32_t size,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = size;
            desc.height = size;
            desc.depthOrArrayLayers = 6 * arrayLayers;
            desc.mipLevelCount = mipLevelCount;
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

    struct SwapChainDesc
    {
        const char* label = nullptr;
        uint32_t width;
        uint32_t height;
        TextureFormat format = TextureFormat::BGRA8UnormSrgb;
        PresentMode presentMode = PresentMode::Fifo;
    };

    struct DeviceFeatures
    {
        /// Whether tessellation support is available.
        bool tessellation = false;
        /// Whether Ray Tracing support is available.
        bool rayTracing = false;
        /// Whether Mesh Shader support is available.
        bool meshShader = false;
        /// Whether VariableRate shading support is available.
        bool variableRateShading = false;
        /// Whether VariableRate shading Tier2 support is available.
        bool variableRateShadingTier2 = false;
    };

    struct DeviceLimits
    {
        /// The maximum pixel size of a 1d image.
        uint32_t maxTextureDimension1D;

        /// The maximum pixel size along one axis of a 2d image.
        uint32_t maxTextureDimension2D;

        /// The maximum pixel size along one axis of a 3d image.
        uint32_t maxTextureDimension3D;

        /// The maximum pixel size along one axis of a cube image.
        uint32_t maxTextureDimensionCube;

        /// The maximum size of an image array.
        uint32_t maxTextureArraySize;

        /// The alignment required for constant buffers.
        uint64_t minConstantBufferOffsetAlignment;

        /// The alignment required for storage buffers.
        uint64_t minStorageBufferOffsetAlignment;

        /// The maximum number of draws when doing indirect drawing.
        uint32_t maxDrawIndirectCount = 1;
    };

    class ALIMER_API IResource : public RefCounted
    {
    public:
        enum class Type
        {
            Buffer,
            Texture,
        };

        [[nodiscard]] virtual Type GetType() const = 0;
        [[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
    };

    class ALIMER_API ITexture : public IResource
    {
    public:
        Type GetType() const override final { return Type::Texture; }
    };

    class ALIMER_API ISwapChain : public RefCounted
    {
    public:
        virtual bool Resize(uint32_t width, uint32_t height) = 0;
    };

    class ALIMER_API ICommandList
    {
    protected:
        ICommandList() = default;

    public:
        virtual ~ICommandList() = default;

        // Non-copyable and non-movable
        ICommandList(const ICommandList&) = delete;
        ICommandList(const ICommandList&&) = delete;
        ICommandList& operator=(const ICommandList&) = delete;
        ICommandList& operator=(const ICommandList&&) = delete;

        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const char* name) = 0;

        virtual void BeginRenderPass(ISwapChain* swapChain, const float clearColor[4]) = 0;
        virtual void EndRenderPass() = 0;
    };

    using TextureHandle = RefCountPtr<ITexture>;
    using SwapChainHandle = RefCountPtr<ISwapChain>;

    class ALIMER_API IDevice : public RefCounted
    {
    public:
        virtual void WaitIdle() = 0;
        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;

        [[nodiscard]] virtual ICommandList* BeginCommandList(CommandQueue queue = CommandQueue::Graphics) = 0;

        /// Create new Texture.
        [[nodiscard]] TextureHandle CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr);

        /// Create new Texture.
        [[nodiscard]] TextureHandle CreateExternalTexture(const void* handle, const TextureDesc& desc);

        /// Create new SwapChain.
        [[nodiscard]] SwapChainHandle CreateSwapChain(void* windowHandle, const SwapChainDesc& desc);

        /// Returns the set of features supported by this device.
        const DeviceFeatures& GetFeatures() const { return features; }

        /// Returns the set of hardware limits for this device.
        const DeviceLimits& GetLimits() const { return limits; }

        ShaderFormat GetShaderFormat() const { return shaderFormat; }

        [[nodiscard]] virtual uint64_t GetFrameCount() const { return frameCount; }
        [[nodiscard]] virtual uint32_t GetFrameIndex() const { return frameIndex; }

    private:
        virtual TextureHandle CreateTextureCore(const TextureDesc& desc, const void* handle, const TextureData* initialData) = 0;
        virtual SwapChainHandle CreateSwapChainCore(void* windowHandle, const SwapChainDesc& desc) = 0;

    protected:
        DeviceFeatures features{};
        DeviceLimits limits{};
        ShaderFormat shaderFormat{};
        uint64_t frameCount = 0;
        uint32_t frameIndex = 0;
    };

    using DeviceHandle = RefCountPtr<IDevice>;

    extern ALIMER_API DeviceHandle GRHIDevice;

    /* Helper methods */
    ALIMER_API const char* GetVendorName(uint32_t vendorId);

    // Returns the number of mip levels given a texture size
    ALIMER_API uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth = 1);
}

namespace std
{
}
