// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "GLCommandContext.h"
#include "GLGraphics.h"
#include "Core/StringUtils.h"
#include "Core/Log.h"

namespace Alimer
{
    namespace
    {
    }

    GLCommandContext::GLCommandContext(_In_ GLGraphicsDevice* device)
        : device{ device }
    {
    }

    GLCommandContext::~GLCommandContext()
    {
    }

    void GLCommandContext::PushDebugGroup(const string_view& name)
    {
        ALIMER_UNUSED(name);
    }

    void GLCommandContext::PopDebugGroup()
    {
    }

    void GLCommandContext::InsertDebugMarker(const string_view& name)
    {
        ALIMER_UNUSED(name);
    }

    void GLCommandContext::BeginDefaultRenderPass(const Color& clearColor, bool clearDepth, bool clearStencil, float depth, uint8_t stencil)
    {
        const u32 width = device->GetBackBufferWidth();
        const u32 height = device->GetBackBufferHeight();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearBufferfv(GL_COLOR, 0, clearColor.data);

        // Enable states for depth-stencil clear
        if (clearDepth)
        {
            glDepthMask(GL_TRUE);
        }

        if (clearStencil)
        {
            glStencilMask(0);
        }

        if (clearDepth && clearStencil)
        {
            glClearBufferfi(GL_DEPTH_STENCIL, 0, clearDepth, stencil);
        }
        else if (clearDepth)
        {
            glClearBufferfv(GL_DEPTH, 0, &depth);
        }
        else if (clearStencil)
        {
            const GLint glClearStencil = stencil;
            glClearBufferiv(GL_STENCIL, 0, &glClearStencil);
        }

        // The viewport and scissor default to cover main window.
        glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
        glViewport(0, 0, width, height);
        glDepthRangef(0.0f, 1.0f);
        glScissor(0, 0, width, height);
    }

    void GLCommandContext::BeginRenderPass(const RenderPassDesc& desc)
    {

    }

    void GLCommandContext::EndRenderPass()
    {

    }

    void GLCommandContext::SetViewport(const Viewport& viewport)
    {
        //glViewportIndexedf();
        glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
            static_cast<GLsizei>(viewport.width), static_cast<GLint>(viewport.height)
        );

        glDepthRangef(viewport.minDepth, viewport.maxDepth);
    }

    void GLCommandContext::SetViewports(const Viewport* viewports, uint32_t count)
    {

    }

    void GLCommandContext::SetStencilReference(uint32_t value)
    {

    }

    void GLCommandContext::SetBlendColor(const Color& color)
    {

    }

    void GLCommandContext::SetBlendColor(const float blendColor[4])
    {

    }
}
