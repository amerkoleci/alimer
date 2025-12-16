// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once


#include "Alimer/Platform/Window.h"
//#include "Alimer/Timer.h"
//#include "Alimer/Animations/AnimationSystem.h"
//#include "Alimer/Scene/Scene.h"

namespace Alimer
{
    struct AppOptions final
    {
        /// Application name.
        std::string name = "Alimer";
    };

    /// Class that provides graphics initialization, application logic, and rendering code.
    class ALIMER_API Application : public Object
    {
        ALIMER_OBJECT(Application, Object);

    public:
        /// Occurs when the application is activated.
        Signal<> Activated;

        /// Occurs when the application is deactivated.
        Signal<> Deactivated;

        ALIMER_DISABLE_COPY_MOVE(Application)

        /// Destructor.
        virtual ~Application();

        static Application& Get();
        static bool IsMainThread();

        /// Setups all subsystem and run's platform main loop.
        void Run();

        /// Tick one frame.
        void Tick();

        /// Request exit application.
        void RequestExit();

        void ResetElapsedTime();

        [[nodiscard]] const AppOptions& GetOptions() const { return _options; }

        /// Gets the main window.
        //[[nodiscard]] Window* GetMainWindow() const { return mainWindow.get(); }

        /// Gets the main Scene.
        //[[nodiscard]] Scene* GetScene() const { return _scene.Get(); }

        /// Gets the Scene Renderer.
        //SceneRenderer* GetSceneRenderer() const { return _scene->GetRenderer(); }

    protected:
        /// Constructor.
        Application();

        virtual void Setup() {}
        virtual void Initialize() {}
        //virtual void Update([[maybe_unused]] const Timer& timer) {}
        //virtual void OnDraw([[maybe_unused]] GraphicsContext& context, [[maybe_unused]] const RenderPassDesc& mainRenderPass) {}
        virtual bool BeginDraw();
        virtual void EndDraw();

        //virtual void OnActivated();
        //virtual void OnDeactivated();
        virtual void OnGui() {}

        AppOptions _options{};
        bool _running{ false };
        bool _exitRequested{ false };
        bool _isActive{ false };
        bool _headless{ false };

        /// Main scene.
        //SceneRef _scene;

    private:
        void InitBeforeRun();
        void DoUpdate();
        void Render();

        //bool PlatformInit();
        //void PlatformShutdown();
        //void PlatformRun();

        static Application* s_Instance;

        //std::unique_ptr<Window> mainWindow;
        // Rendering loop timer.
        //Timer timer;

        // Game systems
        //std::unique_ptr<AnimationSystem> animationSystem;
    };
}
