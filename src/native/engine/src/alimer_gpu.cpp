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
GPUSurface agpuCreateSurface(Window* window)
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

bool agpuAdapterHasFeature(GPUAdapter adapter, GPUFeature feature)
{
    return adapter->HasFeature(feature);
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

/* Device */
GPUDevice agpuCreateDevice(GPUAdapter adapter, const GPUDeviceDesc* desc)
{
    GPUDeviceDesc descDef = _GPUDeviceDesc_Defaults(desc);
    return adapter->CreateDevice(descDef);
}

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

GPUBackendType agpuDeviceGetBackend(GPUDevice device)
{
    return device->GetBackend();
}

bool agpuDeviceHasFeature(GPUDevice device, GPUFeature feature)
{
    return device->HasFeature(feature);
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
void agpuRenderPassEncoderSetViewport(GPURenderPassEncoder renderPassEncoder, const GPUViewport* viewport)
{
    ALIMER_ASSERT(viewport != nullptr);

    renderPassEncoder->SetViewport(viewport);
}

void agpuRenderPassEncoderSetViewports(GPURenderPassEncoder renderPassEncoder, uint32_t viewportCount, const GPUViewport* viewports)
{
    ALIMER_ASSERT(viewportCount > 0);
    ALIMER_ASSERT(viewports != nullptr);

    renderPassEncoder->SetViewports(viewportCount, viewports);
}

void agpuRenderPassEncoderSetScissorRect(GPURenderPassEncoder renderPassEncoder, const GPUScissorRect* scissorRect)
{
    ALIMER_ASSERT(scissorRect != nullptr);

    renderPassEncoder->SetScissorRect(scissorRect);
}

void agpuRenderPassEncoderSetScissorRects(GPURenderPassEncoder renderPassEncoder, uint32_t scissorCount, const GPUScissorRect* scissorRects)
{
    ALIMER_ASSERT(scissorCount > 0);
    ALIMER_ASSERT(scissorRects != nullptr);

    renderPassEncoder->SetScissorRects(scissorCount, scissorRects);
}

void agpuRenderPassEncoderSetBlendColor(GPURenderPassEncoder renderPassEncoder, const float blendColor[4])
{
    renderPassEncoder->SetBlendColor(blendColor);
}

void agpuRenderPassEncoderSetStencilReference(GPURenderPassEncoder renderPassEncoder, uint32_t reference)
{
    renderPassEncoder->SetStencilReference(reference);
}

void agpuRenderPassEncoderSetVertexBuffer(GPURenderPassEncoder renderPassEncoder, uint32_t slot, GPUBuffer buffer, uint64_t offset)
{
    renderPassEncoder->SetVertexBuffer(slot, buffer, offset);
}

void agpuRenderPassEncoderSetIndexBuffer(GPURenderPassEncoder renderPassEncoder, GPUBuffer buffer, GPUIndexType type, uint64_t offset)
{
    renderPassEncoder->SetIndexBuffer(buffer, type, offset);
}

void agpuRenderPassEncoderSetPipeline(GPURenderPassEncoder renderPassEncoder, GPURenderPipeline pipeline)
{
    renderPassEncoder->SetPipeline(pipeline);
}

void agpuRenderPassEncoderDraw(GPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    renderPassEncoder->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void agpuRenderPassEncoderDrawIndexed(GPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
{
    renderPassEncoder->DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void agpuRenderPassEncoderDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    renderPassEncoder->DrawIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuRenderPassEncoderDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    renderPassEncoder->DrawIndexedIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuRenderPassEncoderMultiDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, ALIMER_NULLABLE GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset)
{
    renderPassEncoder->MultiDrawIndirect(indirectBuffer, indirectBufferOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

void agpuRenderPassEncoderMultiDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, ALIMER_NULLABLE GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset)
{
    renderPassEncoder->MultiDrawIndexedIndirect(indirectBuffer, indirectBufferOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

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

GPUBuffer agpuCreateBuffer(GPUDevice device, const GPUBufferDesc* desc, const void* pInitialData)
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

GPUTexture agpuCreateTexture(GPUDevice device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData)
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

/* Sampler */
static GPUSamplerDesc _GPUSamplerDesc_Defaults(const GPUSamplerDesc* desc) {
    GPUSamplerDesc def = {};
    if (desc)
        def = *desc;

    return def;
}


GPUSampler agpuCreateSampler(GPUDevice device, const GPUSamplerDesc* desc)
{
    GPUSamplerDesc descDef = _GPUSamplerDesc_Defaults(desc);
    return device->CreateSampler(descDef);
}

void agpuSamplerSetLabel(GPUSampler sampler, const char* label)
{
    sampler->SetLabel(label);
}

uint32_t agpuSamplerAddRef(GPUSampler sampler)
{
    return sampler->AddRef();
}

uint32_t agpuSamplerRelease(GPUSampler sampler)
{
    return sampler->Release();
}

/* PipelineLayout */
static GPUPipelineLayoutDesc _GPUPipelineLayoutDesc_Defaults(const GPUPipelineLayoutDesc* desc) {
    GPUPipelineLayoutDesc def = *desc;
    return def;
}

GPUPipelineLayout agpuCreatePipelineLayout(GPUDevice device, const GPUPipelineLayoutDesc* desc)
{
    if (!desc)
        return nullptr;

    GPUPipelineLayoutDesc descDef = _GPUPipelineLayoutDesc_Defaults(desc);
    return device->CreatePipelineLayout(descDef);
}

void agpuPipelineLayoutSetLabel(GPUPipelineLayout pipelineLayout, const char* label)
{
    pipelineLayout->SetLabel(label);
}

uint32_t agpuPipelineLayoutAddRef(GPUPipelineLayout pipelineLayout)
{
    return pipelineLayout->AddRef();
}

uint32_t agpuPipelineLayoutRelease(GPUPipelineLayout pipelineLayout)
{
    return pipelineLayout->Release();
}

/* ComputePipeline */
static GPUComputePipelineDesc _GPUComputePipelineDesc_Defaults(const GPUComputePipelineDesc* desc) {
    GPUComputePipelineDesc def = *desc;
    return def;
}

GPUComputePipeline agpuCreateComputePipeline(GPUDevice device, const GPUComputePipelineDesc* desc)
{
    if (!desc)
        return nullptr;

    GPUComputePipelineDesc descDef = _GPUComputePipelineDesc_Defaults(desc);
    return device->CreateComputePipeline(descDef);
}

void agpuComputePipelineSetLabel(GPUComputePipeline computePipeline, const char* label)
{
    computePipeline->SetLabel(label);
}

uint32_t agpuComputePipelineAddRef(GPUComputePipeline computePipeline)
{
    return computePipeline->AddRef();
}

uint32_t agpuComputePipelineRelease(GPUComputePipeline computePipeline)
{
    return computePipeline->Release();
}

/* RenderPipeline */
static GPURenderPipelineDesc _GPURenderPipelineDesc_Defaults(const GPURenderPipelineDesc* desc) {
    GPURenderPipelineDesc def = *desc;

    // RasterizerState
    def.rasterizerState.fillMode = _ALIMER_DEF(def.rasterizerState.fillMode, GPUFillMode_Solid);
    def.rasterizerState.cullMode = _ALIMER_DEF(def.rasterizerState.cullMode, GPUCullMode_Back);
    def.rasterizerState.frontFace = _ALIMER_DEF(def.rasterizerState.frontFace, GPUFrontFace_Clockwise);
    def.rasterizerState.depthClipMode = _ALIMER_DEF(def.rasterizerState.depthClipMode, GPUDepthClipMode_Clip);

    // DepthStencilState
    def.depthStencilState.depthCompareFunction = _ALIMER_DEF(def.depthStencilState.depthCompareFunction, GPUCompareFunction_Always);
    def.depthStencilState.stencilReadMask = _ALIMER_DEF(def.depthStencilState.stencilReadMask, 0xFF);
    def.depthStencilState.stencilWriteMask = _ALIMER_DEF(def.depthStencilState.stencilReadMask, 0xFF);
    def.depthStencilState.frontFace.compareFunction = _ALIMER_DEF(def.depthStencilState.frontFace.compareFunction, GPUCompareFunction_Always);
    def.depthStencilState.frontFace.failOperation = _ALIMER_DEF(def.depthStencilState.frontFace.failOperation, GPUStencilOperation_Keep);
    def.depthStencilState.frontFace.depthFailOperation = _ALIMER_DEF(def.depthStencilState.frontFace.depthFailOperation, GPUStencilOperation_Keep);
    def.depthStencilState.frontFace.passOperation = _ALIMER_DEF(def.depthStencilState.frontFace.passOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.compareFunction = _ALIMER_DEF(def.depthStencilState.backFace.compareFunction, GPUCompareFunction_Always);
    def.depthStencilState.backFace.failOperation = _ALIMER_DEF(def.depthStencilState.backFace.failOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.depthFailOperation = _ALIMER_DEF(def.depthStencilState.backFace.depthFailOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.passOperation = _ALIMER_DEF(def.depthStencilState.backFace.passOperation, GPUStencilOperation_Keep);

    def.primitiveTopology = _ALIMER_DEF(def.primitiveTopology, GPUPrimitiveTopology_TriangleList);
    def.patchControlPoints = _ALIMER_DEF(def.patchControlPoints, 1u);
    def.multisample.count = _ALIMER_DEF(def.multisample.count, 1u);
    def.multisample.mask = _ALIMER_DEF(def.multisample.mask, UINT32_MAX);
    
    for (uint32_t i = 0; i < def.colorAttachmentCount; ++i)
    {
        if (def.colorAttachments[i].format == PixelFormat_Undefined)
            break;

        GPURenderPipelineColorAttachmentDesc& attachment = def.colorAttachments[i];
        attachment.srcColorBlendFactor = _ALIMER_DEF(attachment.srcColorBlendFactor, GPUBlendFactor_One);
        attachment.destColorBlendFactor = _ALIMER_DEF(attachment.destColorBlendFactor, GPUBlendFactor_Zero);
        attachment.colorBlendOperation = _ALIMER_DEF(attachment.colorBlendOperation, GPUBlendOperation_Add);
        attachment.srcAlphaBlendFactor = _ALIMER_DEF(attachment.srcAlphaBlendFactor, GPUBlendFactor_One);
        attachment.destAlphaBlendFactor = _ALIMER_DEF(attachment.destAlphaBlendFactor, GPUBlendFactor_Zero);
        attachment.alphaBlendOperation = _ALIMER_DEF(attachment.alphaBlendOperation, GPUBlendOperation_Add);
    }

    return def;
}

GPURenderPipeline agpuCreateRenderPipeline(GPUDevice device, const GPURenderPipelineDesc* desc)
{
    if (!desc)
        return nullptr;

    GPURenderPipelineDesc descDef = _GPURenderPipelineDesc_Defaults(desc);
    return device->CreateRenderPipeline(descDef);
}

void agpuRenderPipelineSetLabel(GPURenderPipeline renderPipeline, const char* label)
{
    renderPipeline->SetLabel(label);
}

uint32_t agpuRenderPipelineAddRef(GPURenderPipeline renderPipeline)
{
    return renderPipeline->AddRef();
}

uint32_t agpuRenderPipelineRelease(GPURenderPipeline renderPipeline)
{
    return renderPipeline->Release();
}

/* Other */
struct VertexFormatInfo
{
    GPUVertexFormat format;
    uint32_t byteSize;
    uint32_t componentCount;
};

static const VertexFormatInfo kVertexFormatTable[] = {
    { GPUVertexFormat_Undefined,           0, 0 },
    { GPUVertexFormat_UByte,               1, 1 },
    { GPUVertexFormat_UByte2,              2, 2 },
    { GPUVertexFormat_UByte4,              4, 4 },
    { GPUVertexFormat_Byte,                1, 1 },
    { GPUVertexFormat_Byte2,               2, 2 },
    { GPUVertexFormat_Byte4,               4, 4 },
    { GPUVertexFormat_UByteNormalized,     1, 1 },
    { GPUVertexFormat_UByte2Normalized,    2, 2 },
    { GPUVertexFormat_UByte4Normalized,    4, 4 },
    { GPUVertexFormat_ByteNormalized,      2, 2 },
    { GPUVertexFormat_Byte2Normalized,     2, 2 },
    { GPUVertexFormat_Byte4Normalized,     4, 4 },

    { GPUVertexFormat_UShort,              2, 1 },
    { GPUVertexFormat_UShort2,             4, 2 },
    { GPUVertexFormat_UShort4,             8, 4 },
    { GPUVertexFormat_Short,               4, 1 },
    { GPUVertexFormat_Short2,              4, 2 },
    { GPUVertexFormat_Short4,              8, 4 },
    { GPUVertexFormat_UShortNormalized,    2, 1 },
    { GPUVertexFormat_UShort2Normalized,   4, 2 },
    { GPUVertexFormat_UShort4Normalized,   8, 4 },
    { GPUVertexFormat_ShortNormalized,     2, 1 },
    { GPUVertexFormat_Short2Normalized,    4, 2 },
    { GPUVertexFormat_Short4Normalized,    8, 4 },

    { GPUVertexFormat_Half,                2, 1 },
    { GPUVertexFormat_Half2,               4, 2 },
    { GPUVertexFormat_Half4,               8, 4 },
    { GPUVertexFormat_Float,               4, 1 },
    { GPUVertexFormat_Float2,              8, 2 },
    { GPUVertexFormat_Float3,              12, 3 },
    { GPUVertexFormat_Float4,              16, 4 },

    { GPUVertexFormat_UInt,                4, 1 },
    { GPUVertexFormat_UInt2,               8, 2 },
    { GPUVertexFormat_UInt3,               12, 3 },
    { GPUVertexFormat_UInt4,               16, 4 },

    { GPUVertexFormat_Int,                 4, 1 },
    { GPUVertexFormat_Int2,                8, 2 },
    { GPUVertexFormat_Int3,                12, 3 },
    { GPUVertexFormat_Int4,                16, 4 },

    { GPUVertexFormat_Unorm10_10_10_2, 4, 4 },
    { GPUVertexFormat_Unorm8x4BGRA, 4, 4 }
    //{VertexFormat::RG11B10Float,   32, 4,  VertexFormatKind::Float},
    //{VertexFormat::RGB9E5Float,   32, 4, VertexFormatKind::Float},
};

static_assert(
    sizeof(kVertexFormatTable) / sizeof(VertexFormatInfo) == size_t(_GPUVertexFormat_Count),
    "The format info table doesn't have the right number of elements"
    );

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

static const VertexFormatInfo& GetVertexFormatInfo(GPUVertexFormat format)
{
    if (format >= _PixelFormat_Count)
        return kVertexFormatTable[0]; // Undefined

    const VertexFormatInfo& info = kVertexFormatTable[format];
    ALIMER_ASSERT(info.format == format);
    return info;
}

uint32_t agpuGetVertexFormatByteSize(GPUVertexFormat format)
{
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.byteSize;
}

uint32_t agpuGetVertexFormatComponentCount(GPUVertexFormat format)
{
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.componentCount;
}

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
