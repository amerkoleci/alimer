// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_gpu_internal.h"
#include <string>

struct NullAdapter final : public GPUAdapter
{
    GPUAdapterLimits limits{};

    NullAdapter();
    GPUAdapterType GetType() const override { return adapterType; }
    void GetInfo(GPUAdapterInfo* info) const override;
    void GetLimits(GPUAdapterLimits* limits) const override;
    bool HasFeature(GPUFeature feature) const override;
    GPUDevice* CreateDevice(const GPUDeviceDesc& desc) override;

private:
    std::string deviceName;
    uint16_t driverVersion[4];
    std::string driverDescription;
    GPUAdapterType adapterType = GPUAdapterType_Cpu;
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
};

struct NullBuffer final : public GPUBuffer
{
    GPUDeviceAddress deviceAddress = 0;

    GPUDeviceAddress GetDeviceAddress() const override { return deviceAddress; }
};

struct NullTexture final : public GPUTexture
{
};

struct NullSampler final : public GPUSampler
{
};

struct NullBindGroupLayout final : public GPUBindGroupLayoutImpl
{
};

struct NullPipelineLayout final : public GPUPipelineLayoutImpl
{
};

struct NullComputePipeline final : public GPUComputePipeline
{
};

struct NullRenderPipeline final : public GPURenderPipelineImpl
{
};

struct NullQueryHeap final : public GPUQueryHeap
{
};

struct NullCommandBuffer;

struct NullComputePassEncoder final : public GPUComputePassEncoder
{
    NullCommandBuffer* commandBuffer = nullptr;

    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void SetPipeline(GPUComputePipeline* pipeline) override;
    void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
};

struct NullRenderPassEncoder final : public GPURenderPassEncoder
{
    NullCommandBuffer* commandBuffer = nullptr;

    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void SetViewport(const GPUViewport* viewport) override;
    void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) override;
    void SetScissorRect(const GPUScissorRect* scissorRect) override;
    void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) override;
    void SetBlendColor(const Color* color) override;
    void SetStencilReference(uint32_t reference) override;

    void SetVertexBuffer(uint32_t slot, GPUBuffer* buffer, uint64_t offset) override;
    void SetIndexBuffer(GPUBuffer* buffer, GPUIndexType type, uint64_t offset) override;
    void SetPipeline(GPURenderPipeline pipeline) override;
    void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) override;

    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
    void DrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
    void DrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;

    void MultiDrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) override;
    void MultiDrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) override;

    void SetShadingRate(GPUShadingRate rate) override;
};

struct NullCommandBuffer final : public GPUCommandBuffer
{
    static constexpr uint32_t kMaxBarrierCount = 16;

    uint32_t index = 0;
    bool hasLabel = false;
    bool encoderActive = false;
    NullComputePassEncoder* computePassEncoder = nullptr;
    NullRenderPassEncoder* renderPassEncoder = nullptr;

    GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface* surface, GPUTexture** surfaceTexture) override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    GPUComputePassEncoder* BeginComputePass(const GPUComputePassDesc& desc) override;
    GPURenderPassEncoder* BeginRenderPass(const GPURenderPassDesc& desc) override;
};

struct NullCommandQueue final : public GPUCommandQueue
{
    GPUCommandQueueType queueType = _GPUCommandQueueType_Count;

    GPUCommandQueueType GetType() const override { return queueType; }
    GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) override;
    void WaitIdle() override;
    void Submit(uint32_t numCommandBuffers, GPUCommandBuffer** commandBuffers) override;
};

struct NullDevice final : public GPUDevice
{
    NullAdapter* adapter = nullptr;
    NullCommandQueue queues[_GPUCommandQueueType_Count];
    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;
    uint32_t maxFramesInFlight = 0;
    uint64_t timestampFrequency = 0;

    bool HasFeature(GPUFeature feature) const override;
    GPUCommandQueue* GetQueue(GPUCommandQueueType type) override;
    void WaitIdle() override;
    uint64_t CommitFrame() override;

    uint64_t GetTimestampFrequency() const override { return timestampFrequency; }

