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

    delete state.instance;
    memset(&state, 0, sizeof(state));
}

GPUAdapter agpuRequestAdapter(const GPURequestAdapterOptions* options)
{
    return state.instance->RequestAdapter(options);
}

/* Surface */
GPUSurface agpuSurfaceCreate(Window* window)
{
    ALIMER_ASSERT(window);

    return state.instance->CreateSurface(window);
}

GPUResult agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter adapter, GPUSurfaceCapabilities* capabilities)
{
    if (!surface || !adapter)
        return GPUResult_InvalidOperation;

    if (!capabilities)
        return GPUResult_InvalidOperation;

    return surface->GetCapabilities(adapter, capabilities);
}

static GPUSurfaceConfig _GPUSurfaceConfig_Defaults(const GPUSurfaceConfig* config) {
    GPUSurfaceConfig def = *config;
    def.width = _ALIMER_DEF(def.width, 1u);
    def.height = _ALIMER_DEF(def.height, 1u);
    def.presentMode = _ALIMER_DEF(def.presentMode, GPUPresentMode_Fifo);
    return def;
}

bool agpuSurfaceConfigure(GPUSurface surface, const GPUSurfaceConfig* config)
{
    if (!config)
        return false;

    GPUSurfaceConfig configDef = _GPUSurfaceConfig_Defaults(config);
    return surface->Configure(&configDef);
}

void agpuSurfaceUnconfigure(GPUSurface surface)
{
    surface->Unconfigure();
}

uint32_t agpuSurfaceAddRef(GPUSurface surface)
{
    return surface->AddRef();
}

uint32_t agpuSurfaceRelease(GPUSurface surface)
{
    return surface->Release();
}

/* Adapter */
GPUResult agpuAdapterGetInfo(GPUAdapter adapter, GPUAdapterInfo* info)
{
    if (!info)
        return GPUResult_InvalidOperation;

    return adapter->GetInfo(info);
}

GPUResult agpuAdapterGetLimits(GPUAdapter adapter, GPULimits* limits)
{
    if (!limits)
        return GPUResult_InvalidOperation;

    return adapter->GetLimits(limits);
}

static GPUDeviceDesc _GPUDeviceDesc_Defaults(const GPUDeviceDesc* desc)
{
    GPUDeviceDesc def = {};
    if (desc != nullptr)
        def = *desc;

    // 2 or 3
    def.maxFramesInFlight = std::min(_ALIMER_DEF(def.maxFramesInFlight, 2u), 3u);
    return def;
}

GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter, const GPUDeviceDesc* desc)
{
    GPUDeviceDesc descDef = _GPUDeviceDesc_Defaults(desc);
    return adapter->CreateDevice(descDef);
}

/* Device */
void agpuDeviceSetLabel(GPUDevice device, const char* label)
{
    device->SetLabel(label);
}

uint32_t agpuDeviceAddRef(GPUDevice device)
{
    return device->AddRef();
}

uint32_t agpuDeviceRelease(GPUDevice device)
{
    return device->Release();
}

GPUQueue agpuDeviceGetQueue(GPUDevice device, GPUQueueType type)
{
    return device->GetQueue(type);
}

bool agpuDeviceWaitIdle(GPUDevice device)
{
    return device->WaitIdle();
}

uint64_t agpuDeviceCommitFrame(GPUDevice device)
{
    return device->CommitFrame();
}

/* Queue */
GPUQueueType agpuQueueGetType(GPUQueue queue)
{
    return queue->GetQueueType();
}

GPUCommandBuffer agpuQueueAcquireCommandBuffer(GPUQueue queue, const GPUCommandBufferDesc* desc)
{
    return queue->AcquireCommandBuffer(desc);
}

void agpuQueueSubmit(GPUQueue queue, uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers)
{
    queue->Submit(numCommandBuffers, commandBuffers);
}

/* CommandBuffer */
void agpuCommandBufferPushDebugGroup(GPUCommandBuffer commandBuffer, const char* groupLabel)
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void agpuCommandBufferPopDebugGroup(GPUCommandBuffer commandBuffer)
{
    commandBuffer->PopDebugGroup();
}

void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer commandBuffer, const char* markerLabel)
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

GPUAcquireSurfaceResult agpuCommandBufferAcquireSurfaceTexture(GPUCommandBuffer commandBuffer, GPUSurface surface, GPUTexture* surfaceTexture)
{
    return commandBuffer->AcquireSurfaceTexture(surface, surfaceTexture);
}

