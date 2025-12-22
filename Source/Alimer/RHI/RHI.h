// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"
#include "Alimer/Core/PixelFormat.h"
#include "Alimer/Core/UnorderedMap.h"
//#include "Alimer/Math/Color.h"
//#include "Alimer/Math/Viewport.h"
//#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/Shaders/ShaderDefinitions.h"
#include <string>
#include <string_view>
#include <float.h>

// Forward declare IUnknown
typedef struct HWND__* HWND;
struct IUnknown;
struct ANativeWindow;
struct wl_display;
struct wl_surface;

namespace Alimer
{
    using DeviceAddress = uint64_t;

    /* Constants */
    static constexpr uint32_t kNumFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxBindGroups = MAX_BIND_GROUPS;
    static constexpr uint32_t kMaxPushConstantsSize = MAX_PUSH_CONSTANTS_SIZE;
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
    static constexpr uint32_t kInvalidBindlessIndex = static_cast<uint32_t>(-1);
    static constexpr uint32_t kUUIDSize = 16u;
    static constexpr uint32_t kLUIDSize = 8u;

    // These shifts are made so that Vulkan resource bindings slots don't interfere with each other across shader stages:
    // These are also used during shader compilation
    enum
    {
        VULKAN_BINDING_SHIFT_B = 0,
        VULKAN_BINDING_SHIFT_T = 1000, // SRV
        VULKAN_BINDING_SHIFT_U = 2000, // UAV
        VULKAN_BINDING_SHIFT_S = 3000, // Sampler
    };

    /* Enums */
    enum class GraphicsAPI : uint8_t
    {
        Null,
        Vulkan,
        D3D12,
        Metal,

        Count,
    };

    enum class GPUAdapterVendor : uint8_t
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

    enum class ValidationMode : uint8_t
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

    enum class GPUPowerPreference : uint8_t
    {
        HighPerformance,
        LowPower,
    };

    /// Describe the RHIAdapter types
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

    enum class QueueType : uint8_t
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

    enum class TextureLayout : uint8_t
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

    enum class BufferUsage : uint32_t
    {
        None = 0,
        /// Supports input assembly access as VertexBuffer.
        Vertex = 1 << 0,
        /// Supports input assembly access as IndexBuffer.
        Index = 1 << 1,
        /// Supports Constant buffer access.
        Constant = 1 << 2,
        ShaderRead = 1 << 3,
        ShaderWrite = 1 << 4,
        ShaderReadWrite = ShaderRead | ShaderWrite,
        /// Supports indirect buffer access for indirect draw/dispatch.
        Indirect = 1 << 5,
        /// Supports predication access for conditional rendering.
        Predication = 1 << 6,
        /// Supports ray tracing acceleration structure usage.
        RayTracing = 1 << 7,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(BufferUsage);

    enum class TextureType : uint32_t
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

    /// Pipeline statistics available for the PipelineStatistics query type.
    enum class PipelineStatisticsFlags : uint64_t
    {
        None = 0,
        InputAssemblyVertices = ALIMER_BIT(0),     /// Number of vertices read by input assembler.
        InputAssemblyPrimitives = ALIMER_BIT(1),   /// Number of primitives read by the input assembler.
        VSInvocations = ALIMER_BIT(2),  /// Number of times a vertex shader was invoked.
        GSInvocations = ALIMER_BIT(3),  /// Number of times a geometry shader was invoked.
        GSPrimitives = ALIMER_BIT(4),   /// Number of primitives output by a geometry shader.
        CInvocations = ALIMER_BIT(5),   /// Number of primitives that were sent to the rasterizer.
        CPrimitives = ALIMER_BIT(6),    /// Number of primitives that were rendered (the number of primitives output by the Primitive Clipping stage).
        FragmentShaderInvocations = ALIMER_BIT(7),  /// Number of times a fragment shader was invoked.
        HSInvocations = ALIMER_BIT(8),  /// Number of times a hull shader was invoked.
        DSInvocations = ALIMER_BIT(9),  /// Number of times a domain shader was invoked.
        ComputeShaderInvocations = ALIMER_BIT(10), /// Number of times a compute shader was invoked.
        All =
        InputAssemblyVertices | InputAssemblyPrimitives | VSInvocations | GSInvocations | GSPrimitives | CInvocations | CPrimitives |
        FragmentShaderInvocations | HSInvocations | DSInvocations | ComputeShaderInvocations
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(Alimer::PipelineStatisticsFlags);

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

    enum class CullMode : uint8_t
    {
        None = 0,
        Front = 1,
        Back = 2,
    };

    enum class FrontFace : uint8_t
    {
        CounterClockwise = 0,
        Clockwise = 1,
    };