    /* Resource creation */
    GPUBuffer* CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) override;
    GPUTexture* CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) override;
    GPUSampler* CreateSampler(const GPUSamplerDesc& desc) override;
    GPUBindGroupLayout CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) override;
    GPUPipelineLayout CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) override;
    GPUComputePipeline* CreateComputePipeline(const GPUComputePipelineDesc& desc) override;
    GPURenderPipeline CreateRenderPipeline(const GPURenderPipelineDesc& desc) override;
    GPUQueryHeap* CreateQueryHeap(const GPUQueryHeapDesc& desc) override;
};

struct NullSurface final : public GPUSurface
{
    NullTexture* backbufferTexture = nullptr;

    ~NullSurface() override;
    void GetCapabilities(GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities) const override;
    bool Configure(const GPUSurfaceConfig* config_) override;
    void Unconfigure() override;
};

struct NullInstance final : public GPUFactory
{
    std::vector<NullAdapter*> adapters;

    GPUBackendType GetBackend() const override { return GPUBackendType_Null; }
    uint32_t GetAdapterCount() const override { return (uint32_t)adapters.size(); }
    GPUAdapter* GetAdapter(uint32_t index) const override;
    GPUSurface* CreateSurface(GPUSurfaceHandle* surfaceHandle) override;
};


/* NullComputePassEncoder */
void NullComputePassEncoder::EndEncoding()
{
    commandBuffer->encoderActive = false;
}

void NullComputePassEncoder::PushDebugGroup(const char* groupLabel) const
{
    ALIMER_UNUSED(groupLabel);
}

void NullComputePassEncoder::PopDebugGroup() const
{
}

void NullComputePassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    ALIMER_UNUSED(markerLabel);
}

void NullComputePassEncoder::SetPipeline(GPUComputePipeline* pipeline)
{
    //currentPipeline->AddRef(); 
    ALIMER_UNUSED(pipeline);
}

void NullComputePassEncoder::SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size)
{
    ALIMER_UNUSED(pushConstantIndex);
    ALIMER_UNUSED(data);
    ALIMER_UNUSED(size);
}

void NullComputePassEncoder::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    ALIMER_UNUSED(groupCountX);
    ALIMER_UNUSED(groupCountY);
    ALIMER_UNUSED(groupCountZ);
}

void NullComputePassEncoder::DispatchIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    ALIMER_UNUSED(indirectBuffer);
    ALIMER_UNUSED(indirectBufferOffset);
}

/* NullRenderPassEncoder */
void NullRenderPassEncoder::EndEncoding()
{
    commandBuffer->encoderActive = false;
}

void NullRenderPassEncoder::PushDebugGroup(const char* groupLabel) const
{
    ALIMER_UNUSED(groupLabel);
}

void NullRenderPassEncoder::PopDebugGroup() const
{
}

void NullRenderPassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    ALIMER_UNUSED(markerLabel);
}

void NullRenderPassEncoder::SetViewport(const GPUViewport* viewport)
{
    ALIMER_UNUSED(viewport);
}

void NullRenderPassEncoder::SetViewports(uint32_t viewportCount, const GPUViewport* viewports)
{
    ALIMER_UNUSED(viewportCount);
    ALIMER_UNUSED(viewports);
}

void NullRenderPassEncoder::SetScissorRect(const GPUScissorRect* scissorRect)
{
    ALIMER_UNUSED(scissorRect);
}

void NullRenderPassEncoder::SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects)
{
    ALIMER_UNUSED(scissorCount);
    ALIMER_UNUSED(scissorRects);
}

void NullRenderPassEncoder::SetBlendColor(const Color* color)
{
    ALIMER_UNUSED(color);
}

void NullRenderPassEncoder::SetStencilReference(uint32_t reference)
{
    ALIMER_UNUSED(reference);
}

void NullRenderPassEncoder::SetVertexBuffer(uint32_t slot, GPUBuffer* buffer, uint64_t offset)
{
    ALIMER_UNUSED(slot);
    ALIMER_UNUSED(buffer);
    ALIMER_UNUSED(offset);
}

void NullRenderPassEncoder::SetIndexBuffer(GPUBuffer* buffer, GPUIndexType type, uint64_t offset)
{
    ALIMER_UNUSED(buffer);
    ALIMER_UNUSED(type);
    ALIMER_UNUSED(offset);
}

