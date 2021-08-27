// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Window.h"
#include "RHI/RHI.h"

namespace alimer
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
        Signal<int32_t> Exiting;

        /// Destructor.
        virtual ~Application();

        /// Gets the current Application instance.
        static Application* GetCurrent();

        /// Setups all subsystem and run's platform main loop.
        int32_t Run(int argc, const char* argv[]);

        /// Request the application to exit.
        void RequestExit();

        /// Checks whether exit was requested.
        [[nodiscard]] bool IsExitRequested() const;
        [[nodiscard]] Window* GetWindow() const { return window.get(); }

    protected:
        /// Constructor.
        Application();

        // Intial app settings. Override this to set defaults.
        virtual Settings SetupSettings()
        {
            return Settings();
        }

        virtual bool Initialize(int argc, const char* argv[]) { return true; }

        virtual void Update();
        virtual void OnDraw([[maybe_unused]] rhi::ICommandList* commandList) {}

        virtual void BeginRun() {}
        virtual void EndRun() {}

        std::unique_ptr<Window> window;
        rhi::DeviceHandle rhiDevice;

    private:
        // Internal lifecycle methods
        bool InitBeforeRun(int argc, const char* argv[]);

        void PlatformConstruct();
        void PlatformShutdown();
        bool PlatformSetup(const Settings& settings);
        void PlatformUpdate();

        void Render();

        bool running{ false };
    };
}

#if !defined(ALIMER_DEFINE_APPLICATION)
#define ALIMER_DEFINE_APPLICATION(class_name) \
int main(int argc, const char* argv[]) \
{  \
    class_name app;\
    return app.Run(argc, argv); \
}
#endif
