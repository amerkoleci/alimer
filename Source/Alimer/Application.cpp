// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Core/Timer.h"
#include "Alimer/Application.h"
#include "Alimer/Platform/AppPlatform.h"
#include <thread>

using namespace Alimer;

std::thread::id mainThreadId = std::this_thread::get_id();
Application* Application::s_Instance = nullptr;

Application& Application::Get()
{
    return *s_Instance;
}

bool Application::IsMainThread()
{
    return std::this_thread::get_id() == mainThreadId;
}

Application::Application()
{
    ALIMER_ASSERT_MSG(s_Instance == nullptr, "Cannot create more than one Application");
    Log::Init();
    //JobSystem::Initialize();
    mainThreadId = std::this_thread::get_id();
    _platform = AppPlatform::Create(this);

    // Init platform first
    if (!_platform)
    {
        LOGF("Failed to initialize platform");
        return;
    }

    s_Instance = this;
}

Application::~Application()
{
    Log::Shutdown();
#if TODO
    //JobSystem::Shutdown();
#endif
    SafeDelete(_platform);
    s_Instance = nullptr;
}

[[nodiscard]] Window* Application::GetMainWindow() const
{
    return _platform->GetMainWindow();
}

void Application::ResetElapsedTime()
{
    _timer.ResetElapsedTime();
}

void Application::Run()
{
    ALIMER_ASSERT_MSG(!_running, "The Application is already running");

    // Allow config override.
    Setup();

    //if (settings.graphics.preferredApi == GraphicsAPI::Count)
    //{
    //    settings.graphics.preferredApi = RHIGetPlatformPreferredApi();
    //}

    // Run platform main loop.
    _platform->RunMainLoop();

    _running = false;
}

void Application::Tick()
{
    _timer.Tick();
    DoUpdate();
    Render();
}

void Application::RequestExit()
{
    _platform->RequestExit();
}

void Application::DoUpdate()
{
    const float elapsedTime = float(_timer.GetElapsedSeconds());
    ALIMER_UNUSED(elapsedTime);

    Update();
}

void Application::InitBeforeRun()
{
    // We're ready, now init.
    Initialize();
    _running = true;

    LOGI("{} Engine v{} initialized", ENGINE_NAME, ALIMER_VERSION_STR);
}

void Application::Render()
{
    // Don't try to render anything before the first Update.
    if (_timer.GetFrameCount() == 0)
    {
        return;
    }
}

bool Application::BeginDraw()
{
    return false; // !mainWindow->IsMinimized() && !GRHIDevice->IsDeviceLost();
}

void Application::EndDraw()
{
    //GRHIDevice->CommitFrame();
}
