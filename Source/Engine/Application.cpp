// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"

namespace Alimer
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
        //gLog().Start();

        // Initialize platform host.
        PlatformInit();
        //host->Ready.ConnectMember(this, &Game::HostReady);
        //host->Exiting.ConnectMember(this, &Game::HostExiting);

        g_currentApp = this;
    }

    Application::~Application()
    {
        // Shutdown modules.
        //gGraphics().WaitIdle();
        //gGraphics().Shutdown();
        //gAssets().Shutdown();
        PlatformShutdown();
        //gLog().Shutdown();
        g_currentApp = nullptr;
    }

    Application* Application::GetCurrent()
    {
        return g_currentApp;
    }

    //Window* Game::GetMainWindow() const
    //{
    //    return host->GetMainWindow();
    //}

    int Application::Run()
    {
        if (running)
        {
            return EXIT_SUCCESS;
        }

#if !defined(__GNUC__) || __EXCEPTIONS
        try
        {
#endif
            PlatformRun();

            if (blockingRun)
            {
                // If the previous call was blocking, then we can call Endrun
                EndRun();
            }
            else
            {
                // EndRun will be executed on Exit
                endRunRequired = true;
            }

#if !defined(__GNUC__) || __EXCEPTIONS
        }
        catch (std::bad_alloc&)
        {
            //ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
            return EXIT_FAILURE;
        }
#endif

        if (!endRunRequired)
        {
            running = false;
        }

        return exitCode;
    }

    void Application::Tick()
    {
        Render();
    }

    void Application::Update()
    {
    }

    void Application::HostReady()
    {
        InitializeBeforeRun();
    }

    void Application::HostExiting(int32_t exitCode_)
    {
        exitCode = exitCode_;
        //Exiting.Emit(exitCode_);
    }

    void Application::InitializeBeforeRun()
    {
        // Platform logic has been setup and main window has been created.

        // Init modules.
        //gAssets().Start(config.assetsDirectory);
        //
        //// Init graphics module
        //if (!Graphics::Initialize(config.validationMode, config.backendType))
        //{
        //    headless = true;
        //}
        //else
        //{
        //
        //}

        // Show main window.
        //host->GetMainWindow()->Show();

        Initialize();
    }

    void Application::Render()
    {
        // Don't try to render anything before the first Update or rendering is not allowed
        if (!exiting &&
            //frameCount > 0 &&
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
        return true; // gGraphics().BeginFrame();
    }

    void Application::EndDraw()
    {
        //gGraphics().EndFrame();
    }

    void Application::Exit()
    {
        exiting = true;
        //host->Exit();
        if (running && endRunRequired)
        {
            EndRun();
            running = false;
        }
    }
}