    enum class DepthClipMode : uint8_t
    {
        Clip,
        Clamp
    };

    enum class VertexFormat : uint32_t
    {
        Undefined = 0,
        UByte,
        UByte2,
        UByte4,
        Byte,
        Byte2,
        Byte4,
        UByteNormalized,
        UByte2Normalized,
        UByte4Normalized,
        ByteNormalized,
        Byte2Normalized,
        Byte4Normalized,
        UShort,
        UShort2,
        UShort4,
        Short,
        Short2,
        Short4,
        UShortNormalized,
        UShort2Normalized,
        UShort4Normalized,
        ShortNormalized,
        Short2Normalized,
        Short4Normalized,
        Half,
        Half2,
        Half4,
        Float,
        Float2,
        Float3,
        Float4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
        Int,
        Int2,
        Int3,
        Int4,

        UInt1010102Normalized,
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

    enum class IndexType : uint32_t
    {
        UInt16,
        UInt32,
    };

    enum class PrimitiveTopology : uint32_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        PatchList,
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
        Amplification = ALIMER_BIT(3),
        Mesh = ALIMER_BIT(4),

        All = 0x3FFF,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(ShaderStages);

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
    class RHIResource;
    class RHIBuffer;
    class RHITexture;
    class RHITextureView;
    class RHISampler;
    class RHIShaderModule;
    class RHIBindGroupLayout;
    class RHIPipelineLayout;
    class RHIBindGroup;
    class RHICommandQueue;
    class RHIPipeline;
    class RHISwapChain;
    class RHIQueryHeap;
    class RHIDevice;
    class RHIAdapter;
    class RHIFactory;

    using RHIBufferRef = SharedPtr<RHIBuffer>;
    using RHITextureRef = SharedPtr<RHITexture>;
    using RHITextureViewRef = SharedPtr<RHITextureView>;
    using RHISamplerRef = SharedPtr<RHISampler>;
    using RHIShaderModuleRef = SharedPtr<RHIShaderModule>;
    using RHIBindGroupLayoutRef = SharedPtr<RHIBindGroupLayout>;
    using RHIPipelineLayoutRef = SharedPtr<RHIPipelineLayout>;
    using RHIBindGroupRef = SharedPtr<RHIBindGroup>;
    using RHIPipelineRef = SharedPtr<RHIPipeline>;
    using RHIQueryHeapRef = SharedPtr<RHIQueryHeap>;
    using RHISwapChainRef = SharedPtr<RHISwapChain>;
    using RHICommandQueueRef = SharedPtr<RHICommandQueue>;
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

    struct BufferDesc
    {
        const char* label = nullptr;
        uint64_t size = 0u;
        BufferUsage usage = BufferUsage::None;
        MemoryType memoryType = MemoryType::Private;
        uint32_t stride = 0; // Needed for StructuredBuffer
        void* existingHandle = nullptr;
    };

    struct TextureSwizzleChannels
    {
        TextureSwizzle red = TextureSwizzle::Red;
        TextureSwizzle green = TextureSwizzle::Green;
        TextureSwizzle blue = TextureSwizzle::Blue;
        TextureSwizzle alpha = TextureSwizzle::Alpha;
    };

    struct TextureDesc
    {
        /// The name of the texture for debugging purposes.
        const char* label = nullptr;
        TextureType type = TextureType::Texture2D;
        PixelFormat format = PixelFormat::RGBA8Unorm;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        uint32_t mipLevelCount = 1;
        TextureSampleCount sampleCount = TextureSampleCount::Count1;
        TextureUsage usage = TextureUsage::ShaderRead;
        //MemoryType memoryType = MemoryType::Private;
    };

    struct TextureData
    {
        const void* pData = nullptr;
        uint32_t rowPitch = 0;
        uint32_t slicePitch = 0;
    };

    struct TextureViewDesc
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

    enum class BufferBindingType : uint8_t
    {
        Undefined = 0,
        Constant,
        Storage,
        ReadOnlyStorage,
    };

    enum class SamplerBindingType : uint8_t
    {
        Undefined = 0,
        Filtering,
        NonFiltering,
        Comparison,
    };

    enum class TextureSampleType : uint8_t
    {
        Undefined = 0,
        Float,
        UnfilterableFloat,
        Depth,
        Sint,
        Uint,
    };


    enum class StorageTextureAccess : uint8_t
    {
        Undefined = 0,
        WriteOnly,
        ReadOnly,
        ReadWrite
    };

    enum class DescriptorType : uint8_t
    {
        Undefined = 0,
        Buffer,
        Sampler,
        Texture,
        StorageTexture,
    };

    struct BufferBindingLayout final
    {
        BufferBindingType type = BufferBindingType::Undefined;
        bool hasDynamicOffset = false;
        uint64_t minBindingSize = 0;
    };

