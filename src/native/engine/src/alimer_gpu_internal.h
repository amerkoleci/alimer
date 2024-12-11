// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_INTERNAL_H_
#define ALIMER_GPU_INTERNAL_H_

#include "alimer_internal.h"
#include "alimer_gpu.h"
#include <atomic>

enum class TextureLayout : uint8_t
{
    Undefined,
    CopySource, // Use as source of copy operation
    CopyDest, // Use as destination of copy operation
    ShaderResource,
    UnorderedAccess,
    RenderTarget,
    DepthWrite,
    DepthRead,

    Present,
    ShadingRateSurface,
};

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
        return ++refCount;
    }

    virtual uint32_t Release()
    {
        uint32_t newCount = --refCount;
        if (newCount == 0) {
            delete this;
        }
        return newCount;
    }

    virtual void SetLabel([[maybe_unused]] const char* label)
    {
    }

private:
    std::atomic_uint32_t refCount = 1;
};

struct GPUBuffer : public GPUResource
{
    GPUBufferDesc desc;
    virtual GPUDeviceAddress GetDeviceAddress() const = 0;
};

struct GPUTexture : public GPUResource
{
    GPUTextureDesc desc;
};

struct GPUSamplerImpl : public GPUResource
{

};

struct GPUQuerySetImpl : public GPUResource
{

};

struct GPUShaderModuleImpl : public GPUResource
{

};

struct GPUBindGroupLayoutImpl : public GPUResource
{

};

struct GPUBindGroupImpl : public GPUResource
{

};

struct GPUPipelineLayoutImpl : public GPUResource
{

};

struct GPUPipelineImpl : public GPUResource
{

};

struct GPUCommandEncoder : public GPUResource
{
    virtual void EndEncoding() = 0;
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;
};

struct GPURenderCommandEncoder : public GPUCommandEncoder
{

};

struct GPUCommandBuffer : public GPUResource
{
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;

    virtual GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface surface, GPUTexture** surfaceTexture) = 0;
    virtual GPURenderCommandEncoder* BeginRenderPass(const GPURenderPassDesc* desc) = 0;
};

struct GPUQueue : public GPUResource
{
    virtual GPUQueueType GetQueueType() const = 0;
    virtual GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) = 0;
    virtual void Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers) = 0;
};

struct GPUDevice : public GPUResource
{
    virtual GPUQueue* GetQueue(GPUQueueType type) = 0;
    virtual bool WaitIdle() = 0;
    virtual uint64_t CommitFrame() = 0;

    /* Resource creation */
    virtual GPUBuffer* CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) = 0;
    virtual GPUTexture* CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) = 0;
};

struct GPUSurfaceImpl : public GPUResource
{
    virtual GPUResult GetCapabilities(GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities) const = 0;
    virtual bool Configure(const GPUSurfaceConfig* config_) = 0;
    virtual void Unconfigure() = 0;

    GPUSurfaceConfig config;
};

struct GPUAdapter : public GPUResource
{
    virtual GPUResult GetLimits(GPULimits* limits) const = 0;
    virtual GPUDevice* CreateDevice() = 0;
};

struct GPUInstance 
{
public:
    virtual ~GPUInstance() = default;

    virtual GPUSurface CreateSurface(Window* window) = 0;
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
