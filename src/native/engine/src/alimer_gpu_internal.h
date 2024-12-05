// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_INTERNAL_H_
#define ALIMER_GPU_INTERNAL_H_

#include "alimer_internal.h"
#include "alimer_gpu.h"

class GPUResource
{
protected:
    GPUResource() = default;
    virtual ~GPUResource() = default;

public:
    // Non-copyable and non-movable
    GPUResource(const GPUResource&) = delete;
    GPUResource& operator=(const GPUResource&) = delete;
    GPUResource(GPUResource&&) = delete;
    GPUResource& operator=(GPUResource&&) = delete;

    virtual uint32_t AddRef()
    {
        return InterlockedIncrement(&_refCount);
    }

    virtual uint32_t Release()
    {
        uint32_t newCount = InterlockedDecrement(&_refCount);
        if (newCount == 0) {
            delete this;
        }
        return newCount;
    }

    virtual void SetLabel([[maybe_unused]] const char* label)
    {
    }

private:
    uint32_t _refCount = 1;
};

struct GPUBuffer : public GPUResource
{
    virtual uint64_t GetSize() const = 0;
    virtual GPUDeviceAddress GetDeviceAddress() const = 0;
};

struct GPUTexture : public GPUResource
{

};

struct GPUTextureView : public GPUResource
{

};

struct GPUShaderModule : public GPUResource
{

};


struct GPUCommandBuffer : public GPUResource
{

};

struct GPUQueue : public GPUResource
{
    virtual GPUQueueType GetType() const = 0;
    virtual GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) = 0;
    virtual void Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers) = 0;
};

struct GPUDevice : public GPUResource
{
    virtual GPUQueue* GetQueue(GPUQueueType type) = 0;
    virtual bool WaitIdle() = 0;
    virtual uint64_t CommitFrame() = 0;

    /* Resource creation */
    virtual GPUBuffer* CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData) = 0;
    virtual GPUTexture* CreateTexture(const GPUTextureDesc* desc, const void* pInitialData) = 0;
};

struct GPUSurface : public GPUResource
{
    virtual void Configure(const GPUSurfaceConfiguration* config) = 0;
    virtual void Unconfigure() = 0;
};

struct GPUAdapter : public GPUResource
{
    virtual GPUResult GetLimits(GPULimits* limits) const = 0;
    virtual GPUDevice* CreateDevice() = 0;
};

struct GPUInstance : public GPUResource
{
public:
    virtual GPUSurface* CreateSurface(Window* window) = 0;
    virtual GPUAdapter* RequestAdapter(const GPURequestAdapterOptions* options) = 0;
};

#if defined(ALIMER_GPU_VULKAN)
_ALIMER_EXTERN bool Vulkan_IsSupported(void);
_ALIMER_EXTERN GPUInstance* Vulkan_CreateInstance(const GPUConfig* config);
#endif

#if defined(ALIMER_GPU_D3D12)
_ALIMER_EXTERN bool D3D12_IsSupported(void);
_ALIMER_EXTERN GPUInstance* D3D12_CreateInstance(const GPUConfig* config);
#endif

#if defined(ALIMER_GPU_WEBGPU)
_ALIMER_EXTERN bool WGPU_IsSupported(void);
_ALIMER_EXTERN GPUInstance* WGPU_CreateInstance(const GPUConfig* config);
#endif

#endif /* ALIMER_GPU_INTERNAL_H_ */
