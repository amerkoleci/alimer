// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Vector.h"
#include "Alimer/Core/RefCounted.h"
#include "Alimer/Core/PixelFormat.h"
#include "Alimer/Core/UnorderedMap.h"
#include "Alimer/Math/Color.h"
//#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Shaders/ShaderDefinitions.h"
#include <string>
#include <string_view>
#include <float.h>

namespace Alimer
{
    using BindlessIndex = int32_t;
    using GPUAddress = uint64_t;

    /* Constants */
    static constexpr uint32_t kNumFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxBindGroups = 4;
    static constexpr uint32_t kMaxPushConstantsSize = 128;  // 128 bytes is the minimum guaranteed size for push constants in Vulkan
    static constexpr uint32_t kMaxViewportsAndScissors = 16;
    static constexpr uint32_t kMaxVertexBuffers = 16;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047;
    static constexpr uint32_t kMaxVertexBufferStride = 2048;
    static constexpr uint32_t kMaxMipLevels = 16u;
    static constexpr uint32_t kMaxSamplerAnisotropy = 16u;
    static constexpr uint32_t kMipLevelCountUndefined = UINT32_MAX;
    static constexpr uint32_t kArrayLayerCountUndefined = UINT32_MAX;

    static constexpr uint32_t kQuerySetMaxQueries = 8192;
    static constexpr uint64_t kWholeSize = ~0ULL;
    static constexpr uint32_t kUUIDSize = 16u;
    static constexpr uint32_t kLUIDSize = 8u;

    /// Number of dynamic constant buffer slots to be bound in non bindless space.
    static constexpr uint32_t kDynamicConstantBufferCount = 1;

    // Static samplers
    static constexpr uint32_t kStaticSamplerCount = 10;
    static constexpr uint32_t kStaticSamplerRegisterSpaceBegin = 100;

    /// Bindless descriptor limits -> the device can allocate less than this, depending on capabilities
    static constexpr BindlessIndex kInvalidBindlessIndex = -1;
    static constexpr uint32_t kBindlessResourceCapacity = 500000;
    static constexpr uint32_t kBindlessSamplerCapacity = 256;

    // These shifts are made so that Vulkan resource bindings slots don't interfere with each other across shader stages:
    // These are also used during shader compilation
    namespace VulkanRegisterShift
    {
        static constexpr uint32_t kConstantBuffer = 0;  // b
        static constexpr uint32_t kSRV = 1000;          // t
        static constexpr uint32_t kUAV = 2000;          // u
        static constexpr uint32_t kSampler = 3000;      // s
    }

    /* Enums */
    enum class RHIStatus : uint32_t
    {
        Success = 0,
        Error = 1,
    };

    enum class RHIBackend : uint32_t
    {
        Null,
        Vulkan,
        D3D12,
        Metal,

        Count,
    };

    enum class RHIAdapterVendor : uint8_t
    {
        /// Adapter vendor is unknown
        Unknown = 0,

        /// Adapter vendor is NVIDIA
        NVIDIA,

        /// Adapter vendor is AMD
        AMD,

        /// Adapter vendor is Intel
        Intel,

        /// Adapter vendor is ARM
        ARM,

        /// Adapter vendor is Qualcomm
        Qualcomm,

        /// Adapter vendor is Imagination Technologies
        ImgTech,

        /// Adapter vendor is Microsoft (software rasterizer)
        MSFT,

        /// Adapter vendor is Apple
        Apple,

        /// Adapter vendor is Mesa (software rasterizer)
        Mesa,

        /// Adapter vendor is Broadcom (Raspberry Pi)
        Broadcom,

        Count
    };

    enum class RHIValidationMode : uint32_t
    {
        /// No validation is enabled.
        Disabled,
        /// Print warnings and errors
        Enabled,
        /// Print all warnings, errors and info messages
        Verbose,
        /// Enable GPU-based validation
        GPU
    };

    enum class RHIPowerPreference : uint32_t
    {
        Undefined = 0,
        LowPower,
        HighPerformance,
    };

    /// Describe the Adapter types
    enum class RHIAdapterType : uint8_t
    {
        /// The device is typically a separate processor connected to the host via an interlink.
        DiscreteGpu,
        /// The device is typically one embedded in or tightly coupled with the host.
        IntegratedGpu,
        /// The device is typically a virtual node in a virtualization environment.
        VirtualGpu,
        /// The device is typically running on the same processors as the host.
        Cpu,
        /// The device does not match any other available types.
        Other,
    };

    enum class RHIQueueType : uint8_t
    {
        Graphics,
        Compute,
        Copy,
        VideoDecode,

        Count
    };

    enum class RHIMemoryType : uint8_t
    {
        /// CPU no access, GPU read/write
        Private,
        /// CPU write, GPU read
        Upload,
        /// CPU read, GPU write
        Readback
    };

    enum class RHIBufferStates : uint32_t
    {
        Undefined = 0,
        CopyDest = ALIMER_BIT(0),
        CopySource = ALIMER_BIT(1),
        ShaderResource = ALIMER_BIT(2),
        UnorderedAccess = ALIMER_BIT(3),
        VertexBuffer = ALIMER_BIT(4),
        IndexBuffer = ALIMER_BIT(5),
        ConstantBuffer = ALIMER_BIT(6),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIBufferStates);

    enum class RHITextureLayout : uint32_t
    {
        Undefined,
        CopySource,
        CopyDest,
        ResolveSource,
        ResolveDest,
        ShaderResource,
        UnorderedAccess,
        RenderTarget,
        DepthWrite,
        DepthRead,

        Present,
        ShadingRateSurface,
    };

    enum class RHIBufferUsage : uint32_t
    {
        None = 0,
        /// Supports input assembly access as VertexBuffer.
        Vertex = 1 << 0,
        /// Supports input assembly access as IndexBuffer.
        Index = 1 << 1,
        /// Supports Constant buffer access.
        Constant = 1 << 2,
        /// Supports shader read access.
        ShaderRead = 1 << 3,
        /// Supports shader write access.
        ShaderWrite = 1 << 4,
        /// Supports indirect buffer access for indirect draw/dispatch.
        Indirect = 1 << 5,
        /// Supports ray tracing acceleration structure usage.
        RayTracing = 1 << 6,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIBufferUsage);

    /// Defines dimension of Texture
    enum class RHITextureDimension : uint32_t
    {
        /// One-dimensional Texture.
        Texture1D,
        /// Two-dimensional Texture.
        Texture2D,
        /// Three-dimensional Texture.
        Texture3D,
        /// Cubemap Texture.
        TextureCube,
    };

    enum class RHITextureUsage : uint32_t
    {
        None,
        ShaderRead = (1 << 0),
        ShaderWrite = (1 << 1),
        ShaderReadWrite = ShaderRead | ShaderWrite,
        RenderTarget = (1 << 2),
        Transient = (1 << 3),
        ShadingRate = (1 << 4),
        Shared = (1 << 5),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHITextureUsage);

    enum class RHITextureSampleCount : uint32_t
    {
        Count1 = (1 << 0),
        Count2 = (1 << 1),
        Count4 = (1 << 2),
        Count8 = (1 << 3),
        Count16 = (1 << 4),
        Count32 = (1 << 5),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHITextureSampleCount);

