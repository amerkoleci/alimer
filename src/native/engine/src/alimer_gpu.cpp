// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_GPU)
#include "alimer_gpu_internal.h"

static struct {
    GPUInstance* instance = nullptr;
} state;

bool agpuIsBackendSupport(GPUBackendType backend)
{
    switch (backend)
    {
        case GPUBackendType_Null:
            return true;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            return Vulkan_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_D3D12:
#if defined(ALIMER_GPU_D3D12)
            return D3D12_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            return WGPU_IsSupported();
#else
            return false;
#endif

        default:
        case GPUBackendType_Metal:
            return false;
    }
}

bool agpuInit(const GPUConfig* config)
{
    ALIMER_ASSERT(config);

    if (state.instance)
        return true;

    GPUBackendType backend = config->preferredBackend;
    if (config->preferredBackend == GPUBackendType_Undefined)
    {
        if (agpuIsBackendSupport(GPUBackendType_D3D12))
        {
            backend = GPUBackendType_D3D12;
        }
        else if (agpuIsBackendSupport(GPUBackendType_Metal))
        {
            backend = GPUBackendType_Metal;
        }
        else if (agpuIsBackendSupport(GPUBackendType_Vulkan))
        {
            backend = GPUBackendType_Vulkan;
        }
        else if (agpuIsBackendSupport(GPUBackendType_WebGPU))
        {
            backend = GPUBackendType_WebGPU;
        }
    }

    switch (backend)
    {
        case GPUBackendType_Null:
            return true;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            if (Vulkan_IsSupported())
                state.instance = Vulkan_CreateInstance(config);
            break;
#else
            alimerLogError(LogCategory_GPU, "Vulkan is not supported");
            return false;
#endif

        case GPUBackendType_D3D12:
#if defined(ALIMER_GPU_D3D12)
            if (D3D12_IsSupported())
                state.instance = D3D12_CreateInstance(config);
            break;
#else
            alimerLogError(LogCategory_GPU, "D3D12 is not supported");
            return false;
#endif
            break;

        case GPUBackendType_Metal:
            state.instance = nullptr;
            break;

        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            if (WGPU_IsSupported())
                state.instance = WGPU_CreateInstance(config);
            break;
#else
            alimerLogError(LogCategory_GPU, "WebGPU is not supported");
            return false;
#endif

        default:
            break;
    }

    if (!state.instance)
    {
        return false;
    }

    return true;
}

void agpuShutdown(void)
{
    if (!state.instance)
        return;

    state.instance->Release();
    memset(&state, 0, sizeof(state));
}

GPUAdapter* agpuRequestAdapter(const GPURequestAdapterOptions* options)
{
    return state.instance->RequestAdapter(options);
}

/* Surface */
GPUSurface* agpuSurfaceCreate(Window* window)
{
    ALIMER_ASSERT(window);

    return state.instance->CreateSurface(window);
}

void agpuSurfaceConfigure(GPUSurface* surface, const GPUSurfaceConfiguration* config)
{
    surface->Configure(config);
}

void agpuSurfaceUnconfigure(GPUSurface* surface)
{
    surface->Unconfigure();
}

uint32_t agpuSurfaceAddRef(GPUSurface* surface)
{
    return surface->AddRef();
}

uint32_t agpuSurfaceRelease(GPUSurface* surface)
{
    return surface->Release();
}

/* Adapter */
GPUResult agpuAdapterGetLimits(GPUAdapter* adapter, GPULimits* limits)
{
    if (!limits)
        return GPUResult_InvalidOperation;

    return adapter->GetLimits(limits);
}

GPUDevice* agpuAdapterCreateDevice(GPUAdapter* adapter)
{
    return adapter->CreateDevice();
}

/* Device */
uint32_t agpuDeviceAddRef(GPUDevice* device)
{
    return device->AddRef();
}

uint32_t agpuDeviceRelease(GPUDevice* device)
{
    return device->Release();
}

GPUQueue* agpuDeviceGetQueue(GPUDevice* device, GPUQueueType type)
{
    return device->GetQueue(type);
}

bool agpuDeviceWaitIdle(GPUDevice* device)
{
    return device->WaitIdle();
}

uint64_t agpuDeviceCommitFrame(GPUDevice* device)
{
    return device->CommitFrame();
}

/* Queue */
GPUQueueType agpuQueueGetType(GPUQueue* queue)
{
    return queue->GetType();
}

GPUCommandBuffer* agpuQueueAcquireCommandBuffer(GPUQueue* queue, const GPUCommandBufferDesc* desc)
{
    return queue->AcquireCommandBuffer(desc);
}

void agpuQueueSubmit(GPUQueue* queue, uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers)
{
    queue->Submit(numCommandBuffers, commandBuffers);
}

/* CommandBuffer */

/* Buffer */
GPUBuffer* agpuDeviceCreateBuffer(GPUDevice* device, const GPUBufferDesc* desc, const void* pInitialData)
{
    if (!desc)
        return nullptr;

    // TODO: Validation
    //if (descriptor->size > adapterProperties.limits.bufferMaxSize)
    //{
    //    LOGE("Buffer size too large: {}, limit: {}", desc.size, adapterProperties.limits.bufferMaxSize);
    //    return nullptr;
    //}

    return device->CreateBuffer(desc, pInitialData);
}

uint32_t agpuBufferAddRef(GPUBuffer* buffer)
{
    return buffer->AddRef();
}

uint32_t agpuBufferRelease(GPUBuffer* buffer)
{
    return buffer->Release();
}

uint64_t agpuBufferGetSize(GPUBuffer* buffer)
{
    return buffer->GetSize();
}

GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer* buffer)
{
    return buffer->GetDeviceAddress();
}

/* Texture */
GPUTexture* agpuDeviceCreateTexture(GPUDevice* device, const GPUTextureDesc* desc)
{
    if (!desc)
        return nullptr;

    return device->CreateTexture(desc, nullptr);
}

uint32_t agpuTextureAddRef(GPUTexture* texture)
{
    return texture->AddRef();
}

uint32_t agpuTextureRelease(GPUTexture* texture)
{
    return texture->Release();
}

#endif /* defined(ALIMER_GPU) */
