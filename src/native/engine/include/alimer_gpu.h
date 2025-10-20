// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_H_
#define ALIMER_GPU_H_

#include "alimer.h"

/* Forward declarations */
typedef struct GPUFactory                   GPUFactory;
typedef struct GPUAdapter                   GPUAdapter;
typedef struct GPUSurface                   GPUSurface;
typedef struct GPUDevice                    GPUDevice;
typedef struct GPUQueueImpl*                GPUQueue;
typedef struct GPUCommandBufferImpl*        GPUCommandBuffer;
typedef struct GPUComputePassEncoderImpl*   GPUComputePassEncoder;
typedef struct GPURenderPassEncoderImpl*    GPURenderPassEncoder;
typedef struct GPUBufferImpl*               GPUBuffer;
typedef struct GPUTextureImpl*              GPUTexture;
typedef struct GPUSampler                   GPUSampler;
typedef struct GPUQueryHeap                 GPUQueryHeap;
typedef struct GPUBindGroupLayoutImpl*      GPUBindGroupLayout;
typedef struct GPUBindGroupImpl*            GPUBindGroup;
typedef struct GPUPipelineLayoutImpl*       GPUPipelineLayout;
typedef struct GPUComputePipeline           GPUComputePipeline;
typedef struct GPURenderPipelineImpl*       GPURenderPipeline;

/* Types */
typedef uint64_t GPUDeviceAddress;

/* Constants */
#define GPU_MAX_INFLIGHT_FRAMES (3u)
#define GPU_MAX_COLOR_ATTACHMENTS (8u)
#define GPU_MAX_VERTEX_BUFFER_BINDINGS (8u)
#define GPU_WHOLE_SIZE (UINT64_MAX)
#define GPU_LOD_CLAMP_NONE (1000.0F)

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

typedef enum GPUIndexType {
    GPUIndexType_Uint16 = 0,
    GPUIndexType_Uint32 = 1,

    _GPUIndexType_Count,
    _GPUIndexType_Force32 = 0x7FFFFFFF
} GPUIndexType ALIMER_ENUM_ATTRIBUTE;

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
} GPUCompareFunction ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUBlendFactor {
    GPUBlendFactor_Undefined = 0,
    GPUBlendFactor_Zero,
    GPUBlendFactor_One,
    GPUBlendFactor_SourceColor,
    GPUBlendFactor_OneMinusSourceColor,
    GPUBlendFactor_SourceAlpha,
    GPUBlendFactor_OneMinusSourceAlpha,
    GPUBlendFactor_DestinationColor,
    GPUBlendFactor_OneMinusDestinationColor,
    GPUBlendFactor_DestinationAlpha,
    GPUBlendFactor_OneMinusDestinationAlpha,
    GPUBlendFactor_SourceAlphaSaturated,
    GPUBlendFactor_BlendColor,
    GPUBlendFactor_OneMinusBlendColor,
    GPUBlendFactor_BlendAlpha,
    GPUBlendFactor_OneMinusBlendAlpha,
    GPUBlendFactor_Source1Color,
    GPUBlendFactor_OneMinusSource1Color,
    GPUBlendFactor_Source1Alpha,
    GPUBlendFactor_OneMinusSource1Alpha,

    _GPUBlendFactor_Count,
    _GPUBlendFactor_Force32 = 0x7FFFFFFF
} GPUBlendFactor ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUBlendOperation {
    GPUBlendOperation_Undefined = 0,
    GPUBlendOperation_Add,
    GPUBlendOperation_Subtract,
    GPUBlendOperation_ReverseSubtract,
    GPUBlendOperation_Min,
    GPUBlendOperation_Max,

    _GPUBlendOperation_Count,
    _GPUBlendOperation_Force32 = 0x7FFFFFFF
} GPUBlendOperation ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUStencilOperation {
    GPUStencilOperation_Undefined = 0,
    GPUStencilOperation_Keep,
    GPUStencilOperation_Zero,
    GPUStencilOperation_Replace,
    GPUStencilOperation_IncrementClamp,
    GPUStencilOperation_DecrementClamp,
    GPUStencilOperation_Invert,
    GPUStencilOperation_IncrementWrap,
    GPUStencilOperation_DecrementWrap,

    _GPUStencilOperation_Count,
    _GPUStencilOperation_Force32 = 0x7FFFFFFF
} GPUStencilOperation ALIMER_ENUM_ATTRIBUTE;

