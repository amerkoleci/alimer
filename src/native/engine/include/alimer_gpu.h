// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_H_
#define ALIMER_GPU_H_

#include "alimer.h"

/* Forward declarations */
typedef struct GPUAdapterImpl*              GPUAdapter;
typedef struct GPUSurfaceImpl*              GPUSurface;
typedef struct GPUDeviceImpl*               GPUDevice;
typedef struct GPUQueueImpl*                GPUQueue;
typedef struct GPUCommandBufferImpl*        GPUCommandBuffer;
typedef struct GPUComputePassEncoderImpl*   GPUComputePassEncoder;
typedef struct GPURenderPassEncoderImpl*    GPURenderPassEncoder;
typedef struct GPUBufferImpl*               GPUBuffer;
typedef struct GPUTextureImpl*              GPUTexture;
typedef struct GPUSamplerImpl*              GPUSampler;
typedef struct GPUQuerySetImpl*             GPUQuerySet;
typedef struct GPUShaderModuleImpl*         GPUShaderModule;
typedef struct GPUBindGroupLayoutImpl*      GPUBindGroupLayout;
typedef struct GPUBindGroupImpl*            GPUBindGroup;
typedef struct GPUPipelineLayoutImpl*       GPUPipelineLayout;
typedef struct GPUComputePipelineImpl*      GPUComputePipeline;
typedef struct GPURenderPipelineImpl*       GPURenderPipeline;

/* Types */
typedef uint32_t GPUBool;
typedef uint64_t GPUDeviceAddress;

/* Constants */
#define GPU_MAX_INFLIGHT_FRAMES (3u)
#define GPU_MAX_COLOR_ATTACHMENTS (8u)
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
    GPULoadAction_Undefined = 0,
    GPULoadAction_Discard,
    GPULoadAction_Load,
    GPULoadAction_Clear,

    _GPULoadAction_Count,
    _GPULoadAction_Force32 = 0x7FFFFFFF
} GPULoadAction;

typedef enum GPUStoreAction {
    GPUStoreAction_Undefined = 0,
    GPUStoreAction_Discard,
    GPUStoreAction_Store,

    _GPUStoreAction_Count,
    _GPUStoreAction_Force32 = 0x7FFFFFFF
} GPUStoreAction;

typedef enum GPUPresentMode {
    GPUPresentMode_Undefined = 0,
    GPUPresentMode_Fifo,
    GPUPresentMode_FifoRelaxed,
    GPUPresentMode_Immediate,
    GPUPresentMode_Mailbox,

    _GPUPresentMode_Count,
    _GPUPresentMode_Force32 = 0x7FFFFFFF
} GPUPresentMode;

typedef enum GPUShaderStage {
    GPUShaderStage_Undefined,
    GPUShaderStage_Vertex,
    GPUShaderStage_Fragment,
    GPUShaderStage_Compute,
    GPUShaderStage_Amplification,
    GPUShaderStage_Mesh,

    _GPUShaderStage_Count,
    _GPUShaderStage_Force32 = 0x7FFFFFFF
} GPUShaderStage;

typedef enum GPUVertexStepMode {
    GPUVertexStepMode_Undefined = 0,
    GPUVertexStepMode_Vertex = 1,
    GPUVertexStepMode_Instance = 2,

    _GPUVertexStepMode_Force32 = 0x7FFFFFFF
} GPUVertexStepMode;

typedef enum GPUAcquireSurfaceResult {
    /// Everything is good and we can render this frame
    GPUAcquireSurfaceResult_SuccessOptimal = 0,
    /// Still OK - the surface can present the frame, but in a suboptimal way. The surface may need reconfiguration.
    GPUAcquireSurfaceResult_SuccessSuboptimal,
    /// A timeout was encountered while trying to acquire the next frame.
    GPUAcquireSurfaceResult_Timeout,
    /// The underlying surface has changed, and therefore the swap chain must be updated.
    GPUAcquireSurfaceResult_Outdated,
    /// The swap chain has been lost and needs to be recreated.
    GPUAcquireSurfaceResult_Lost,
    /// There is no more memory left to allocate a new frame.
    GPUAcquireSurfaceResult_OutOfMemory,
    /// Acquiring a texture failed with a generic error. Check error callbacks for more information.
    GPUAcquireSurfaceResult_Other,

    _GPUAcquireSurfaceResult_Force32 = 0x7FFFFFFF
} GPUAcquireSurfaceResult;

