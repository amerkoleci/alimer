// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#define TEST_PHYSICS

#include "alimer.h"
#include "alimer_gpu.h"
#if defined(TEST_PHYSICS)
#include "alimer_physics.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

GPUQueue* graphicsQueue = NULL;

void Render()
{
    GPUCommandBuffer* commandBuffer = agpuQueueAcquireCommandBuffer(graphicsQueue, NULL);
    agpuQueueSubmit(graphicsQueue, 1u, &commandBuffer);
}

int main()
{
    if (!alimerPlatformInit())
    {
        return EXIT_FAILURE;
    }

    GPUConfig config = {};
    //config.preferredBackend = GPUBackendType_Vulkan;
#if defined(_DEBUG)
    config.validationMode = GPUValidationMode_Enabled;
#endif
    if (!agpuInit(&config))
    {
        return EXIT_FAILURE;
    }

#if defined(TEST_PHYSICS)
    // Physics
    PhysicsConfig physicsConfig = {};
    if (!alimerPhysicsInit(&physicsConfig))
    {
        return EXIT_FAILURE;
    }

    PhysicsWorldConfig physicsWorldConfig = {};
    PhysicsWorld* physicsWorld = alimerPhysicsWorldCreate(&physicsWorldConfig);
#endif

    const WindowDesc windowDesc = {
        .title = "01 - Hello World",
        .width = 1280,
        .height = 720,
        .flags = WindowFlags_Hidden | WindowFlags_Resizable
    };
    Window* window = alimerWindowCreate(&windowDesc);
    alimerWindowSetCentered(window);

    GPUSurface* surface = agpuSurfaceCreate(window);

    GPURequestAdapterOptions adapterOptions = {
        .compatibleSurface = surface
    };
    GPUAdapter* adapter = agpuRequestAdapter(&adapterOptions);

    GPULimits limits;
    agpuAdapterGetLimits(adapter, &limits);
    GPUDevice* device = agpuAdapterCreateDevice(adapter);

    const float vertices[] = {
        // positions            colors
         0.0f, 0.5f, 0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
    };
    GPUBuffer* vertexBuffer = agpuDeviceCreateBuffer(device, &(GPUBufferDesc) {
        .usage = GPUBufferUsage_Vertex,
        .size = sizeof(vertices)
    }, vertices);
    agpuBufferRelease(vertexBuffer);

    graphicsQueue = agpuDeviceGetQueue(device, GPUQueueType_Graphics);

    // GPU setup ready, show window
    alimerWindowShow(window);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Render, 0, false);
#else
    bool running = true;
    while (running)
    {
        Event evt;
        while (alimerPollEvent(&evt))
        {
            if (evt.type == EventType_Quit)
            {
                running = false;
                break;
            }
        }

        Render();
        //surface.Present();
        agpuDeviceCommitFrame(device);
    }
#endif

    agpuSurfaceRelease(surface);
    agpuDeviceRelease(device);
    alimerWindowDestroy(window);
    agpuShutdown();
#if defined(TEST_PHYSICS)
    alimerPhysicsWorldDestroy(physicsWorld);
    alimerPhysicsShutdown();
#endif
    alimerPlatformShutdown();
    return EXIT_SUCCESS;
}