typedef enum GPULoadAction {
    GPULoadAction_Undefined = 0,
    GPULoadAction_Discard,
    GPULoadAction_Load,
    GPULoadAction_Clear,

    _GPULoadAction_Count,
    _GPULoadAction_Force32 = 0x7FFFFFFF
} GPULoadAction ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUStoreAction {
    GPUStoreAction_Undefined = 0,
    GPUStoreAction_Discard,
    GPUStoreAction_Store,

    _GPUStoreAction_Count,
    _GPUStoreAction_Force32 = 0x7FFFFFFF
} GPUStoreAction ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUPresentMode {
    GPUPresentMode_Undefined = 0,
    GPUPresentMode_Fifo,
    GPUPresentMode_FifoRelaxed,
    GPUPresentMode_Immediate,
    GPUPresentMode_Mailbox,

    _GPUPresentMode_Count,
    _GPUPresentMode_Force32 = 0x7FFFFFFF
} GPUPresentMode ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUShaderStage {
    GPUShaderStage_Undefined,
    GPUShaderStage_Vertex,
    GPUShaderStage_Hull,
    GPUShaderStage_Domain,
    GPUShaderStage_Fragment,
    GPUShaderStage_Compute,
    GPUShaderStage_Amplification,
    GPUShaderStage_Mesh,

    _GPUShaderStage_Count,
    _GPUShaderStage_Force32 = 0x7FFFFFFF
} GPUShaderStage ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUVertexStepMode {
    GPUVertexStepMode_Vertex = 0,
    GPUVertexStepMode_Instance = 1,

    _GPUVertexStepMode_Force32 = 0x7FFFFFFF
} GPUVertexStepMode ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUFillMode {
    _GPUFillMode_Default = 0,
    GPUFillMode_Solid,
    GPUFillMode_Wireframe,

    _GPUFillMode_Force32 = 0x7FFFFFFF
} GPUFillMode ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUCullMode {
    _GPUCullMode_Default = 0,
    GPUCullMode_None,
    GPUCullMode_Front,
    GPUCullMode_Back,

    _GPUCullMode_Force32 = 0x7FFFFFFF
} GPUCullMode ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUFrontFace {
    _GPUFrontFace_Default = 0,
    GPUFrontFace_Clockwise,
    GPUFrontFace_CounterClockwise,

    _GPUFrontFace_Force32 = 0x7FFFFFFF
} GPUFrontFace ALIMER_ENUM_ATTRIBUTE;

typedef enum GPUDepthClipMode {
    _GPUDepthClipMode_Default = 0,
    GPUDepthClipMode_Clip,
    GPUDepthClipMode_Clamp,

    _GPUDepthClipMode_Force32 = 0x7FFFFFFF
} GPUDepthClipMode;

typedef enum GPUSamplerMinMagFilter {
    GPUSamplerMinMagFilter_Nearest = 0,
    GPUSamplerMinMagFilter_Linear = 1,

    _GPUSamplerMinMagFilter_Force32 = 0x7FFFFFFF
} GPUSamplerMinMagFilter;

typedef enum GPUSamplerMipFilter {
    GPUSamplerMipFilter_Nearest = 0,
    GPUSamplerMipFilter_Linear = 1,

    _GPUSamplerMipFilter_Force32 = 0x7FFFFFFF
} GPUSamplerMipFilter;

typedef enum GPUSamplerAddressMode {
    GPUSamplerAddressMode_ClampToEdge = 0,
    GPUSamplerAddressMode_MirrorClampToEdge = 1,
    GPUSamplerAddressMode_Repeat = 2,
    GPUSamplerAddressMode_MirrorRepeat = 3,

    _GPUSamplerAddressMode_Force32 = 0x7FFFFFFF
} GPUSamplerAddressMode;

