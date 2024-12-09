// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_H_
#define ALIMER_GPU_H_

#include "alimer.h"

#if !defined(GPU_NULLABLE)
#define GPU_NULLABLE
#endif

/* Forward declarations */
typedef struct GPUAdapter               GPUAdapter;
typedef struct GPUSurface               GPUSurface;
typedef struct GPUDevice                GPUDevice;
typedef struct GPUQueue                 GPUQueue;
typedef struct GPUCommandBuffer         GPUCommandBuffer;
typedef struct GPURenderCommandEncoder  GPURenderCommandEncoder;
typedef struct GPUBuffer                GPUBuffer;
typedef struct GPUTexture               GPUTexture;
typedef struct GPUTextureView           GPUTextureView;
typedef struct GPUShaderModule          GPUShaderModule;

/* Types */
typedef uint64_t GPUDeviceAddress;

/* Constants */
#define GPU_MAX_INFLIGHT_FRAMES (2u)
#define GPU_MAX_VERTEX_BUFFER_BINDINGS (8u)

/* Enums */
typedef enum GPUResult {
    GPUResult_Success = 0,
    GPUResult_InvalidOperation = -1,

    _GPUResult_Force32 = 0x7FFFFFFF
} GPUResult;

typedef enum GPUMemoryType {
    /// CPU no access, GPU read/write
    GPUMemoryType_Private,
    /// CPU write, GPU read
    GPUMemoryType_Upload,
    /// CPU read, GPU write
    GPUMemoryType_Readback,

    GPUMemoryType_Count,
    _GPUMemoryType_Force32 = 0x7FFFFFFF
} GPUMemoryType;

typedef uint64_t GPUBufferUsage;
static const GPUBufferUsage GPUBufferUsage_None = 0;
static const GPUBufferUsage GPUBufferUsage_Vertex = (1 << 0);
static const GPUBufferUsage GPUBufferUsage_Index = (1 << 1);
/// Supports Constant buffer access.
static const GPUBufferUsage GPUBufferUsage_Constant = (1 << 2);
static const GPUBufferUsage GPUBufferUsage_ShaderRead = (1 << 3);
static const GPUBufferUsage GPUBufferUsage_ShaderWrite = (1 << 4);
/// Supports indirect buffer access for indirect draw/dispatch.
static const GPUBufferUsage GPUBufferUsage_Indirect = (1 << 5);
/// Supports predication access for conditional rendering.
static const GPUBufferUsage GPUBufferUsage_Predication = (1 << 6);
/// Supports ray tracing acceleration structure usage.
static const GPUBufferUsage GPUBufferUsage_RayTracing = (1 << 7);

typedef uint64_t GPUTextureUsage;
static const GPUTextureUsage GPUTextureUsage_None = 0;
static const GPUTextureUsage GPUTextureUsage_ShaderRead = (1 << 0);
static const GPUTextureUsage GPUTextureUsage_ShaderWrite = (1 << 1);
static const GPUTextureUsage GPUTextureUsage_RenderTarget = (1 << 2);
static const GPUTextureUsage GPUTextureUsage_Transient = (1 << 3);
static const GPUTextureUsage GPUTextureUsage_ShadingRate = (1 << 4);

typedef enum GPUTextureAspect {
    GPUTextureAspect_All = 0,
    GPUTextureAspect_DepthOnly = 1,
    GPUTextureAspect_StencilOnly = 2,

    _GPUTextureAspect_Count,
    _GPUTextureAspect_Force32 = 0x7FFFFFFF
} GPUTextureAspect;

typedef enum GPUBackendType {
    GPUBackendType_Undefined = 0,
    GPUBackendType_Null,
    GPUBackendType_Vulkan,
    GPUBackendType_D3D12,
    GPUBackendType_Metal,
    GPUBackendType_WebGPU,

    _GPUBackendType_Count,
    _GPUBackendType_Force32 = 0x7FFFFFFF
} GPUBackendType;