typedef enum GPUAdapterVendor {
    /// Adapter vendor is unknown
    GPUAdapterVendor_Unknown = 0,

    /// Adapter vendor is NVIDIA
    GPUAdapterVendor_NVIDIA,

    /// Adapter vendor is AMD
    GPUAdapterVendor_AMD,

    /// Adapter vendor is Intel
    GPUAdapterVendor_Intel,

    /// Adapter vendor is ARM
    GPUAdapterVendor_ARM,

    /// Adapter vendor is Qualcomm
    GPUAdapterVendor_Qualcomm,

    /// Adapter vendor is Imagination Technologies
    GPUAdapterVendor_ImgTech,

    /// Adapter vendor is Microsoft (software rasterizer)
    GPUAdapterVendor_MSFT,

    /// Adapter vendor is Apple
    GPUAdapterVendor_Apple,

    /// Adapter vendor is Mesa (software rasterizer)
    GPUAdapterVendor_Mesa,

    /// Adapter vendor is Broadcom (Raspberry Pi)
    GPUAdapterVendor_Broadcom,

    _GPUAdapterVendor_Count,
    _GPUAdapterVendor_Force32 = 0x7FFFFFFF
} GPUAdapterVendor;

typedef enum GPUAdapterType {
    GPUAdapterType_DiscreteGPU,
    GPUAdapterType_IntegratedGPU,
    GPUAdapterType_CPU,
    GPUAdapterType_Unknown,

    _GPUAdapterType_Force32 = 0x7FFFFFFF
} GPUAdapterType;

typedef enum GPUConservativeRasterizationTier
{
    GPUConservativeRasterizationTier_NotSupported = 0,
    GPUConservativeRasterizationTier_1 = 1,
    GPUConservativeRasterizationTier_2 = 2,
    GPUConservativeRasterizationTier_3 = 3,

    _GPUConservativeRasterizationTier_Force32 = 0x7FFFFFFF
} GPUConservativeRasterizationTier;

typedef enum GPUFeature {
    GPUFeature_DepthClipControl,
    GPUFeature_Depth32FloatStencil8,
    GPUFeature_TimestampQuery,
    GPUFeature_PipelineStatisticsQuery,
    GPUFeature_TextureCompressionBC,
    GPUFeature_TextureCompressionETC2,
    GPUFeature_TextureCompressionASTC,
    GPUFeature_TextureCompressionASTC_HDR,
    GPUFeature_IndirectFirstInstance,
    GPUFeature_DualSourceBlending,
    GPUFeature_ShaderFloat16,

    GPUFeature_GPUUploadHeapSupported,
    GPUFeature_CopyQueueTimestampQueriesSupported,
    GPUFeature_CacheCoherentUMA,
    GPUFeature_ShaderOutputViewportIndex,
    GPUFeature_ConservativeRasterization,

    _GPUFeature_Force32 = 0x7FFFFFFF
} GPUFeature;

/* Flags/Bitmask Enums */
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
/// Supports shared handle usage.
static const GPUTextureUsage GPUTextureUsage_Shared = (1 << 5);

/* Structs */
typedef struct GPUScissorRect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} GPUScissorRect;

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
    const char* label DEFAULT_INITIALIZER(nullptr);
    uint64_t size DEFAULT_INITIALIZER(0);
    GPUBufferUsage usage DEFAULT_INITIALIZER(GPUBufferUsage_None);
    GPUMemoryType memoryType DEFAULT_INITIALIZER(GPUMemoryType_Private);
} GPUBufferDesc;

typedef struct GPUTextureDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    TextureDimension dimension DEFAULT_INITIALIZER(TextureDimension_2D);
    PixelFormat format DEFAULT_INITIALIZER(PixelFormat_RGBA8Unorm);
    GPUTextureUsage usage DEFAULT_INITIALIZER(GPUTextureUsage_None);
    uint32_t width DEFAULT_INITIALIZER(0);
    uint32_t height DEFAULT_INITIALIZER(0);
    uint32_t depthOrArrayLayers DEFAULT_INITIALIZER(1);
    uint32_t mipLevelCount DEFAULT_INITIALIZER(1);
    uint32_t sampleCount DEFAULT_INITIALIZER(1);
} GPUTextureDesc;

typedef struct GPUTextureData {
    const void* pData DEFAULT_INITIALIZER(nullptr);
    uint32_t rowPitch DEFAULT_INITIALIZER(0);
    uint32_t slicePitch DEFAULT_INITIALIZER(0);
} GPUTextureData;

typedef struct GPUSamplerDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
} GPUSamplerDesc;

typedef struct GPUBindGroupLayoutDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
} GPUBindGroupLayoutDesc;

typedef struct GPUPipelineLayoutDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
} GPUPipelineLayoutDesc;

typedef struct GPUShaderModuleDesc {
    size_t bytecodeSize;
    const void* bytecode;
} GPUShaderModuleDesc;