typedef enum GPUPrimitiveTopology {
    GPUPrimitiveTopology_Undefined = 0,
    GPUPrimitiveTopology_TriangleList,
    GPUPrimitiveTopology_PointList,
    GPUPrimitiveTopology_LineList,
    GPUPrimitiveTopology_LineStrip,
    GPUPrimitiveTopology_TriangleStrip,
    GPUPrimitiveTopology_PatchList,

    _GPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} GPUPrimitiveTopology;

typedef enum GPUShadingRate {
    GPUShadingRate_1X1,	// Default/full shading rate
    GPUShadingRate_1X2,
    GPUShadingRate_2X1,
    GPUShadingRate_2X2,
    GPUShadingRate_2X4,
    GPUShadingRate_4X2,
    GPUShadingRate_4X4,

    _GPUShadingRate_Count,
    _GPUShadingRate_Force32 = 0x7FFFFFFF
} GPUShadingRate;

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

typedef enum GPUShaderModel {
    GPUShaderModel_6_0,
    GPUShaderModel_6_1,
    GPUShaderModel_6_2,
    GPUShaderModel_6_3,
    GPUShaderModel_6_4,
    GPUShaderModel_6_5,
    GPUShaderModel_6_6,
    GPUShaderModel_6_7,
    GPUShaderModel_6_8,
    GPUShaderModel_6_9,
    GPUShaderModel_Highest = GPUShaderModel_6_9,

    _GPUShaderModel_Force32 = 0x7FFFFFFF
} GPUShaderModel;

typedef enum GPUConservativeRasterizationTier {
    GPUConservativeRasterizationTier_NotSupported = 0,
    GPUConservativeRasterizationTier_1 = 1,
    GPUConservativeRasterizationTier_2 = 2,
    GPUConservativeRasterizationTier_3 = 3,

    _GPUConservativeRasterizationTier_Force32 = 0x7FFFFFFF
} GPUConservativeRasterizationTier;

typedef enum GPUVariableRateShadingTier {
    GPUVariableRateShadingTier_NotSupported = 0,
    GPUVariableRateShadingTier_1 = 1,
    GPUVariableRateShadingTier_2 = 2,

    _GPUVariableRateShadingTier_Force32 = 0x7FFFFFFF
} GPUVariableRateShadingTier;

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
    GPUFeature_MultiDrawIndirect,

    GPUFeature_SamplerMirrorClampToEdge,
    GPUFeature_SamplerClampToBorder,
    GPUFeature_SamplerMinMax,

    GPUFeature_Tessellation,
    GPUFeature_DepthBoundsTest,
    GPUFeature_GPUUploadHeapSupported,
    GPUFeature_CopyQueueTimestampQueriesSupported,
    GPUFeature_CacheCoherentUMA,
    GPUFeature_ShaderOutputViewportIndex,
    GPUFeature_ConservativeRasterization,
    GPUFeature_VariableRateShading,
    GPUFeature_RayTracing,
    GPUFeature_MeshShader,
    GPUFeature_Predication,

    _GPUFeature_Force32 = 0x7FFFFFFF
} GPUFeature;

/* Flags/Bitmask Enums */
typedef uint32_t GPUBufferUsage;
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

typedef uint32_t GPUTextureUsage;
static const GPUTextureUsage GPUTextureUsage_None = 0;
static const GPUTextureUsage GPUTextureUsage_ShaderRead = (1 << 0);
static const GPUTextureUsage GPUTextureUsage_ShaderWrite = (1 << 1);
static const GPUTextureUsage GPUTextureUsage_RenderTarget = (1 << 2);
static const GPUTextureUsage GPUTextureUsage_Transient = (1 << 3);
static const GPUTextureUsage GPUTextureUsage_ShadingRate = (1 << 4);
/// Supports shared handle usage.
static const GPUTextureUsage GPUTextureUsage_Shared = (1 << 5);