GPUComputePassEncoder agpuCommandBufferBeginComputePass(GPUCommandBuffer commandBuffer, const GPUComputePassDesc* desc)
{
    GPUComputePassDesc descDef = {};
    if (desc)
        descDef = *desc;

    return commandBuffer->BeginComputePass(descDef);
}

GPURenderPassEncoder agpuCommandBufferBeginRenderPass(GPUCommandBuffer commandBuffer, const GPURenderPassDesc* desc)
{
    if (!desc)
    {
        alimerLogError(LogCategory_GPU, "Invalid RenderPass description");
        return nullptr;
    }

    return commandBuffer->BeginRenderPass(*desc);
}

/* ComputePassEncoder */
void agpuComputePassEncoderDispatch(GPUComputePassEncoder computePassEncoder, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    computePassEncoder->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void agpuComputePassEncoderDispatchIndirect(GPUComputePassEncoder computePassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    computePassEncoder->DispatchIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuComputePassEncoderEnd(GPUComputePassEncoder computePassEncoder)
{
    computePassEncoder->EndEncoding();
}

void agpuComputePassEncoderPushDebugGroup(GPUComputePassEncoder computePassEncoder, const char* groupLabel)
{
    computePassEncoder->PushDebugGroup(groupLabel);
}

void agpuComputePassEncoderPopDebugGroup(GPUComputePassEncoder computePassEncoder)
{
    computePassEncoder->PopDebugGroup();
}

void agpuComputePassEncoderInsertDebugMarker(GPUComputePassEncoder computePassEncoder, const char* markerLabel)
{
    computePassEncoder->InsertDebugMarker(markerLabel);
}

/* RenderCommandEncoder */
void agpuRenderPassEncoderEnd(GPURenderPassEncoder renderPassEncoder)
{
    renderPassEncoder->EndEncoding();
}

void agpuRenderPassEncoderPushDebugGroup(GPURenderPassEncoder renderPassEncoder, const char* groupLabel)
{
    renderPassEncoder->PushDebugGroup(groupLabel);
}

void agpuRenderPassEncoderPopDebugGroup(GPURenderPassEncoder renderPassEncoder)
{
    renderPassEncoder->PopDebugGroup();
}

void agpuRenderPassEncoderInsertDebugMarker(GPURenderPassEncoder renderPassEncoder, const char* markerLabel)
{
    renderPassEncoder->InsertDebugMarker(markerLabel);
}

/* Buffer */
static GPUBufferDesc _GPUBufferDesc_Defaults(const GPUBufferDesc* desc) {
    GPUBufferDesc def = *desc;
    return def;
}

GPUBuffer agpuDeviceCreateBuffer(GPUDevice device, const GPUBufferDesc* desc, const void* pInitialData)
{
    if (!desc)
        return nullptr;

    GPUBufferDesc descDef = _GPUBufferDesc_Defaults(desc);

    // TODO: Validation
    //if (descriptor->size > adapterProperties.limits.bufferMaxSize)
    //{
    //    LOGE("Buffer size too large: {}, limit: {}", desc.size, adapterProperties.limits.bufferMaxSize);
    //    return nullptr;
    //}

    return device->CreateBuffer(descDef, pInitialData);
}

void agpuBufferSetLabel(GPUBuffer buffer, const char* label)
{
    buffer->SetLabel(label);
}

uint32_t agpuBufferAddRef(GPUBuffer buffer)
{
    return buffer->AddRef();
}

uint32_t agpuBufferRelease(GPUBuffer buffer)
{
    return buffer->Release();
}

uint64_t agpuBufferGetSize(GPUBuffer buffer)
{
    return buffer->desc.size;
}

GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer buffer)
{
    return buffer->GetDeviceAddress();
}

/* Texture */
static GPUTextureDesc _GPUTextureDesc_Defaults(const GPUTextureDesc* desc) {
    GPUTextureDesc def = *desc;
    def.dimension = _ALIMER_DEF(def.dimension, TextureDimension_2D);
    def.format = _ALIMER_DEF(def.format, PixelFormat_RGBA8Unorm);
    def.width = _ALIMER_DEF(def.width, 1u);
    def.height = _ALIMER_DEF(def.height, 1u);
    def.depthOrArrayLayers = _ALIMER_DEF(def.depthOrArrayLayers, 1u);
    if (def.mipLevelCount == 0)
    {
        def.mipLevelCount = GetMipLevelCount(def.width, def.height, def.depthOrArrayLayers);
    }
    def.sampleCount = _ALIMER_DEF(def.sampleCount, 1u);
    return def;
}

GPUTexture agpuDeviceCreateTexture(GPUDevice device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData)
{
    if (!desc)
        return nullptr;

    GPUTextureDesc descDef = _GPUTextureDesc_Defaults(desc);
    return device->CreateTexture(descDef, pInitialData);
}

void agpuTextureSetLabel(GPUTexture texture, const char* label)
{
    texture->SetLabel(label);
}

TextureDimension agpuTextureGetDimension(GPUTexture texture)
{
    return texture->desc.dimension;
}

PixelFormat agpuTextureGetFormat(GPUTexture texture)
{
    return texture->desc.format;
}

GPUTextureUsage agpuTextureGetUsage(GPUTexture texture)
{
    return texture->desc.usage;
}

uint32_t agpuTextureGetWidth(GPUTexture texture)
{
    return texture->desc.width;
}

uint32_t agpuTextureGetHeight(GPUTexture texture)
{
    return texture->desc.height;
}

uint32_t agpuTextureGetDepthOrArrayLayers(GPUTexture texture)
{
    return texture->desc.depthOrArrayLayers;
}

uint32_t agpuTextureGetMipLevelCount(GPUTexture texture)
{
    return texture->desc.mipLevelCount;
}

uint32_t agpuTextureGetSampleCount(GPUTexture texture)
{
    return texture->desc.sampleCount;
}

uint32_t agpuTextureGetLevelWidth(GPUTexture texture, uint32_t mipLevel)
{
    return std::max(texture->desc.width >> mipLevel, 1u);
}

uint32_t agpuTextureGetLevelHeight(GPUTexture texture, uint32_t mipLevel)
{
    return std::max(texture->desc.height >> mipLevel, 1u);
}

uint32_t agpuTextureAddRef(GPUTexture texture)
{
    return texture->AddRef();
}

uint32_t agpuTextureRelease(GPUTexture texture)
{
    return texture->Release();
}

/* Other */
enum class KnownGPUAdapterVendor
{
    AMD = 0x01002,
    NVIDIA = 0x010DE,
    INTEL = 0x08086,
    ARM = 0x013B5,
    QUALCOMM = 0x05143,
    IMGTECH = 0x01010,
    MSFT = 0x01414,
    APPLE = 0x0106B,
    MESA = 0x10005,
    BROADCOM = 0x014e4
};

GPUAdapterVendor agpuGPUAdapterVendorFromID(uint32_t vendorId)
{
    switch (vendorId)
    {
        case (uint32_t)KnownGPUAdapterVendor::AMD:
            return GPUAdapterVendor_AMD;
        case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
            return GPUAdapterVendor_NVIDIA;
        case (uint32_t)KnownGPUAdapterVendor::INTEL:
            return GPUAdapterVendor_Intel;
        case (uint32_t)KnownGPUAdapterVendor::ARM:
            return GPUAdapterVendor_ARM;
        case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
            return GPUAdapterVendor_Qualcomm;
        case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
            return GPUAdapterVendor_ImgTech;
        case (uint32_t)KnownGPUAdapterVendor::MSFT:
            return GPUAdapterVendor_MSFT;
        case (uint32_t)KnownGPUAdapterVendor::APPLE:
            return GPUAdapterVendor_Apple;
        case (uint32_t)KnownGPUAdapterVendor::MESA:
            return GPUAdapterVendor_Mesa;
        case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
            return GPUAdapterVendor_Broadcom;

        default:
            return GPUAdapterVendor_Unknown;
    }
}

uint32_t agpuGPUAdapterVendorToID(GPUAdapterVendor vendor)
{
    switch (vendor)
    {
        case GPUAdapterVendor_AMD:
            return (uint32_t)KnownGPUAdapterVendor::AMD;
        case GPUAdapterVendor_NVIDIA:
            return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
        case GPUAdapterVendor_Intel:
            return (uint32_t)KnownGPUAdapterVendor::INTEL;
        case GPUAdapterVendor_ARM:
            return (uint32_t)KnownGPUAdapterVendor::ARM;
        case GPUAdapterVendor_Qualcomm:
            return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
        case GPUAdapterVendor_ImgTech:
            return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
        case GPUAdapterVendor_MSFT:
            return (uint32_t)KnownGPUAdapterVendor::MSFT;
        case GPUAdapterVendor_Apple:
            return (uint32_t)KnownGPUAdapterVendor::APPLE;
        case GPUAdapterVendor_Mesa:
            return (uint32_t)KnownGPUAdapterVendor::MESA;
        case GPUAdapterVendor_Broadcom:
            return (uint32_t)KnownGPUAdapterVendor::BROADCOM;

        default:
            return 0;
    }
}

#endif /* defined(ALIMER_GPU) */
