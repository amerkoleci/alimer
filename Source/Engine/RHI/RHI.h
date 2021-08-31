// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Math/Color.h"

#define RHI_DEFINE_ENUM_BITWISE_OPERATORS(T) \
    inline constexpr T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline constexpr T& operator |= (T &a, T b) { return a = a | b; } \
    inline constexpr T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } \
    inline constexpr T& operator &= (T &a, T b) { return a = a & b; } \
    inline constexpr T operator ~ (T a) { return T(~uint32_t(a)); } \
    inline constexpr bool operator !(T a) { return uint32_t(a) == 0; } \
    inline constexpr T operator ^ (T a, T b) { return T(uint32_t(a) ^ uint32_t(b)); } \
    inline constexpr T& operator ^= (T &a, T b) { return a = a ^ b; } \
    inline constexpr bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline constexpr bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

#if defined(RHI_SHARED_LIBRARY_BUILD)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllexport)
#   elif defined(__GNUC__)
#       define RHI_API __attribute__((visibility("default")))
#   else
#       define RHI_API
#       pragma warning "Unknown dynamic link import/export semantics."
#   endif
#elif defined(RHI_SHARED_LIBRARY_INCLUDE)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllimport)
#   else
#       define RHI_API
#   endif
#else
#   define RHI_API
#endif

namespace alimer
{
    class Window;
}

namespace alimer::rhi
{
    /* Constants */
    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 4;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxCommandLists = 32u;
    //static constexpr uint32_t kMaxUniformBufferBindings = 14;
    //static constexpr uint32_t kMaxDescriptorBindings = 32;
    //static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;
    //static constexpr uint32_t kInvalidBindlessIndex = static_cast<uint32_t>(-1);

    static constexpr uint32_t kAllMipLevels = static_cast<uint32_t>(-1);
    static constexpr uint32_t kAllArraySlices = static_cast<uint32_t>(-1);

    static constexpr uint32_t KnownVendorId_AMD = 0x1002;
    static constexpr uint32_t KnownVendorId_Intel = 0x8086;
    static constexpr uint32_t KnownVendorId_Nvidia = 0x10DE;
    static constexpr uint32_t KnownVendorId_Microsoft = 0x1414;
    static constexpr uint32_t KnownVendorId_ARM = 0x13B5;
    static constexpr uint32_t KnownVendorId_ImgTec = 0x1010;
    static constexpr uint32_t KnownVendorId_Qualcomm = 0x5143;

    /* Enums */
    enum class ValidationMode : uint32_t
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

    enum class GraphicsAPI : uint8_t
    {
        D3D11,
        D3D12,
        Vulkan
    };


    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,

        Count
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
    RHI_DEFINE_ENUM_BITWISE_OPERATORS(ResourceStates);

    enum class Format : uint32_t
    {
        Undefined = 0,
        // 8-bit formats
        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,
        // 16-bit formats
        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16Float,
        RG8UNorm,
        RG8SNorm,
        RG8UInt,
        RG8SInt,
        // Packed 16-Bit Pixel Formats
        BGRA4UNorm,
        B5G6R5UNorm,
        B5G5R5A1UNorm,
        // 32-bit formats
        R32UInt,
        R32SInt,
        R32Float,
        RG16UNorm,
        RG16SNorm,
        RG16UInt,
        RG16SInt,
        RG16Float,
        RGBA8UNorm,
        RGBA8UNormSrgb,
        RGBA8SNorm,
        RGBA8UInt,
        RGBA8SInt,
        BGRA8UNorm,
        BGRA8UNormSrgb,
        // Packed 32-Bit formats
        RGB10A2UNorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit formats
        RG32UInt,
        RG32SInt,
        RG32Float,
        RGBA16UNorm,
        RGBA16SNorm,
        RGBA16UInt,
        RGBA16SInt,
        RGBA16Float,
        // 128-Bit formats
        RGBA32UInt,
        RGBA32SInt,
        RGBA32Float,
        // Depth-stencil formats
        Depth16UNorm,
        Depth32Float,
        Depth24UNormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1UNorm,
        BC1UNormSrgb,
        BC2UNorm,
        BC2UNormSrgb,
        BC3UNorm,
        BC3UNormSrgb,
        BC4UNorm,
        BC4SNorm,
        BC5UNorm,
        BC5SNorm,
        BC6HUFloat,
        BC6HSFloat,
        BC7UNorm,
        BC7UNormSrgb,
        Count,
    };