typedef enum GPUValidationMode {
    GPUValidationMode_Disabled = 0,
    GPUValidationMode_Enabled,
    GPUValidationMode_Verbose,
    GPUValidationMode_GPU,

    _GPUValidationMode_Count,
    _GPUValidationMode_Force32 = 0x7FFFFFFF
} GPUValidationMode;

typedef enum GPUPowerPreference {
    GPUPowerPreference_Undefined = 0,
    GPUPowerPreference_LowPower = 1,
    GPUPowerPreference_HighPerformance = 2,

    _GPUPowerPreference_Force32 = 0x7FFFFFFF
} GPUPowerPreference;

typedef enum GPUQueueType {
    GPUQueueType_Graphics = 0,
    GPUQueueType_Compute,
    GPUQueueType_Copy,
    GPUQueueType_VideoDecode,

    GPUQueueType_Count,
    _GPUQueueType_Force32 = 0x7FFFFFFF
} GPUQueueType;

typedef enum GPUVertexFormat {
    GPUVertexFormat_Undefined = 0,
    GPUVertexFormat_UByte,
    GPUVertexFormat_UByte2,
    GPUVertexFormat_UByte4,
    GPUVertexFormat_Byte,
    GPUVertexFormat_Byte2,
    GPUVertexFormat_Byte4,
    GPUVertexFormat_UByteNormalized,
    GPUVertexFormat_UByte2Normalized,
    GPUVertexFormat_UByte4Normalized,
    GPUVertexFormat_ByteNormalized,
    GPUVertexFormat_Byte2Normalized,
    GPUVertexFormat_Byte4Normalized,
    GPUVertexFormat_UShort,
    GPUVertexFormat_UShort2,
    GPUVertexFormat_UShort4,
    GPUVertexFormat_Short,
    GPUVertexFormat_Short2,
    GPUVertexFormat_Short4,
    GPUVertexFormat_UShortNormalized,
    GPUVertexFormat_UShort2Normalized,
    GPUVertexFormat_UShort4Normalized,
    GPUVertexFormat_ShortNormalized,
    GPUVertexFormat_Short2Normalized,
    GPUVertexFormat_Short4Normalized,
    GPUVertexFormat_Half,
    GPUVertexFormat_Half2,
    GPUVertexFormat_Half4,
    GPUVertexFormat_Float,
    GPUVertexFormat_Float2,
    GPUVertexFormat_Float3,
    GPUVertexFormat_Float4,
    GPUVertexFormat_UInt,
    GPUVertexFormat_UInt2,
    GPUVertexFormat_UInt3,
    GPUVertexFormat_UInt4,
    GPUVertexFormat_Int,
    GPUVertexFormat_Int2,
    GPUVertexFormat_Int3,
    GPUVertexFormat_Int4,
    GPUVertexFormat_Unorm10_10_10_2,
    GPUVertexFormat_Unorm8x4BGRA,

    _GPUVertexFormat_Count,
    _GPUVertexFormat_Force32 = 0x7FFFFFFF
} GPUVertexFormat;

typedef enum GPUCompareFunction {
    GPUCompareFunction_Undefined = 0,
    GPUCompareFunction_Never,
    GPUCompareFunction_Less,
    GPUCompareFunction_Equal,
    GPUCompareFunction_LessEqual,
    GPUCompareFunction_Greater,
    GPUCompareFunction_NotEqual,
    GPUCompareFunction_GreaterEqual,
    GPUCompareFunction_Always,

    _GPUCompareFunction_Count,
    _GPUCompareFunction_Force32 = 0x7FFFFFFF
} GPUCompareFunction;

typedef enum GPULoadAction {
    GPULoadAction_Load,
    GPULoadAction_Clear,
    GPULoadAction_Discard,

    _GPULoadAction_Count,
    _GPULoadAction_Force32 = 0x7FFFFFFF
} GPULoadAction;