    struct SamplerBindingLayout final
    {
        SamplerBindingType type = SamplerBindingType::Undefined;
        const SamplerDesc* staticSampler = nullptr;
    };

    struct TextureBindingLayout final
    {
        TextureSampleType sampleType = TextureSampleType::Undefined;
        //TextureViewDimension viewDimension;
        bool multisampled = false;
    };

    struct StorageTextureBindingLayout final
    {
        StorageTextureAccess access = StorageTextureAccess::Undefined;
        PixelFormat format = PixelFormat::Undefined;
        //TextureViewDimension viewDimension;
    };

    struct BindGroupLayoutEntry
    {
        uint32_t                    binding = 0;
        uint32_t                    count = 1;
        ShaderStages                visibility = ShaderStages::All;
        BufferBindingLayout         buffer;
        SamplerBindingLayout        sampler;
        TextureBindingLayout        texture;
        StorageTextureBindingLayout storageTexture;
    };

    struct ShaderModuleDesc
    {
        const char* label = nullptr;
        size_t bytecodeSize;
        const void* bytecode;
    };

    struct BindGroupLayoutDesc
    {
        const char* label = nullptr;
        size_t entryCount = 0;
        const BindGroupLayoutEntry* entries = nullptr;
    };

    struct PushConstantRange
    {
        /// The shader stage the constants will be accessible to.
        ShaderStages visibility = ShaderStages::All;
        /// Size in bytes.
        uint32_t size = 0;
        /// Register index to bind to (supplied in shader).
        uint32_t shaderRegister = 0;
    };

    struct PipelineLayoutDesc
    {
        const char* label = nullptr;
        size_t bindGroupLayoutCount = 0;
        const RHIBindGroupLayoutRef* bindGroupLayouts = nullptr;
        const PushConstantRange* pushConstantRange = nullptr;
    };

    struct BindGroupEntry
    {
        uint32_t            binding = 0;
        uint32_t            arrayElement = 0;
        RHIBufferRef        buffer;
        uint64_t            offset = 0;
        uint64_t            size = 0;
        RHISamplerRef       sampler;
        RHITextureViewRef   textureView;
    };

    struct BindGroupSamplerEntry
    {
        uint32_t        binding = 0;
        RHISamplerRef   sampler;
        uint32_t        arrayElement = 0;
    };

    struct BindGroupDesc
    {
        const char* label = nullptr;
        size_t entryCount = 0;
        BindGroupEntry const* entries = nullptr;
        //size_t samplerCount;
        //BindGroupSamplerEntry const* samplers;
    };

    struct VertexAttribute
    {
        uint32_t        location = 0;
        uint32_t        bufferIndex = 0;
        VertexFormat    format = VertexFormat::Undefined;
        uint32_t        offset = 0;
    };

    struct VertexBufferLayout
    {
        uint32_t            binding = 0;
        uint32_t            stride = 0;
        VertexStepMode      stepMode = VertexStepMode::Vertex;
    };

    struct VertexInputDesc
    {
        uint32_t bufferCount = 0;
        const VertexBufferLayout* buffers = nullptr;
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

    struct RenderPipelineDesc
    {
        const char* label = nullptr;
        RHIPipelineLayout* layout = nullptr;

        RHIShaderModuleRef vertexShader = nullptr;
        RHIShaderModuleRef fragmentShader = nullptr;
        RHIShaderModuleRef amplificationShader = nullptr;
        RHIShaderModuleRef meshShader = nullptr;

        const VertexInputDesc* vertexInput = nullptr;

        BlendState blendState{};
        RasterizerState rasterizerState{};
        DepthStencilState depthStencilState{};

        PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t patchControlPoints = 0;

        uint32_t           colorAttachmentCount = 0;
        const PixelFormat* colorAttachmentFormats = nullptr;
        PixelFormat        depthStencilFormat = PixelFormat::Undefined;
        TextureSampleCount sampleCount = TextureSampleCount::Count1;
    };

    struct ComputePipelineDesc
    {
        const char* label = nullptr;
        RHIPipelineLayout* layout = nullptr;
        RHIShaderModuleRef shader;
    };

    struct RayTracingPipelineDesc
    {
        const char* label = nullptr;
        RHIPipelineLayout* layout = nullptr;
    };

    struct QueryHeapDesc
    {
        const char* label = nullptr;
        /// ie: Timestamp, Occlusion, PipelineStatistics
        QueryType type = QueryType::Timestamp;
        /// Total size of the heap in number of queries.
        uint32_t count;
        /// Mask of pipeline statistics that the pool will collect. Only valid for the QueryType::PipelineStatistics type.
        PipelineStatisticsFlags pipelineStatisticsMask = PipelineStatisticsFlags::None;
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

