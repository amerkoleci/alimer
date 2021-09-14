// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "RHI/RHI.h"
#include "Graphics/Texture.h"
#include "Math/Viewport.h"

namespace Alimer
{
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

        rhi::ResourceStates initialState = rhi::ResourceStates::Unknown;
        rhi::ResourceStates finalState = rhi::ResourceStates::RenderTarget;

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

    //struct GPUAllocation
    //{
    //    /// The buffer associated with this memory.
    //    rhi::BufferHandle buffer = nullptr;
    //
    //    /// Offset from start of buffer resource.
    //    uint64_t offset = 0;
    //    /// Reserved size of this allocation.
    //    uint64_t size = 0;
    //    /// The CPU-writeable address.
    //    uint8_t* data = nullptr;
    //};

    class ALIMER_API CommandContext
    {
    protected:
        CommandContext() = default;
        virtual ~CommandContext() = default;

    public:
        // Non-copyable and non-movable
        CommandContext(const CommandContext&) = delete;
        CommandContext(const CommandContext&&) = delete;
        CommandContext& operator=(const CommandContext&) = delete;
        CommandContext& operator=(const CommandContext&&) = delete;

        virtual void PushDebugGroup(const string_view& name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const string_view& name) = 0;

        virtual void BeginDefaultRenderPass(const Color& clearColor, bool clearDepth = true, bool clearStencil = true, float depth = 1.0f, uint8_t stencil = 0) = 0;
        virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
        virtual void EndRenderPass() = 0;

        //virtual void SetViewport(const Rect& rect) = 0;
        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetViewports(const Viewport* viewports, uint32_t count) = 0;

        //virtual void SetScissorRect(const Rect& rect) = 0;
        //virtual void SetScissorRects(const Rect* rects, uint32_t count) = 0;

        virtual void SetStencilReference(uint32_t value) = 0;
        virtual void SetBlendColor(const Color& color) = 0;
        virtual void SetBlendColor(const float blendColor[4]) = 0;

        //virtual void SetPipeline(_In_ Pipeline* pipeline) = 0;
        //virtual void SetVertexBuffer(uint32_t index, const rhi::IBuffer* buffer) = 0;
        //virtual void SetIndexBuffer(const rhi::IBuffer* buffer, uint64_t offset, IndexType indexType) = 0;
        //virtual void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) = 0;
        //virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t baseInstance = 0) = 0;
        //
        //GPUAllocation AllocateGPU(uint64_t size, uint64_t alignment);
        //
        //void BindConstantBuffer(uint32_t binding, const rhi::IBuffer* buffer);
        //void BindConstantBuffer(uint32_t binding, const rhi::IBuffer* buffer, uint64_t offset, uint64_t range);
        //void BindConstantBufferData(uint32_t binding, uint32_t size, const void* data);
        //
        //template<typename T>
        //void BindConstantBufferData(const T& data, uint32_t binding)
        //{
        //    GPUAllocation allocation = AllocateGPU(sizeof(T), 256);
        //    std::memcpy(allocation.data, &data, sizeof(T));
        //    BindConstantBufferCore(binding , allocation.buffer, allocation.offset, allocation.size);
        //}

    private:
        //virtual void BindConstantBufferCore(uint32_t binding, const rhi::IBuffer* buffer, uint64_t offset, uint64_t range) = 0;

        //struct FrameAllocator
        //{
        //    rhi::BufferHandle buffer;
        //    // Current offset, it increases on every allocation
        //    uint64_t currentOffset = 0;
        //    uint64_t frameIndex = 0;
        //} frameAllocators[kMaxFramesInFlight];
    };
}

