// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"
#include "Graphics/Graphics.h"
#include "Math/Color.h"
#include "Core/Log.h"

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
        gLog().Start();

        PlatformConstruct();

        g_currentApp = this;
    }

    Application::~Application()
    {
        // Shutdown modules.
        gGraphics().WaitIdle();
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
        settings = SetupSettings();

        if (settings.graphicsApi == GraphicsAPI::Default)
        {
            // TODO: Handle best API per platform (Windows -> D3D11 etc)
            settings.graphicsApi = GraphicsAPI::OpenGL;
        }

        if (!PlatformSetup())
        {
            return false;
        }

        // Init graphics module
        PresentationParameters presentationParameters = {};
        //ValidationMode validationMode = ValidationMode::Disabled;
#if ALIMER_DEBUG
        //validationMode = ValidationMode::Enabled;
#endif

        if (!Graphics::Initialize(*window, presentationParameters))
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

        while (!window->ShouldClose())
        {
            PlatformUpdate();
            Render();
        }

        // Wait for pending GPU operations before shutdown.
        gGraphics().WaitIdle();

        running = false;
        return 0;
    }

    void Application::RequestExit()
    {
        exiting = true;
        paused = true;

        if (running)
        {
            window->Close();
            //OnExit(exitCode);

            running = false;
        }
    }

    void Application::Update()
    {
    }

    bool Application::BeginDraw()
    {
        return gGraphics().BeginFrame();
    }

    void Application::EndDraw()
    {
        gGraphics().EndFrame();

        if (settings.graphicsApi == GraphicsAPI::OpenGL)
        {
            window->SwapBuffers();
        }
    }

    void Application::Render()
    {
        // Don't try to render anything before the first Update or rendering is not allowed
        if (!exiting &&
            !window->IsMinimized() &&
            BeginDraw())
        {
            // Custom application draw.
            //ICommandList* commandList = GRHIDevice->BeginCommandList();
            //commandList->PushDebugGroup("Frame");
            //commandList->BeginRenderPass(window->GetSwapChain(), Color::CornflowerBlue);
            
            //OnDraw(commandList);
            
            //commandList->EndRenderPass();
            //commandList->PopDebugGroup();
            EndDraw();
        }
    }
}
