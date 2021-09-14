// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandContext.h"
#include "D3D11Backend.h"

namespace Alimer
{
    class D3D11GraphicsDevice;

    class D3D11CommandContext final : public CommandContext
    {
    public:
        D3D11CommandContext(_In_ D3D11GraphicsDevice* device, _In_ ID3D11DeviceContext1* context);
        ~D3D11CommandContext() override;

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

        auto GetHandle() const noexcept { return context; }

    private:
        D3D11GraphicsDevice* device;
        ID3D11DeviceContext1* context;
        ID3DUserDefinedAnnotation* annotation;
    };
}