typedef uint32_t GPUColorWriteMask;
static const GPUColorWriteMask GPUColorWriteMask_None = 0x0000000000000000;
static const GPUColorWriteMask GPUColorWriteMask_Red = 0x0000000000000001;
static const GPUColorWriteMask GPUColorWriteMask_Green = 0x0000000000000002;
static const GPUColorWriteMask GPUColorWriteMask_Blue = 0x0000000000000004;
static const GPUColorWriteMask GPUColorWriteMask_Alpha = 0x0000000000000008;
static const GPUColorWriteMask GPUColorWriteMask_All = 0x000000000000000F /* Red | Green | Blue | Alpha */;

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
    GPUSamplerMinMagFilter  minFilter DEFAULT_INITIALIZER(GPUSamplerMinMagFilter_Nearest);
    GPUSamplerMinMagFilter  magFilter DEFAULT_INITIALIZER(GPUSamplerMinMagFilter_Nearest);
    GPUSamplerMipFilter     mipFilter DEFAULT_INITIALIZER(GPUSamplerMipFilter_Nearest);
    GPUSamplerAddressMode   addressModeU DEFAULT_INITIALIZER(GPUSamplerAddressMode_ClampToEdge);
    GPUSamplerAddressMode   addressModeV DEFAULT_INITIALIZER(GPUSamplerAddressMode_ClampToEdge);
    GPUSamplerAddressMode   addressModeW DEFAULT_INITIALIZER(GPUSamplerAddressMode_ClampToEdge);
    uint16_t                maxAnisotropy DEFAULT_INITIALIZER(1);
    GPUCompareFunction      compareFunction DEFAULT_INITIALIZER(GPUCompareFunction_Never);
    float                   lodMinClamp DEFAULT_INITIALIZER(0.0f);
    float                   lodMaxClamp DEFAULT_INITIALIZER(GPU_LOD_CLAMP_NONE);
} GPUSamplerDesc;

typedef struct GPUBindGroupLayoutDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
} GPUBindGroupLayoutDesc;

typedef struct GPUPushConstantRange {
    uint32_t binding;
    uint32_t size;
} GPUPushConstantRange;

typedef struct GPUPipelineLayoutDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    uint32_t pushConstantRangeCount DEFAULT_INITIALIZER(0);
    const GPUPushConstantRange* pushConstantRanges DEFAULT_INITIALIZER(nullptr);
} GPUPipelineLayoutDesc;

typedef struct GPUShaderDesc {
    GPUShaderStage stage;
    size_t bytecodeSize;
    const void* bytecode;
    const char* entryPoint DEFAULT_INITIALIZER("main");
} GPUShaderDesc;

typedef struct GPUComputePipelineDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout layout;
    GPUShaderDesc shader;
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
} GPUVertexBufferLayout ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPUVertexLayout {
    uint32_t bufferCount DEFAULT_INITIALIZER(0);
    const GPUVertexBufferLayout* buffers DEFAULT_INITIALIZER(nullptr);
} GPUVertexLayout ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPURasterizerState {
    GPUFillMode fillMode DEFAULT_INITIALIZER(GPUFillMode_Solid);
    GPUCullMode cullMode DEFAULT_INITIALIZER(GPUCullMode_Back);
    GPUFrontFace frontFace DEFAULT_INITIALIZER(GPUFrontFace_Clockwise);
    float depthBias DEFAULT_INITIALIZER(0.0f);
    float depthBiasSlopeScale DEFAULT_INITIALIZER(0.0f);
    float depthBiasClamp DEFAULT_INITIALIZER(0.0f);
    GPUDepthClipMode depthClipMode DEFAULT_INITIALIZER(GPUDepthClipMode_Clip);
    Bool32 conservativeRaster DEFAULT_INITIALIZER(false);
} GPURasterizerState ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPUStencilFaceState {
    GPUCompareFunction compareFunction DEFAULT_INITIALIZER(GPUCompareFunction_Always);
    GPUStencilOperation failOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
    GPUStencilOperation depthFailOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
    GPUStencilOperation passOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
} GPUStencilFaceState ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPUDepthStencilState {
    Bool32 depthWriteEnabled DEFAULT_INITIALIZER(false);
    GPUCompareFunction depthCompareFunction DEFAULT_INITIALIZER(GPUCompareFunction_Always);
    uint8_t stencilReadMask DEFAULT_INITIALIZER(0xFF);
    uint8_t stencilWriteMask DEFAULT_INITIALIZER(0xFF);
    GPUStencilFaceState frontFace;
    GPUStencilFaceState backFace;
    Bool32 depthBoundsTestEnable DEFAULT_INITIALIZER(false);  /* Only if GPUFeature_DepthBoundsTest is supported */
} GPUDepthStencilState ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPUMultisampleState {
    uint32_t count;
    uint32_t mask;
    Bool32 alphaToCoverageEnabled;
} GPUMultisampleState ALIMER_STRUCT_ATTRIBUTE;