    struct RenderPassDesc
    {
        uint32_t                                colorAttachmentCount = 0;
        const RenderPassColorAttachment*        colorAttachments = nullptr;
        const RenderPassDepthStencilAttachment* depthStencilAttachment = nullptr;
    };

    struct RHISwapChainDesc
    {
        std::string_view label;
        uint32_t width = 0;
        uint32_t height = 0;
        //TextureUsage usage = TextureUsage::RenderTarget;
        PixelFormat colorFormat = PixelFormat::BGRA8UnormSrgb;
        PresentMode presentMode = PresentMode::Fifo;
        bool fullscreen = false;
    };

    struct RHIDeviceDesc
    {
        std::string_view label;
    };

    struct RHIFactoryDesc
    {
        std::string_view label;
        GraphicsAPI preferredApi = GraphicsAPI::Count;
        ValidationMode validationMode = ValidationMode::Disabled;
    };

    struct RHIAdapterLimits
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

    struct RHIAdapterProperties
    {
        std::string         deviceName;
        uint32_t            vendorID = 0;
        uint32_t            deviceID = 0;
        RHIAdapterType      adapterType = RHIAdapterType::Other;
        std::string         driverDescription;
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

        /* WGPU */
        WGPU_Instance = 0x00040001,
        WGPU_Adapter = 0x00040002,
        WGPU_Device = 0x00040003,
        WGPU_Buffer = 0x00040004,
        WGPU_Texture = 0x00040006,
        WGPU_TextureView = 0x00040007,
    };

    struct RHINativeHandle
    {
        RHINativeHandleType type = RHINativeHandleType::Unknown;

        union {
            uint64_t integer;
            void* pointer;
        };

        RHINativeHandle(uint64_t i) : integer(i) {}  // NOLINT(cppcoreguidelines-pro-type-member-init)
        RHINativeHandle(void* p) : pointer(p) {}     // NOLINT(cppcoreguidelines-pro-type-member-init)

        template<typename T> operator T* () const { return static_cast<T*>(pointer); }
        operator bool() const { return type != RHINativeHandleType::Unknown; }
    };

    class ALIMER_API RHIObject : public RefCounted
    {
    public:
        virtual void SetLabel(const char* label) { ALIMER_UNUSED(label); }
        virtual RHINativeHandle GetNativeHandle(RHINativeHandleType type) { (void)type; return nullptr; }

    protected:
        RHIObject() = default;
    };

    class ALIMER_API RHIResource : public RHIObject
    {
    public:
        enum class Type
        {
            Unknown,
            Buffer,
            Texture,
            RayTracingAccelerationStructure,
        };

        constexpr bool IsBuffer() const { return type == Type::Buffer; }
        constexpr bool IsTexture() const { return type == Type::Texture; }
        constexpr bool IsRayTracingAccelerationStructure() const { return type == Type::RayTracingAccelerationStructure; }

    protected:
        RHIResource(Type type)
            : type{ type }
        {

        }

        Type type;
    };

    class ALIMER_API RHIBuffer : public RHIResource
    {
    public:
        [[nodiscard]] uint64_t GetSize() const { return size; }
        [[nodiscard]] BufferUsage GetUsage() const { return usage; }
        [[nodiscard]] uint32_t GetStride() const { return stride; }
        [[nodiscard]] virtual void* GetMappedData() const = 0;
        [[nodiscard]] virtual DeviceAddress GetDeviceAddress() const = 0;

    protected:
        RHIBuffer(const BufferDesc& desc)
            : RHIResource(Type::Buffer)
            , size(desc.size)
            , usage(desc.usage)
            , stride(desc.stride)
        {
        }

        uint64_t size;
        BufferUsage usage = BufferUsage::None;
        uint32_t stride;
    };

    class ALIMER_API RHITexture : public RHIResource
    {
    protected:
        RHITexture(const TextureDesc& desc)
            : RHIResource(Type::Texture)
            , type(desc.type)
            , format(desc.format)
            , width(desc.width)
            , height(desc.height)
            , depthOrArrayLayers(desc.depthOrArrayLayers)
            , mipLevelCount(desc.mipLevelCount)
            , sampleCount(desc.sampleCount)
            , usage(desc.usage)
        {
        }

    public:
        [[nodiscard]] constexpr TextureType GetType() const { return type; }
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
            if (type != TextureType::Texture3D)
            {
                return 1;
            }

            return (mipLevel == 0) || (mipLevel < mipLevelCount) ? Alimer::Max(1u, depthOrArrayLayers >> mipLevel) : 0;
        }

        [[nodiscard]] constexpr uint32_t GetArrayLayers() const
        {
            if (type == TextureType::Texture3D)
            {
                return 1;
            }

            return depthOrArrayLayers;
        }

