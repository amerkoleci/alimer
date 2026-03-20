// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Core/Timer.h"
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

    // Init platform first
    if (!PlatformInit())
    {
        LOGF("Failed to initialize platform");
        return;
    }

    s_Instance = this;
}

Application::~Application()
{
    _rhiDevice->WaitIdle();

    _rhiDevice.Reset();
    Log::Shutdown();
#if TODO
    //JobSystem::Shutdown();
#endif
    PlatformShutdown();
    s_Instance = nullptr;
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
    PlatformRunMainLoop();

    _running = false;
}

void Application::Tick()
{
    //Profiler::BeginFrame();
    // Start the Dear ImGui frame
    //ImGuiHelper::BeginFrame();
    _timer.Tick();
    DoUpdate();
    Render();
    //Input::Update();
}

void Application::RequestExit()
{
    _exitRequested = true;
}

void Application::DoUpdate()
{
    const float elapsedTime = float(_timer.GetElapsedSeconds());
    ALIMER_UNUSED(elapsedTime);

    Update();
}

void Application::InitBeforeRun()
{
    // Create RHI factory and device.
    RHIFactoryDesc factoryDesc{};
#if defined(_DEBUG)
    factoryDesc.validationMode = RHIValidationMode::Enabled;
#endif
    _rhiFactory = RHIFactory::Create(factoryDesc);
    _mainWindow->CreateSurface(_rhiFactory);
    _rhiAdapter = _rhiFactory->GetBestAdapter();
    RHIDeviceDesc deviceDesc{
        .label = "Main RHI Device"
    };
    _rhiDevice = _rhiAdapter->CreateDevice(deviceDesc);
    _mainWindow->CreateSwapChain(_rhiDevice);

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

    if (!BeginDraw())
    {
        return;
    }

    // ImGui
    OnGui();

    RHICommandBuffer* commandBuffer = _rhiDevice->BeginCommandBuffer(QueueType::Graphics, "Frame");
    //RHITexture* swapChainTexture = _mainWindow->GetSwapChain()->AcquireNextTexture();
    RHITexture* swapChainTexture = commandBuffer->AcquireSwapChainTexture(_mainWindow->GetSwapChain());
    if (swapChainTexture != nullptr)
    {
        Draw(commandBuffer, swapChainTexture);
    }

    //commandBuffer->Present(MainWindow.SwapChain!);

    // Execute 
    //GraphicsDevice.GraphicsQueue.Execute(commandBuffer);

    //Profiler::EndFrame(commandBuffer);
    EndDraw();
}

bool Application::BeginDraw()
{
    return !_mainWindow->IsMinimized() && !_rhiDevice->IsDeviceLost();
}

void Application::EndDraw()
{
    _rhiDevice->CommitFrame();
}
