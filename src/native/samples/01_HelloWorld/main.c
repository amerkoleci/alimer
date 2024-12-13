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
#include <string.h> // memset
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

Window* window = NULL;
GPUSurface surface = NULL;
GPUDevice device = NULL;
GPUQueue graphicsQueue = NULL;
GPUBuffer vertexBuffer = NULL;
GPURenderPipeline renderPipeline = NULL;

GPUShaderModule LoadShader(const char* shaderFileName)
{
    //const char* shaderExt = ".spv";
    //if (vgpuDeviceGetBackend(device) == VGPUBackend_D3D12)
    //{
    //    shaderExt = ".cso";
    //}

    FILE* handle = fopen(shaderFileName, "rb");
    if (!handle)
    {
        return NULL;
    }

    // Get file size
    fseek(handle, 0, SEEK_END);
    size_t length = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    GPUShaderModuleDesc desc = {
        .bytecodeSize = length,
        .bytecode = malloc(length)
    };
    fread((void*)desc.bytecode, length, 1, handle);
    GPUShaderModule result = agpuDeviceCreateShaderModule(device, &desc);
    free((void*)desc.bytecode);
    fclose(handle);
    return result;
}

void Render()
{
    if (alimerWindowIsMinimized(window))
        return;

    GPUCommandBuffer commandBuffer = agpuQueueAcquireCommandBuffer(graphicsQueue, NULL);

    GPUTexture surfaceTexture = NULL;
    GPUAcquireSurfaceResult result = agpuCommandBufferAcquireSurfaceTexture(commandBuffer, surface, &surfaceTexture);
    if (result == GPUAcquireSurfaceResult_SuccessOptimal)
    {
        GPURenderPassColorAttachment colorAttachment = {
            .texture = surfaceTexture,
            .loadAction = GPULoadAction_Clear,
            .storeAction = GPUStoreAction_Store,
            .clearColor = {0.3f, 0.3f, 0.3f, 1.0}
        };

        GPURenderPassDesc renderPass = {
            .label = "RenderPass",
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment
        };

        GPURenderPassEncoder encoder = agpuCommandBufferBeginRenderPass(commandBuffer, &renderPass);
        agpuRenderPassEncoderEnd(encoder);
    }

    agpuQueueSubmit(graphicsQueue, 1u, &commandBuffer);
}

int main()
{
    if (!alimerPlatformInit())
    {
        return EXIT_FAILURE;
    }

    GPUConfig config = {
        .preferredBackend = GPUBackendType_Vulkan,
#if defined(_DEBUG)
        .validationMode = GPUValidationMode_Enabled
#else
        .validationMode = GPUValidationMode_Disabled
#endif
    };
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
    window = alimerWindowCreate(&windowDesc);
    alimerWindowSetCentered(window);

    surface = agpuSurfaceCreate(window);

    GPURequestAdapterOptions adapterOptions = {
        .compatibleSurface = surface
    };
    GPUAdapter adapter = agpuRequestAdapter(&adapterOptions);

    GPUAdapterInfo adapterInfo;
    GPULimits adapterLimits;
    agpuAdapterGetInfo(adapter, &adapterInfo);
    agpuAdapterGetLimits(adapter, &adapterLimits);

    //GPUSurfaceCapabilities surfaceCaps;
    //agpuSurfaceGetCapabilities(surface, adapter, &surfaceCaps);

    GPUDeviceDesc deviceDesc = {
        .label = "Test Device"
    };
    device = agpuAdapterCreateDevice(adapter, &deviceDesc);

    GPUSurfaceConfig surfaceConfig = {
        .device = device,
        //.format = surfaceCaps.preferredFormat,
        .width = windowDesc.width,
        .height = windowDesc.height
    };
    agpuSurfaceConfigure(surface, &surfaceConfig);

    const float vertices[] = {
        // positions            colors
         0.0f, 0.5f, 0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
    };
    vertexBuffer = agpuDeviceCreateBuffer(device, &(GPUBufferDesc) {
        .usage = GPUBufferUsage_Vertex,
        .size = sizeof(vertices)
    }, vertices);

    GPUShaderModule vertexShader = LoadShader("shaders/triangleVertex.spv");
    //GPUShaderModule fragmentShader = LoadShader("shaders/triangleFragment.spv");
    //shaders[1].entryPointName = "fragmentMain";

    GPUPipelineLayout pipelineLayout = agpuDeviceCreatePipelineLayout(device, &(GPUPipelineLayoutDesc) {
        .label = "PipelineLayout"
    });

    GPUVertexAttribute vertexAttributes[2];
    vertexAttributes[0].format = GPUVertexFormat_Float3;
    vertexAttributes[0].offset = 0;
    vertexAttributes[0].shaderLocation = 0;
    vertexAttributes[1].format = GPUVertexFormat_Float4;
    vertexAttributes[1].offset = 12;
    vertexAttributes[1].shaderLocation = 1;

    GPUVertexBufferLayout vertexBufferLayout = {
        .stepMode = GPUVertexStepMode_Vertex,
        .stride = 0,
        .attributeCount = 2,
        .attributes = vertexAttributes
    };

    renderPipeline = agpuDeviceCreateRenderPipeline(device, &(GPURenderPipelineDesc) {
        .label = "RenderPipeline",
        .layout = pipelineLayout,
        .vertex = {
            .module = vertexShader,
            .entryPoint = "vertexMain",
            .bufferCount = 1,
            .buffers = &vertexBufferLayout
        }
    });
    agpuShaderModuleRelease(vertexShader);
    agpuPipelineLayoutRelease(pipelineLayout);

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

    agpuRenderPipelineRelease(renderPipeline);
    agpuBufferRelease(vertexBuffer);
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