        [[nodiscard]] constexpr TextureUsage GetUsage() const { return usage; }
        [[nodiscard]] constexpr uint32_t GetMipLevelCount() const { return mipLevelCount; }
        [[nodiscard]] constexpr TextureSampleCount GetSampleCount() const { return sampleCount; }

        [[nodiscard]] RHITextureView* GetDefaultView() const;
        [[nodiscard]] RHITextureView* GetView(const TextureViewDesc* desc = nullptr) const;

    protected:
        virtual RHITextureViewRef CreateView(const TextureViewDesc& desc)  const = 0;

    protected:
        TextureType type;
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
        RHITextureView(const RHITexture* texture_, const TextureViewDesc& desc)
            : texture(texture_)
            , format(desc.format)
            , aspect(desc.aspect)
            , baseMipLevel(desc.baseMipLevel)
            , mipLevelCount(desc.mipLevelCount)
            , baseArrayLayer(desc.baseArrayLayer)
            , arrayLayerCount(desc.arrayLayerCount)
            , swizzle(desc.swizzle)
        {
        }

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

    class ALIMER_API RHISampler : public RHIObject
    {
    public:
        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;
    };

    class ALIMER_API RHIShaderModule : public RHIObject
    {
    };

    class ALIMER_API RHIBindGroupLayout : public RHIObject
    {
    public:

    protected:
    };

    class ALIMER_API RHIPipelineLayout : public RHIObject
    {
    public:

    protected:
    };

    class ALIMER_API RHIBindGroup : public RHIObject
    {
    public:
        [[nodiscard]] virtual RHIBindGroupLayout* GetBindGroupLayout() const = 0;

        virtual void Update(size_t entryCount, const BindGroupEntry* entries) = 0;
        //void UpdateConstantBuffer(const RHIBuffer* buffer, uint32_t binding, uint32_t arrayElement = 0);
    };

    class ALIMER_API RHICommandQueue : public RHIObject
    {
    public:
        virtual QueueType GetType() const = 0;
    };

    class ALIMER_API RHIPipeline : public RHIObject
    {
    public:
        enum class Type
        {
            Render,
            Compute,
            RayTracing,
        };

        virtual Type GetType() const = 0;

    protected:
    };

    class ALIMER_API RHIQueryHeap : public RHIObject
    {
    public:
        virtual uint32_t GetCount() const = 0;
        virtual QueryType GetType() const = 0;
    };

    class RHISurface;
    using RHISurfaceRef = SharedPtr<RHISurface>;

    class ALIMER_API RHISurface : public RHIObject
    {
    public:
        enum class Type
        {
            AndroidWindow,
            MetalLayer,
            WindowsHWND,
            IDCompositionVisual,
            SwapChainPanel,
            SurfaceHandle,
            WaylandSurface,
            XlibWindow,
        };

        Type GetType() const { return type; }

        static RHISurfaceRef CreateAndroid(ANativeWindow* window);
        static RHISurfaceRef CreateMetalLayer(/*CAMetalLayer*/void* layer);
        static RHISurfaceRef CreateWayland(wl_display* waylandDisplay, wl_surface* waylandSurface);
        static RHISurfaceRef CreateXlib(/*Display*/void* display, /*Window*/uint64_t window);
        static RHISurfaceRef CreateWin32([[maybe_unused]] HWND hwnd);
        static RHISurfaceRef CreateIDCompositionVisual(IUnknown* visual);
        static RHISurfaceRef CreateSwapChainPanel(IUnknown* swapChainPanel);

        // Valid to call if the type is Android
        ANativeWindow* GetAndroidNativeWindow() const;
        // Valid to call if the type is MetalLayer
        void* GetMetalLayer() const;

        // Valid to call if the type is WaylandSurface
        wl_display* GetWaylandDisplay() const;
        wl_surface* GetWaylandSurface() const;

        // Valid to call if the type is XlibWindow
        void* GetXDisplay() const;
        uint64_t GetXWindow() const;

        // Valid to call if the type is WindowsHWND
        HWND GetHWND() const;

        // Valid to call if the type is IDCompositionVisual
        IUnknown* GetIDCompositionVisual() const;
        // Valid to call if the type is SwapChainPanel
        IUnknown* GetSwapChainPanel() const;

        // Valid to call if the type is SurfaceHandle
        void* GetSurfaceHandle() const;

    private:
        RHISurface(Type type);

        Type type;

        // MetalLayer
        void* metalLayer = nullptr;
        // ANativeWindow
        ANativeWindow* androidNativeWindow = nullptr;

        // Wayland
        wl_display* waylandDisplay = nullptr;
        wl_surface* waylandSurface = nullptr;
        // Xlib
        void* xDisplay = nullptr;
        uint64_t xWindow = 0;

