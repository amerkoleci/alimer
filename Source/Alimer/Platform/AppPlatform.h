// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"

namespace Alimer
{
    class Window;
    class Application;

    class ALIMER_API AppPlatform
    {
        friend class Application;

    public:
        virtual ~AppPlatform() = default;

        static AppPlatform* Create(Application* app);

        /// Runs the main loop of the application.
        virtual void RunMainLoop() = 0;

        /// Requests the application to exit.
        virtual void RequestExit() = 0;

        /// Gets the main window.
        [[nodiscard]] Window* GetMainWindow() const { return _mainWindow.get(); }

    protected:
        AppPlatform(Application* app)
            : _app(app)
        {
        }

        /// <summary>
        /// Called when the platform is ready, window has been created and application systems can be initialized.
        /// </summary>
        virtual void OnReady();

        Application* _app;
        std::unique_ptr<Window> _mainWindow;
    };
}