typedef struct GPURenderPipelineColorAttachmentDesc {
    PixelFormat         format DEFAULT_INITIALIZER(PixelFormat_Undefined);
    GPUBlendFactor      srcColorBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_One);
    GPUBlendFactor      destColorBlendFactor  DEFAULT_INITIALIZER(GPUBlendFactor_Zero);
    GPUBlendOperation   colorBlendOperation DEFAULT_INITIALIZER(GPUBlendOperation_Add);
    GPUBlendFactor      srcAlphaBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_One);
    GPUBlendFactor      destAlphaBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_Zero);
    GPUBlendOperation   alphaBlendOperation DEFAULT_INITIALIZER(GPUBlendOperation_Add);
    GPUColorWriteMask   colorWriteMask DEFAULT_INITIALIZER(GPUColorWriteMask_All);
} GPURenderPipelineColorAttachmentDesc;

typedef struct GPURenderPipelineDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout                       layout;

    uint32_t                                shaderCount;
    const GPUShaderDesc* shaders;

    GPURasterizerState                      rasterizerState;
    GPUDepthStencilState                    depthStencilState;
    const GPUVertexLayout* vertexLayout DEFAULT_INITIALIZER(nullptr);
    GPUPrimitiveTopology                    primitiveTopology DEFAULT_INITIALIZER(GPUPrimitiveTopology_TriangleList);
    uint32_t                                patchControlPoints DEFAULT_INITIALIZER(1);
    GPUMultisampleState                     multisample;
    uint32_t                                colorAttachmentCount DEFAULT_INITIALIZER(0);
    GPURenderPipelineColorAttachmentDesc    colorAttachments[GPU_MAX_COLOR_ATTACHMENTS];
    PixelFormat                             depthStencilAttachmentFormat DEFAULT_INITIALIZER(PixelFormat_Undefined);
} GPURenderPipelineDesc;

typedef struct GPUComputePassDesc {
    const char* label;
} GPUComputePassDesc;

typedef struct GPURenderPassColorAttachment {
    GPUTexture      texture DEFAULT_INITIALIZER(nullptr);
    uint32_t        mipLevel DEFAULT_INITIALIZER(0);
    GPULoadAction   loadAction DEFAULT_INITIALIZER(GPULoadAction_Discard);
    GPUStoreAction  storeAction DEFAULT_INITIALIZER(GPUStoreAction_Store);
    GPUColor        clearColor;
} GPURenderPassColorAttachment;

