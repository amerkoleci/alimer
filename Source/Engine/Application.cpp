// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"
#include "Core/Log.h"
#include "Graphics/Graphics.h"

namespace alimer
{
    namespace
    {
        static Application* g_currentApp = nullptr;

#ifdef __EMSCRIPTEN__
        void MainLoop(void* arg)
        {
            static_cast<Application*>(arg)->Tick();
        }
#endif
    }


    Application::Application()
    {
        //ALIMER_VERIFY_MSG(g_currentApp == nullptr, "Cannot create more than one Application");

         // Init log first.
        gLog().Start();

        PlatformConstruct();

        g_currentApp = this;
    }

    Application::~Application()
    {
        // Shutdown modules.
        //gGraphics().WaitIdle();
        gGraphics().Shutdown();
        //gAssets().Shutdown();
        window.reset();
        gLog().Shutdown();
        PlatformShutdown();
        g_currentApp = nullptr;
    }

    Application* Application::GetCurrent()
    {
        return g_currentApp;
    }

    bool Application::InitBeforeRun(int argc, const char* argv[])
    {
        // Defaults
        Settings settings = SetupSettings();

        if (!PlatformSetup(settings))
        {
            return false;
        }

        // Init graphics module
        if (!GraphicsInitialize(window.get()))
        {
            return false;
        }

        // Show window
        window->Show();

        // TODO: Setup rest
        if (!Initialize(argc, argv))
        {
            return false;
        }

        return true;
    }

    int Application::Run(int argc, const char* argv[])
    {
        if (running)
        {
            return EXIT_SUCCESS;
        }

        if (!InitBeforeRun(argc, argv))
        {
            return 1;
        }

        while (!IsExitRequested())
        {
            PlatformUpdate();
            Render();
        }

        running = false;
        return 0;
    }

    void Application::Update()
    {
    }

    void Application::Render()
    {
        // Don't try to render anything before the first Update or rendering is not allowed
        if (//frameCount > 0 &&
            //!host->GetMainWindow()->IsMinimized() &&
            BeginDraw())
        {
            // Custom application draw.
            //CommandBuffer* commandBuffer = gGraphics().BeginCommandBuffer();
            //{
            //    commandBuffer->PushDebugGroup("Frame");
            //
            //    RHI::RenderPassDescriptor renderPass;
            //    renderPass.colorAttachments[0].texture = host->GetMainWindow()->GetSwapChain()->GetCurrentTexture();
            //    renderPass.colorAttachments[0].loadAction = LoadAction::Clear;
            //    renderPass.colorAttachments[0].storeAction = StoreAction::Store;
            //    renderPass.colorAttachments[0].clearColor = { 0.1f, 0.2f, 0.3f, 1.0f };
            //
            //    if (host->GetMainWindow()->GetDepthStencilFormat() != PixelFormat::Undefined)
            //    {
            //        //renderPass.depthStencilAttachment.view = host->GetMainWindow()->GetDepthStencilTexture()->GetDefaultView();
            //        renderPass.depthStencilAttachment.clearDepth = 1.0f;
            //    }
            //
            //    commandBuffer->BeginRenderPass(renderPass);
            //
            //    OnDraw(commandBuffer);
            //
            //    // End main window render pass
            //    commandBuffer->EndRenderPass();
            //
            //    commandBuffer->PopDebugGroup();
            //}

            //gGraphics().GetQueue().Submit(commandBuffer);
            EndDraw();
        }
    }

    bool Application::BeginDraw()
    {
        return gGraphics().BeginFrame();
    }

    void Application::EndDraw()
    {
        gGraphics().EndFrame();
    }
}
