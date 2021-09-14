// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D11CommandContext.h"
#include "D3D11Graphics.h"
#include "Core/StringUtils.h"
#include "Core/Log.h"

namespace Alimer
{
    namespace
    {
        static_assert(sizeof(Alimer::Viewport) == sizeof(D3D11_VIEWPORT), "Size mismatch");
        static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D11_VIEWPORT, TopLeftX), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D11_VIEWPORT, TopLeftY), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D11_VIEWPORT, Width), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D11_VIEWPORT, Height), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D11_VIEWPORT, MinDepth), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D11_VIEWPORT, MaxDepth), "Layout mismatch");
    }

    D3D11CommandContext::D3D11CommandContext(_In_ D3D11GraphicsDevice* device, _In_ ID3D11DeviceContext1* context)
        : device{ device }
        , context{ context }
    {
        ThrowIfFailed(context->QueryInterface(IID_PPV_ARGS(&annotation)));
    }

    D3D11CommandContext::~D3D11CommandContext()
    {
        SafeRelease(annotation);
        SafeRelease(context);
    }

    void D3D11CommandContext::PushDebugGroup(const string_view& name)
    {
        auto wide = ToUtf16(name);
        annotation->BeginEvent(wide.c_str());
    }

    void D3D11CommandContext::PopDebugGroup()
    {
        annotation->EndEvent();
    }

    void D3D11CommandContext::InsertDebugMarker(const string_view& name)
    {
        auto wide = ToUtf16(name);
        annotation->SetMarker(wide.c_str());
    }

    void D3D11CommandContext::BeginDefaultRenderPass(const Color& clearColor, bool clearDepth, bool clearStencil, float depth, uint8_t stencil)
    {
        const u32 width = device->GetBackBufferWidth();
        const u32 height = device->GetBackBufferHeight();

        auto rtv = device->GetBackBufferView();
        auto dsv = device->GetDepthStencilView();

        context->ClearRenderTargetView(rtv, clearColor.data);
        context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        context->OMSetRenderTargets(1, &rtv, dsv);

        // The viewport and scissor default to cover all of the attachments
        const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
        const D3D11_RECT scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

        context->RSSetViewports(1, &viewport);
        context->RSSetScissorRects(1, &scissorRect);
    }

    void D3D11CommandContext::BeginRenderPass(const RenderPassDesc& desc)
    {

    }

    void D3D11CommandContext::EndRenderPass()
    {

    }

    void D3D11CommandContext::SetViewport(const Viewport& viewport)
    {
        context->RSSetViewports(1, (D3D11_VIEWPORT*)&viewport);
    }

    void D3D11CommandContext::SetViewports(const Viewport* viewports, uint32_t count)
    {

    }

    void D3D11CommandContext::SetStencilReference(uint32_t value)
    {

    }

    void D3D11CommandContext::SetBlendColor(const Color& color)
    {

    }

    void D3D11CommandContext::SetBlendColor(const float blendColor[4])
    {

    }
}