        // WindowsHwnd
        HWND hwnd = nullptr;

        // IDCompositionVisual/SwapChainPanel
        IUnknown* idCompositionVisualOrSwapChainPanel = nullptr;
        // SurfaceHandle
        void* surfaceHandle = nullptr;
    };

    class ALIMER_API RHISwapChain : public RHIObject
    {
    public:
        [[nodiscard]] RHISurface* GetSurface() const { return surface.Get(); }
        [[nodiscard]] virtual PixelFormat GetColorFormat() const = 0;

    protected:
        RHISwapChain(RHISurface* surface);

        RHISurfaceRef surface;
    };

    struct GPUAllocation
    {
        void* data = nullptr;
        RHIBufferRef buffer;
        uint64_t offset = 0;

        /// Returns true if the allocation was successful
        inline bool IsValid() const { return data != nullptr && buffer != nullptr; }
    };

    class ALIMER_API CopyContext
    {
    public:
        /// Destructor.
        virtual ~CopyContext() = default;
        virtual void PushDebugGroup(std::string_view name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(std::string_view name) = 0;

        GPUAllocation AllocateGPU(uint64_t size);
        virtual void UpdateBuffer(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size = 0);

        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) = 0;
        virtual void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) = 0;

        virtual RHINativeHandle GetNativeHandle(RHINativeHandleType type) { (void)type; return nullptr; }

    protected:
        struct GPULinearAllocator
        {
            RHIBufferRef buffer;
            uint64_t offset = 0ull;

            void Reset()
            {
                offset = 0ull;
            }
        };

        GPULinearAllocator frameAllocators[kNumFramesInFlight];
    };

    class ALIMER_API ComputeContext : public CopyContext
    {
    public:
        /// Destructor.
        virtual ~ComputeContext() = default;

        virtual void SetPipeline(RHIPipeline* pipeline) = 0;

        /// Bind a BindGroup.
        virtual void SetBindGroup(uint32_t groupIndex, RHIBindGroup* bindGroup) = 0;
        virtual void SetPushConstants(const void* data, uint32_t size, uint32_t offset = 0) = 0;

        template<typename T>
        void SetPushConstants(const T& data, uint32_t offset = 0)
        {
            SetPushConstants(&data, sizeof(T), offset);
        }

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        virtual void BeginQuery(const RHIQueryHeap* heap, uint32_t index) = 0;
        virtual void EndQuery(const RHIQueryHeap* heap, uint32_t index) = 0;
        virtual void ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) = 0;
        virtual void ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count) = 0;
    private:
        void PreDispatchValidation();

    protected:
        virtual void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

        bool insideRenderPass{ false };
    };

    class ALIMER_API GraphicsContext : public ComputeContext
    {
    public:
        /// Destructor.
        virtual ~GraphicsContext() = default;

        void BeginRenderPass(const RenderPassDesc& desc);
        void EndRenderPass();

        virtual RHITexture* AcquireSwapChainTexture(RHISwapChain* swapChain) = 0;

        virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
        //virtual void SetViewport(const Viewport& viewport) = 0;
        //virtual void SetViewports(uint32_t count, const Viewport* viewports) = 0;
        virtual void SetScissorRect(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
        //virtual void SetScissorRect(const Rect& rect) = 0;
        //virtual void SetScissorRects(uint32_t count, const Rect* scissorRects) = 0;
        virtual void SetStencilReference(uint32_t reference) = 0;
        virtual void SetBlendColor(float red, float green, float blue, float alpha) = 0;
        //virtual void SetBlendColor(const Color& color) = 0;
        virtual void SetShadingRate(ShadingRate rate) = 0;
        virtual void SetDepthBounds(float minBounds, float maxBounds) = 0;

        /// Bind a vertex buffer.
        virtual void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset = 0) = 0;

        /// Bind a vertex buffers.
        virtual void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) = 0;

        /// Bind an index buffer.
        virtual void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, IndexType indexType) = 0;