typedef enum GPUStoreAction {
    GPUStoreAction_Store,
    GPUStoreAction_Discard,

    _GPUStoreAction_Count,
    _GPUStoreAction_Force32 = 0x7FFFFFFF
} GPUStoreAction;

/* Structs */
typedef struct ScissorRect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} ScissorRect;

typedef struct GPUViewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
} GPUViewport;

typedef struct GPUColor {
    float r;
    float g;
    float b;
    float a;
} GPUColor;

typedef struct GPUCommandBufferDesc {
    const char* label;
} GPUCommandBufferDesc;

typedef struct GPUBufferDesc {
    const char* label;
    uint64_t size;
    GPUBufferUsage usage;
    GPUMemoryType memoryType;
} GPUBufferDesc;

typedef struct GPUTextureDesc {
    const char* label;
    TextureDimension dimension;
    PixelFormat format;
    GPUTextureUsage usage;
    uint64_t width;
    uint64_t height;
    uint32_t depthOrArrayLayers;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
} GPUTextureDesc;

typedef struct GPUTextureData
{
    const void* pData;
    uint32_t rowPitch;
    uint32_t slicePitch;
} GPUTextureData;

typedef struct GPURenderPassColorAttachment {
    GPU_NULLABLE GPUTexture* texture;
    uint32_t mipLevel;
    GPULoadAction loadAction;
    GPUStoreAction storeAction;
    GPUColor clearColor;
} GPURenderPassColorAttachment;

typedef struct GPURenderPassDepthStencilAttachment {
    GPUTexture* texture;
    uint32_t mipLevel;
    GPULoadAction depthLoadAction;
    GPUStoreAction depthStoreAction;
    float depthClearValue;
    bool depthReadOnly;
    GPULoadAction stencilLoadActin;
    GPUStoreAction stencilStoreAction;
    uint32_t stencilClearValue;
    bool stencilReadOnly;
} GPURenderPassDepthStencilAttachment;

typedef struct GPURenderPassDesc {
    const char* label;
    uint32_t colorAttachmentCount;
    const GPURenderPassColorAttachment* colorAttachments;
    const GPURenderPassDepthStencilAttachment* depthStencilAttachment;
} GPURenderPassDesc;

typedef struct GPURequestAdapterOptions {
    GPUSurface* compatibleSurface;
    GPUPowerPreference powerPreference;
} GPURequestAdapterOptions;

typedef struct GPULimits {
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    uint32_t maxTextureDimensionCube;
    uint32_t maxTextureArrayLayers;
    uint32_t maxConstantBufferBindingSize;
    uint32_t maxStorageBufferBindingSize;
    uint32_t minConstantBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment;
    uint64_t maxBufferSize;
    uint32_t maxColorAttachments;
    uint32_t maxComputeWorkgroupStorageSize;
    uint32_t maxComputeInvocationsPerWorkgroup;
    uint32_t maxComputeWorkgroupSizeX;
    uint32_t maxComputeWorkgroupSizeY;
    uint32_t maxComputeWorkgroupSizeZ;
    uint32_t maxComputeWorkgroupsPerDimension;
} GPULimits;

typedef struct GPUSurfaceCapabilities {
    PixelFormat preferredFormat;
    GPUTextureUsage supportedUsage;
    uint32_t formatCount;
    const PixelFormat* formats;
} GPUSurfaceCapabilities;

typedef struct GPUSurfaceConfig {
    GPUDevice* device;
    PixelFormat format;
    uint32_t width;
    uint32_t height;
} GPUSurfaceConfig;

typedef struct GPUConfig {
    GPUBackendType preferredBackend;
    GPUValidationMode validationMode;
} GPUConfig;

ALIMER_API bool agpuIsBackendSupport(GPUBackendType backend);
ALIMER_API bool agpuInit(const GPUConfig* config);
ALIMER_API void agpuShutdown(void);
ALIMER_API GPUAdapter* agpuRequestAdapter(const GPURequestAdapterOptions* options);