typedef struct GPUComputeState {
    GPUShaderModule module;
    const char* entryPoint DEFAULT_INITIALIZER("main");
} GPUComputeState;

typedef struct GPUComputePipelineDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout layout;
    GPUComputeState compute;
} GPUComputePipelineDesc;

typedef struct GPUVertexAttribute {
    GPUVertexFormat format;
    uint32_t offset;
    uint32_t shaderLocation;
} GPUVertexAttribute;

typedef struct GPUVertexBufferLayout {
    uint32_t stride;
    GPUVertexStepMode stepMode;
    uint32_t attributeCount;
    const GPUVertexAttribute* attributes;
} GPUVertexBufferLayout;

typedef struct GPUVertexState {
    GPUShaderModule module;
    const char* entryPoint DEFAULT_INITIALIZER("main");
    uint32_t bufferCount DEFAULT_INITIALIZER(0);
    const GPUVertexBufferLayout* buffers DEFAULT_INITIALIZER(nullptr);
} GPUVertexState;

typedef struct GPUMultisampleState {
    uint32_t count;
    uint32_t mask;
    GPUBool alphaToCoverageEnabled;
} GPUMultisampleState;

typedef struct GPURenderPipelineDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout layout;
    GPUVertexState vertex;
    const GPUMultisampleState* multisample DEFAULT_INITIALIZER(nullptr);
} GPURenderPipelineDesc;

typedef struct GPURenderPassColorAttachment {
    GPUTexture texture DEFAULT_INITIALIZER(nullptr);
    uint32_t mipLevel DEFAULT_INITIALIZER(0);
    GPULoadAction loadAction DEFAULT_INITIALIZER(GPULoadAction_Discard);
    GPUStoreAction storeAction;
    GPUColor clearColor;
} GPURenderPassColorAttachment;

typedef struct GPURenderPassDepthStencilAttachment {
    GPUTexture texture;
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

typedef struct GPUComputePassDesc {
    const char* label;
} GPUComputePassDesc;

typedef struct GPURenderPassDesc {
    const char* label;
    uint32_t colorAttachmentCount;
    const GPURenderPassColorAttachment* colorAttachments;
    const GPURenderPassDepthStencilAttachment* depthStencilAttachment;
} GPURenderPassDesc;

typedef struct GPURequestAdapterOptions {
    GPUSurface compatibleSurface;
    GPUPowerPreference powerPreference;
} GPURequestAdapterOptions;

typedef struct GPUDeviceDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    uint32_t maxFramesInFlight DEFAULT_INITIALIZER(2);
} GPUDeviceDesc;

typedef struct GPUAdapterInfo {
    const char* deviceName;
    uint16_t driverVersion[4];
    const char* driverDescription;
    GPUAdapterType adapterType;
    GPUAdapterVendor vendor;
    uint32_t vendorID;
    uint32_t deviceID;
} GPUAdapterInfo;

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
    uint32_t maxViewports;
    float    viewportBoundsMin;
    float    viewportBoundsMax;

    uint32_t maxComputeWorkgroupStorageSize;
    uint32_t maxComputeInvocationsPerWorkgroup;
    uint32_t maxComputeWorkgroupSizeX;
    uint32_t maxComputeWorkgroupSizeY;
    uint32_t maxComputeWorkgroupSizeZ;
    uint32_t maxComputeWorkgroupsPerDimension;

    GPUConservativeRasterizationTier conservativeRasterizationTier;
} GPULimits;

typedef struct GPUSurfaceCapabilities {
    PixelFormat preferredFormat;
    GPUTextureUsage supportedUsage;
    uint32_t formatCount;
    const PixelFormat* formats;
} GPUSurfaceCapabilities;

typedef struct GPUSurfaceConfig {
    GPUDevice device;
    PixelFormat format;
    uint32_t width;
    uint32_t height;
    GPUPresentMode presentMode;
} GPUSurfaceConfig;

typedef struct GPUConfig {
    GPUBackendType preferredBackend;
    GPUValidationMode validationMode;
} GPUConfig;

/* Indirect Commands Structs */
typedef struct GPUDispatchIndirectCommand
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
} GPUDispatchIndirectCommand;

ALIMER_API bool agpuIsBackendSupport(GPUBackendType backend);
ALIMER_API bool agpuInit(const GPUConfig* config);
ALIMER_API void agpuShutdown(void);
ALIMER_API GPUAdapter agpuRequestAdapter(const GPURequestAdapterOptions* options);