typedef struct GPURenderPassDepthStencilAttachment {
    GPUTexture      texture DEFAULT_INITIALIZER(nullptr);
    uint32_t        mipLevel DEFAULT_INITIALIZER(0);
    GPULoadAction   depthLoadAction DEFAULT_INITIALIZER(GPULoadAction_Clear);
    GPUStoreAction  depthStoreAction DEFAULT_INITIALIZER(GPUStoreAction_Discard);
    float           depthClearValue DEFAULT_INITIALIZER(1.0f);
    bool            depthReadOnly DEFAULT_INITIALIZER(false);
    GPULoadAction   stencilLoadAction DEFAULT_INITIALIZER(GPULoadAction_Clear);
    GPUStoreAction  stencilStoreAction DEFAULT_INITIALIZER(GPUStoreAction_Discard);
    uint32_t        stencilClearValue DEFAULT_INITIALIZER(0);
    bool            stencilReadOnly DEFAULT_INITIALIZER(false);
} GPURenderPassDepthStencilAttachment;

typedef struct GPURenderPassDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    uint32_t                                    colorAttachmentCount DEFAULT_INITIALIZER(0);
    const GPURenderPassColorAttachment* colorAttachments DEFAULT_INITIALIZER(nullptr);
    const GPURenderPassDepthStencilAttachment* depthStencilAttachment DEFAULT_INITIALIZER(nullptr);
    GPUTexture                                  shadingRateTexture DEFAULT_INITIALIZER(nullptr);
} GPURenderPassDesc;

typedef struct GPURequestAdapterOptions {
    GPUSurface* compatibleSurface DEFAULT_INITIALIZER(nullptr);
    GPUPowerPreference powerPreference DEFAULT_INITIALIZER(GPUPowerPreference_Undefined);
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
    uint32_t maxBindGroups;
    uint32_t maxConstantBufferBindingSize;
    uint32_t maxStorageBufferBindingSize;
    uint32_t minConstantBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment;
    uint32_t maxPushConstantsSize;
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

    /* Highest supported shader model */
    GPUShaderModel shaderModel;

    /* Only if GPUFeature_ConservativeRasterization is supported */
    GPUConservativeRasterizationTier conservativeRasterizationTier;

    /* Only if GPUFeature_VariableRateShading is supported */
    GPUVariableRateShadingTier variableShadingRateTier;
    uint32_t variableShadingRateImageTileSize;
    Bool32 isAdditionalVariableShadingRatesSupported;
} GPULimits;

typedef struct GPUSurfaceCapabilities {
    PixelFormat preferredFormat;
    GPUTextureUsage supportedUsage;
    uint32_t formatCount;
    const PixelFormat* formats;
    uint32_t presentModeCount;
    const GPUPresentMode* presentModes;
} GPUSurfaceCapabilities;

