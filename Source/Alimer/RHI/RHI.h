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
    static constexpr uint32_t kMaxPushConstantsSize = 128;
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

    /// Number of constant buffer slots to be bound in non bindless space.
    static constexpr uint32_t kContantBufferCount = 1;

    /// Bindless descriptor limits -> the device can allocate less than this, depending on capabilities
    static constexpr BindlessIndex kInvalidBindlessIndex = -1;
    static constexpr uint32_t kBindlessResourceCapacity = 500000;
    static constexpr uint32_t kBindlessSamplerCapacity = 256;

    // These shifts are made so that Vulkan resource bindings slots don't interfere with each other across shader stages:
    // These are also used during shader compilation
    namespace VulkanRegisterShift
    {
        static constexpr uint32_t kContantBuffer = 0;   // b
        static constexpr uint32_t kSRV = 1000;          // t
        static constexpr uint32_t kUAV = 2000;          // u
        static constexpr uint32_t kSampler = 3000;      // s
    }

    /* Enums */
    enum class RHIBackend : uint32_t
    {
        Null,
        Vulkan,
        D3D12,
        Metal,

        Count,
    };

    enum class AdapterVendor : uint8_t
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

    enum class MemoryType : uint8_t
    {
        /// CPU no access, GPU read/write
        Private,
        /// CPU write, GPU read
        Upload,
        /// CPU read, GPU write
        Readback
    };

    enum class BufferStates : uint32_t
    {
        Undefined = 0,
        CopyDest = ALIMER_BIT(0),
        CopySource = ALIMER_BIT(1),
        ShaderResource = ALIMER_BIT(2),
        UnorderedAccess = ALIMER_BIT(3),
        VertexBuffer = ALIMER_BIT(4),
        IndexBuffer = ALIMER_BIT(5),
        ConstantBuffer = ALIMER_BIT(6),
        Predication = ALIMER_BIT(7),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(BufferStates);

    enum class TextureLayout : uint32_t
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
        /// Supports predication access for conditional rendering.
        Predication = 1 << 7,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(RHIBufferUsage);

    /// Defines dimension of Texture
    enum class TextureDimension : uint32_t
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

    enum class TextureUsage : uint32_t
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
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(TextureUsage);

    enum class TextureSampleCount : uint32_t
    {
        Count1 = (1 << 0),
        Count2 = (1 << 1),
        Count4 = (1 << 2),
        Count8 = (1 << 3),
        Count16 = (1 << 4),
        Count32 = (1 << 5),
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(TextureSampleCount);

    enum class TextureAspect : uint32_t
    {
        All = 0,
        DepthOnly = 1,
        StencilOnly = 2,
    };

    enum class TextureSwizzle : uint8_t
    {
        Zero = 0,
        One = 1,
        Red = 2,
        Green = 3,
        Blue = 4,
        Alpha = 5,
    };

    enum class SamplerMinMagFilter : uint32_t
    {
        Point,
        Linear,
    };

    enum class SamplerMipFilter : uint32_t
    {
        Point,
        Linear,
    };

    enum class SamplerAddressMode : uint32_t
    {
        Clamp,
        Wrap,
        Mirror,
        MirrorOnce,
        Border,
    };

    enum class SamplerReductionType : uint8_t
    {
        Standard,
        Comparison,
        Minimum,
        Maximum
    };

    enum class SamplerBorderColor : uint32_t
    {
        FloatTransparentBlack,
        FloatOpaqueBlack,
        FloatOpaqueWhite,
        UintTransparentBlack,
        UintOpaqueBlack,
        UintOpaqueWhite,
    };

    enum class QueryType : uint32_t
    {
        /// Used for occlusion query heap or occlusion queries
        Occlusion,
        /// Can be used in the same heap as occlusion
        BinaryOcclusion,
        /// Create a heap to contain timestamp queries
        Timestamp,
        /// Create a heap to contain a structure of `PipelineStatistics`
        PipelineStatistics,
    };

    enum class CompareFunction : uint32_t
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
    enum class StencilOperation : uint32_t
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

    enum class BlendFactor : uint32_t
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

    enum class BlendOperation : uint32_t
    {
        Add = 0,
        Subtract = 1,
        ReverseSubtract = 2,
        Min = 3,
        Max = 4,
    };

    enum class ColorWriteMask : uint32_t
    {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = 15
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(ColorWriteMask);

    enum class FillMode : uint8_t
    {
        Solid,
        Wireframe,
    };

    enum class CullMode : uint32_t
    {
        None = 0,
        Front = 1,
        Back = 2,
    };

    enum class FrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1,
    };

    enum class DepthClipMode : uint32_t
    {
        Clip,
        Clamp
    };

    enum class VertexAttributeSemantic : uint32_t
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

    enum class VertexAttributeFormat : uint32_t
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

    enum class VertexStepMode : uint32_t
    {
        /// Vertex data is advanced every vertex.
        Vertex = 0,
        /// Vertex data is advanced every instance.
        Instance = 1
    };

    /// Index buffer element format.
    enum class IndexFormat : uint32_t
    {
        /// Undefined index format, used to disable index buffer stripping <see cref="RenderPipelineDescriptor.StripIndexFormat"/>.
        Undefined,
        /// 16-bit unsigned integer indices.
        Uint16,
        /// 32-bit unsigned integer indices.
        Uint32,
    };

    enum class PrimitiveTopology : uint32_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    };

    enum class LoadAction : uint32_t
    {
        Load,
        Clear,
        DontCare,
    };

    enum class StoreAction : uint32_t
    {
        Store,
        DontCare
    };

    enum class ShaderStages : uint32_t
    {
        None = 0,
        Vertex = ALIMER_BIT(0),
        Fragment = ALIMER_BIT(1),
        Compute = ALIMER_BIT(2),
        Mesh = ALIMER_BIT(3),
        Amplification = ALIMER_BIT(4),

        All = 0x3FFF,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(ShaderStages);

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

    enum class PresentMode : uint32_t
    {
        Immediate,
        Mailbox,
        Fifo,
    };

    enum class ShadingRate : uint8_t
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

    enum class PredicationOperation : uint8_t
    {
        EqualZero,
        NotEqualZero,
    };

    enum class ShaderModel : uint32_t
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

    enum class ConservativeRasterizationTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,
        Tier3 = 3,
    };

    enum class VariableRateShadingTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,

    };

    enum class RayTracingTier
    {
        NotSupported = 0,
        Tier1 = 1,
        Tier2 = 2,
    };

    enum class MeshShaderTier
    {
        NotSupported = 0,
        Tier1 = 1,
    };

    enum class Feature : uint16_t
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

        Bindless,
        DepthResolveMinMax,
        StencilResolveMinMax,
        ShaderOutputViewportIndex,
        CacheCoherentUMA,
        Predication,
    };

    enum class PixelFormatSupport : uint32_t
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
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(PixelFormatSupport);

    /* Forward declarations */
    class RHIBuffer;
    class RHITexture;
    class RHITextureView;
    class Sampler;
    class ShaderModule;
    class CommandEncoder;
    class ComputePassEncoder;
    class RenderPassEncoder;
    class CommandBuffer;
    class CommandQueue;
    class ComputePipeline;
    class RenderPipeline;
    class RHISurface;
    class QueryHeap;
    class RHIDevice;
    class RHIAdapter;
    class RHIFactory;

    using RHIBufferRef = SharedPtr<RHIBuffer>;
    using RHITextureRef = SharedPtr<RHITexture>;
    using RHITextureViewRef = SharedPtr<RHITextureView>;
    using SamplerRef = SharedPtr<Sampler>;
    using ShaderModuleRef = SharedPtr<ShaderModule>;
    using ComputePipelineRef = SharedPtr<ComputePipeline>;
    using RenderPipelineRef = SharedPtr<RenderPipeline>;
    using QueryHeapRef = SharedPtr<QueryHeap>;
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

    struct ScissorRect
    {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    struct Viewport
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
        MemoryType memoryType = MemoryType::Private;
        //uint32_t stride = 0; // Needed for StructuredBuffer
    };

    struct TextureSwizzleChannels
    {
        TextureSwizzle red = TextureSwizzle::Red;
        TextureSwizzle green = TextureSwizzle::Green;
        TextureSwizzle blue = TextureSwizzle::Blue;
        TextureSwizzle alpha = TextureSwizzle::Alpha;
    };

    struct TextureDescriptor
    {
        TextureDimension dimension = TextureDimension::Texture2D;
        PixelFormat format = PixelFormat::RGBA8Unorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        uint32_t mipLevelCount = 1;
        TextureSampleCount sampleCount = TextureSampleCount::Count1;
        TextureUsage usage = TextureUsage::ShaderRead;
        const char* label = nullptr;
        //MemoryType memoryType = MemoryType::Private;

        static constexpr TextureDescriptor Texture1D(
            PixelFormat format,
            uint32_t width,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            TextureDescriptor desc;
            desc.dimension = TextureDimension::Texture1D;
            desc.format = format;
            desc.width = width;
            desc.height = 1u;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = TextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }

        static constexpr TextureDescriptor Texture2D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            TextureSampleCount sampleCount = TextureSampleCount::Count1,
            const char* label = nullptr)
        {
            TextureDescriptor desc;
            desc.dimension = TextureDimension::Texture2D;
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

        static constexpr TextureDescriptor Texture3D(
            PixelFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            TextureDescriptor desc;
            desc.dimension = TextureDimension::Texture3D;
            desc.format = format;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = depth;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = TextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }

        static constexpr TextureDescriptor TextureCube(
            PixelFormat format,
            uint32_t size,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            const char* label = nullptr)
        {
            TextureDescriptor desc;
            desc.dimension = TextureDimension::TextureCube;
            desc.format = format;
            desc.width = size;
            desc.height = size;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.sampleCount = TextureSampleCount::Count1;
            desc.usage = usage;
            desc.label = label;
            return desc;
        }
    };

    struct TextureData
    {
        const void* pData = nullptr;
        uint32_t rowPitch = 0;
        uint32_t slicePitch = 0;
    };

    struct RHITextureViewDesc
    {
        PixelFormat format = PixelFormat::Undefined;
        TextureAspect aspect = TextureAspect::All;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = kMipLevelCountUndefined;
        uint32_t baseArrayLayer = 0;                            // For Texture3D, this is WSlice.
        uint32_t arrayLayerCount = kArrayLayerCountUndefined;   // For cube maps, this is a multiple of 6.
        TextureSwizzleChannels swizzle{};

        /// The name of the texture for debugging purposes.
        const char* label = nullptr;
    };

    struct SamplerDesc
    {
        const char* label = nullptr;
        SamplerReductionType reductionType = SamplerReductionType::Standard;
        SamplerMinMagFilter minFilter = SamplerMinMagFilter::Point;
        SamplerMinMagFilter magFilter = SamplerMinMagFilter::Point;
        SamplerMipFilter mipFilter = SamplerMipFilter::Point;
        SamplerAddressMode addressModeU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeW = SamplerAddressMode::Clamp;
        SamplerBorderColor borderColor = SamplerBorderColor::FloatTransparentBlack;
        float lodMinClamp = 0.0f;
        float lodMaxClamp = FLT_MAX;
        CompareFunction compareFunction = CompareFunction::Never;
        uint16_t maxAnisotropy = 1u;
    };

    struct ShaderModuleDesc
    {
        const char* label = nullptr;
        size_t byteCodeSize;
        const void* byteCode;
    };

    struct RHIComputePipelineDesc
    {
        const char* label = nullptr;
        ShaderModuleRef shader;
    };

    struct VertexAttribute
    {
        VertexAttributeSemantic semantic = VertexAttributeSemantic::Undefined;
        VertexAttributeFormat format = VertexAttributeFormat::Undefined;
        uint32_t offset = 0;
        uint32_t semanticIndex = 0;
    };

    struct VertexBufferLayout
    {
        uint32_t stride = 0;
        VertexStepMode stepMode = VertexStepMode::Vertex;
        uint32_t attributeCount = 0;
        const VertexAttribute* attributes = nullptr;
    };

    struct RenderTargetBlendState
    {
        BlendFactor srcColorBlendFactor = BlendFactor::One;
        BlendFactor destColorBlendFactor = BlendFactor::Zero;
        BlendOperation colorBlendOp = BlendOperation::Add;
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor destAlphaBlendFactor = BlendFactor::Zero;
        BlendOperation alphaBlendOp = BlendOperation::Add;
        ColorWriteMask colorWriteMask = ColorWriteMask::All;
    };

    struct BlendState
    {
        bool alphaToCoverageEnable = false;
        bool independentBlendEnable = false;

        RenderTargetBlendState renderTargets[kMaxColorAttachments] = {};
    };

    struct RasterizerState
    {
        FillMode fillMode = FillMode::Solid;
        CullMode cullMode = CullMode::Back;
        FrontFace frontFace = FrontFace::CounterClockwise;
        float depthBias = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeScale = 0.0f;
        DepthClipMode depthClipMode = DepthClipMode::Clip;
        bool conservativeRasterEnable = false;
    };

    struct StencilFaceState
    {
        StencilOperation failOp = StencilOperation::Keep;
        StencilOperation depthFailOp = StencilOperation::Keep;
        StencilOperation passOp = StencilOperation::Keep;
        CompareFunction compareFunc = CompareFunction::Always;
    };

    struct DepthStencilState
    {
        bool depthWriteEnabled = true;
        CompareFunction depthCompare = CompareFunction::Less;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
        StencilFaceState frontFace{};
        StencilFaceState backFace{};
        bool depthBoundsTestEnable = false;
    };

    struct RHIRenderPipelineDesc
    {
        const char* label = nullptr;

        ShaderModuleRef vertexShader = nullptr;
        ShaderModuleRef fragmentShader = nullptr;
        ShaderModuleRef meshShader = nullptr;
        ShaderModuleRef amplificationShader = nullptr;

        uint32_t vertexBufferLayoutCount = 0;
        const VertexBufferLayout* vertexBufferLayouts = nullptr;

        BlendState blendState{};
        RasterizerState rasterizerState{};
        DepthStencilState depthStencilState{};

        PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;

        IndexFormat stripIndexFormat = IndexFormat::Undefined;

        PixelFormat colorAttachmentFormats[kMaxColorAttachments] = {};
        PixelFormat depthStencilFormat = PixelFormat::Undefined;
        TextureSampleCount sampleCount = TextureSampleCount::Count1;
    };

    struct QueryHeapDescriptor
    {
        const char* label = nullptr;
        /// ie: Timestamp, Occlusion, PipelineStatistics
        QueryType type = QueryType::Timestamp;
        /// Total size of the heap in number of queries.
        uint32_t count = 1u;
    };

    struct RenderPassColorAttachment
    {
        RHITextureView* view = nullptr;
        RHITextureView* resolveView = nullptr;

        LoadAction          loadAction = LoadAction::Load;
        StoreAction         storeAction = StoreAction::Store;
        ClearColorValue     clearColor{};
    };

    struct RenderPassDepthStencilAttachment
    {
        RHITextureView* view = nullptr;

        LoadAction      depthLoadAction = LoadAction::Clear;
        StoreAction     depthStoreAction = StoreAction::DontCare;
        float           depthClearValue = 1.0f;
        bool            depthReadOnly = false;

        LoadAction      stencilLoadAction = LoadAction::Clear;
        StoreAction     stencilStoreAction = StoreAction::DontCare;
        uint8_t         stencilClearValue = 0;
        bool            stencilReadOnly = false;
    };

    struct RHIComputePassDesc
    {
        const char* label = nullptr;
    };

    struct RenderPassDesc
    {
        uint32_t                                colorAttachmentCount = 0;
        const RenderPassColorAttachment* colorAttachments = nullptr;
        const RenderPassDepthStencilAttachment* depthStencilAttachment = nullptr;
        const char* label = nullptr;
    };

    struct RHISurfaceConfig
    {
        const char* label = nullptr;
        PixelFormat format = PixelFormat::BGRA8UnormSrgb;
        uint32_t width = 0;
        uint32_t height = 0;
        RHICompositeAlphaMode alphaMode = RHICompositeAlphaMode::Auto;
        PresentMode presentMode = PresentMode::Fifo;
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

        /// The maximum bind groups.
        uint32_t maxBindGroups = 0;

        /// The maximum range of constant buffer.
        uint64_t maxConstantBufferBindingSize = 0;

        /// The maximum size of storage buffer.
        uint64_t maxStorageBufferBindingSize = 0;

        /// The alignment required for constant buffers.
        uint64_t minConstantBufferOffsetAlignment = 0;

        /// The alignment required for storage buffers.
        uint64_t minStorageBufferOffsetAlignment = 0;

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
        ShaderModel highestShaderModel = ShaderModel::Model_6_0;

        /* ConservativeRasterization tier */
        ConservativeRasterizationTier conservativeRasterizationTier = ConservativeRasterizationTier::NotSupported;

        // VariableRateShading
        VariableRateShadingTier variableShadingRateTier = VariableRateShadingTier::NotSupported;
        uint32_t variableShadingRateImageTileSize = 0;
        bool isAdditionalVariableShadingRatesSupported = false;

        // Ray tracing
        RayTracingTier rayTracingTier = RayTracingTier::NotSupported;
        uint32_t rayTracingShaderGroupIdentifierSize = 0;
        uint32_t rayTracingShaderTableAlignment = 0;
        uint32_t rayTracingShaderTableMaxStride = 0;
        uint32_t rayTracingShaderRecursionMaxDepth = 0;
        uint32_t rayTracingMaxGeometryCount = 0;
        uint32_t rayTracingScratchAlignment = 0;

        /* Mesh shader */
        MeshShaderTier meshShaderTier = MeshShaderTier::NotSupported;
    };

    struct AdapterProperties
    {
        std::string         deviceName;
        uint32_t            vendorID = 0;
        uint32_t            deviceID = 0;
        RHIAdapterType      type = RHIAdapterType::Other;
        std::string         driverDescription;
        uint8_t             uuid[kUUIDSize];
        uint64_t            luid[kLUIDSize];
        uint64_t            videoMemorySize = 0;
        uint64_t            systemMemorySize = 0;
    };

    /* Types */
    enum class NativeHandleType : uint32_t
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

    struct NativeHandle
    {
        NativeHandleType type = NativeHandleType::Unknown;

        union {
            uint64_t integer;
            void* pointer;
        };

        NativeHandle(uint64_t i) : integer(i) {}  // NOLINT(cppcoreguidelines-pro-type-member-init)
        NativeHandle(void* p) : pointer(p) {}     // NOLINT(cppcoreguidelines-pro-type-member-init)

        template<typename T> operator T* () const { return static_cast<T*>(pointer); }
        operator bool() const { return type != NativeHandleType::Unknown; }
    };

    class ALIMER_API RHIObject : public RefCounted
    {
    public:
        virtual void SetLabel(const char* label) { ALIMER_UNUSED(label); }
        virtual NativeHandle GetNativeHandle(NativeHandleType type) { (void)type; return nullptr; }

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
        [[nodiscard]] virtual BufferStates GetCurrentState() const { return currentState; }

    protected:
        RHIBuffer(const RHIBufferDesc& desc_)
            : desc(desc)
        {
        }

        RHIBufferDesc desc;
        mutable BufferStates currentState = BufferStates::Undefined;
    };

    class ALIMER_API RHITexture : public RHIObject
    {
    protected:
        RHITexture(const TextureDescriptor& desc)
            : dimension(desc.dimension)
            , format(desc.format)
            , width(desc.width)
            , height(desc.height)
            , depthOrArrayLayers(desc.depthOrArrayLayers)
            , mipLevelCount(desc.mipLevelCount)
            , sampleCount(desc.sampleCount)
            , usage(desc.usage)
        {}

    public:
        [[nodiscard]] constexpr TextureDimension GetDimension() const { return dimension; }
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
            if (dimension != TextureDimension::Texture3D)
            {
                return 1;
            }

            return (mipLevel == 0) || (mipLevel < mipLevelCount) ? Alimer::Max(1u, depthOrArrayLayers >> mipLevel) : 0;
        }

        [[nodiscard]] constexpr uint32_t GetArrayLayers() const
        {
            if (dimension == TextureDimension::Texture3D)
            {
                return 1;
            }

            return depthOrArrayLayers;
        }

        [[nodiscard]] constexpr TextureUsage GetUsage() const { return usage; }
        [[nodiscard]] constexpr uint32_t GetMipLevelCount() const { return mipLevelCount; }
        [[nodiscard]] constexpr TextureSampleCount GetSampleCount() const { return sampleCount; }

        [[nodiscard]] RHITextureView* GetDefaultView() const;
        [[nodiscard]] RHITextureView* GetView(const RHITextureViewDesc* desc = nullptr) const;

    protected:
        virtual RHITextureViewRef CreateView(const RHITextureViewDesc& desc)  const = 0;

    protected:
        TextureDimension dimension;
        PixelFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t depthOrArrayLayers;
        uint32_t mipLevelCount;
        TextureSampleCount sampleCount;
        TextureUsage usage;

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
        [[nodiscard]] constexpr TextureAspect GetAspect() const { return aspect; }
        [[nodiscard]] constexpr uint32_t GetBaseMipLevel() const { return baseMipLevel; }
        [[nodiscard]] constexpr uint32_t GetMipLevelCount() const { return mipLevelCount; }
        [[nodiscard]] constexpr uint32_t GetBaseArrayLayer() const { return baseArrayLayer; }
        [[nodiscard]] constexpr uint32_t GetArrayLayerCount() const { return arrayLayerCount; }
        [[nodiscard]] const TextureSwizzleChannels& GetSwizzle() const { return swizzle; }

        [[nodiscard]] constexpr uint32_t GetWidth() const { return texture->GetWidth(baseMipLevel); }
        [[nodiscard]] constexpr uint32_t GetHeight() const { return texture->GetHeight(baseMipLevel); }

    protected:
        const RHITexture* texture;
        PixelFormat format;
        TextureAspect aspect;
        uint32_t baseMipLevel;
        uint32_t mipLevelCount;
        uint32_t baseArrayLayer;    // For Texture3D, this is WSlice.
        uint32_t arrayLayerCount;   // For cube maps, this is a multiple of 6.
        TextureSwizzleChannels swizzle;
    };

    class ALIMER_API Sampler : public RHIObject
    {
    public:
        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;
    };

    class ALIMER_API ShaderModule : public RHIObject
    {
    public:
    };

    class ALIMER_API RHIQueue : public RHIObject
    {
    public:
        virtual RHIQueueType GetType() const = 0;
    };

    class ALIMER_API ComputePipeline : public RHIObject
    {};

    class ALIMER_API RenderPipeline : public RHIObject
    {};

    class ALIMER_API QueryHeap : public RHIObject
    {
    public:
        virtual uint32_t GetCount() const = 0;
        virtual QueryType GetType() const = 0;
    };

    class ALIMER_API RHISurface : public RHIObject
    {
    public:
        virtual void Configure(RHIDevice* device, const RHISurfaceConfig& config) = 0;
        virtual void Unconfigure() = 0;
        virtual void Resize(uint32_t newWidth, uint32_t newHeight) = 0;
    };

    struct GPUAllocation
    {
        void* data = nullptr;
        RHIBufferRef buffer;
        uint64_t offset = 0;

        /// Returns true if the allocation was successful
        inline bool IsValid() const { return data != nullptr && buffer != nullptr; }
    };

    struct DescriptorBindingTable
    {
        RHIBufferRef CBV[kContantBufferCount];
        uint64_t CBV_offset[kContantBufferCount] = {};
    };

    class ALIMER_API CommandEncoder
    {
    public:
        virtual ~CommandEncoder() = default;

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
        virtual CommandBuffer* GetCommandBuffer() const = 0;

    protected:
        virtual void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) = 0;

        DescriptorBindingTable table{};
    };

    class ALIMER_API ComputePassEncoder : public CommandEncoder
    {
    public:
        virtual ~ComputePassEncoder() = default;

        GPUAllocation AllocateGPU(uint64_t size);
        virtual void UploadBufferData(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size = 0);
        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) = 0;
        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) = 0;

        virtual void SetPipeline(ComputePipeline* pipeline) = 0;

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64u);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8u, uint32_t groupSizeY = 8u);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
        void DispatchIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0);

    protected:
        virtual void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void DispatchIndirectCore(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
    };

    class ALIMER_API RenderPassEncoder : public CommandEncoder
    {
    public:
        virtual ~RenderPassEncoder() = default;

        virtual void SetPipeline(RenderPipeline* pipeline) = 0;

        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetViewports(const Viewport* viewports, uint32_t count) = 0;
        virtual void SetScissorRect(const ScissorRect& rect) = 0;
        virtual void SetScissorRects(const ScissorRect* scissorRects, uint32_t count) = 0;
        virtual void SetStencilReference(uint32_t referenceValue) = 0;
        virtual void SetBlendColor(const Color& color) = 0;
        virtual void SetShadingRate(ShadingRate rate) = 0;
        virtual void SetDepthBounds(float minBounds, float maxBounds) = 0;

        /// Bind a vertex buffer.
        virtual void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset = 0) = 0;

        /// Bind a vertex buffers.
        virtual void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) = 0;

        /// Bind an index buffer.
        virtual void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexFormat format) = 0;

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

    class ALIMER_API CommandBuffer
    {
    public:
        /// Destructor.
        virtual ~CommandBuffer() = default;

        /// Returns the GPU device that this command buffer belongs to.
        virtual RHIDevice* GetDevice() const = 0;

        virtual void PushDebugGroup(std::string_view name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(std::string_view name) = 0;

        virtual void BeginQuery(const QueryHeap* heap, uint32_t index) = 0;
        virtual void EndQuery(const QueryHeap* heap, uint32_t index) = 0;
        virtual void ResolveQuery(const QueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) = 0;
        virtual void ResetQuery(const QueryHeap* heap, uint32_t index, uint32_t count) = 0;

        ComputePassEncoder* BeginComputePass(const RHIComputePassDesc& desc);
        RenderPassEncoder* BeginRenderPass(const RenderPassDesc& desc);

        /// Acquires the next available texture for rendering or processing operations and queue's for presentation.
        virtual RHITexture* AcquireSurfaceTexture(RHISurface* surface) = 0;

        virtual void BeginPredication(const RHIBuffer* buffer, uint64_t offset, PredicationOperation operation) = 0;
        virtual void EndPredication() = 0;

        virtual NativeHandle GetNativeHandle(NativeHandleType type) { (void)type; return nullptr; }

    protected:
        void Reset(uint32_t frameIndex);

        virtual ComputePassEncoder* BeginComputePassCore(const RHIComputePassDesc& desc) = 0;
        virtual RenderPassEncoder* BeginRenderPassCore(const RenderPassDesc& desc) = 0;

        uint32_t _frameIndex{ 0 };
        bool _encoderActive{ false };
    };

    struct GpuLinearAllocator
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
        RHITextureRef CreateTexture(const TextureDescriptor& descriptor, const TextureData* initialData = nullptr);
        RHITextureRef CreateTextureFromNativeHandle(NativeHandle handle, const TextureDescriptor& descriptor);
        SamplerRef CreateSampler(const SamplerDesc& desc);

        ShaderModuleRef CreateShaderModule(const ShaderModuleDesc& descriptor);

        ComputePipelineRef CreateComputePipeline(const RHIComputePipelineDesc& desc);
        RenderPipelineRef CreateRenderPipeline(const RHIRenderPipelineDesc& desc);
        QueryHeapRef CreateQueryHeap(const QueryHeapDescriptor& descriptor);

        virtual void WriteShadingRateValue(ShadingRate rate, void* dest) const = 0;

        virtual CommandBuffer* BeginCommandBuffer(RHIQueueType queueType, std::string_view label = "") = 0;

        virtual RHIAdapter* GetAdapter() const = 0;

        constexpr bool IsDeviceLost() const noexcept { return _deviceLost; }
        constexpr uint64_t GetFrameCount() const noexcept { return _frameCount; }
        constexpr uint32_t GetFrameIndex() const noexcept { return _frameIndex; }

        /// Return the device limits.
        [[nodiscard]] const RHIDeviceLimits& GetLimits() const { return _limits; }
        [[nodiscard]] virtual bool QueryFeatureSupport(Feature feature) = 0;
        [[nodiscard]] virtual PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) = 0;
        //[[nodiscard]] virtual bool QueryVertexFormatSupport(VertexFormat format) = 0;

        [[nodiscard]] constexpr uint64_t GetTimestampFrequency() const { return _timestampFrequency; }
        [[nodiscard]] GpuLinearAllocator& GetFrameAllocator() { return _frameAllocators[_frameIndex]; }

    protected:
        virtual void InitResources();
        virtual bool ValidateTextureDesc(const TextureDescriptor& desc);
        virtual RHIBufferRef CreateBufferCore(const RHIBufferDesc& desc, NativeHandle nativeHandle, const void* initialData) = 0;
        virtual RHITextureRef CreateTextureCore(const TextureDescriptor& desc, const TextureData* initialData) = 0;
        virtual RHITextureRef CreateTextureFromNativeHandleCore(NativeHandle handle, const TextureDescriptor& desc) = 0;
        virtual SamplerRef CreateSamplerCore(const SamplerDesc& desc) = 0;
        virtual ShaderModuleRef CreateShaderModuleCore(const ShaderModuleDesc& desc) = 0;
        virtual ComputePipelineRef CreateComputePipelineCore(const RHIComputePipelineDesc& desc) = 0;
        virtual RenderPipelineRef CreateRenderPipelineCore(const RHIRenderPipelineDesc& desc) = 0;
        virtual QueryHeapRef CreateQueryHeapCore(const QueryHeapDescriptor& desc) = 0;

        RHIDeviceLimits _limits{};
        uint64_t _timestampFrequency = 0;

        bool _deviceLost{ false };
        uint64_t _frameCount{};
        uint32_t _frameIndex{};

        //uint32_t uploadBufferTextureRowAlignment = 1u;
        //uint32_t uploadBufferTextureSliceAlignment = 1u;

        Vector<SamplerRef> staticSamplers;
        GpuLinearAllocator _frameAllocators[kNumFramesInFlight];
    };

    class ALIMER_API RHIAdapter
    {
    public:
        virtual RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) = 0;

        /// Return the physical adapter properties.
        [[nodiscard]] const AdapterProperties& GetProperties() const { return properties; }

    protected:
        AdapterProperties properties{};
    };

    class ALIMER_API RHIFactory : public RHIObject
    {
    public:
        /// Returns the API kind that the RHI backend is running on top of.
        virtual RHIBackend GetBackend() const = 0;

        uint32_t GetAdapterCount() const;
        RHIAdapter* GetAdapter(uint32_t index) const;
        RHIAdapter* GetBestAdapter() const;

        virtual RHISurfaceRef CreateSurface(void* window, void* display) = 0;

    protected:
        Vector<RHIAdapter*> _adapters;
    };

    ALIMER_API bool IsBackendSupported(RHIBackend backend);
    ALIMER_API RHIBackend GetPlatformPreferredBackend();
    ALIMER_API RHIFactoryRef RHICreateFactory(const RHIFactoryDesc& desc);

    ALIMER_API RHIBufferRef RHICreateBuffer(RHIDevice* device, uint64_t size, RHIBufferUsage usage = RHIBufferUsage::ShaderRead, MemoryType memoryType = MemoryType::Private, const void* initialData = nullptr, const char* label = nullptr);
    ALIMER_API RHIBufferRef RHICreateBuffer(RHIDevice* device, const RHIBufferDesc& desc, const void* initialData = nullptr);

    template<typename T>
    RHIBufferRef RHICreateBuffer(RHIDevice* device, _In_reads_(count) const T* data, uint32_t count, RHIBufferUsage usage = RHIBufferUsage::ShaderRead) noexcept
    {
        return RHICreateBuffer(device, sizeof(T) * count, usage, MemoryType::Private, data);
    }

    template<typename T>
    RHIBufferRef RHICreateBuffer(RHIDevice* device, const T& data, RHIBufferUsage usage = RHIBufferUsage::ShaderRead) noexcept
    {
        return RHICreateBuffer(device, data.size() * sizeof(typename T::value_type), usage, MemoryType::Private, data.data());
    }

    ALIMER_API const std::string ToString(RHIBackend type);
    ALIMER_API const std::string ToString(RHIAdapterType type);

    ALIMER_API AdapterVendor VendorIdToAdapterVendor(uint32_t vendorId);
    ALIMER_API uint32_t AdapterVendorToVendorId(AdapterVendor vendor);

    ALIMER_API uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth = 1u, uint32_t minDimension = 1u, uint32_t requiredAlignment = 1u);

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t mipLevelCount) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount;
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice, uint32_t mipLevelCount, uint32_t arrayLayers) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount + planeSlice * mipLevelCount * arrayLayers;
    }

    ALIMER_API bool BlendEnabled(const RenderTargetBlendState* state);
    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);

    enum class VertexFormatKind : uint8_t
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

    struct VertexAttributeFormatInfo
    {
        VertexAttributeFormat format;
        uint32_t byteSize;
        uint32_t componentCount;
        VertexFormatKind kind;
    };
    ALIMER_API const VertexAttributeFormatInfo& GetVertexAttributeFormatInfo(VertexAttributeFormat format);

    /* Shader loading */
    struct ShaderMacro
    {
        std::string name;
        std::string definition;

        ShaderMacro(const std::string& _name, const std::string& _definition)
            : name(_name)
            , definition(_definition)
        {}
    };

    ALIMER_API ShaderModuleRef RHILoadShader(RHIDevice* device, ShaderStages stage, const char* fileName, const Vector<ShaderMacro>* pDefines = nullptr);
}
