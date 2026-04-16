// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Core/Timer.h"
#include "Alimer/Core/JobSystem.h"
#include "Alimer/Input.h"
#include "Alimer/Application.h"
#include "Alimer/Assets/AssetManager.h"
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
    JobSystem::Initialize();
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
    gAssets().Shutdown();

    _rhiDevice.Reset();
    _rhiFactory.Reset();
    Input::Shutdown();
    Log::Shutdown();
    JobSystem::Shutdown();
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
    Input::Update();
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
    JobSystem::Context ctx;
    JobSystem::Execute(ctx, [](JobSystem::JobArgs /*args*/) { Input::Initialize(); Input::Update(); });
    JobSystem::Execute(ctx, [&](JobSystem::JobArgs /*args*/) { gAssets().Start(_options.assetsDirectory); });

    // Create RHI factory and device.
    RHIFactoryDesc factoryDesc{};
    factoryDesc.preferredBackend = _options.graphics.preferredBackend;
    factoryDesc.validationMode = _options.graphics.validationMode;
    factoryDesc.label = _options.name;
    _rhiFactory = RHICreateFactory(factoryDesc);
    if (!_rhiFactory)
    {
        LOGE("RHI: Failed to initialize");
        _headless = true;
    }
    else
    {
        _mainWindow->CreateSurface(_rhiFactory);
        _rhiAdapter = _rhiFactory->GetBestAdapter();
        RHIDeviceDesc deviceDesc{
            .label = "Main RHI Device"
        };
        _rhiDevice = _rhiAdapter->CreateDevice(deviceDesc);
        _mainWindow->CreateSwapChain(_rhiDevice);
    }

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

    CommandBuffer* commandBuffer = _rhiDevice->BeginCommandBuffer(RHIQueueType::Graphics, "Frame");
    //RHITexture* swapChainTexture = _mainWindow->GetSwapChain()->AcquireNextTexture();
    RHITexture* swapChainTexture = commandBuffer->AcquireSurfaceTexture(_mainWindow->GetSurface());
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