typedef struct GPUSurfaceConfig {
    GPUDevice* device;
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
typedef struct GPUDispatchIndirectCommand {
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
} GPUDispatchIndirectCommand;

typedef struct GPUDrawIndexedIndirectCommand {
    uint32_t    indexCount;
    uint32_t    instanceCount;
    uint32_t    firstIndex;
    int32_t     baseVertex;
    uint32_t    firstInstance;
} GPUDrawIndexedIndirectCommand;

typedef struct GPUDrawIndirectCommand {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
} GPUDrawIndirectCommand;

ALIMER_API bool agpuIsBackendSupport(GPUBackendType backend);
ALIMER_API GPUFactory* agpuCreateFactory(const GPUConfig* config);
ALIMER_API void agpuFactoryDestroy(GPUFactory* factory);
ALIMER_API GPUBackendType agpuFactoryGetBackend(GPUFactory* factory);
ALIMER_API GPUAdapter* agpuFactoryRequestAdapter(GPUFactory* factory, ALIMER_NULLABLE const GPURequestAdapterOptions* options);

/* Adapter */
ALIMER_API GPUResult agpuAdapterGetInfo(GPUAdapter* adapter, GPUAdapterInfo* info);
ALIMER_API GPUResult agpuAdapterGetLimits(GPUAdapter* adapter, GPULimits* limits);
ALIMER_API bool agpuAdapterHasFeature(GPUAdapter* adapter, GPUFeature feature);

/* Surface */
ALIMER_API GPUSurface* agpuCreateSurface(GPUFactory* factory, Window* window);
ALIMER_API GPUResult agpuSurfaceGetCapabilities(GPUSurface* surface, GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities);
ALIMER_API bool agpuSurfaceConfigure(GPUSurface* surface, const GPUSurfaceConfig* config);
ALIMER_API void agpuSurfaceUnconfigure(GPUSurface* surface);
ALIMER_API uint32_t agpuSurfaceAddRef(GPUSurface* surface);
ALIMER_API uint32_t agpuSurfaceRelease(GPUSurface* surface);

/* Device */
ALIMER_API GPUDevice* agpuCreateDevice(GPUAdapter* adapter, const GPUDeviceDesc* desc);
ALIMER_API void agpuDeviceSetLabel(GPUDevice* device, const char* label);
ALIMER_API uint32_t agpuDeviceAddRef(GPUDevice* device);
ALIMER_API uint32_t agpuDeviceRelease(GPUDevice* device);
ALIMER_API bool agpuDeviceHasFeature(GPUDevice device, GPUFeature feature);
ALIMER_API GPUQueue agpuDeviceGetQueue(GPUDevice device, GPUQueueType type);
ALIMER_API bool agpuDeviceWaitIdle(GPUDevice device);

/// Commit the current frame and advance to next frame
ALIMER_API uint64_t agpuDeviceCommitFrame(GPUDevice device);

/* Queue */
ALIMER_API GPUQueueType agpuQueueGetType(GPUQueue queue);
ALIMER_API GPUCommandBuffer agpuQueueAcquireCommandBuffer(GPUQueue queue, const GPUCommandBufferDesc* desc);
ALIMER_API void agpuQueueSubmit(GPUQueue queue, uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers);

/* CommandBuffer */
ALIMER_API void agpuCommandBufferPushDebugGroup(GPUCommandBuffer commandBuffer, const char* groupLabel);
ALIMER_API void agpuCommandBufferPopDebugGroup(GPUCommandBuffer commandBuffer);
ALIMER_API void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer commandBuffer, const char* markerLabel);
ALIMER_API GPUAcquireSurfaceResult agpuCommandBufferAcquireSurfaceTexture(GPUCommandBuffer commandBuffer, GPUSurface* surface, GPUTexture* surfaceTexture);
ALIMER_API GPUComputePassEncoder agpuCommandBufferBeginComputePass(GPUCommandBuffer commandBuffer, const GPUComputePassDesc* desc);
ALIMER_API GPURenderPassEncoder agpuCommandBufferBeginRenderPass(GPUCommandBuffer commandBuffer, const GPURenderPassDesc* desc);

/* ComputePassEncoder */
ALIMER_API void agpuComputePassEncoderSetPipeline(GPUComputePassEncoder computePassEncoder, GPUComputePipeline* pipeline);
ALIMER_API void agpuComputePassEncoderSetPushConstants(GPUComputePassEncoder computePassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size);
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
ALIMER_API void agpuRenderPassEncoderSetBlendColor(GPURenderPassEncoder renderPassEncoder, const float blendColor[4]);
ALIMER_API void agpuRenderPassEncoderSetStencilReference(GPURenderPassEncoder renderPassEncoder, uint32_t reference);
ALIMER_API void agpuRenderPassEncoderSetVertexBuffer(GPURenderPassEncoder renderPassEncoder, uint32_t slot, GPUBuffer buffer, uint64_t offset);
ALIMER_API void agpuRenderPassEncoderSetIndexBuffer(GPURenderPassEncoder renderPassEncoder, GPUBuffer buffer, GPUIndexType type, uint64_t offset);
ALIMER_API void agpuRenderPassEncoderSetPipeline(GPURenderPassEncoder renderPassEncoder, GPURenderPipeline pipeline);
ALIMER_API void agpuRenderPassEncoderSetPushConstants(GPURenderPassEncoder renderPassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size);
ALIMER_API void agpuRenderPassEncoderDraw(GPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
ALIMER_API void agpuRenderPassEncoderDrawIndexed(GPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
ALIMER_API void agpuRenderPassEncoderDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_API void agpuRenderPassEncoderDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_API void agpuRenderPassEncoderMultiDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, ALIMER_NULLABLE GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset);
ALIMER_API void agpuRenderPassEncoderMultiDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, ALIMER_NULLABLE GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset);
ALIMER_API void agpuRenderPassEncoderSetShadingRate(GPURenderPassEncoder renderPassEncoder, GPUShadingRate rate);
ALIMER_API void agpuRenderPassEncoderEnd(GPURenderPassEncoder renderPassEncoder);
ALIMER_API void agpuRenderPassEncoderPushDebugGroup(GPURenderPassEncoder renderPassEncoder, const char* groupLabel);
ALIMER_API void agpuRenderPassEncoderPopDebugGroup(GPURenderPassEncoder renderPassEncoder);
ALIMER_API void agpuRenderPassEncoderInsertDebugMarker(GPURenderPassEncoder renderPassEncoder, const char* markerLabel);

