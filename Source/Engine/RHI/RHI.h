// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"

namespace Alimer
{
    class Window;
}

namespace Alimer::rhi
{
    /// Number of MSAA samples to use. 1xMSAA and 4xMSAA are most broadly supported
    enum class SampleCount : uint32_t
    {
        Count1 = 1,
        Count2 = 2,
        Count4 = 4,
        Count8 = 8,
        Count16 = 16,
        Count32 = 32,
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
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ShaderStages);

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

    enum class ColorWriteMask : uint8_t
    {
        None = 0,
        Red = 0x01,
        Green = 0x02,
        Blue = 0x04,
        Alpha = 0x08,
        All = 0x0F
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ColorWriteMask);

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
    class ISampler;
    class IShader;
    class IPipeline;

    /* Structs */
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
        VertexFormat format = VertexFormat::Undefined;
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
        std::string label;
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
        PixelFormat             colorFormats[kMaxColorAttachments] = {};
        PixelFormat             depthStencilFormat = PixelFormat::Undefined;
        SampleCount             sampleCount = SampleCount::Count1;
    };

    struct RenderPassColorAttachment
    {
        Texture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t slice = 0;

        Texture* resolveTexture = nullptr;
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
        Texture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t slice = 0;

        Texture* resolveTexture = nullptr;
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

    /* Objects */
    class ALIMER_API Object : public RefCounted
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
            ApiSetName();
        }

    protected:
        /// Constructor.
        Object() = default;

        virtual void ApiSetName() {}
        std::string name;
    };

    class ALIMER_API DeviceChild : public Object
    {
    public:
    };

    class ALIMER_API IResource : public DeviceChild
    {
    protected:
        //[[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
    };

    class ALIMER_API ISampler : public DeviceChild
    {
    public:
        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;
        //[[nodiscard]] virtual uint32_t GetBindlessIndex() const = 0;
    };

    class ALIMER_API IShader : public DeviceChild
    {
    public:
        [[nodiscard]] virtual ShaderStages GetStage() const = 0;
    };

    class ALIMER_API IPipeline : public DeviceChild
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
        virtual void SetVertexBuffer(uint32_t index, const Buffer* buffer) = 0;
        virtual void SetIndexBuffer(const Buffer* buffer, uint64_t offset, IndexType indexType) = 0;
        virtual void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t baseInstance = 0) = 0;
    };

    using ShaderHandle = RefCountPtr<IShader>;
    using SamplerHandle = RefCountPtr<ISampler>;
    using PipelineHandle = RefCountPtr<IPipeline>;

    /* Helper methods */
    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);

    

    ALIMER_API const char* ToString(CompareFunction func);
    ALIMER_API const char* ToString(SamplerFilter filter);
    ALIMER_API const char* ToString(SamplerAddressMode mode);
    ALIMER_API const char* ToString(SamplerBorderColor borderColor);

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
    template<> struct hash<Alimer::rhi::TextureSubresourceSet>
    {
        std::size_t operator()(const Alimer::rhi::TextureSubresourceSet& set) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, set.baseMipLevel);
            Alimer::HashCombine(hash, set.numMipLevels);
            Alimer::HashCombine(hash, set.baseArraySlice);
            Alimer::HashCombine(hash, set.numArraySlices);
            return hash;
        }
    };

    template<> struct hash<Alimer::rhi::RenderTargetBlendState>
    {
        std::size_t operator()(const Alimer::rhi::RenderTargetBlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::rhi::hash_combine(hash, state.blendEnable);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.srcBlend);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.destBlend);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.blendOp);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.srcBlendAlpha);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.destBlendAlpha);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.blendOpAlpha);
            Alimer::rhi::hash_combine(hash, (uint8_t)state.writeMask);
            return hash;
        }
    };

    template<> struct hash<Alimer::rhi::BlendState>
    {
        std::size_t operator()(const Alimer::rhi::BlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::rhi::hash_combine(hash, state.alphaToCoverageEnable);
            Alimer::rhi::hash_combine(hash, state.independentBlendEnable);
            for (const auto& target : state.renderTargets)
            {
                Alimer::rhi::hash_combine(hash, target);
            }
            return hash;
        }
    };

    template<> struct hash<Alimer::rhi::StencilFaceState>
    {
        std::size_t operator()(const Alimer::rhi::StencilFaceState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::rhi::hash_combine(hash, (uint32_t)state.failOp);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.passOp);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.depthFailOp);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.compare);
            return hash;
        }
    };

    template<> struct hash<Alimer::rhi::DepthStencilState>
    {
        std::size_t operator()(const Alimer::rhi::DepthStencilState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::rhi::hash_combine(hash, state.depthWriteEnable);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.depthCompare);
            Alimer::rhi::hash_combine(hash, state.stencilReadMask);
            Alimer::rhi::hash_combine(hash, state.stencilWriteMask);
            Alimer::rhi::hash_combine(hash, state.frontFace);
            Alimer::rhi::hash_combine(hash, state.backFace);
            return hash;
        }
    };

    template<> struct hash<Alimer::rhi::RasterizerState>
    {
        std::size_t operator()(const Alimer::rhi::RasterizerState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::rhi::hash_combine(hash, (uint32_t)state.cullMode);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.frontFace);
            Alimer::rhi::hash_combine(hash, (uint32_t)state.frontFace);
            Alimer::rhi::hash_combine(hash, state.fillMode);
            Alimer::rhi::hash_combine(hash, state.depthBias);
            Alimer::rhi::hash_combine(hash, state.depthBiasSlopeScale);
            Alimer::rhi::hash_combine(hash, state.depthBiasClamp);
            return hash;
        }
    };
}

