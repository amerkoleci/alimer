// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"

namespace Alimer::rhi
{
    /* Structs */
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

        virtual void SetPipeline(_In_ Pipeline* pipeline) = 0;
        virtual void SetVertexBuffer(uint32_t index, const Buffer* buffer) = 0;
        virtual void SetIndexBuffer(const Buffer* buffer, uint64_t offset, IndexType indexType) = 0;
        virtual void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t baseInstance = 0) = 0;
    };
}