    enum class RHITextureAspect : uint32_t
    {
        All = 0,
        DepthOnly = 1,
        StencilOnly = 2,
    };

    enum class RHITextureSwizzle : uint8_t
    {
        Zero = 0,
        One = 1,
        Red = 2,
        Green = 3,
        Blue = 4,
        Alpha = 5,
    };

    enum class RHISamplerMinMagFilter : uint32_t
    {
        Point,
        Linear,
    };

    enum class RHISamplerMipFilter : uint32_t
    {
        Point,
        Linear,
    };

    enum class RHISamplerAddressMode : uint32_t
    {
        Clamp,
        Wrap,
        Mirror,
        MirrorOnce,
        Border,
    };

    enum class RHISamplerReductionType : uint8_t
    {
        Standard,
        Comparison,
        Minimum,
        Maximum
    };

    enum class RHISamplerBorderColor : uint32_t
    {
        FloatTransparentBlack,
        FloatOpaqueBlack,
        FloatOpaqueWhite,
        UintTransparentBlack,
        UintOpaqueBlack,
        UintOpaqueWhite,
    };

    enum class RHIQueryType : uint32_t
    {
        /// Create a heap to contain timestamp queries
        Timestamp,
        /// Used for occlusion query heap or occlusion queries
        Occlusion,
        /// Can be used in the same heap as occlusion
        BinaryOcclusion,
        /// Create a heap to contain a structure of `PipelineStatistics`
        PipelineStatistics,
    };

    enum class RHICompareFunction : uint32_t
    {
        Never = 0,
        Less = 1,
        Equal = 2,
        LessEqual = 3,
        Greater = 4,
        NotEqual = 5,
        GreaterEqual = 6,
        Always = 7,
    };

    /// Operation to perform on the stencil value.
    enum class RHIStencilOperation : uint32_t
    {
        /// Keep stencil value unchanged.
        Keep = 0,
        /// Set stencil value to zero.
        Zero = 1,
        /// Replace stencil value with value provided in most recent call to SetStencilReference
        Replace = 2,
        /// Bitwise  inverts stencil value.
        Invert = 3,
        /// Increments stencil value by one, clamping on overflow.
        IncrementClamp = 4,
        /// Decrements stencil value by one, clamping on underflow.
        DecrementClamp = 5,
        /// Increments stencil value by one, wrapping on overflow.
        IncrementWrap = 6,
        /// Decrements stencil value by one, wrapping on underflow.
        DecrementWrap = 7,
    };

    enum class RHIBlendFactor : uint32_t
    {
        Zero = 0,
        One = 1,
        SourceColor = 2,
        OneMinusSourceColor = 3,
        SourceAlpha = 4,
        OneMinusSourceAlpha = 5,
        DestinationColor = 6,
        OneMinusDestinationColor = 7,
        DestinationAlpha = 8,
        OneMinusDestinationAlpha = 9,
        SourceAlphaSaturate = 10,
        BlendColor = 11,
        OneMinusBlendColor = 12,
        BlendAlpha = 13,
        OneMinusBlendAlpha = 14,
        Source1Color = 15,
        OneMinusSource1Color = 16,
        Source1Alpha = 17,
        OneMinusSource1Alpha = 18,
    };

    enum class RHIBlendOperation : uint32_t
    {
        Add = 0,
        Subtract = 1,
        ReverseSubtract = 2,
        Min = 3,
        Max = 4,
    };

    enum class RHIColorWriteMask : uint32_t
    {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = 15
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIColorWriteMask);

    enum class RHIFillMode : uint32_t
    {
        Solid,
        Wireframe,
    };

    enum class RHICullMode : uint32_t
    {
        None = 0,
        Front = 1,
        Back = 2,
    };