        /// Draw non-indexed geometry.
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
        /// Draw indexed geometry.
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0) = 0;
        /// Draw primitives with indirect parameters
        virtual void DrawIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0) = 0;
        /// Draw primitives with indirect parameters and indexed vertices
        virtual void DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset = 0) = 0;

        virtual void DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;
        virtual void DispatchMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
        virtual void DispatchMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount) = 0;

        virtual void BeginPredication(const RHIBuffer* buffer, uint64_t offset, PredicationOperation operation) = 0;
        virtual void EndPredication() = 0;

    protected:
        void Reset(uint32_t frameIndex);
        virtual void BeginRenderPassCore(const RenderPassDesc& desc) = 0;
        virtual void EndRenderPassCore() = 0;

        ShadingRate currentShadingRate{ ShadingRate::Invalid };
    };

    class ALIMER_API RHIDevice : public RHIObject
    {
    public:
        /// Wait for GPU to finish pending operations.
        virtual bool WaitIdle() = 0;

        /// Commit the current frame and advance to next frame
        virtual uint64_t CommitFrame() = 0;

        RHIBufferRef CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr);
        RHITextureRef CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr);
        RHITextureRef CreateTextureFromNativeHandle(RHINativeHandle handle, const TextureDesc& desc);
        RHISamplerRef CreateSampler(const SamplerDesc& desc);

        RHIShaderModuleRef CreateShaderModule(const ShaderModuleDesc& desc);
        RHIBindGroupLayoutRef CreateBindGroupLayout(const BindGroupLayoutDesc& desc);
        RHIPipelineLayoutRef CreatePipelineLayout(const PipelineLayoutDesc& desc);
        RHIBindGroupRef CreateBindGroup(RHIBindGroupLayout* layout, const BindGroupDesc& desc);

        RHIPipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc);
        RHIPipelineRef CreateComputePipeline(const ComputePipelineDesc& desc);
        RHIPipelineRef CreateRayTracingPipeline(const RayTracingPipelineDesc& desc);
        RHIQueryHeapRef CreateQueryHeap(const QueryHeapDesc& desc);

        RHISwapChainRef CreateSwapChain(RHISurface* surface, const RHISwapChainDesc& desc);

        virtual void WriteShadingRateValue(ShadingRate rate, void* dest) const = 0;

        virtual GraphicsContext* BeginGraphicsContext(const std::string& label = "") = 0;
        virtual ComputeContext* BeginComputeContext(const std::string& label = "") = 0;

        virtual RHIAdapter* GetAdapter() const = 0;

        constexpr bool IsDeviceLost() const noexcept { return _deviceLost; }
        constexpr uint64_t GetFrameCount() const noexcept { return frameCount; }
        constexpr uint32_t GetFrameIndex() const noexcept { return frameIndex; }


        [[nodiscard]] virtual bool QueryFeatureSupport(RHIFeature feature) = 0;
        [[nodiscard]] virtual PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) = 0;
        //[[nodiscard]] virtual bool QueryVertexFormatSupport(VertexFormat format) = 0;

        [[nodiscard]] constexpr uint64_t GetTimestampFrequency() const { return timestampFrequency; }

    protected:
        virtual void InitResources();
        virtual bool ValidateTextureDesc(const TextureDesc& desc);
        virtual RHIBufferRef CreateBufferCore(const BufferDesc& desc, const void* initialData) = 0;
        virtual RHITextureRef CreateTextureCore(const TextureDesc& desc, const TextureData* initialData) = 0;
        virtual RHITextureRef CreateTextureFromNativeHandleCore(RHINativeHandle handle, const TextureDesc& desc) = 0;
        virtual RHISamplerRef CreateSamplerCore(const SamplerDesc& desc) = 0;
        virtual RHIShaderModuleRef CreateShaderModuleCore(const ShaderModuleDesc& desc) = 0;
        virtual RHIBindGroupLayoutRef CreateBindGroupLayoutCore(const BindGroupLayoutDesc& desc) = 0;
        virtual RHIPipelineLayoutRef CreatePipelineLayoutCore(const PipelineLayoutDesc& desc) = 0;
        virtual RHIBindGroupRef CreateBindGroupCore(RHIBindGroupLayout* layout, const BindGroupDesc& desc) = 0;
        virtual RHIPipelineRef CreateRenderPipelineCore(const RenderPipelineDesc& desc) = 0;
        virtual RHIPipelineRef CreateComputePipelineCore(const ComputePipelineDesc& desc) = 0;
        virtual RHIPipelineRef CreateRayTracingPipelineCore(const RayTracingPipelineDesc& desc) = 0;
        virtual RHIQueryHeapRef CreateQueryHeapCore(const QueryHeapDesc& desc) = 0;
        virtual RHISwapChainRef CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc) = 0;

        bool _deviceLost{ false };
        uint64_t frameCount{};
        uint32_t frameIndex{};

        uint32_t uploadBufferTextureRowAlignment = 1u;
        uint32_t uploadBufferTextureSliceAlignment = 1u;

        uint64_t timestampFrequency = 0;
        Vector<RHISamplerRef> staticSamplers;
    };

    class ALIMER_API RHIAdapter
    {
    public:
        virtual RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) = 0;

        virtual RHIAdapterType GetType() const = 0;

        /// Return the physical adapter properties.
        [[nodiscard]] const RHIAdapterProperties& GetAdapterProperties() const { return _properties; }

        /// Return the adapter limits.
        [[nodiscard]] const RHIAdapterLimits& GetLimits() const { return _limits; }

    protected:
        RHIAdapterProperties _properties{};
        RHIAdapterLimits _limits{};
    };


    class ALIMER_API RHIFactory : public RHIObject
    {
    public:
        static RHIFactoryRef Create(const RHIFactoryDesc& desc);

        /// Returns the API kind that the RHI backend is running on top of.
        virtual GraphicsAPI GetGraphicsAPI() const = 0;

        uint32_t GetAdapterCount() const;
        RHIAdapter* GetAdapter(uint32_t index) const;
        RHIAdapter* GetBestAdapter() const;

    protected:
        Vector<RHIAdapter*> _adapters;
    };

    /** A global pointer to the RHI device. */
    extern ALIMER_API RHIDeviceRef GRHIDevice;

    ALIMER_API bool RHIIsSupported(GraphicsAPI backend);
    ALIMER_API GraphicsAPI RHIGetPlatformPreferredApi();
    ALIMER_API bool RHIInit(RHIAdapter* adapter, const RHIDeviceDesc& desc);
    ALIMER_API void RHIShutdown();

    ALIMER_FORCE_INLINE GraphicsContext* RHIBeginGraphicsContext(const std::string& label = "")
    {
        return GRHIDevice->BeginGraphicsContext(label);
    }

    ALIMER_API RHIBufferRef RHICreateBuffer(uint64_t size, BufferUsage usage = BufferUsage::ShaderReadWrite, const void* initialData = nullptr, const char* label = nullptr);
    ALIMER_API RHIBufferRef RHICreateBuffer(const BufferDesc& desc, const void* initialData = nullptr);

    template<typename T>
    RHIBufferRef RHICreateStaticBuffer(_In_reads_(count) const T* data, uint32_t count, BufferUsage usage = BufferUsage::ShaderReadWrite) noexcept
    {
        return RHICreateBuffer(sizeof(T) * count, usage, data);
    }

    template<typename T>
    RHIBufferRef RHICreateStaticBuffer(const T& data, BufferUsage usage = BufferUsage::ShaderReadWrite) noexcept
    {
        return RHICreateBuffer(data.size() * sizeof(typename T::value_type), usage, data.data());
    }

    ALIMER_API RHITextureRef RHICreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr);
    ALIMER_API RHISamplerRef RHICreateSampler(const SamplerDesc& desc);
    ALIMER_API RHIShaderModuleRef RHICreateShaderModule(const ShaderModuleDesc& desc);
    ALIMER_API RHIBindGroupLayoutRef RHICreateBindGroupLayout(const BindGroupLayoutDesc& desc);
    ALIMER_API RHIPipelineLayoutRef RHICreatePipelineLayout(const PipelineLayoutDesc& desc);
    ALIMER_API RHIBindGroupRef RHICreateBindGroup(RHIBindGroupLayout* layout, const BindGroupDesc& desc);
    ALIMER_API RHIPipelineRef RHICreateRenderPipeline(const RenderPipelineDesc& desc);
    ALIMER_API RHIPipelineRef RHICreateComputePipeline(const ComputePipelineDesc& desc);
    ALIMER_API RHIPipelineRef RHICreateRayTracingPipeline(const RayTracingPipelineDesc& desc);
    ALIMER_API RHIQueryHeapRef RHICreateQueryHeap(const QueryHeapDesc& desc);
    ALIMER_API RHISwapChainRef RHICreateSwapChain(RHISurface* surface, const RHISwapChainDesc& desc);

    ALIMER_API const std::string ToString(GraphicsAPI type);
    ALIMER_API const std::string ToString(RHIAdapterType type);

    ALIMER_API GPUAdapterVendor VendorIdToAdapterVendor(uint32_t vendorId);
    ALIMER_API uint32_t AdapterVendorToVendorId(GPUAdapterVendor vendor);

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
    ALIMER_API DescriptorType GetDescriptorType(const BindGroupLayoutEntry& entry);

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

    struct VertexFormatInfo
    {
        VertexFormat format;
        uint32_t byteSize;
        uint32_t componentCount;
        VertexFormatKind baseType;
    };
    ALIMER_API const VertexFormatInfo& GetVertexFormatInfo(VertexFormat format);

    /* Shader loading */
    struct ShaderMacro
    {
        std::string name;
        std::string definition;

        ShaderMacro(const std::string& _name, const std::string& _definition)
            : name(_name)
            , definition(_definition)
        {
        }
    };

    ALIMER_API RHIShaderModuleRef RHILoadShader(ShaderStages stage, const char* fileName, const std::vector<ShaderMacro>* pDefines = nullptr);
}
