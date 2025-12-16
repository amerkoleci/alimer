// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Application.h"

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

#if TODO
    if (!PlatformInit())
    {
        LOGF("Failed to initialize platform");
        return;
    }
#endif 

    s_Instance = this;
}

Application::~Application()
{
    Log::Shutdown();
#if TODO
    //JobSystem::Shutdown();
    PlatformShutdown();
#endif
    s_Instance = nullptr;
}

void Application::ResetElapsedTime()
{
    //timer.ResetElapsedTime();
}

void Application::Run()
{
    ALIMER_ASSERT_MSG(!_running, "The Application is already running");

    // Allow config override.
    Setup();

    //if (settings.window.title.empty())
    //{
    //    settings.window.title = settings.name;
    //}

    //if (settings.graphics.preferredApi == GraphicsAPI::Count)
    //{
    //    settings.graphics.preferredApi = RHIGetPlatformPreferredApi();
    //}

#if TODO
    // Run platform main loop.
    PlatformRun();
#endif

    _running = false;
    _exitRequested = false;
}

void Application::Tick()
{
    Render();
}

void Application::RequestExit()
{
    _exitRequested = true;
}

void Application::DoUpdate()
{
}

void Application::InitBeforeRun()
{
    _running = true;
    _exitRequested = false;

    LOGI("{} Engine v{} initialized", ENGINE_NAME, ALIMER_VERSION_STR);
}

void Application::Render()
{
}

bool Application::BeginDraw()
{
    return false; // !mainWindow->IsMinimized() && !GRHIDevice->IsDeviceLost();
}

void Application::EndDraw()
{
    //GRHIDevice->CommitFrame();
}