void NullRenderPassEncoder::SetPipeline(GPURenderPipeline pipeline)
{
    // currentPipeline->AddRef();
    ALIMER_UNUSED(pipeline);
}

void NullRenderPassEncoder::SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size)
{
    ALIMER_UNUSED(pushConstantIndex);
    ALIMER_UNUSED(data);
    ALIMER_UNUSED(size);
}

void NullRenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    ALIMER_UNUSED(vertexCount);
    ALIMER_UNUSED(instanceCount);
    ALIMER_UNUSED(firstVertex);
    ALIMER_UNUSED(firstInstance);
}

void NullRenderPassEncoder::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
{
    ALIMER_UNUSED(indexCount);
    ALIMER_UNUSED(instanceCount);
    ALIMER_UNUSED(firstIndex);
    ALIMER_UNUSED(baseVertex);
    ALIMER_UNUSED(firstInstance);
}

void NullRenderPassEncoder::DrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    ALIMER_UNUSED(indirectBuffer);
    ALIMER_UNUSED(indirectBufferOffset);
}

void NullRenderPassEncoder::DrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    ALIMER_UNUSED(indirectBuffer);
    ALIMER_UNUSED(indirectBufferOffset);
}

void NullRenderPassEncoder::MultiDrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer, uint64_t drawCountBufferOffset)
{
    ALIMER_UNUSED(indirectBuffer);
    ALIMER_UNUSED(indirectBufferOffset);
    ALIMER_UNUSED(maxDrawCount);
    ALIMER_UNUSED(drawCountBuffer);
    ALIMER_UNUSED(drawCountBufferOffset);
}

void NullRenderPassEncoder::MultiDrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer, uint64_t drawCountBufferOffset)
{
    ALIMER_UNUSED(indirectBuffer);
    ALIMER_UNUSED(indirectBufferOffset);
    ALIMER_UNUSED(maxDrawCount);
    ALIMER_UNUSED(drawCountBuffer);
    ALIMER_UNUSED(drawCountBufferOffset);
}

void NullRenderPassEncoder::SetShadingRate(GPUShadingRate rate)
{
    ALIMER_UNUSED(rate);
}

/* NullCommandBuffer */
GPUAcquireSurfaceResult NullCommandBuffer::AcquireSurfaceTexture(GPUSurface* surface, GPUTexture** surfaceTexture)
{
    NullSurface* backendSurface = static_cast<NullSurface*>(surface);

    *surfaceTexture = backendSurface->backbufferTexture;
    return GPUAcquireSurfaceResult_SuccessOptimal;
}

void NullCommandBuffer::PushDebugGroup(const char* groupLabel) const
{
    ALIMER_UNUSED(groupLabel);
}

void NullCommandBuffer::PopDebugGroup() const
{
}

void NullCommandBuffer::InsertDebugMarker(const char* markerLabel) const
{
    ALIMER_UNUSED(markerLabel);
}

GPUComputePassEncoder* NullCommandBuffer::BeginComputePass(const GPUComputePassDesc& desc)
{
    ALIMER_UNUSED(desc);

    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    encoderActive = true;
    return computePassEncoder;
}

GPURenderPassEncoder* NullCommandBuffer::BeginRenderPass(const GPURenderPassDesc& desc)
{
    ALIMER_UNUSED(desc);

    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    encoderActive = true;
    return renderPassEncoder;
}

/* D3D12Queue */
GPUCommandBuffer* NullCommandQueue::AcquireCommandBuffer(const GPUCommandBufferDesc* desc)
{
    // TODO:
    return nullptr;
}

void NullCommandQueue::WaitIdle()
{
}

void NullCommandQueue::Submit(uint32_t numCommandBuffers, GPUCommandBuffer** commandBuffers)
{
    ALIMER_UNUSED(numCommandBuffers);
    ALIMER_UNUSED(commandBuffers);
}

/* NullDevice */
bool NullDevice::HasFeature(GPUFeature feature) const
{
    return false;
}

GPUCommandQueue* NullDevice::GetQueue(GPUCommandQueueType type)
{
    return &queues[type];
}

void NullDevice::WaitIdle()
{
}

