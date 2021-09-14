// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "GLGraphics.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "Core/Log.h"
#include "Window.h"

namespace Alimer
{
    namespace
    {
#ifndef __APPLE__
        void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
        {
            // these are basically never useful
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION &&
                type == GL_DEBUG_TYPE_OTHER)
            {
                return;
            }

            const char* typeName = "";
            const char* severityName = "";

            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR: typeName = "Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeName = "Deprecated behavior"; break;
            case GL_DEBUG_TYPE_MARKER: typeName = "Marker"; break;
            case GL_DEBUG_TYPE_OTHER: typeName = "Other"; break;
            case GL_DEBUG_TYPE_PERFORMANCE: typeName = "Performance"; break;
            case GL_DEBUG_TYPE_PORTABILITY: typeName = "Portability"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP: typeName = "Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP: typeName = "Pop Group"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeName = "Undefined behavior"; break;
            }

            switch (severity)
            {
            case GL_DEBUG_SEVERITY_HIGH: severityName = "HIGH"; break;
            case GL_DEBUG_SEVERITY_MEDIUM: severityName = "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_LOW: severityName = "LOW"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: severityName = "NOTIFICATION"; break;
            }

            if (type == GL_DEBUG_TYPE_ERROR)
            {
                if (severity == GL_DEBUG_SEVERITY_HIGH)
                {
                    LOGF("GL ({}:{}) {}", typeName, severityName, message);
                }
                else
                {
                    LOGE("GL ({}:{}) {}", typeName, severityName, message);
                }
            }
            else if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
            {
                LOGW("GL ({}:%{}) {}", typeName, severityName, message);
            }
            else
            {
                LOGI("GL ({}) {}", typeName, message);
            }
        }
#endif
    }

    GLGraphics::GLGraphics(Window& window, const GraphicsCreateInfo& createInfo)
        : Graphics(window)
    {
        // Core in version 4.3 or GLES 3.2
#ifndef __APPLE__
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_message_callback, nullptr);
#endif

        glViewport(0, 0, window.GetSize().x, window.GetSize().y);
    }

    GLGraphics::~GLGraphics()
    {

    }

    void GLGraphics::WaitIdle()
    {

    }

    bool GLGraphics::BeginFrame()
    {
        const float clearColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, clearColor);

        return true;
    }

    void GLGraphics::EndFrame()
    {

    }

    void GLGraphics::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    TextureRef GLGraphics::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef GLGraphics::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef GLGraphics::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef GLGraphics::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
