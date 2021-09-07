// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"
#include "Graphics/Graphics.h"
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
        Settings settings = SetupSettings();

        if (!PlatformSetup(settings))
        {
            return false;
        }

        // Init graphics module
        PresentationParameters presentationParameters = {};
#if ALIMER_DEBUG
        presentationParameters.validationMode = ValidationMode::Enabled;
#endif

        if (!Graphics::Initialize(window.get(), presentationParameters))
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

        // Wait for pending GPU operations before shutdown.
        gGraphics().WaitIdle();

        running = false;
        return 0;
    }

    void Application::Update()
    {
    }

    void Application::Render()
    {
        // Don't try to render anything before the first Update or rendering is not allowed
        if (//rhiDevice->GetFrameCount() > 0 &&
            !window->IsMinimized()
            )
        {
            // Custom application draw.
            rhi::ICommandList* commandList = gGraphics().BeginFrame();
            if (commandList == nullptr)
            {
                // Cannot draw
                return;
            }

            commandList->PushDebugGroup("Frame");
            commandList->BeginDefaultRenderPass(Colors::CornflowerBlue, true, false);

            OnDraw(commandList);

            commandList->EndRenderPass();
            commandList->PopDebugGroup();
            gGraphics().EndFrame();
        }
    }
}
