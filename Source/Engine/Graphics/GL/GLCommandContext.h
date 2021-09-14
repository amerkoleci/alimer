// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandContext.h"
#include "GLBackend.h"

namespace Alimer
{
    class GLGraphicsDevice;

    class GLCommandContext final : public CommandContext
    {
    public:
        GLCommandContext(_In_ GLGraphicsDevice* device);
        ~GLCommandContext() override;

        void PushDebugGroup(const string_view& name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const string_view& name) override;

        void BeginDefaultRenderPass(const Color& clearColor, bool clearDepth = true, bool clearStencil = true, float depth = 1.0f, uint8_t stencil = 0) override;
        void BeginRenderPass(const RenderPassDesc& desc) override;
        void EndRenderPass() override;

        //virtual void SetViewport(const Rect& rect) = 0;
        void SetViewport(const Viewport& viewport) override;
        void SetViewports(const Viewport* viewports, uint32_t count) override;

        //virtual void SetScissorRect(const Rect& rect) = 0;
        //virtual void SetScissorRects(const Rect* rects, uint32_t count) = 0;

        void SetStencilReference(uint32_t value) override;
        void SetBlendColor(const Color& color) override;
        void SetBlendColor(const float blendColor[4]) override;

    private:
        GLGraphicsDevice* device;
    };
}