/* Surface */
ALIMER_API GPUSurface* agpuSurfaceCreate(Window* window);
ALIMER_API GPUResult agpuSurfaceGetCapabilities(GPUSurface* surface, GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities);
ALIMER_API bool agpuSurfaceConfigure(GPUSurface* surface, const GPUSurfaceConfig* config);
ALIMER_API void agpuSurfaceUnconfigure(GPUSurface* surface);
ALIMER_API GPUResult agpuSurfaceGetCurrentTexture(GPUSurface* surface, GPUTexture** surfaceTexture);
ALIMER_API GPUResult agpuSurfacePresent(GPUSurface* surface);
ALIMER_API uint32_t agpuSurfaceAddRef(GPUSurface* surface);
ALIMER_API uint32_t agpuSurfaceRelease(GPUSurface* surface);

/* Adapter */
ALIMER_API GPUResult agpuAdapterGetLimits(GPUAdapter* adapter, GPULimits* limits);
ALIMER_API GPUDevice* agpuAdapterCreateDevice(GPUAdapter* adapter);
ALIMER_API uint32_t agpuAdapterAddRef(GPUAdapter* adapter);
ALIMER_API uint32_t agpuAdapterRelease(GPUAdapter* adapter);

/* Device */
ALIMER_API uint32_t agpuDeviceAddRef(GPUDevice* device);
ALIMER_API uint32_t agpuDeviceRelease(GPUDevice* device);
ALIMER_API GPUQueue* agpuDeviceGetQueue(GPUDevice* device, GPUQueueType type);
ALIMER_API bool agpuDeviceWaitIdle(GPUDevice* device);

/// Commit the current frame and advance to next frame
ALIMER_API uint64_t agpuDeviceCommitFrame(GPUDevice* device);

ALIMER_API GPUBuffer* agpuDeviceCreateBuffer(GPUDevice* device, const GPUBufferDesc* desc, const void* pInitialData);
ALIMER_API GPUTexture* agpuDeviceCreateTexture(GPUDevice* device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData);

/* Queue */
ALIMER_API GPUQueueType agpuQueueGetType(GPUQueue* queue);
ALIMER_API GPUCommandBuffer* agpuQueueAcquireCommandBuffer(GPUQueue* queue, const GPUCommandBufferDesc* desc);
ALIMER_API void agpuQueueSubmit(GPUQueue* queue, uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers);

/* CommandBuffer */
ALIMER_API void agpuCommandBufferPushDebugGroup(GPUCommandBuffer* commandBuffer, const char* groupLabel);
ALIMER_API void agpuCommandBufferPopDebugGroup(GPUCommandBuffer* commandBuffer);
ALIMER_API void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer* commandBuffer, const char* markerLabel);
ALIMER_API GPURenderCommandEncoder* agpuCommandBufferBeginRenderPass(GPUCommandBuffer* commandBuffer, const GPURenderPassDesc* desc);

/* RenderCommandEncoder */
ALIMER_API void agpuRenderCommandEncoderPushDebugGroup(GPURenderCommandEncoder* encoder, const char* groupLabel);
ALIMER_API void agpuRenderCommandEncoderPopDebugGroup(GPURenderCommandEncoder* encoder);
ALIMER_API void agpuRenderCommandEncoderInsertDebugMarker(GPURenderCommandEncoder* encoder, const char* markerLabel);
ALIMER_API void agpuRenderPassEncoderEnd(GPURenderCommandEncoder* encoder);

/* Buffer */
ALIMER_API uint32_t agpuBufferAddRef(GPUBuffer* buffer);
ALIMER_API uint32_t agpuBufferRelease(GPUBuffer* buffer);
ALIMER_API uint64_t agpuBufferGetSize(GPUBuffer* buffer);
ALIMER_API GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer* buffer);

/* Texture */
ALIMER_API uint32_t agpuTextureAddRef(GPUTexture* texture);
ALIMER_API uint32_t agpuTextureRelease(GPUTexture* texture);

#endif /* ALIMER_GPU_H_ */