    enum class GPUResourceUsage : uint32_t
    {
        Default,
        Dynamic,
        StagingUpload,
        StagingReadback,
    };

    enum class CompareFunction : uint32_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    enum class PrimitiveTopology : uint32_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        PatchList,
        Count
    };

    enum class BufferUsage : uint32_t
    {
        None,
    };
    RHI_DEFINE_ENUM_BITWISE_OPERATORS(BufferUsage);

    enum class TextureDimension : uint32_t
    {
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture2DMS,
        Texture2DMSArray,
        TextureCube,
        TextureCubeArray,
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
    RHI_DEFINE_ENUM_BITWISE_OPERATORS(TextureUsage);

    enum class ShaderStages : uint32_t
    {
        None = 0x0000,

        Compute = 0x0020,

        Vertex = 0x0001,
        Hull = 0x0002,
        Domain = 0x0004,
        Geometry = 0x0008,
        Pixel = 0x0010,
        Amplification = 0x0040,
        Mesh = 0x0080,
        AllGraphics = 0x00FE,

        RayGeneration = 0x0100,
        AnyHit = 0x0200,
        ClosestHit = 0x0400,
        Miss = 0x0800,
        Intersection = 0x1000,
        Callable = 0x2000,
        AllRayTracing = 0x3F00,

        All = 0x3FFF,
    };
    RHI_DEFINE_ENUM_BITWISE_OPERATORS(ShaderStages);

    enum class SamplerFilter : uint32_t
    {
        Point,
        Linear
    };

    enum class SamplerAddressMode : uint32_t
    {
        Wrap = 0,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };

    enum class SamplerBorderColor : uint32_t
    {
        TransparentBlack = 0,
        OpaqueBlack,
        OpaqueWhite,
    };


    enum class VertexStepRate : uint32_t
    {
        Vertex = 0,
        Instance
    };

    enum class BlendFactor : uint32_t
    {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationColor,
        OneMinusDestinationColor,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        SourceAlphaSaturated,
        BlendColor,
        OneMinusBlendColor,
        Source1Color,
        OneMinusSource1Color,
        Source1Alpha,
        OneMinusSource1Alpha,
    };

    enum class BlendOperation : uint32_t
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class ColorWriteMask : uint32_t
    {
        None = 0,
        Red = 0x01,
        Green = 0x02,
        Blue = 0x04,
        Alpha = 0x08,
        All = 0x0F
    };
    RHI_DEFINE_ENUM_BITWISE_OPERATORS(ColorWriteMask);

    enum class StencilOperation : uint32_t
    {
        Keep,
        Zero,
        Replace,
        IncrementClamp,
        DecrementClamp,
        Invert,
        IncrementWrap,
        DecrementWrap,
    };

    enum class FillMode : uint32_t
    {
        Solid,
        Wireframe,
    };

    enum class CullMode : uint32_t
    {
        None,
        Front,
        Back
    };

    enum class FaceWinding : uint32_t
    {
        Clockwise,
        CounterClockwise,
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

    /* Forward declarations */
    class IBuffer;
    class ITexture;
    class ISampler;
    class IShader;
    class IPipeline;
    class IDevice;

    /* Structs */
    struct BufferDesc
    {
        uint32_t size = 0;
        uint32_t stride = 0;
        BufferUsage usage = BufferUsage::None;
        GPUResourceUsage resourceUsage = GPUResourceUsage::Default;
        Format format = Format::Undefined;
    };

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
        ResourceStates initialState = ResourceStates::Unknown;

        static inline TextureDesc Tex1D(
            Format format,
            uint32_t width,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::ShaderRead) noexcept
        {
            TextureDesc desc;
            desc.dimension = arraySize > 1 ? TextureDimension::Texture1DArray : TextureDimension::Texture1D;
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
            Format format,
            uint32_t width,
            uint32_t height,
            uint32_t arraySize = 1,
            uint32_t mipLevels = 1,
            TextureUsage usage = TextureUsage::ShaderRead,
            uint32_t sampleCount = 1) noexcept
        {
            TextureDesc desc;
            if (sampleCount > 1)
            {
                desc.dimension = arraySize > 1 ? TextureDimension::Texture2DMSArray : TextureDimension::Texture2DMS;
            }
            else
            {
                desc.dimension = arraySize > 1 ? TextureDimension::Texture2DArray : TextureDimension::Texture2D;
            }

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
            Format format,
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
            Format format,
            uint32_t size,
            uint32_t mipLevels = 1,
            uint32_t arraySize = 1,
            TextureUsage usage = TextureUsage::ShaderRead) noexcept
        {
            TextureDesc desc;
            desc.dimension = arraySize > 1 ? TextureDimension::TextureCubeArray : TextureDimension::TextureCube;
            desc.width = size;
            desc.height = size;
            desc.depthOrArraySize = arraySize;
            desc.mipLevels = mipLevels;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }
    };

    struct SamplerDesc
    {
        SamplerFilter minFilter = SamplerFilter::Point;
        SamplerFilter magFilter = SamplerFilter::Point;
        SamplerFilter mipmapFilter = SamplerFilter::Point;
        SamplerAddressMode addressModeU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeW = SamplerAddressMode::Clamp;
        float mipLodBias = 0.0f;
        uint16_t maxAnisotropy = 1;
        CompareFunction compare = CompareFunction::Never;
        float minLod = 0.0f;
        float maxLod = FLT_MAX;
        SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
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

    struct VertexBufferLayout
    {
        uint32_t stride = 0;
        VertexStepRate stepRate = VertexStepRate::Vertex;
    };

    struct VertexAttribute
    {
        //VertexFormat format = VertexFormat::Undefined;
        Format format = Format::Undefined;
        uint32_t offset = 0;
        uint32_t bufferIndex = 0;
    };

    struct VertexLayout
    {
        VertexBufferLayout buffers[kMaxVertexBufferBindings];
        VertexAttribute attributes[kMaxVertexAttributes];
    };

    struct RenderTargetBlendState
    {
        bool blendEnable = false;
        BlendFactor srcBlend = BlendFactor::One;
        BlendFactor destBlend = BlendFactor::Zero;
        BlendOperation blendOp = BlendOperation::Add;
        BlendFactor srcBlendAlpha = BlendFactor::One;
        BlendFactor destBlendAlpha = BlendFactor::Zero;
        BlendOperation blendOpAlpha = BlendOperation::Add;
        ColorWriteMask writeMask = ColorWriteMask::All;
    };

    struct BlendState
    {
        bool alphaToCoverageEnable = false;
        bool independentBlendEnable = false;

        RenderTargetBlendState renderTargets[kMaxColorAttachments] = {};
    };

    struct StencilFaceState
    {
        StencilOperation failOp = StencilOperation::Keep;
        StencilOperation passOp = StencilOperation::Keep;
        StencilOperation depthFailOp = StencilOperation::Keep;
        CompareFunction compare = CompareFunction::Always;
    };

    struct DepthStencilState
    {
        bool depthWriteEnable = true;
        CompareFunction depthCompare = CompareFunction::Less;
        bool stencilEnable = false;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
        StencilFaceState frontFace;
        StencilFaceState backFace;
    };

    struct RasterizerState
    {
        CullMode cullMode = CullMode::Back;
        FaceWinding frontFace = FaceWinding::Clockwise;
        FillMode fillMode = FillMode::Solid;
        float depthBias = 0.0f;
        float depthBiasSlopeScale = 0.0f;
        float depthBiasClamp = 0.0f;
    };

    struct RenderPipelineDesc
    {
        IShader* vertex = nullptr;
        IShader* hull = nullptr;
        IShader* domain = nullptr;
        IShader* geometry = nullptr;
        IShader* pixel = nullptr;
        //IShader* mesh = nullptr;
        //IShader* amplification = nullptr;

        VertexLayout            vertexLayout;
        BlendState              blendState;
        DepthStencilState       depthStencilState;
        RasterizerState         rasterizerState;
        PrimitiveTopology       primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t                patchControlPoints = 0;
    };


    struct RenderPassColorAttachment
    {
        ITexture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t slice = 0;

        ITexture* resolveTexture = nullptr;
        uint32_t resolveLevel = 0;
        uint32_t resolveSlice = 0;
        LoadAction loadAction = LoadAction::Discard;
        StoreAction storeAction = StoreAction::Store;

        ResourceStates initialState = ResourceStates::Unknown;
        ResourceStates finalState = ResourceStates::RenderTarget;

        Color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct RenderPassDepthStencilAttachment
    {
        ITexture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t slice = 0;

        ITexture* resolveTexture = nullptr;
        uint32_t resolveLevel = 0;
        uint32_t resolveSlice = 0;

        LoadAction depthLoadAction = LoadAction::Clear;
        StoreAction depthStoreAction = StoreAction::Discard;
        LoadAction stencilLoadAction = LoadAction::Clear;
        StoreAction stencilStoreAction = StoreAction::Discard;
        float clearDepth = 1.0;
        uint8_t clearStencil = 0;
        bool depthStencilReadOnly = false;
    };

    struct RenderPassDesc
    {
        uint32_t colorAttachmentCount = 0;
        RenderPassColorAttachment colorAttachments[kMaxColorAttachments] = {};
        RenderPassDepthStencilAttachment depthStencilAttachment;
    };

    struct PresentationParameters
    {
        ValidationMode validationMode = ValidationMode::Disabled;

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        uint32_t backBufferCount = 3;
        Format depthStencilFormat = Format::Depth32Float;
        bool vsyncEnabled = false;
        bool isFullScreen = false;
    };

    /* Objects */
    class RHI_API Object : public RefCounted
    {
    public:
        /// Destructor. 
        virtual ~Object() = default;

        /// Get the object name.
        [[nodiscard]] const std::string& GetName() const { return name; }

        /// Set the object name.
        void SetName(const std::string_view& newName)
        {
            name = newName;
            ApiSetName(newName);
        }

    protected:
        /// Constructor.
        Object() = default;

    private:
        virtual void ApiSetName(const std::string_view& newName) = 0;
        std::string name;
    };

    class RHI_API DeviceChild : public Object
    {
    public:
        [[nodiscard]] virtual IDevice* GetDevice() const = 0;
    };

    class RHI_API IResource : public DeviceChild
    {
    protected:
        //[[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
    };

    class RHI_API IBuffer : public IResource
    {
    public:
        [[nodiscard]] virtual const BufferDesc& GetDesc() const = 0;
    };

    class RHI_API ITexture : public IResource
    {
    public:
        [[nodiscard]] virtual const TextureDesc& GetDesc() const = 0;
    };

    class RHI_API ISampler : public DeviceChild
    {
    public:
        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;
        //[[nodiscard]] virtual uint32_t GetBindlessIndex() const = 0;
    };

    class RHI_API IShader : public DeviceChild
    {
    public:
        [[nodiscard]] virtual ShaderStages GetStage() const = 0;
    };

    class RHI_API IPipeline : public DeviceChild
    {
    public:
    };

    class ALIMER_API ICommandList
    {
    protected:
        ICommandList() = default;
        virtual ~ICommandList() = default;

    public:
        // Non-copyable and non-movable
        ICommandList(const ICommandList&) = delete;
        ICommandList(const ICommandList&&) = delete;
        ICommandList& operator=(const ICommandList&) = delete;
        ICommandList& operator=(const ICommandList&&) = delete;

        virtual void PushDebugGroup(const std::string_view& name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const std::string_view& name) = 0;

        virtual void BeginDefaultRenderPass(const Color& clearColor, bool clearDepth = true, bool clearStencil = true, float depth = 1.0f, uint8_t stencil = 0) = 0;
        virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
        virtual void EndRenderPass() = 0;

        virtual void SetPipeline(_In_ IPipeline* pipeline) = 0;
        virtual void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) = 0;
    };

    using BufferHandle = RefCountPtr<IBuffer>;
    using TextureHandle = RefCountPtr<ITexture>;
    using SamplerHandle = RefCountPtr<ISampler>;
    using ShaderHandle = RefCountPtr<IShader>;
    using PipelineHandle = RefCountPtr<IPipeline>;
    using DeviceHandle = RefCountPtr<IDevice>;

    class RHI_API IDevice : public RefCounted
    {
    public:
        static DeviceHandle Create(_In_ alimer::Window* window, const PresentationParameters& presentationParameters);

        virtual void WaitIdle() = 0;
        virtual ICommandList* BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Resize(uint32_t newWidth, uint32_t newHeight) = 0;

        [[nodiscard]] virtual ITexture* GetCurrentBackBuffer() const = 0;
        [[nodiscard]] virtual ITexture* GetBackBuffer(uint32_t index) const = 0;
        [[nodiscard]] virtual uint32_t GetCurrentBackBufferIndex() const = 0;
        [[nodiscard]] virtual uint32_t GetBackBufferCount() const = 0;
        [[nodiscard]] virtual ITexture* GetBackBufferDepthStencilTexture() const = 0;

        [[nodiscard]] virtual uint64_t GetFrameCount() const = 0;

        // Returns the API kind that the RHI backend is running on top of.
        virtual GraphicsAPI GetGraphicsAPI() const = 0;

        /// Return backbuffer width.
        [[nodiscard]] uint32_t GetBackBufferWidth() const { return backBufferWidth; }
        /// Return backbuffer height.
        [[nodiscard]] uint32_t GetBackBufferHeight() const { return backBufferHeight; }

        /// Create new texture.
        TextureHandle CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr);

        /// Create new texture from external handle.
        TextureHandle CreateExternalTexture(void* nativeHandle, const TextureDesc& desc);

        /// Create new buffer.
        [[nodiscard]] BufferHandle CreateBuffer(const BufferDesc& desc, const void* initialData);

        /// Create new sampler.
        [[nodiscard]] virtual SamplerHandle CreateSampler(const SamplerDesc& desc) = 0;

        /// Create new shader.
        [[nodiscard]] virtual ShaderHandle CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main") = 0;

        /// Create new render pipeline.
        [[nodiscard]] virtual PipelineHandle CreateRenderPipeline(const RenderPipelineDesc& desc) = 0;

    private:
        bool VerifyTextureDesc(const TextureDesc& desc);
        virtual TextureHandle CreateTextureCore(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) = 0;
        virtual BufferHandle CreateBufferCore(const BufferDesc& desc, void* nativeHandle, const void* initialData) = 0;

    protected:
        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        bool vsyncEnabled = false;
    };

    /* Helper methods */
    enum class PixelFormatKind
    {
        Integer,
        Normalized,
        Float,
        DepthStencil
    };

    struct FormatInfo
    {
        Format format;
        const std::string name;
        uint8_t bytesPerBlock;
        uint8_t blockSize;
        PixelFormatKind kind;
        bool hasRed : 1;
        bool hasGreen : 1;
        bool hasBlue : 1;
        bool hasAlpha : 1;
        bool hasDepth : 1;
        bool hasStencil : 1;
        bool isSigned : 1;
        bool isSRGB : 1;
    };

    RHI_API extern const FormatInfo kFormatDesc[];
    RHI_API const FormatInfo& GetFormatInfo(Format format);

    /// Get the number of bits per format.
    constexpr uint32_t GetFormatBytesPerBlock(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].bytesPerBlock;
    }

    constexpr uint32_t GetFormatBlockSize(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].blockSize;
    }

    /// Check if the format has a depth component
    constexpr bool IsDepthFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasDepth;
    }

    /// Check if the format has a stencil component
    constexpr bool IsStencilFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].hasStencil;
    }

    /// Check if the format has depth or stencil components
    constexpr bool IsDepthStencilFormat(Format format)
    {
        return IsDepthFormat(format) || IsStencilFormat(format);
    }

    /// Check if the format is a compressed format
    constexpr bool IsBlockCompressedFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);

        switch (format)
        {
            case Format::BC1UNorm:
            case Format::BC1UNormSrgb:
            case Format::BC2UNorm:
            case Format::BC2UNormSrgb:
            case Format::BC3UNorm:
            case Format::BC3UNormSrgb:
            case Format::BC4UNorm:
            case Format::BC4SNorm:
            case Format::BC5UNorm:
            case Format::BC5SNorm:
            case Format::BC6HUFloat:
            case Format::BC6HSFloat:
            case Format::BC7UNorm:
            case Format::BC7UNormSrgb:
                return true;
        }

        return false;
    }

    /// Get the format Type
    constexpr PixelFormatKind GetFormatKind(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].kind;
    }

    constexpr const std::string& ToString(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].name;
    }

    /// Check if a format represents sRGB color space.
    constexpr bool IsSrgbFormat(Format format)
    {
        ALIMER_ASSERT(kFormatDesc[(uint32_t)format].format == format);
        return kFormatDesc[(uint32_t)format].isSRGB;
    }

    /// Convert a SRGB format to linear. If the format is already linear no conversion will be made.
    constexpr Format SRGBToLinearFormat(Format format)
    {
        switch (format)
        {
            case Format::BC1UNormSrgb:
                return Format::BC1UNorm;
            case Format::BC2UNormSrgb:
                return Format::BC2UNorm;
            case Format::BC3UNormSrgb:
                return Format::BC3UNorm;
            case Format::BGRA8UNormSrgb:
                return Format::BGRA8UNorm;
            case Format::RGBA8UNormSrgb:
                return Format::RGBA8UNorm;
            case Format::BC7UNormSrgb:
                return Format::BC7UNorm;
            default:
                ALIMER_ASSERT(IsSrgbFormat(format) == false);
                return format;
        }
    }

    /// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format no conversion will be made.
    constexpr Format LinearToSRGBFormat(Format format)
    {
        switch (format)
        {
            case Format::BC1UNorm:
                return Format::BC1UNormSrgb;
            case Format::BC2UNorm:
                return Format::BC2UNormSrgb;
            case Format::BC3UNorm:
                return Format::BC3UNormSrgb;
            case Format::BGRA8UNorm:
                return Format::BGRA8UNormSrgb;
            case Format::RGBA8UNorm:
                return Format::RGBA8UNormSrgb;
            case Format::BC7UNorm:
                return Format::BC7UNormSrgb;
            default:
                return format;
        }
    }

    RHI_API const char* ToString(CompareFunction func);
    RHI_API const char* ToString(TextureDimension dimension);

    inline const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case KnownVendorId_AMD:
                return "AMD";
            case KnownVendorId_ImgTec:
                return "IMAGINATION";
            case KnownVendorId_Nvidia:
                return "Nvidia";
            case KnownVendorId_ARM:
                return "ARM";
            case KnownVendorId_Qualcomm:
                return "Qualcom";
            case KnownVendorId_Intel:
                return "Intel";
            default:
                return "Unknown";
        }
    }

    template <class T>
    void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

