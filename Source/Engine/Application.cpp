// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"
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

        static inline void RHILogFunction(RHI::LogLevel level, const char* message)
        {
            switch (level)
            {
                case RHI::LogLevel::Info:
                    LOGI("RHI: {}", message);
                    break;
                case RHI::LogLevel::Warn:
                    LOGW("RHI: {}", message);
                    break;
                case RHI::LogLevel::Debug:
                    LOGD("RHI: {}", message);
                    break;
                case RHI::LogLevel::Error:
                    LOGE("RHI: {}", message);
                    break;
                default:
                    break;
            }
        }
    }

    RHI::DeviceHandle GRHIDevice;

    Application::Application()
    {
        //ALIMER_VERIFY_MSG(g_currentApp == nullptr, "Cannot create more than one Application");

         // Init log first.
        gLog().Start();

        RHI::SetLogFunction(RHILogFunction);

        PlatformConstruct();

        g_currentApp = this;
    }

    Application::~Application()
    {
        // Shutdown modules.
        GRHIDevice->WaitIdle();
        GRHIDevice.Reset();
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
        RHI::ValidationMode validationMode = RHI::ValidationMode::Disabled;
#if ALIMER_DEBUG
        validationMode = RHI::ValidationMode::Enabled;
#endif

        GRHIDevice = RHI::CreateDevice(RHI::GraphicsAPI::Vulkan, validationMode);
        if (!GRHIDevice)
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
        GRHIDevice->WaitIdle();

        running = false;
        return 0;
    }

    void Application::Update()
    {
    }

    bool Application::BeginDraw()
    {
        return GRHIDevice->BeginFrame();
    }

    void Application::EndDraw()
    {
        GRHIDevice->EndFrame();
    }

    void Application::Render()
    {
        // Don't try to render anything before the first Update or rendering is not allowed
        if (!exitRequested &&
            !window->IsMinimized() &&
            BeginDraw())
        {
            // Custom application draw.
            RHI::ICommandList* commandList = GRHIDevice->BeginCommandList();
            commandList->PushDebugGroup("Frame");

            commandList->BeginRenderPass(window->GetSwapChain(), Color::CornflowerBlue);
            
            OnDraw(commandList);
            
            commandList->EndRenderPass();
            commandList->PopDebugGroup();
            EndDraw();
        }
    }
}
