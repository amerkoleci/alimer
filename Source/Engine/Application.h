// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

//#include "Core/Main.h"
#include "PlatformDef.h"
#include <stdint.h>
#include <string>

namespace Alimer
{
    class GameHost;

    struct Config
    {
        std::string title = "Alimer";
        int32_t width = 1200;
        int32_t height = 800;
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

		void Tick();

		/// Request the game to exit.
		void Exit();

        // Gets the config data used to run the application
        const Config& GetConfig() const { return config; }

		/// Checks whether exit was requested.
        //[[nodiscard]] bool IsExitRequested() const noexcept { return exiting; }
        //[[nodiscard]] Window* GetMainWindow() const;

	protected:
		/// Constructor.
        Application();

		virtual void Initialize() {}
		virtual void Update();
		virtual void OnDraw(/* [[maybe_unused]] CommandBuffer* commandBuffer*/) {}

        virtual void BeginRun() {}
        virtual void EndRun() {}

        virtual bool BeginDraw();
        virtual void EndDraw();

    private:
        void PlatformInit();
        void PlatformShutdown();
        void PlatformRun();
        void HostReady();
        void HostExiting(int32_t exitCode);

        void InitializeBeforeRun();
        void Render();

	protected:
        Config config{};
        bool headless{ false };
		bool running{ false };
		bool paused{ false };
        bool exiting{ false };
        int32_t exitCode{ 0 };
        bool endRunRequired{ false };

    private:
        bool blockingRun{ true };
	};
}

#if !defined(ALIMER_DEFINE_APPLICATION)
#define ALIMER_DEFINE_APPLICATION(className) \
int RunApp() \
{ \
    std::unique_ptr<className> app = std::make_unique<className>();\
    return app->Run(); \
} \
ALIMER_DEFINE_MAIN(RunApp());
#endif