/* Buffer */
ALIMER_API GPUBuffer agpuCreateBuffer(GPUDevice* device, const GPUBufferDesc* desc, const void* pInitialData);
ALIMER_API void agpuBufferSetLabel(GPUBuffer buffer, const char* label);
ALIMER_API uint32_t agpuBufferAddRef(GPUBuffer buffer);
ALIMER_API uint32_t agpuBufferRelease(GPUBuffer buffer);
ALIMER_API uint64_t agpuBufferGetSize(GPUBuffer buffer);
ALIMER_API GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer buffer);

/* Texture */
ALIMER_API GPUTexture agpuCreateTexture(GPUDevice* device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData);
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
ALIMER_API GPUSampler* agpuCreateSampler(GPUDevice* device, const GPUSamplerDesc* desc);
ALIMER_API void agpuSamplerSetLabel(GPUSampler* sampler, const char* label);
ALIMER_API uint32_t agpuSamplerAddRef(GPUSampler* sampler);
ALIMER_API uint32_t agpuSamplerRelease(GPUSampler* sampler);

/* PipelineLayout */
ALIMER_API GPUPipelineLayout agpuCreatePipelineLayout(GPUDevice* device, const GPUPipelineLayoutDesc* desc);
ALIMER_API void agpuPipelineLayoutSetLabel(GPUPipelineLayout pipelineLayout, const char* label);
ALIMER_API uint32_t agpuPipelineLayoutAddRef(GPUPipelineLayout pipelineLayout);
ALIMER_API uint32_t agpuPipelineLayoutRelease(GPUPipelineLayout pipelineLayout);

/* ComputePipeline */
ALIMER_API GPUComputePipeline* agpuCreateComputePipeline(GPUDevice* device, const GPUComputePipelineDesc* desc);
ALIMER_API void agpuComputePipelineSetLabel(GPUComputePipeline* computePipeline, const char* label);
ALIMER_API uint32_t agpuComputePipelineAddRef(GPUComputePipeline* computePipeline);
ALIMER_API uint32_t agpuComputePipelineRelease(GPUComputePipeline* computePipeline);

/* RenderPipeline */
ALIMER_API GPURenderPipeline agpuCreateRenderPipeline(GPUDevice* device, const GPURenderPipelineDesc* desc);
ALIMER_API void agpuRenderPipelineSetLabel(GPURenderPipeline renderPipeline, const char* label);
ALIMER_API uint32_t agpuRenderPipelineAddRef(GPURenderPipeline renderPipeline);
ALIMER_API uint32_t agpuRenderPipelineRelease(GPURenderPipeline renderPipeline);

/* Other */
ALIMER_API uint32_t agpuGetVertexFormatByteSize(GPUVertexFormat format);
ALIMER_API uint32_t agpuGetVertexFormatComponentCount(GPUVertexFormat format);
ALIMER_API GPUAdapterVendor agpuGPUAdapterVendorFromID(uint32_t vendorId);
ALIMER_API uint32_t agpuGPUAdapterVendorToID(GPUAdapterVendor vendor);

#endif /* ALIMER_GPU_H_ */