/* Surface */
ALIMER_API GPUSurface agpuSurfaceCreate(Window* window);
ALIMER_API GPUResult agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter adapter, GPUSurfaceCapabilities* capabilities);
ALIMER_API bool agpuSurfaceConfigure(GPUSurface surface, const GPUSurfaceConfig* config);
ALIMER_API void agpuSurfaceUnconfigure(GPUSurface surface);
ALIMER_API uint32_t agpuSurfaceAddRef(GPUSurface surface);
ALIMER_API uint32_t agpuSurfaceRelease(GPUSurface surface);

/* Adapter */
ALIMER_API GPUResult agpuAdapterGetInfo(GPUAdapter adapter, GPUAdapterInfo* info);
ALIMER_API GPUResult agpuAdapterGetLimits(GPUAdapter adapter, GPULimits* limits);
ALIMER_API bool agpuAdapterHasFeature(GPUAdapter adapter, GPUFeature feature);
ALIMER_API GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter, const GPUDeviceDesc* desc);

/* Device */
ALIMER_API void agpuDeviceSetLabel(GPUDevice device, const char* label);
ALIMER_API uint32_t agpuDeviceAddRef(GPUDevice device);
ALIMER_API uint32_t agpuDeviceRelease(GPUDevice device);
ALIMER_API bool agpuDeviceHasFeature(GPUDevice device, GPUFeature feature);
ALIMER_API GPUQueue agpuDeviceGetQueue(GPUDevice device, GPUQueueType type);
ALIMER_API bool agpuDeviceWaitIdle(GPUDevice device);

/// Commit the current frame and advance to next frame
ALIMER_API uint64_t agpuDeviceCommitFrame(GPUDevice device);

ALIMER_API GPUBuffer agpuDeviceCreateBuffer(GPUDevice device, const GPUBufferDesc* desc, const void* pInitialData);
ALIMER_API GPUTexture agpuDeviceCreateTexture(GPUDevice device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData);
ALIMER_API GPUPipelineLayout agpuDeviceCreatePipelineLayout(GPUDevice device, const GPUPipelineLayoutDesc* desc);
ALIMER_API GPUShaderModule agpuDeviceCreateShaderModule(GPUDevice device, const GPUShaderModuleDesc* desc);
ALIMER_API GPUComputePipeline agpuDeviceCreateComputePipeline(GPUDevice device, const GPUComputePipelineDesc* desc);
ALIMER_API GPURenderPipeline agpuDeviceCreateRenderPipeline(GPUDevice device, const GPURenderPipelineDesc* desc);

/* Queue */
ALIMER_API GPUQueueType agpuQueueGetType(GPUQueue queue);
ALIMER_API GPUCommandBuffer agpuQueueAcquireCommandBuffer(GPUQueue queue, const GPUCommandBufferDesc* desc);
ALIMER_API void agpuQueueSubmit(GPUQueue queue, uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers);

/* CommandBuffer */
ALIMER_API void agpuCommandBufferPushDebugGroup(GPUCommandBuffer commandBuffer, const char* groupLabel);
ALIMER_API void agpuCommandBufferPopDebugGroup(GPUCommandBuffer commandBuffer);
ALIMER_API void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer commandBuffer, const char* markerLabel);
ALIMER_API GPUAcquireSurfaceResult agpuCommandBufferAcquireSurfaceTexture(GPUCommandBuffer commandBuffer, GPUSurface surface, GPUTexture* surfaceTexture);
ALIMER_API GPUComputePassEncoder agpuCommandBufferBeginComputePass(GPUCommandBuffer commandBuffer, const GPUComputePassDesc* desc);
ALIMER_API GPURenderPassEncoder agpuCommandBufferBeginRenderPass(GPUCommandBuffer commandBuffer, const GPURenderPassDesc* desc);

