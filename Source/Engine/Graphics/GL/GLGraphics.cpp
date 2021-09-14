// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "GLGraphics.h"
#include "GLCommandContext.h"
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

    GL_Buffer::~GL_Buffer()
    {
        glDeleteBuffers(1, &id);
    }

    GLGraphicsDevice::GLGraphicsDevice(Window& window, const GraphicsCreateInfo& createInfo)
        : Graphics(window)
    {
        // Core in version 4.3 or GLES 3.2
#ifndef __APPLE__
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(gl_message_callback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
#endif

        // Create main context.
        mainContext = std::make_unique<GLCommandContext>(this);

        glViewport(0, 0, window.GetSize().x, window.GetSize().y);
    }

    GLGraphicsDevice::~GLGraphicsDevice()
    {
        mainContext.reset();
    }

    CommandContext* GLGraphicsDevice::GetImmediateContext() const
    {
        return mainContext.get();
    }

    void GLGraphicsDevice::WaitIdle()
    {

    }

    bool GLGraphicsDevice::BeginFrame()
    {
        return true;
    }

    void GLGraphicsDevice::EndFrame()
    {

    }

    void GLGraphicsDevice::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    bool GLGraphicsDevice::CreateBuffer(const BufferDesc* desc, const void* initialData, GPUBuffer* pBuffer) const
    {
        auto internalState = std::make_shared<GL_Buffer>();
        pBuffer->internalState = internalState;

        glGenBuffers(1, &internalState->id);
        glBindBuffer(GL_ARRAY_BUFFER, internalState->id);
        glBufferData(GL_ARRAY_BUFFER, desc->size, initialData, GL_STATIC_DRAW);

        //auto mMappedData = glMapBufferRange(GL_ARRAY_BUFFER, 0, desc->size, GL_MAP_WRITE_BIT);

        return true;
    }

    TextureRef GLGraphicsDevice::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef GLGraphicsDevice::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef GLGraphicsDevice::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef GLGraphicsDevice::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
