// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#define TEST_PHYSICS

#include "alimer_image.h"
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
#include <assert.h>

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

    Image* image = alimerImageCreate1D(PixelFormat_RGBA8Unorm, 512, 1, 0);
    assert(alimerImageGetMipLevelCount(image) == 10);
    alimerImageDestroy(image);

#if defined(ALIMER_GPU)
    GPUFactoryDesc factoryDesc = {
        .preferredBackend = GPUBackendType_Vulkan,
#if defined(_DEBUG)
        .validationMode = GPUValidationMode_Enabled
#else
        .validationMode = GPUValidationMode_Disabled
#endif
    };
    GPUFactory* gpuFactory = agpuCreateFactory(&factoryDesc);
    GPUBackendType backend = agpuFactoryGetBackend(gpuFactory);
    if (backend == GPUBackendType_Undefined)
    {
        alimerLogError(LogCategory_GPU, "No GPU backend is available.");
        return EXIT_FAILURE;
    }

    GPUAdapter* gpuAdapter = agpuFactoryGetBestAdapter(gpuFactory);
    GPUAdapterInfo adapterInfo;
    agpuAdapterGetInfo(gpuAdapter, &adapterInfo);

    GPUAdapterLimits adapterLimits;
    agpuAdapterGetLimits(gpuAdapter, &adapterLimits);

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

#if defined(ALIMER_GPU)
    GPUSurfaceHandle* surfaceHandle = alimerWindowCreateSurfaceHandle(window);
    GPUSurface* surface = agpuCreateSurface(gpuFactory, surfaceHandle);
    GPUSurfaceConfig surfaceConfig = {
        .device = device,
        //.usage = GPUSurfaceUsage_Present,
        .format = PixelFormat_BGRA8Unorm,
        .width = windowDesc.width,
        .height = windowDesc.height,
        .presentMode = GPUPresentMode_Fifo
    };
    agpuSurfaceConfigure(surface, &surfaceConfig);
#endif

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
        GPUTexture* surfaceTexture = NULL;
        GPUAcquireSurfaceResult acquireResult = agpuCommandBufferAcquireSurfaceTexture(commandBuffer, surface, &surfaceTexture);
        if (acquireResult == GPUAcquireSurfaceResult_SuccessOptimal)
        {
            GPURenderPassDesc renderPassDesc = {
                .colorAttachmentCount = 1,
                .colorAttachments = &(GPURenderPassColorAttachment){
                    .texture = surfaceTexture,
                    .loadAction = GPULoadAction_Clear,
                    .storeAction = GPUStoreAction_Store,
                    .clearColor = { 0.1f, 0.2f, 0.3f, 1.0f }
                },
                .depthStencilAttachment = NULL
            };
            GPURenderPassEncoder* renderPass = agpuCommandBufferBeginRenderPass(commandBuffer, &renderPassDesc);
            agpuRenderPassEncoderEnd(renderPass);
        }
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

    agpuSurfaceRelease(surface);
    agpuSurfaceHandleDestroy(surfaceHandle);

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