    enum class RHIFrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1,
    };

    enum class RHIDepthClipMode : uint32_t
    {
        Clip,
        Clamp
    };

    enum class RHIVertexAttributeSemantic : uint32_t
    {
        Undefined,
        Position,
        Normal,
        Tangent,
        TexCoord,
        Color,
        BlendIndices,
        BlendWeight,

        Custom
    };

    enum class RHIVertexAttributeFormat : uint32_t
    {
        Undefined = 0,
        Uint8,
        Uint8x2,
        Uint8x4,
        Sint8,
        Sint8x2,
        Sint8x4,
        Unorm8,
        Unorm8x2,
        Unorm8x4,
        Snorm8,
        Snorm8x2,
        Snorm8x4,

        Uint16,
        Uint16x2,
        Uint16x4,
        Sint16,
        Sint16x2,
        Sint16x4,
        Unorm16,
        Unorm16x2,
        Unorm16x4,
        Snorm16,
        Snorm16x2,
        Snorm16x4,
        Float16,
        Float16x2,
        Float16x4,

        Float32,
        Float32x2,
        Float32x3,
        Float32x4,
        Uint32,
        Uint32x2,
        Uint32x3,
        Uint32x4,
        Sint32,
        Sint32x2,
        Sint32x3,
        Sint32x4,

        //Int1010102Normalized,
        Unorm10_10_10_2,
        Unorm8x4BGRA,
        //RG11B10Float,
        //RGB9E5Float,

        Count
    };

    enum class RHIVertexStepMode : uint32_t
    {
        /// Vertex data is advanced every vertex.
        Vertex = 0,
        /// Vertex data is advanced every instance.
        Instance = 1
    };

    /// Index buffer element format.
    enum class RHIIndexFormat : uint32_t
    {
        /// Undefined index format, used to disable index buffer stripping <see cref="RenderPipelineDescriptor.StripIndexFormat"/>.
        Undefined,
        /// 16-bit unsigned integer indices.
        Uint16,
        /// 32-bit unsigned integer indices.
        Uint32,
    };

    enum class RHIPrimitiveTopology : uint32_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    };

    enum class RHILoadAction : uint32_t
    {
        Load,
        Clear,
        Discard,
    };

    enum class RHIStoreAction : uint32_t
    {
        Store,
        Discard
    };

    enum class RHIShaderStages : uint32_t
    {
        None = 0,
        Vertex = ALIMER_BIT(0),
        Fragment = ALIMER_BIT(1),
        Compute = ALIMER_BIT(2),
        Mesh = ALIMER_BIT(3),
        Amplification = ALIMER_BIT(4),

        All = 0x3FFF,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIShaderStages);

    enum class RHICompositeAlphaMode : uint32_t
    {
        /// Lets the backend implementation choose the best mode (supported, and with the best performance) between RHICompositeAlphaMode::Opaque or RHICompositeAlphaMode::Inherit.
        Auto = 0,
        /// The alpha component of the image is ignored and teated as if it is always 1.0.
        Opaque,
        /// The alpha component is respected and non-alpha components are assumed to be already multiplied with the alpha component. For example, (0.5, 0, 0, 0.5) is semi-transparent bright red.
        Premultiplied,
        /// The alpha component is respected and non-alpha components are assumed to NOT be already multiplied with the alpha component. For example, (1.0, 0, 0, 0.5) is semi-transparent bright red.
        Unpremultiplied,
        /// The handling of the alpha component is unknown to WebGPU and should be handled by the application using system-specific APIs. This mode may be unavailable (for example on Wasm).
        Inherit,
    };

    enum class RHIColorSpace : uint32_t
    {
        SRGB,			// SDR color space (8 or 10 bits per channel)
        HDR10_ST2084,	// HDR10 color space (10 bits per channel)
        HDR_LINEAR,		// HDR color space (16 bits per channel)
    };

    enum class RHIPresentMode : uint32_t
    {
        Immediate,
        Mailbox,
        Fifo,
    };

    enum class RHIShadingRate : uint8_t
    {
        Rate1x1,	// Default/full shading rate
        Rate1x2,
        Rate2x1,
        Rate2x2,
        Rate2x4,
        Rate4x2,
        Rate4x4,

        Invalid
    };

    enum class RHIShaderModel : uint32_t
    {
        Model_6_0,
        Model_6_1,
        Model_6_2,
        Model_6_3,
        Model_6_4,
        Model_6_5,
        Model_6_6,
        Model_6_7,
        Model_6_8,
        Model_6_9,

        Highest = Model_6_9,
    };

    enum class RHIConservativeRasterizationTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,
        Tier3 = 3,
    };

    enum class RHIVariableRateShadingTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,

    };

    enum class RHIRayTracingTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,
    };

    enum class RHIMeshShaderTier
    {
        NotSupported = 0,
        Tier1 = 1,
    };

    enum class RHIFeature : uint16_t
    {
        TimestampQuery,
        PipelineStatisticsQuery,
        TextureCompressionBC,
        TextureCompressionETC2,
        TextureCompressionASTC,
        TextureCompressionASTC_HDR,
        IndirectFirstInstance,
        ShaderFloat16,

        GPUUploadHeapSupported,
        CopyQueueTimestampQuery,
        TessellationShader,
        DepthBoundsTest,

        SamplerMirrorOnce,
        SamplerBorder,
        SamplerMinMax,

        DepthResolveMinMax,
        StencilResolveMinMax,
        ShaderOutputViewportIndex,
        CacheCoherentUMA,
    };

    enum class RHIPixelFormatSupport : uint32_t
    {
        None = 0,

        Texture = (1 << 0),
        RenderTarget = (1 << 1),
        DepthStencil = (1 << 2),
        Blendable = (1 << 3),

        ShaderLoad = (1 << 4),
        ShaderSample = (1 << 5),
        ShaderUavLoad = (1 << 6),
        ShaderUavStore = (1 << 7),
        ShaderAtomic = (1 << 8),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIPixelFormatSupport);

    /* Forward declarations */
    class RHIBuffer;
    class RHITexture;
    class RHITextureView;
    class RHISampler;
    class RHIShaderModule;
    class RHICommandEncoder;
    class RHIComputePassEncoder;
    class RHIRenderPassEncoder;
    class RHICommandBuffer;
    class RHIQueue;
    class RHIComputePipeline;
    class RHIRenderPipeline;
    class RHISurfaceSource;
    class RHISurface;
    class RHIQueryHeap;
    class RHIDevice;
    class RHIAdapter;
    class RHIFactory;

    using RHIBufferRef = SharedPtr<RHIBuffer>;
    using RHITextureRef = SharedPtr<RHITexture>;
    using RHITextureViewRef = SharedPtr<RHITextureView>;
    using RHISamplerRef = SharedPtr<RHISampler>;
    using RHIShaderModuleRef = SharedPtr<RHIShaderModule>;
    using RHIComputePipelineRef = SharedPtr<RHIComputePipeline>;
    using RHIRenderPipelineRef = SharedPtr<RHIRenderPipeline>;
    using RHIQueryHeapRef = SharedPtr<RHIQueryHeap>;
    using RHISurfaceSourceRef = SharedPtr<RHISurfaceSource>;
    using RHISurfaceRef = SharedPtr<RHISurface>;
    using RHIDeviceRef = SharedPtr<RHIDevice>;
    using RHIFactoryRef = SharedPtr<RHIFactory>;

    /* Structs */
    union ClearColorValue
    {
        float       float32[4];
        int32_t     int32[4];
        uint32_t    uint32[4];
    };

    struct ClearDepthStencilValue
    {
        float       depth;
        uint32_t    stencil;
    };

    struct RHIScissorRect
    {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    struct RHIViewport
    {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    struct RHIBufferDesc
    {
        const char* label = nullptr;
        uint64_t size = 0u;
        RHIBufferUsage usage = RHIBufferUsage::None;
        RHIMemoryType memoryType = RHIMemoryType::Private;
        //uint32_t stride = 0; // Needed for StructuredBuffer
    };

    struct TextureSwizzleChannels
    {
        RHITextureSwizzle red = RHITextureSwizzle::Red;
        RHITextureSwizzle green = RHITextureSwizzle::Green;
        RHITextureSwizzle blue = RHITextureSwizzle::Blue;
        RHITextureSwizzle alpha = RHITextureSwizzle::Alpha;
    };

    struct RHITextureDesc
    {
        RHITextureDimension dimension = RHITextureDimension::Texture2D;
        PixelFormat format = PixelFormat::RGBA8Unorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        uint32_t mipLevelCount = 1;
        RHITextureSampleCount sampleCount = RHITextureSampleCount::Count1;
        RHITextureUsage usage = RHITextureUsage::ShaderRead;
        const char* label = nullptr;
        //MemoryType memoryType = MemoryType::Private;

        static constexpr RHITextureDesc Texture1D(
            PixelFormat format,
            uint32_t width,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            RHITextureUsage usage = RHITextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            RHITextureDesc desc;
            desc.dimension = RHITextureDimension::Texture1D;
            desc.format = format;
            desc.width = width;
            desc.height = 1u;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = RHITextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }

        static constexpr RHITextureDesc Texture2D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            RHITextureUsage usage = RHITextureUsage::ShaderRead,
            RHITextureSampleCount sampleCount = RHITextureSampleCount::Count1,
            const char* label = nullptr)
        {
            RHITextureDesc desc;
            desc.dimension = RHITextureDimension::Texture2D;
            desc.format = format;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = sampleCount;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }

        static constexpr RHITextureDesc Texture3D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevelCount = 1,
            RHITextureUsage usage = RHITextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            RHITextureDesc desc;
            desc.dimension = RHITextureDimension::Texture3D;
            desc.format = format;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = depth;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = RHITextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }

        static constexpr RHITextureDesc TextureCube(
            PixelFormat format,
            uint32_t size,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            RHITextureUsage usage = RHITextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            RHITextureDesc desc;
            desc.dimension = RHITextureDimension::TextureCube;
            desc.format = format;
            desc.width = size;
            desc.height = size;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = RHITextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }
    };

    struct RHITextureData
    {
        const void* pData = nullptr;
        uint32_t rowPitch = 0;
        uint32_t slicePitch = 0;
    };

    struct RHITextureViewDesc
    {
        PixelFormat format = PixelFormat::Undefined;
        RHITextureAspect aspect = RHITextureAspect::All;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = kMipLevelCountUndefined;
        uint32_t baseArrayLayer = 0;                            // For Texture3D, this is WSlice.
        uint32_t arrayLayerCount = kArrayLayerCountUndefined;   // For cube maps, this is a multiple of 6.
        TextureSwizzleChannels swizzle{};

        /// The name of the texture for debugging purposes.
        const char* label = nullptr;
    };

    struct RHISamplerDesc
    {
        const char* label = nullptr;
        RHISamplerReductionType reductionType = RHISamplerReductionType::Standard;
        RHISamplerMinMagFilter minFilter = RHISamplerMinMagFilter::Point;
        RHISamplerMinMagFilter magFilter = RHISamplerMinMagFilter::Point;
        RHISamplerMipFilter mipFilter = RHISamplerMipFilter::Point;
        RHISamplerAddressMode addressModeU = RHISamplerAddressMode::Clamp;
        RHISamplerAddressMode addressModeV = RHISamplerAddressMode::Clamp;
        RHISamplerAddressMode addressModeW = RHISamplerAddressMode::Clamp;
        RHISamplerBorderColor borderColor = RHISamplerBorderColor::FloatTransparentBlack;
        float lodMinClamp = 0.0f;
        float lodMaxClamp = FLT_MAX;
        RHICompareFunction compareFunction = RHICompareFunction::Never;
        uint16_t maxAnisotropy = 1u;
    };

    struct RHIShaderModuleDesc
    {
        const char* label = nullptr;
        RHIShaderStages stage = RHIShaderStages::None;
        size_t byteCodeSize;
        const void* byteCode;
        const char* entryPoint = "main";
    };

    struct RHIComputePipelineDesc
    {
        const char* label = nullptr;
        RHIShaderModuleRef shader;
    };

    struct RHIVertexAttribute
    {
        RHIVertexAttributeSemantic semantic = RHIVertexAttributeSemantic::Undefined;
        RHIVertexAttributeFormat format = RHIVertexAttributeFormat::Undefined;
        uint32_t offset = 0;
        uint32_t semanticIndex = 0;
    };

    struct RHIVertexBufferLayout
    {
        uint32_t stride = 0;
        RHIVertexStepMode stepMode = RHIVertexStepMode::Vertex;
        uint32_t attributeCount = 0;
        const RHIVertexAttribute* attributes = nullptr;
    };

    struct RHIRenderTargetBlendState
    {
        RHIBlendFactor srcColorBlendFactor = RHIBlendFactor::One;
        RHIBlendFactor destColorBlendFactor = RHIBlendFactor::Zero;
        RHIBlendOperation colorBlendOp = RHIBlendOperation::Add;
        RHIBlendFactor srcAlphaBlendFactor = RHIBlendFactor::One;
        RHIBlendFactor destAlphaBlendFactor = RHIBlendFactor::Zero;
        RHIBlendOperation alphaBlendOp = RHIBlendOperation::Add;
        RHIColorWriteMask colorWriteMask = RHIColorWriteMask::All;
    };

    struct RHIBlendState
    {
        bool alphaToCoverageEnable = false;
        bool independentBlendEnable = false;

        RHIRenderTargetBlendState renderTargets[kMaxColorAttachments] = {};
    };

    struct RHIRasterizerState
    {
        RHIFillMode fillMode = RHIFillMode::Solid;
        RHICullMode cullMode = RHICullMode::Back;
        RHIFrontFace frontFace = RHIFrontFace::CounterClockwise;
        float depthBias = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeScale = 0.0f;
        RHIDepthClipMode depthClipMode = RHIDepthClipMode::Clip;
        bool conservativeRasterEnable = false;
    };

    struct RHIStencilFaceState
    {
        RHIStencilOperation failOp = RHIStencilOperation::Keep;
        RHIStencilOperation depthFailOp = RHIStencilOperation::Keep;
        RHIStencilOperation passOp = RHIStencilOperation::Keep;
        RHICompareFunction compareFunc = RHICompareFunction::Always;
    };

    struct RHIDepthStencilState
    {
        bool depthWriteEnabled = true;
        RHICompareFunction depthCompare = RHICompareFunction::Less;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
        RHIStencilFaceState frontFace{};
        RHIStencilFaceState backFace{};
        bool depthBoundsTestEnable = false;
    };

    struct RHIRenderPipelineDesc
    {
        const char* label = nullptr;

        RHIShaderModuleRef vertexShader = nullptr;
        RHIShaderModuleRef fragmentShader = nullptr;
        RHIShaderModuleRef meshShader = nullptr;
        RHIShaderModuleRef amplificationShader = nullptr;

        uint32_t vertexBufferLayoutCount = 0;
        const RHIVertexBufferLayout* vertexBufferLayouts = nullptr;

        RHIBlendState blendState{};
        RHIRasterizerState rasterizerState{};
        RHIDepthStencilState depthStencilState{};

        RHIPrimitiveTopology primitiveTopology = RHIPrimitiveTopology::TriangleList;

        RHIIndexFormat stripIndexFormat = RHIIndexFormat::Undefined;

        PixelFormat colorAttachmentFormats[kMaxColorAttachments] = {};
        PixelFormat depthStencilFormat = PixelFormat::Undefined;
        RHITextureSampleCount sampleCount = RHITextureSampleCount::Count1;
    };

    struct RHIQueryHeapDesc
    {
        const char* label = nullptr;
        RHIQueryType type = RHIQueryType::Timestamp;
        uint32_t count = 1u;
    };

    struct RHIRenderPassColorAttachment
    {
        RHITextureView* view = nullptr;
        RHITextureView* resolveView = nullptr;

        RHILoadAction loadAction = RHILoadAction::Load;
        RHIStoreAction storeAction = RHIStoreAction::Store;
        ClearColorValue clearColor{};
    };

    struct RHIRenderPassDepthStencilAttachment
    {
        RHITextureView* view = nullptr;

        RHILoadAction depthLoadAction = RHILoadAction::Clear;
        RHIStoreAction depthStoreAction = RHIStoreAction::Discard;
        float depthClearValue = 1.0f;
        bool depthReadOnly = false;

        RHILoadAction stencilLoadAction = RHILoadAction::Clear;
        RHIStoreAction stencilStoreAction = RHIStoreAction::Discard;
        uint8_t stencilClearValue = 0;
        bool stencilReadOnly = false;
    };

    struct RHIComputePassDesc
    {
        const char* label = nullptr;
    };

    struct RHIRenderPassDesc
    {
        const char* label = nullptr;
        uint32_t colorAttachmentCount = 0;
        const RHIRenderPassColorAttachment* colorAttachments = nullptr;
        const RHIRenderPassDepthStencilAttachment* depthStencilAttachment = nullptr;
        const RHITexture* shadingRateTexture = nullptr;
    };

    struct RHISurfaceCapabilities
    {
        RHITextureUsage supportedUsages;
        PixelFormat preferredFormat;
        uint32_t formatCount;
        const PixelFormat* formats;
        uint32_t presentModeCount;
        const RHIPresentMode* presentModes;
        uint32_t alphaModeCount;
        const RHICompositeAlphaMode* alphaModes;
    };

    struct RHISurfaceConfig
    {
        const char* label = nullptr;
        PixelFormat format = PixelFormat::BGRA8UnormSrgb;
        uint32_t width = 0;
        uint32_t height = 0;
        RHICompositeAlphaMode alphaMode = RHICompositeAlphaMode::Auto;
        RHIPresentMode presentMode = RHIPresentMode::Fifo;
    };

    struct RHIDeviceDesc
    {
        const char* label = nullptr;
    };

    struct RHIFactoryDesc
    {
        RHIBackend preferredBackend = RHIBackend::Count;
        RHIValidationMode validationMode = RHIValidationMode::Disabled;
        std::string label;
    };

    struct RHIDeviceLimits
    {
        /// The maximum dimesion of 1D texture.
        uint32_t maxTextureDimension1D = 0;

        /// The maximum dimesion of 2D texture.
        uint32_t maxTextureDimension2D = 0;

        /// The maximum dimesion of 3D texture.
        uint32_t maxTextureDimension3D = 0;

        /// The maximum dimesion of cube texture.
        uint32_t maxTextureDimensionCube = 0;

        /// The maximum size of an image array.
        uint32_t maxTextureArrayLayers = 0;

        /// The maximum range of constant buffer.
        uint64_t maxConstantBufferBindingSize = 0;

        /// The maximum size of storage buffer.
        uint64_t maxStorageBufferBindingSize = 0;

        /// The alignment required for constant buffers.
        uint64_t minConstantBufferOffsetAlignment = 0;

        /// The alignment required for storage buffers.
        uint64_t minStorageBufferOffsetAlignment = 0;

        /// The alignment required for texture data in buffer when using it as texture upload source.
        uint64_t textureRowPitchAlignment = 0;

        /// The alignment required for texture data in buffer when using it as texture upload source for 3D textures.
        uint64_t textureDepthPitchAlignment = 0;

        /// The alignment required for linear buffer allocations.
        uint64_t minLinearAllocatorOffsetAlignment = 0;

        /// The maximum size of buffer.
        uint64_t maxBufferSize;

        uint32_t maxVertexBuffers = 0;
        uint32_t maxVertexAttributes = 0;
        uint32_t maxVertexBufferArrayStride = 0;

        /// The maximum number of color attachments.
        uint32_t maxColorAttachments = 0;

        /// The maximum number of viewports.
        uint32_t maxViewports = 0;

        uint16_t samplerMaxAnisotropy;

        uint32_t maxComputeWorkgroupStorageSize = 0;
        uint32_t maxComputeInvocationsPerWorkgroup = 0;
        uint32_t maxComputeWorkgroupSizeX = 0;
        uint32_t maxComputeWorkgroupSizeY = 0;
        uint32_t maxComputeWorkgroupSizeZ = 0;
        uint32_t maxComputeWorkgroupsPerDimension = 0;

        /* Highest supported shader model */
        RHIShaderModel highestShaderModel = RHIShaderModel::Model_6_0;

        /* ConservativeRasterization tier */
        RHIConservativeRasterizationTier conservativeRasterizationTier = RHIConservativeRasterizationTier::NotSupported;

        // VariableRateShading
        RHIVariableRateShadingTier variableShadingRateTier = RHIVariableRateShadingTier::NotSupported;
        uint32_t variableShadingRateImageTileSize = 0;
        bool isAdditionalVariableShadingRatesSupported = false;

        // Ray tracing
        RHIRayTracingTier rayTracingTier = RHIRayTracingTier::NotSupported;
        uint32_t rayTracingShaderGroupIdentifierSize = 0;
        uint32_t rayTracingShaderTableAlignment = 0;
        uint32_t rayTracingShaderTableMaxStride = 0;
        uint32_t rayTracingShaderRecursionMaxDepth = 0;
        uint32_t rayTracingMaxGeometryCount = 0;
        uint32_t rayTracingScratchAlignment = 0;

        /* Mesh shader */
        RHIMeshShaderTier meshShaderTier = RHIMeshShaderTier::NotSupported;
    };

    struct RHIAdapterProperties
    {
        std::string         deviceName;
        uint16_t            driverVersion[4];
        std::string         driverDescription;
        RHIAdapterVendor    vendor = RHIAdapterVendor::Unknown;
        uint32_t            vendorID = 0;
        uint32_t            deviceID = 0;
        RHIAdapterType      type = RHIAdapterType::Other;
        uint8_t             uuid[kUUIDSize];
        uint64_t            luid[kLUIDSize];
        uint64_t            videoMemorySize = 0;
        uint64_t            systemMemorySize = 0;
    };

    /* Types */
    enum class RHINativeHandleType : uint32_t
    {
        Unknown = 0x00000000,
        SharedHandle = 0x00000001,

        /* DirectX */
        DXGI_Factory = 0x00020001,
        DXGI_Adapter = 0x00020002,
        D3D12_Device = 0x00020003,
        D3D12_GraphicsCommandList = 0x00020004,
        D3D12_CommandAllocator = 0x00020005,
        D3D12_Resource = 0x00020006,

        /* Vulkan */
        VK_Instance = 0x00030001,
        VK_PhysicalDevice = 0x00030002,
        VK_Device = 0x00030003,
        VK_CommandBuffer = 0x00030004,
        VK_Buffer = 0x00030005,
        VK_Image = 0x00030006,
        VK_ImageView = 0x00030007,
    };

    struct RHINativeHandle
    {
        union {
            uint64_t integer;
            void* pointer;
        };

        RHINativeHandle(uint64_t i) : integer(i) {}  // NOLINT(cppcoreguidelines-pro-type-member-init)
        RHINativeHandle(void* p) : pointer(p) {}     // NOLINT(cppcoreguidelines-pro-type-member-init)

        template<typename T> operator T* () const { return static_cast<T*>(pointer); }
        operator bool() const { return pointer != nullptr; }
    };

    class ALIMER_API RHIObject : public RefCounted
    {
    public:
        virtual void SetLabel(const char* label) { ALIMER_UNUSED(label); }
        virtual RHINativeHandle GetNativeHandle(RHINativeHandleType type) { (void)type; return nullptr; }

    protected:
        RHIObject() = default;
    };

    class ALIMER_API RHIBuffer : public RHIObject
    {
    public:
        [[nodiscard]] uint64_t GetSize() const { return desc.size; }
        [[nodiscard]] RHIBufferUsage GetUsage() const { return desc.usage; }
        [[nodiscard]] virtual void* GetMappedData() const = 0;
        [[nodiscard]] virtual GPUAddress GetGPUAddress() const = 0;
        [[nodiscard]] virtual RHIBufferStates GetCurrentState() const { return currentState; }

    protected:
        RHIBuffer(const RHIBufferDesc& desc_)
            : desc(desc_)
        {}

        RHIBufferDesc desc;
        mutable RHIBufferStates currentState = RHIBufferStates::Undefined;
    };

    class ALIMER_API RHITexture : public RHIObject
    {
        friend class RHICommandBuffer; // SetLayout

    protected:
        RHITexture(const RHITextureDesc& desc, RHITextureLayout initialLayout);

    public:
        [[nodiscard]] constexpr RHITextureDimension GetDimension() const { return dimension; }
        [[nodiscard]] constexpr PixelFormat GetFormat() const { return format; }
        [[nodiscard]] constexpr uint32_t GetWidth(uint32_t mipLevel = 0) const
        {
            return (mipLevel == 0) || (mipLevel < mipLevelCount) ? Alimer::Max(1u, width >> mipLevel) : 1u;
        }

        [[nodiscard]] constexpr uint32_t GetHeight(uint32_t mipLevel = 0) const
        {
            return (mipLevel == 0) || (mipLevel < mipLevelCount) ? Alimer::Max(1u, height >> mipLevel) : 1u;
        }

        [[nodiscard]] constexpr uint32_t GetDepth(uint32_t mipLevel = 0) const
        {
            if (dimension != RHITextureDimension::Texture3D)
            {
                return 1;
            }

            return (mipLevel == 0) || (mipLevel < mipLevelCount) ? Alimer::Max(1u, depthOrArrayLayers >> mipLevel) : 0;
        }

        [[nodiscard]] constexpr uint32_t GetArrayLayers() const
        {
            if (dimension == RHITextureDimension::Texture3D)
            {
                return 1;
            }

            return depthOrArrayLayers;
        }

        [[nodiscard]] constexpr RHITextureUsage GetUsage() const { return usage; }
        [[nodiscard]] constexpr uint32_t GetMipLevelCount() const { return mipLevelCount; }
        [[nodiscard]] constexpr RHITextureSampleCount GetSampleCount() const { return sampleCount; }
        [[nodiscard]] constexpr bool IsMultisampled() const { return sampleCount > RHITextureSampleCount::Count1; }

        [[nodiscard]] RHITextureView* GetDefaultView() const;
        [[nodiscard]] RHITextureView* GetView(const RHITextureViewDesc* desc = nullptr) const;

        [[nodiscard]] uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice = 0) const;
        [[nodiscard]] RHITextureLayout GetLayout(uint32_t mipLevel, uint32_t arrayLayer, uint32_t placeSlice = 0) const;
        [[nodiscard]] RHITextureLayout GetLayout(uint32_t subresource) const;

    protected:
        virtual RHITextureViewRef CreateView(const RHITextureViewDesc& desc)  const = 0;
        void SetLayout(RHITextureLayout newLayout) const;
        void SetLayout(uint32_t subresource, RHITextureLayout newLayout) const;
        void SetLayout(RHITextureLayout newLayout, uint32_t mipLevel, uint32_t arrayLayer, uint32_t placeSlice = 0) const;
        void SetLayout(RHITextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const;

    protected:
        RHITextureDimension dimension;
        PixelFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t depthOrArrayLayers;
        uint32_t mipLevelCount;
        RHITextureSampleCount sampleCount;
        RHITextureUsage usage;
        mutable std::vector<RHITextureLayout> subresourceLayouts;

    private:
        mutable RHITextureViewRef defaultView;
        mutable UnorderedMap<size_t, RHITextureViewRef> views;
    };

    class ALIMER_API RHITextureView : public RHIObject
    {
    protected:
        RHITextureView(const RHITexture* texture_, const RHITextureViewDesc& desc)
            : texture(texture_)
            , format(desc.format)
            , aspect(desc.aspect)
            , baseMipLevel(desc.baseMipLevel)
            , mipLevelCount(desc.mipLevelCount)
            , baseArrayLayer(desc.baseArrayLayer)
            , arrayLayerCount(desc.arrayLayerCount)
            , swizzle(desc.swizzle)
        {}

    public:
        [[nodiscard]] const RHITexture* GetTexture() const { return texture; }
        [[nodiscard]] constexpr PixelFormat GetFormat() const { return format; }
        [[nodiscard]] constexpr RHITextureAspect GetAspect() const { return aspect; }
        [[nodiscard]] constexpr uint32_t GetBaseMipLevel() const { return baseMipLevel; }
        [[nodiscard]] constexpr uint32_t GetMipLevelCount() const { return mipLevelCount; }
        [[nodiscard]] constexpr uint32_t GetBaseArrayLayer() const { return baseArrayLayer; }
        [[nodiscard]] constexpr uint32_t GetArrayLayerCount() const { return arrayLayerCount; }
        [[nodiscard]] const TextureSwizzleChannels& GetSwizzle() const { return swizzle; }

        [[nodiscard]] constexpr uint32_t GetWidth() const { return texture->GetWidth(baseMipLevel); }
        [[nodiscard]] constexpr uint32_t GetHeight() const { return texture->GetHeight(baseMipLevel); }
        [[nodiscard]] uint32_t GetSubresourceIndex(uint32_t planeSlice = 0) const;

    protected:
        const RHITexture* texture;
        PixelFormat format;
        RHITextureAspect aspect;
        uint32_t baseMipLevel;
        uint32_t mipLevelCount;
        uint32_t baseArrayLayer;    // For Texture3D, this is WSlice.
        uint32_t arrayLayerCount;   // For cube maps, this is a multiple of 6.
        TextureSwizzleChannels swizzle;
    };

    class ALIMER_API RHISampler : public RHIObject
    {
    public:
        [[nodiscard]] virtual const RHISamplerDesc& GetDesc() const = 0;
    };

    class ALIMER_API RHIShaderModule : public RHIObject
    {
    public:
    };

    class ALIMER_API RHIQueue : public RHIObject
    {
    public:
        virtual RHIQueueType GetType() const = 0;
    };

    class ALIMER_API RHIComputePipeline : public RHIObject
    {};

    class ALIMER_API RHIRenderPipeline : public RHIObject
    {};

    class ALIMER_API RHIQueryHeap : public RHIObject
    {
    public:
        virtual uint32_t GetCount() const = 0;
        virtual RHIQueryType GetType() const = 0;
    };

    class ALIMER_API RHISurfaceSource final : public RHIObject
    {
    public:
        enum class Type
        {
            WindowsHWND,
            SwapChainPanel,
            AndroidWindow,
            WaylandSurface,
            XlibWindow,
            MetalLayer,
        };

        Type GetType() const { return type; }

        static RHISurfaceSourceRef CreateWin32(/*HWND*/void* hwnd);
        static RHISurfaceSourceRef CreateSwapChainPanel(/*IUnknown*/void* swapChainPanel);
        static RHISurfaceSourceRef CreateAndroid(/*ANativeWindow*/void* window);
        static RHISurfaceSourceRef CreateWayland(/*wl_display*/void* waylandDisplay, /*wl_surface*/void* waylandSurface);
        static RHISurfaceSourceRef CreateXlib(/*Display*/void* display, /*Window*/uint64_t window);
        static RHISurfaceSourceRef CreateMetalLayer(/*CAMetalLayer*/void* layer);

        // Valid to call if the type is WindowsHWND
        void* GetHWND() const;
        // Valid to call if the type is SwapChainPanel
        void* GetSwapChainPanel() const;
        // Valid to call if the type is Android
        void* GetAndroidWindow() const;
        // Valid to call if the type is WaylandSurface
        void* GetWaylandDisplay() const;
        void* GetWaylandSurface() const;
        // Valid to call if the type is XlibWindow
        void* GetXDisplay() const;
        uint64_t GetXWindow() const;
        // Valid to call if the type is MetalLayer
        void* GetMetalLayer() const;

    private:
        RHISurfaceSource(Type type);

        Type type;

        // WindowsHwnd
        void* hwnd = nullptr;
        // IDCompositionVisual/SwapChainPanel
        void* idCompositionVisualOrSwapChainPanel = nullptr;
        // ANativeWindow
        void* androidWindow = nullptr;
        // Wayland
        void* waylandDisplay = nullptr;
        void* waylandSurface = nullptr;
        // Xlib
        void* xDisplay = nullptr;
        uint64_t xWindow = 0;
        // MetalLayer
        void* metalLayer = nullptr;
    };

    class ALIMER_API RHISurface : public RHIObject
    {
    public:
        /// Return the physical adapter surface capabilities.
        virtual RHIStatus GetCapabilities(RHIAdapter* adapter, RHISurfaceCapabilities* capabilities) = 0;

        void Configure(RHIDevice* device, const RHISurfaceConfig& config);
        void Unconfigure();
        void Resize(uint32_t newWidth, uint32_t newHeight);

        RHIColorSpace GetColorSpace() const { return colorSpace; }

    protected:
        RHISurface(RHISurfaceSource* source_);
        ~RHISurface() override;

        virtual void ConfigureCore(RHIDevice* device) = 0;
        virtual void UnconfigureCore() = 0;
        virtual void ResizeCore() = 0;

        RHISurfaceSourceRef source;
        RHISurfaceCapabilities capabilities{};
        PixelFormat format = PixelFormat::Undefined;
        uint32_t width = 0;
        uint32_t height = 0;
        RHICompositeAlphaMode alphaMode = RHICompositeAlphaMode::Auto;
        RHIPresentMode presentMode = RHIPresentMode::Fifo;
        RHIColorSpace colorSpace = RHIColorSpace::SRGB;

    private:
        bool configured = false;
    };

    struct GPUAllocation
    {
        void* data = nullptr;
        RHIBufferRef buffer;
        uint64_t offset = 0;

        /// Returns true if the allocation was successful
        inline bool IsValid() const { return data != nullptr && buffer != nullptr; }
    };

    struct RHIDescriptorBindingTable
    {
        RHIBufferRef CBV[kDynamicConstantBufferCount];
        uint64_t CBV_offset[kDynamicConstantBufferCount] = {};
    };

    class ALIMER_API RHICommandEncoder
    {
    public:
        virtual ~RHICommandEncoder() = default;

        virtual void PushDebugGroup(std::string_view groupLabel) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(std::string_view markerLabel) = 0;

        // Do we expose barriers?
        void SetConstantBuffer(uint32_t slot, RHIBuffer* buffer, uint64_t offset = 0);
        void SetPushConstants(const void* data, uint32_t size, uint32_t offset = 0);

        template<typename T>
        void SetPushConstants(const T& data, uint32_t offset = 0)
        {
            SetPushConstants(&data, sizeof(T), offset);
        }

        virtual void End() = 0;

        /// Returns the command buffer that is currently encoding commands.
        virtual RHICommandBuffer* GetCommandBuffer() const = 0;

    protected:
        virtual void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) = 0;

        RHIDescriptorBindingTable table{};
    };

    class ALIMER_API RHIComputePassEncoder : public RHICommandEncoder
    {
    public:
        virtual ~RHIComputePassEncoder() = default;

        GPUAllocation AllocateGPU(uint64_t size);
        virtual void UploadBufferData(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size = 0);
        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) = 0;
        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) = 0;

        virtual void SetPipeline(RHIComputePipeline* pipeline) = 0;

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64u);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8u, uint32_t groupSizeY = 8u);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
        void DispatchIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0);

    protected:
        virtual void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void DispatchIndirectCore(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
    };

    class ALIMER_API RHIRenderPassEncoder : public RHICommandEncoder
    {
    public:
        virtual ~RHIRenderPassEncoder() = default;

        virtual void SetPipeline(RHIRenderPipeline* pipeline) = 0;

        virtual void SetViewport(const RHIViewport& viewport) = 0;
        virtual void SetViewports(const RHIViewport* viewports, uint32_t count) = 0;
        virtual void SetScissorRect(const RHIScissorRect& scissorRect) = 0;
        virtual void SetScissorRects(const RHIScissorRect* scissorRects, uint32_t count) = 0;
        virtual void SetStencilReference(uint32_t reference) = 0;
        virtual void SetBlendColor(const Color& color) = 0;
        virtual void SetShadingRate(RHIShadingRate rate) = 0;
        virtual void SetDepthBounds(float minBounds, float maxBounds) = 0;

        /// Bind a vertex buffer.
        virtual void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset = 0) = 0;

        /// Bind a vertex buffers.
        virtual void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) = 0;

        /// Bind an index buffer.
        virtual void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, RHIIndexFormat format) = 0;

        /// Draw non-indexed geometry.
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
        /// Draw indexed geometry.
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0) = 0;
        /// Draw primitives with indirect parameters
        virtual void DrawIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0) = 0;
        /// Draw primitives with indirect parameters and indexed vertices
        virtual void DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0) = 0;

        virtual void DrawMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;
        virtual void DrawMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
        virtual void DrawMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount) = 0;

    protected:
    };

    class ALIMER_API RHICommandBuffer
    {
    public:
        /// Destructor.
        virtual ~RHICommandBuffer() = default;

        /// Returns the GPU device that this command buffer belongs to.
        virtual RHIDevice* GetDevice() const = 0;

        virtual void PushDebugGroup(std::string_view name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(std::string_view name) = 0;

        virtual void BeginQuery(const RHIQueryHeap* heap, uint32_t index) = 0;
        virtual void EndQuery(const RHIQueryHeap* heap, uint32_t index) = 0;
        virtual void ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) = 0;
        virtual void ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count) = 0;

        RHIComputePassEncoder* BeginComputePass(const RHIComputePassDesc& desc);
        RHIRenderPassEncoder* BeginRenderPass(const RHIRenderPassDesc& desc);

        /// Acquires the next available texture for rendering or processing operations and queue's for presentation.
        virtual RHITexture* AcquireSurfaceTexture(RHISurface* surface) = 0;

        virtual RHINativeHandle GetNativeHandle(RHINativeHandleType type) { (void)type; return nullptr; }

    protected:
        void Reset(uint32_t frameIndex);
        void SetTextureLayout(const RHITexture* texture, uint32_t subresource, RHITextureLayout newLayout) const;
        void SetTextureLayout(const RHITexture* texture, RHITextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const;

        virtual RHIComputePassEncoder* BeginComputePassCore(const RHIComputePassDesc& desc) = 0;
        virtual RHIRenderPassEncoder* BeginRenderPassCore(const RHIRenderPassDesc& desc) = 0;

        uint32_t _frameIndex{ 0 };
        bool _encoderActive{ false };
    };

    struct RHILinearAllocator
    {
        RHIBufferRef buffer;
        uint64_t offset = 0ull;

        void Reset()
        {
            offset = 0ull;
        }
    };

    class ALIMER_API RHIDevice : public RHIObject
    {
    public:
        /// Returns the API kind that the RHI backend is running on top of.
        virtual RHIBackend GetBackend() const = 0;

        /// Wait for GPU to finish pending operations.
        virtual bool WaitIdle() = 0;

        /// Commit the current frame and advance to next frame
        virtual uint64_t CommitFrame() = 0;

        RHIBufferRef CreateBuffer(const RHIBufferDesc& desc, const void* initialData = nullptr);
        RHITextureRef CreateTexture(const RHITextureDesc& desc, const RHITextureData* initialData = nullptr);
        RHITextureRef CreateTextureFromNativeHandle(RHINativeHandle handle, const RHITextureDesc& desc);
        RHISamplerRef CreateSampler(const RHISamplerDesc& desc);

        RHIShaderModuleRef CreateShaderModule(const RHIShaderModuleDesc& descriptor);

        RHIComputePipelineRef CreateComputePipeline(const RHIComputePipelineDesc& desc);
        RHIRenderPipelineRef CreateRenderPipeline(const RHIRenderPipelineDesc& desc);
        RHIQueryHeapRef CreateQueryHeap(const RHIQueryHeapDesc& desc);

        virtual void WriteShadingRateValue(RHIShadingRate rate, void* dest) const = 0;

        virtual RHICommandBuffer* BeginCommandBuffer(RHIQueueType queueType, std::string_view label = "") = 0;

        virtual RHIAdapter* GetAdapter() const = 0;

        constexpr bool IsDeviceLost() const noexcept { return _deviceLost; }
        constexpr uint64_t GetFrameCount() const noexcept { return _frameCount; }
        constexpr uint32_t GetFrameIndex() const noexcept { return _frameIndex; }

        /// Return the device limits.
        [[nodiscard]] const RHIDeviceLimits& GetLimits() const { return _limits; }
        [[nodiscard]] virtual bool QueryFeatureSupport(RHIFeature feature) = 0;
        [[nodiscard]] virtual RHIPixelFormatSupport QueryPixelFormatSupport(PixelFormat format) = 0;
        //[[nodiscard]] virtual bool QueryVertexFormatSupport(VertexFormat format) = 0;

        [[nodiscard]] constexpr uint64_t GetTimestampFrequency() const { return _timestampFrequency; }
        [[nodiscard]] RHILinearAllocator& GetFrameAllocator() { return _frameAllocators[_frameIndex]; }

    protected:
        virtual void InitResources();
        virtual bool ValidateTextureDesc(const RHITextureDesc& desc);

        virtual RHIBufferRef CreateBufferCore(const RHIBufferDesc& desc, RHINativeHandle nativeHandle, const void* initialData) = 0;
        virtual RHITextureRef CreateTextureCore(const RHITextureDesc& desc, RHINativeHandle nativeHandle, const RHITextureData* initialData) = 0;
        virtual RHISamplerRef CreateSamplerCore(const RHISamplerDesc& desc) = 0;
        virtual RHIShaderModuleRef CreateShaderModuleCore(const RHIShaderModuleDesc& desc) = 0;
        virtual RHIComputePipelineRef CreateComputePipelineCore(const RHIComputePipelineDesc& desc) = 0;
        virtual RHIRenderPipelineRef CreateRenderPipelineCore(const RHIRenderPipelineDesc& desc) = 0;
        virtual RHIQueryHeapRef CreateQueryHeapCore(const RHIQueryHeapDesc& desc) = 0;

        RHIDeviceLimits _limits{};
        uint64_t _timestampFrequency = 0;

        bool _deviceLost{ false };
        uint64_t _frameCount{};
        uint32_t _frameIndex{};

        Vector<RHISamplerRef> staticSamplers;
        RHILinearAllocator _frameAllocators[kNumFramesInFlight];
    };

    class ALIMER_API RHIAdapter
    {
    public:
        virtual RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) = 0;
        virtual RHINativeHandle GetNativeHandle(RHINativeHandleType type) { (void)type; return nullptr; }

        /// Return the physical adapter properties.
        [[nodiscard]] const RHIAdapterProperties& GetProperties() const { return properties; }

    protected:
        RHIAdapterProperties properties{};
    };

    class ALIMER_API RHIFactory : public RHIObject
    {
    public:
        /// Returns the API kind that the RHI backend is running on top of.
        virtual RHIBackend GetBackend() const = 0;

        uint32_t GetAdapterCount() const;
        RHIAdapter* GetAdapter(uint32_t index) const;
        RHIAdapter* GetBestAdapter() const;

        virtual RHISurfaceRef CreateSurface(RHISurfaceSource* source) = 0;

    protected:
        Vector<RHIAdapter*> _adapters;
    };

    ALIMER_API bool IsBackendSupported(RHIBackend backend);
    ALIMER_API RHIBackend GetPlatformPreferredBackend();
    ALIMER_API RHIFactoryRef RHICreateFactory(const RHIFactoryDesc& desc);

    ALIMER_API RHIBufferRef RHICreateBuffer(RHIDevice* device, uint64_t size, RHIBufferUsage usage = RHIBufferUsage::ShaderRead, RHIMemoryType memoryType = RHIMemoryType::Private, const void* initialData = nullptr, const char* label = nullptr);
    ALIMER_API RHIBufferRef RHICreateBuffer(RHIDevice* device, const RHIBufferDesc& desc, const void* initialData = nullptr);

    template<typename T>
    RHIBufferRef RHICreateBuffer(RHIDevice* device, _In_reads_(count) const T* data, uint32_t count, RHIBufferUsage usage = RHIBufferUsage::ShaderRead) noexcept
    {
        return RHICreateBuffer(device, sizeof(T) * count, usage, RHIMemoryType::Private, data);
    }

    template<typename T>
    RHIBufferRef RHICreateBuffer(RHIDevice* device, const T& data, RHIBufferUsage usage = RHIBufferUsage::ShaderRead) noexcept
    {
        return RHICreateBuffer(device, data.size() * sizeof(typename T::value_type), usage, RHIMemoryType::Private, data.data());
    }

    ALIMER_API const std::string ToString(RHIBackend type);
    ALIMER_API const std::string ToString(RHIAdapterType type);

    ALIMER_API RHIAdapterVendor VendorIDToAdapterVendor(uint32_t vendorID);
    ALIMER_API uint32_t AdapterVendorToVendorID(RHIAdapterVendor vendor);

    ALIMER_API uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth = 1u, uint32_t minDimension = 1u, uint32_t requiredAlignment = 1u);

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t mipLevelCount) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount;
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice, uint32_t mipLevelCount, uint32_t arrayLayers) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount + planeSlice * mipLevelCount * arrayLayers;
    }

    ALIMER_API bool BlendEnabled(const RHIRenderTargetBlendState* state);
    ALIMER_API bool StencilTestEnabled(const RHIDepthStencilState* depthStencil);

    enum class RHIVertexFormatKind : uint8_t
    {
        /// Unsigned normalized formats.
        Unorm,
        /// Signed normalized formats
        Snorm,
        /// Unsigned integer formats
        Uint,
        /// Signed integer formats
        Sint,
        /// Floating-point formats.
        Float,
    };

    struct RHIVertexAttributeFormatInfo final
    {
        RHIVertexAttributeFormat format;
        uint32_t byteSize;
        uint32_t componentCount;
        RHIVertexFormatKind kind;
    };
    ALIMER_API const RHIVertexAttributeFormatInfo& GetVertexAttributeFormatInfo(RHIVertexAttributeFormat format);

    /* Shader loading */
    struct RHIShaderMacro
    {
        std::string name;
        std::string definition;

        RHIShaderMacro(const std::string& _name, const std::string& _definition)
            : name(_name)
            , definition(_definition)
        {}
    };

    ALIMER_API RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName);
    ALIMER_API RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName, const Vector<RHIShaderMacro>& defines);
    ALIMER_API RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName, const RHIShaderMacro* pDefines, size_t definesCount);
}
