// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_GPU)
#include "alimer_gpu_internal.h"

static struct {
    GPUInstance* instance = nullptr;
} state;

Bool32 agpuIsBackendSupport(GPUBackendType backend)
{
    switch (backend)
    {
        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            return WGPU_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_D3D12:
            return false;

        case GPUBackendType_Metal:
            return false;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            return Vulkan_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_Null:
            return true;

        default:
            return false;
    }
}

Bool32 agpuInit(const GPUConfig* config)
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
        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            if (WGPU_IsSupported())
                state.instance = WGPU_CreateInstance(config);
            break;
#else
            alimerLogError(LogCategory_GPU, "WebGPU is not supported");
            return false;
#endif

        case GPUBackendType_D3D12:
            state.instance = nullptr;
            break;

        case GPUBackendType_Metal:
            state.instance = nullptr;
            break;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            if (Vulkan_IsSupported())
                state.instance = Vulkan_CreateInstance(config);
            break;
#else
            alimerLogError(LogCategory_GPU, "Vulkan is not supported");
            return false;
#endif

        case GPUBackendType_Null:
            return true;

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

GPUSurface agpuCreateSurface(Window* window)
{
    ALIMER_ASSERT(window);

    return state.instance->CreateSurface(window);
}

GPUAdapter agpuRequestAdapter(const GPURequestAdapterOptions* options)
{
    return state.instance->RequestAdapter(options);
}

/* Surface */
uint32_t agpuSurfaceAddRef(GPUSurface surface)
{
    return surface->AddRef();
}

uint32_t agpuSurfaceRelease(GPUSurface surface)
{
    return surface->Release();
}

/* Adapter */
GPUResult agpuAdapterGetLimits(GPUAdapter adapter, GPULimits* limits)
{
    if (!limits)
        return GPUResult_InvalidOperation;

    return adapter->GetLimits(limits);
}

GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter)
{
    return adapter->CreateDevice();
}

/* Device */
void agpuDeviceRelease(GPUDevice device)
{
    device->Release();
}

GPUQueue agpuDeviceGetQueue(GPUDevice device, GPUQueueType type)
{
    return device->GetQueue(type);
}

uint64_t agpuDeviceCommitFrame(GPUDevice device)
{
    return device->CommitFrame();
}

/* Queue */
GPUCommandBuffer agpuQueueCreateCommandBuffer(GPUQueue queue, const GPUCommandBufferDescriptor* descriptor)
{
    return queue->CreateCommandBuffer(descriptor);
}

/* CommandBuffer */

/* Buffer */
GPUBuffer agpuCreateBuffer(GPUDevice device, const GPUBufferDescriptor* descriptor, const void* pInitialData)
{
    if (!descriptor)
        return nullptr;

    // TODO: Validation
    //if (descriptor->size > adapterProperties.limits.bufferMaxSize)
    //{
    //    LOGE("Buffer size too large: {}, limit: {}", desc.size, adapterProperties.limits.bufferMaxSize);
    //    return nullptr;
    //}

    return device->CreateBuffer(descriptor, pInitialData);
}

uint32_t agpuBufferAddRef(GPUBuffer buffer)
{
    return buffer->AddRef();
}

uint32_t agpuBufferRelease(GPUBuffer buffer)
{
    return buffer->Release();
}


#endif /* defined(ALIMER_GPU) */
