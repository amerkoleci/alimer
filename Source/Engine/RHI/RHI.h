// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"

namespace Alimer::rhi
{
    /* Forward declarations */
    class IShader;
    class IPipeline;

    /* Structs */
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
        uint32_t                sampleCount = 1;
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

    class ALIMER_API IResource : public Object
    {
    protected:
        //[[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
    };

    class ALIMER_API IShader : public Object
    {
    public:
        [[nodiscard]] virtual ShaderStages GetStage() const = 0;
    };

    class ALIMER_API IPipeline : public Object
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
    using PipelineHandle = RefCountPtr<IPipeline>;

    /* Helper methods */
    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);

    ALIMER_API const char* ToString(CompareFunction func);
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
}