uint64_t NullDevice::CommitFrame()
{
    // Begin new frame
    frameCount++;
    frameIndex = frameCount % maxFramesInFlight;

    return frameCount;
}

GPUBuffer* NullDevice::CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData)
{
    NullBuffer* buffer = new NullBuffer();
    buffer->desc = desc;

    return buffer;
}

GPUTexture* NullDevice::CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData)
{
    NullTexture* texture = new NullTexture();
    texture->desc = desc;

    return texture;
}

GPUSampler* NullDevice::CreateSampler(const GPUSamplerDesc& desc)
{
    ALIMER_UNUSED(desc);

    NullSampler* sampler = new NullSampler();
    return sampler;
}

GPUBindGroupLayout NullDevice::CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc)
{
    NullBindGroupLayout* layout = new NullBindGroupLayout();
    return layout;
}

GPUPipelineLayout NullDevice::CreatePipelineLayout(const GPUPipelineLayoutDesc& desc)
{
    NullPipelineLayout* layout = new NullPipelineLayout();
    return layout;
}

GPUComputePipeline* NullDevice::CreateComputePipeline(const GPUComputePipelineDesc& desc)
{
    NullComputePipeline* pipeline = new NullComputePipeline();

    return pipeline;
}

GPURenderPipeline NullDevice::CreateRenderPipeline(const GPURenderPipelineDesc& desc)
{
    NullRenderPipeline* pipeline = new NullRenderPipeline();
    return pipeline;
}

GPUQueryHeap* NullDevice::CreateQueryHeap(const GPUQueryHeapDesc& desc)
{
    NullQueryHeap* queryHeap = new NullQueryHeap();

    return queryHeap;
}

/* NullSurface */
NullSurface::~NullSurface()
{
    Unconfigure();
}

void NullSurface::GetCapabilities(GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities) const
{
    capabilities->preferredFormat = PixelFormat_BGRA8Unorm;
    capabilities->supportedUsage = GPUTextureUsage_RenderTarget;
    static const PixelFormat kSupportedFormats[] = {
        PixelFormat_BGRA8Unorm,
    };
    capabilities->formats = kSupportedFormats;
    capabilities->formatCount = ALIMER_COUNT_OF(kSupportedFormats);
}

bool NullSurface::Configure(const GPUSurfaceConfig* config_)
{
    Unconfigure();
    return true;
}

void NullSurface::Unconfigure()
{
}

/* NullAdapter */
NullAdapter::NullAdapter()
{
    deviceName = "Null backend Adapter";
}

void NullAdapter::GetInfo(GPUAdapterInfo* info) const
{
    memset(info, 0, sizeof(GPUAdapterInfo));

    string::copy_safe(info->deviceName, sizeof(info->deviceName), deviceName.c_str());
    memcpy(info->driverVersion, driverVersion, sizeof(uint16_t) * 4);
    info->driverDescription = driverDescription.c_str();
    info->adapterType = adapterType;
    info->vendor = agpuGPUAdapterVendorFromID(vendorID);
    info->vendorID = vendorID;
    info->deviceID = deviceID;
}

void NullAdapter::GetLimits(GPUAdapterLimits* limits) const
{
    memcpy(limits, &this->limits, sizeof(GPUAdapterLimits));
}

bool NullAdapter::HasFeature(GPUFeature feature) const
{
    return false;
}

GPUDevice* NullAdapter::CreateDevice(const GPUDeviceDesc& desc)
{
    NullDevice* device = new NullDevice();
    device->adapter = this;
    device->maxFramesInFlight = desc.maxFramesInFlight;

    return device;
}

/* NullInstance */
GPUAdapter* NullInstance::GetAdapter(uint32_t index) const
{
    if (index >= adapters.size())
        return nullptr;

    return adapters[index];
}

GPUSurface* NullInstance::CreateSurface(GPUSurfaceHandle* surfaceHandle)
{
    NullSurface* surface = new NullSurface();
    
    return surface;
}

GPUFactory* Null_CreateInstance(const GPUFactoryDesc* desc)
{
    ALIMER_UNUSED(desc);

    NullInstance* instance = new NullInstance();

    return instance;
}
