// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Window.h"
#include "Alimer/Core/Timer.h"
#include "Alimer/RHI/RHI.h"
//#include "Alimer/Animations/AnimationSystem.h"
//#include "Alimer/Scene/Scene.h"

namespace Alimer
{
    struct WindowDesc final
    {
        std::string title = "Alimer";

        //Int2 position = { WindowPositionCentered, WindowPositionCentered };
        //SizeI size = { 1280, 720 };
        uint32_t width = 1280;
        uint32_t height = 720;
        WindowFlags flags = WindowFlags::Resizable | WindowFlags::Hidden;
    };

    struct AppOptions final
    {
        /// Application name.
        std::string name = "Alimer";
        WindowDesc window;
    };

    class AppPlatform;

    /// Class that provides graphics initialization, application logic, and rendering code.
    class ALIMER_API Application : public Object
    {
        ALIMER_OBJECT(Application, Object);
        friend class AppPlatform;

    public:
        /// Occurs when the application is activated.
        Signal<> Activated;

        /// Occurs when the application is deactivated.
        Signal<> Deactivated;

        /// Destructor.
        virtual ~Application();

        ALIMER_DISABLE_COPY_MOVE(Application);

        static Application& Get();
        static bool IsMainThread();

        /// Setups all subsystem and run's platform main loop.
        void Run();

        /// Tick one frame.
        void Tick();

        /// Request exit application.
        void RequestExit();

        /// Reset the elapsed time counter.
        void ResetElapsedTime();

        [[nodiscard]] const AppOptions& GetOptions() const { return _options; }

        /// Gets the main window.
        [[nodiscard]] Window* GetMainWindow() const;

        /// Gets the main Scene.
        //[[nodiscard]] Scene* GetScene() const { return _scene.Get(); }

    protected:
        /// Constructor.
        Application();

        virtual void Setup() {}
        virtual void Initialize() {}
        virtual void Update() {}
        //virtual void OnDraw([[maybe_unused]] GraphicsContext& context, [[maybe_unused]] const RenderPassDesc& mainRenderPass) {}
        virtual bool BeginDraw();
        virtual void EndDraw();

        //virtual void OnActivated();
        //virtual void OnDeactivated();
        virtual void OnGui() {}

        AppOptions _options{};
        bool _running{ false };
        bool _isActive{ false };
        bool _headless{ false };
        Timer _timer;

    private:
        void InitBeforeRun();
        void DoUpdate();
        void Render();

        static Application* s_Instance;
        AppPlatform* _platform = nullptr;
        RHIFactoryRef _rhiFactory = nullptr;
        RHIAdapter* _rhiAdapter = nullptr;
        RHIDevice* _rhiDevice = nullptr;
    };
}
