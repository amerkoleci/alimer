// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/CommandLine.h"
#include "Window.h"
#include "Graphics/CommandBuffer.h"

namespace Alimer
{
    struct Settings
    {
        std::string title = "Alimer";
        int32_t     width = 1200;
        int32_t     height = 800;
        bool        resizable = true;
        bool        fullscreen = false;
    };

    /// Class that provides graphics initialization, application logic, and rendering code.
    class ALIMER_API Application
    {
    public:
        /// Occurs when the game is about to exit.
        //Signal<int32_t> Exiting;

        /// Destructor.
        virtual ~Application();

        /// Gets the current Application instance.
        static Application* GetCurrent();

        /// Setups all subsystem and run's platform main loop.
        int32_t Run();

        /// Request the application to exit.
        void RequestExit();

        /// Checks whether exit was requested.
        [[nodiscard]] bool IsExitRequested() const { return exiting; }
        [[nodiscard]] Window* GetWindow() const { return window.get(); }

    protected:
        /// Constructor.
        Application();

        // Intial app settings. Override this to set defaults.
        virtual Settings SetupSettings()
        {
            return Settings();
        }

        virtual bool Initialize() { return true; }

        virtual void Update();
        virtual void OnDraw([[maybe_unused]] CommandBuffer& context) {}

        virtual void BeginRun() {}
        virtual void EndRun() {}

        virtual bool BeginDraw();
        virtual void EndDraw();

        std::unique_ptr<Window> window;
        Settings settings{};

    private:
        // Internal lifecycle methods
        bool InitBeforeRun();

        void PlatformConstruct();
        void PlatformShutdown();
        bool PlatformSetup();
        void PlatformUpdate();

        void Render();

        bool running{ false };
        bool exiting{ false };
        bool paused{ false };
    };
}

#if !defined(ALIMER_DEFINE_APPLICATION)

#if ALIMER_PLATFORM_WINDOWS && !defined(ALIMER_WIN32_CONSOLE)
#include "PlatformInclude.h"
#define ALIMER_DEFINE_APPLICATION(class_name) \
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) \
{ \
    UNREFERENCED_PARAMETER(hPrevInstance); \
    UNREFERENCED_PARAMETER(lpCmdLine); \
    Alimer::CommandLine::Parse(GetCommandLine());\
    class_name app;\
    return app.Run(); \
}
#else
#define ALIMER_DEFINE_APPLICATION(class_name) \
int main(int argc, char** argv) \
{ \
    Alimer::CommandLine::Parse(argc, argv);\
    class_name app;\
    return app.Run(); \
}
#endif
#endif
