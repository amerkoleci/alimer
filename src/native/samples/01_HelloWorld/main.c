// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#define TEST_PHYSICS

#include "alimer.h"
#if defined(ALIMER_AUDIO)
#include "alimer_audio.h"
#endif

#if defined(ALIMER_GPU)
#include "alimer_gpu.h"
#endif

#if defined(ALIMER_PHYSICS)
#include "alimer_physics.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

Window* window = NULL;

int main(void)
{
    if (!alimerPlatformInit())
    {
        return EXIT_FAILURE;
    }

#if defined(ALIMER_AUDIO)
    AudioContext* context = alimerAudioContextInit();
    AudioEngine* engine = alimerAudioEngineCreate(context, NULL);
#endif

#if defined(ALIMER_GPU)
    GPUFactory* gpuFactory = agpuCreateFactory(NULL);
    GPUBackendType backend = agpuFactoryGetBackend(gpuFactory);
    if (backend == GPUBackendType_Undefined)
    {
        alimerLogError(LogCategory_GPU, "No GPU backend is available.");
        return EXIT_FAILURE;
    }

    GPUAdapter* gpuAdapter = agpuFactoryRequestAdapter(gpuFactory, NULL);
    GPUAdapterInfo adapterInfo;
    agpuAdapterGetInfo(gpuAdapter, &adapterInfo);

    GPUDevice* device = agpuCreateDevice(gpuAdapter, NULL);
    GPUCommandQueue* graphicsQueue = agpuDeviceGetCommandQueue(device, GPUCommandQueueType_Graphics);
#endif

#if defined(ALIMER_PHYSICS)
    // Physics
    PhysicsConfig physicsConfig = { 0 };
    if (!alimerPhysicsInit(&physicsConfig))
    {
        return EXIT_FAILURE;
    }

    PhysicsWorldConfig physicsWorldConfig = { 0 };
    PhysicsWorld* physicsWorld = alimerPhysicsWorldCreate(&physicsWorldConfig);
#endif

    const WindowDesc windowDesc = {
        .title = "01 - Hello World",
        .width = 1280,
        .height = 720,
        .flags = WindowFlags_Hidden | WindowFlags_Resizable
    };
    window = alimerWindowCreate(&windowDesc);
    alimerWindowSetCentered(window);

    // GPU setup ready, show window
    alimerWindowShow(window);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Render, 0, false);
#else
    bool running = true;
    uint64_t frameCount = 0;
    while (running)
    {
        PlatformEvent evt;
        while (alimerPlatformPollEvent(&evt))
        {
            if (evt.type == EventType_Quit)
            {
                running = false;
                break;
            }
        }

        GPUCommandBuffer* commandBuffer = agpuCommandQueueAcquireCommandBuffer(graphicsQueue, NULL);  
        agpuCommandQueueSubmit(graphicsQueue, 1, &commandBuffer);

        //Render();
        frameCount = agpuDeviceCommitFrame(device);
    }

    (void)(frameCount);
#endif

    alimerWindowDestroy(window);

#if defined(ALIMER_AUDIO)
    alimerAudioEngineDestroy(engine);
    alimerAudioContextDestroy(context);
#endif

#if defined(ALIMER_GPU)
    agpuDeviceWaitIdle(device);
    agpuDeviceRelease(device);
    agpuFactoryDestroy(gpuFactory);
#endif

#if defined(ALIMER_PHYSICS)
    alimerPhysicsWorldDestroy(physicsWorld);
    alimerPhysicsShutdown();
#endif
    alimerPlatformShutdown();
    return EXIT_SUCCESS;
}