#undef RHI_DEFINE_ENUM_BITWISE_OPERATORS

namespace std
{
    template<> struct hash<alimer::rhi::TextureSubresourceSet>
    {
        std::size_t operator()(const alimer::rhi::TextureSubresourceSet& set) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, set.baseMipLevel);
            alimer::rhi::hash_combine(hash, set.numMipLevels);
            alimer::rhi::hash_combine(hash, set.baseArraySlice);
            alimer::rhi::hash_combine(hash, set.numArraySlices);
            return hash;
        }
    };

    template<> struct hash<alimer::rhi::RenderTargetBlendState>
    {
        std::size_t operator()(const alimer::rhi::RenderTargetBlendState& state) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, state.blendEnable);
            alimer::rhi::hash_combine(hash, (uint32_t)state.srcBlend);
            alimer::rhi::hash_combine(hash, (uint32_t)state.destBlend);
            alimer::rhi::hash_combine(hash, (uint32_t)state.blendOp);
            alimer::rhi::hash_combine(hash, (uint32_t)state.srcBlendAlpha);
            alimer::rhi::hash_combine(hash, (uint32_t)state.destBlendAlpha);
            alimer::rhi::hash_combine(hash, (uint32_t)state.blendOpAlpha);
            alimer::rhi::hash_combine(hash, (uint8_t)state.writeMask);
            return hash;
        }
    };

    template<> struct hash<alimer::rhi::BlendState>
    {
        std::size_t operator()(const alimer::rhi::BlendState& state) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, state.alphaToCoverageEnable);
            alimer::rhi::hash_combine(hash, state.independentBlendEnable);
            for (const auto& target : state.renderTargets)
            {
                alimer::rhi::hash_combine(hash, target);
            }
            return hash;
        }
    };

    template<> struct hash<alimer::rhi::StencilFaceState>
    {
        std::size_t operator()(const alimer::rhi::StencilFaceState& state) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, (uint32_t)state.failOp);
            alimer::rhi::hash_combine(hash, (uint32_t)state.passOp);
            alimer::rhi::hash_combine(hash, (uint32_t)state.depthFailOp);
            alimer::rhi::hash_combine(hash, (uint32_t)state.compare);
            return hash;
        }
    };

    template<> struct hash<alimer::rhi::DepthStencilState>
    {
        std::size_t operator()(const alimer::rhi::DepthStencilState& state) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, state.depthWriteEnable);
            alimer::rhi::hash_combine(hash, (uint32_t)state.depthCompare);
            alimer::rhi::hash_combine(hash, state.stencilEnable);
            alimer::rhi::hash_combine(hash, state.stencilReadMask);
            alimer::rhi::hash_combine(hash, state.stencilWriteMask);
            alimer::rhi::hash_combine(hash, state.frontFace);
            alimer::rhi::hash_combine(hash, state.backFace);
            return hash;
        }
    };

    template<> struct hash<alimer::rhi::RasterizerState>
    {
        std::size_t operator()(const alimer::rhi::RasterizerState& state) const noexcept
        {
            size_t hash = 0;
            alimer::rhi::hash_combine(hash, (uint32_t)state.cullMode);
            alimer::rhi::hash_combine(hash, (uint32_t)state.frontFace);
            alimer::rhi::hash_combine(hash, (uint32_t)state.frontFace);
            alimer::rhi::hash_combine(hash, state.fillMode);
            alimer::rhi::hash_combine(hash, state.depthBias);
            alimer::rhi::hash_combine(hash, state.depthBiasSlopeScale);
            alimer::rhi::hash_combine(hash, state.depthBiasClamp);
            return hash;
        }
    };
}