/* ComputePassEncoder */
ALIMER_API void agpuComputePassEncoderDispatch(GPUComputePassEncoder computePassEncoder, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
ALIMER_API void agpuComputePassEncoderDispatchIndirect(GPUComputePassEncoder computePassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_API void agpuComputePassEncoderEnd(GPUComputePassEncoder computePassEncoder);
ALIMER_API void agpuComputePassEncoderPushDebugGroup(GPUComputePassEncoder computePassEncoder, const char* groupLabel);
ALIMER_API void agpuComputePassEncoderPopDebugGroup(GPUComputePassEncoder computePassEncoder);
ALIMER_API void agpuComputePassEncoderInsertDebugMarker(GPUComputePassEncoder computePassEncoder, const char* markerLabel);

/* RenderCommandEncoder */
ALIMER_API void agpuRenderPassEncoderSetViewport(GPURenderPassEncoder renderPassEncoder, const GPUViewport* viewport);
ALIMER_API void agpuRenderPassEncoderSetViewports(GPURenderPassEncoder renderPassEncoder, uint32_t viewportCount, const GPUViewport* viewports);
ALIMER_API void agpuRenderPassEncoderSetScissorRect(GPURenderPassEncoder renderPassEncoder, const GPUScissorRect* scissorRect);
ALIMER_API void agpuRenderPassEncoderSetScissorRects(GPURenderPassEncoder renderPassEncoder, uint32_t scissorCount, const GPUScissorRect* scissorRects);
ALIMER_API void agpuRenderPassEncoderSetStencilReference(GPURenderPassEncoder renderPassEncoder, uint32_t reference);
ALIMER_API void agpuRenderPassEncoderEnd(GPURenderPassEncoder renderPassEncoder);
ALIMER_API void agpuRenderPassEncoderPushDebugGroup(GPURenderPassEncoder renderPassEncoder, const char* groupLabel);
ALIMER_API void agpuRenderPassEncoderPopDebugGroup(GPURenderPassEncoder renderPassEncoder);
ALIMER_API void agpuRenderPassEncoderInsertDebugMarker(GPURenderPassEncoder renderPassEncoder, const char* markerLabel);

/* Buffer */
ALIMER_API void agpuBufferSetLabel(GPUBuffer buffer, const char* label);
ALIMER_API uint32_t agpuBufferAddRef(GPUBuffer buffer);
ALIMER_API uint32_t agpuBufferRelease(GPUBuffer buffer);
ALIMER_API uint64_t agpuBufferGetSize(GPUBuffer buffer);
ALIMER_API GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer buffer);

/* Texture */
ALIMER_API void agpuTextureSetLabel(GPUTexture texture, const char* label);
ALIMER_API TextureDimension agpuTextureGetDimension(GPUTexture texture);
ALIMER_API PixelFormat agpuTextureGetFormat(GPUTexture texture);
ALIMER_API GPUTextureUsage agpuTextureGetUsage(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetWidth(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetHeight(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetDepthOrArrayLayers(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetMipLevelCount(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetSampleCount(GPUTexture texture);
ALIMER_API uint32_t agpuTextureGetLevelWidth(GPUTexture texture, uint32_t mipLevel);
ALIMER_API uint32_t agpuTextureGetLevelHeight(GPUTexture texture, uint32_t mipLevel);
ALIMER_API uint32_t agpuTextureAddRef(GPUTexture texture);
ALIMER_API uint32_t agpuTextureRelease(GPUTexture texture);

/* Sampler */
ALIMER_API void agpuSamplerSetLabel(GPUSampler sampler, const char* label);
ALIMER_API uint32_t agpuSamplerAddRef(GPUSampler sampler);
ALIMER_API uint32_t agpuSamplerRelease(GPUSampler sampler);

/* PipelineLayout */
ALIMER_API void agpuPipelineLayoutSetLabel(GPUPipelineLayout pipelineLayout, const char* label);
ALIMER_API uint32_t agpuPipelineLayoutAddRef(GPUPipelineLayout pipelineLayout);
ALIMER_API uint32_t agpuPipelineLayoutRelease(GPUPipelineLayout pipelineLayout);

/* ShaderModule */
ALIMER_API void agpuShaderModuleSetLabel(GPUShaderModule shaderModule, const char* label);
ALIMER_API uint32_t agpuShaderModuleAddRef(GPUShaderModule shaderModule);
ALIMER_API uint32_t agpuShaderModuleRelease(GPUShaderModule shaderModule);

/* ComputePipeline */
ALIMER_API void agpuComputePipelineSetLabel(GPUComputePipeline computePipeline, const char* label);
ALIMER_API uint32_t agpuComputePipelineAddRef(GPUComputePipeline computePipeline);
ALIMER_API uint32_t agpuComputePipelineRelease(GPUComputePipeline computePipeline);

/* RenderPipeline */
ALIMER_API void agpuRenderPipelineSetLabel(GPURenderPipeline renderPipeline, const char* label);
ALIMER_API uint32_t agpuRenderPipelineAddRef(GPURenderPipeline renderPipeline);
ALIMER_API uint32_t agpuRenderPipelineRelease(GPURenderPipeline renderPipeline);

/* Other */
ALIMER_API uint32_t agpuVertexFormatGetByteSize(GPUVertexFormat format);
ALIMER_API GPUAdapterVendor agpuGPUAdapterVendorFromID(uint32_t vendorId);
ALIMER_API uint32_t agpuGPUAdapterVendorToID(GPUAdapterVendor vendor);

#endif /* ALIMER_GPU_H_ */
