// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_H_
#define ALIMER_GPU_H_ 1

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __declspec(dllexport)
#        else
#            define _ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define _ALIMER_EXPORT
#        endif
#    endif
#else
#    define _ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_GPU_API _ALIMER_EXTERN _ALIMER_EXPORT

#ifdef __cplusplus
#   define DEFAULT_INITIALIZER(x) = x
#else
#   define DEFAULT_INITIALIZER(x)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Version API */
#define ALIMER_GPU_VERSION_MAJOR    1
#define ALIMER_GPU_VERSION_MINOR    0
#define ALIMER_GPU_VERSION_PATCH	0

/* Forward declarations */
typedef struct GPUFactoryImpl*              GPUFactory;
typedef struct GPUAdapterImpl*              GPUAdapter;
typedef struct GPUSurfaceSourceImpl*        GPUSurfaceSource;
typedef struct GPUSurfaceImpl*              GPUSurface;
typedef struct GPUDeviceImpl*               GPUDevice;
typedef struct GPUCommandQueueImpl*         GPUCommandQueue;
typedef struct GPUCommandBufferImpl*        GPUCommandBuffer;
typedef struct GPUComputePassEncoderImpl*   GPUComputePassEncoder;
typedef struct GPURenderPassEncoderImpl*    GPURenderPassEncoder;
typedef struct GPUBufferImpl*               GPUBuffer;
typedef struct GPUTextureImpl*              GPUTexture;
typedef struct GPUSamplerImpl*              GPUSampler;
typedef struct GPUQueryHeapImpl*            GPUQueryHeap;
typedef struct GPUBindGroupLayoutImpl*      GPUBindGroupLayout;
typedef struct GPUBindGroupImpl*            GPUBindGroup;
typedef struct GPUPipelineLayoutImpl*       GPUPipelineLayout;
typedef struct GPUShaderModuleImpl*         GPUShaderModule;
typedef struct GPUComputePipelineImpl*      GPUComputePipeline;
typedef struct GPURenderPipelineImpl*       GPURenderPipeline;

/* Types */
typedef uint32_t GPUBool;
typedef uint64_t GPUDeviceAddress;

/* Constants */
#define GPU_MAX_ADAPTER_NAME_SIZE  (256u)
#define GPU_MAX_COLOR_ATTACHMENTS (8u)
#define GPU_MAX_VERTEX_BUFFER_BINDINGS (8u)
#define GPU_WHOLE_SIZE (UINT64_MAX)
#define GPU_LOD_CLAMP_NONE (1000.0F)

/* Enums */
typedef enum GPULogLevel {
    GPULogLevel_Off = 0,
    GPULogLevel_Error = 1,
    GPULogLevel_Warn = 2,
    GPULogLevel_Info = 3,
    GPULogLevel_Debug = 4,
    VGPULogLevel_Trace = 5,

    _GPULogLevel_Count,
    _GPULogLevel_Force32 = 0x7FFFFFFF
} GPULogLevel;

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

typedef enum GPUCommandQueueType {
    GPUCommandQueueType_Graphics = 0,
    GPUCommandQueueType_Compute,
    GPUCommandQueueType_Copy,
    //GPUCommandQueueType_VideoDecode,
    //GPUCommandQueueType_VideoEncode,

    _GPUCommandQueueType_Count,
    _GPUCommandQueueType_Force32 = 0x7FFFFFFF
} GPUCommandQueueType;

typedef enum GPUPixelFormat {
    GPUPixelFormat_Undefined = 0,
    // 8-bit formats
    GPUPixelFormat_R8Unorm,
    GPUPixelFormat_R8Snorm,
    GPUPixelFormat_R8Uint,
    GPUPixelFormat_R8Sint,
    // 16-bit formats
    GPUPixelFormat_R16Unorm,
    GPUPixelFormat_R16Snorm,
    GPUPixelFormat_R16Uint,
    GPUPixelFormat_R16Sint,
    GPUPixelFormat_R16Float,
    GPUPixelFormat_RG8Unorm,
    GPUPixelFormat_RG8Snorm,
    GPUPixelFormat_RG8Uint,
    GPUPixelFormat_RG8Sint,
    // Packed 16-Bit formats
    GPUPixelFormat_B5G6R5Unorm,
    GPUPixelFormat_BGR5A1Unorm,
    GPUPixelFormat_BGRA4Unorm,
    // 32-bit formats
    GPUPixelFormat_R32Uint,
    GPUPixelFormat_R32Sint,
    GPUPixelFormat_R32Float,
    GPUPixelFormat_RG16Unorm,
    GPUPixelFormat_RG16Snorm,
    GPUPixelFormat_RG16Uint,
    GPUPixelFormat_RG16Sint,
    GPUPixelFormat_RG16Float,
    GPUPixelFormat_RGBA8Unorm,
    GPUPixelFormat_RGBA8UnormSrgb,
    GPUPixelFormat_RGBA8Snorm,
    GPUPixelFormat_RGBA8Uint,
    GPUPixelFormat_RGBA8Sint,
    GPUPixelFormat_BGRA8Unorm,
    GPUPixelFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    GPUPixelFormat_RGB10A2Unorm,
    GPUPixelFormat_RGB10A2Uint,
    GPUPixelFormat_RG11B10Ufloat,
    GPUPixelFormat_RGB9E5Ufloat,
    // 64-bit formats
    GPUPixelFormat_RG32Uint,
    GPUPixelFormat_RG32Sint,
    GPUPixelFormat_RG32Float,
    GPUPixelFormat_RGBA16Unorm,
    GPUPixelFormat_RGBA16Snorm,
    GPUPixelFormat_RGBA16Uint,
    GPUPixelFormat_RGBA16Sint,
    GPUPixelFormat_RGBA16Float,
    // 128-bit formats
    GPUPixelFormat_RGBA32Uint,
    GPUPixelFormat_RGBA32Sint,
    GPUPixelFormat_RGBA32Float,
    // Depth-stencil formats
    GPUPixelFormat_Depth16Unorm,
    GPUPixelFormat_Depth24UnormStencil8,
    GPUPixelFormat_Depth32Float,
    GPUPixelFormat_Depth32FloatStencil8,
    // BC compressed formats
    GPUPixelFormat_BC1RGBAUnorm,
    GPUPixelFormat_BC1RGBAUnormSrgb,
    GPUPixelFormat_BC2RGBAUnorm,
    GPUPixelFormat_BC2RGBAUnormSrgb,
    GPUPixelFormat_BC3RGBAUnorm,
    GPUPixelFormat_BC3RGBAUnormSrgb,
    GPUPixelFormat_BC4RUnorm,
    GPUPixelFormat_BC4RSnorm,
    GPUPixelFormat_BC5RGUnorm,
    GPUPixelFormat_BC5RGSnorm,
    GPUPixelFormat_BC6HRGBUfloat,
    GPUPixelFormat_BC6HRGBFloat,
    GPUPixelFormat_BC7RGBAUnorm,
    GPUPixelFormat_BC7RGBAUnormSrgb,
    // ETC2/EAC compressed formats
    GPUPixelFormat_ETC2RGB8Unorm,
    GPUPixelFormat_ETC2RGB8UnormSrgb,
    GPUPixelFormat_ETC2RGB8A1Unorm,
    GPUPixelFormat_ETC2RGB8A1UnormSrgb,
    GPUPixelFormat_ETC2RGBA8Unorm,
    GPUPixelFormat_ETC2RGBA8UnormSrgb,
    GPUPixelFormat_EACR11Unorm,
    GPUPixelFormat_EACR11Snorm,
    GPUPixelFormat_EACRG11Unorm,
    GPUPixelFormat_EACRG11Snorm,
    // ASTC compressed formats
    GPUPixelFormat_ASTC4x4Unorm,
    GPUPixelFormat_ASTC4x4UnormSrgb,
    GPUPixelFormat_ASTC5x4Unorm,
    GPUPixelFormat_ASTC5x4UnormSrgb,
    GPUPixelFormat_ASTC5x5Unorm,
    GPUPixelFormat_ASTC5x5UnormSrgb,
    GPUPixelFormat_ASTC6x5Unorm,
    GPUPixelFormat_ASTC6x5UnormSrgb,
    GPUPixelFormat_ASTC6x6Unorm,
    GPUPixelFormat_ASTC6x6UnormSrgb,
    GPUPixelFormat_ASTC8x5Unorm,
    GPUPixelFormat_ASTC8x5UnormSrgb,
    GPUPixelFormat_ASTC8x6Unorm,
    GPUPixelFormat_ASTC8x6UnormSrgb,
    GPUPixelFormat_ASTC8x8Unorm,
    GPUPixelFormat_ASTC8x8UnormSrgb,
    GPUPixelFormat_ASTC10x5Unorm,
    GPUPixelFormat_ASTC10x5UnormSrgb,
    GPUPixelFormat_ASTC10x6Unorm,
    GPUPixelFormat_ASTC10x6UnormSrgb,
    GPUPixelFormat_ASTC10x8Unorm,
    GPUPixelFormat_ASTC10x8UnormSrgb,
    GPUPixelFormat_ASTC10x10Unorm,
    GPUPixelFormat_ASTC10x10UnormSrgb,
    GPUPixelFormat_ASTC12x10Unorm,
    GPUPixelFormat_ASTC12x10UnormSrgb,
    GPUPixelFormat_ASTC12x12Unorm,
    GPUPixelFormat_ASTC12x12UnormSrgb,
    // ASTC HDR compressed formats
    GPUPixelFormat_ASTC4x4HDR,
    GPUPixelFormat_ASTC5x4HDR,
    GPUPixelFormat_ASTC5x5HDR,
    GPUPixelFormat_ASTC6x5HDR,
    GPUPixelFormat_ASTC6x6HDR,
    GPUPixelFormat_ASTC8x5HDR,
    GPUPixelFormat_ASTC8x6HDR,
    GPUPixelFormat_ASTC8x8HDR,
    GPUPixelFormat_ASTC10x5HDR,
    GPUPixelFormat_ASTC10x6HDR,
    GPUPixelFormat_ASTC10x8HDR,
    GPUPixelFormat_ASTC10x10HDR,
    GPUPixelFormat_ASTC12x10HDR,
    GPUPixelFormat_ASTC12x12HDR,

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _GPUPixelFormat_Count,
    _GPUPixelFormat_Force32 = 0x7FFFFFFF
} GPUPixelFormat;

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

typedef enum GPUTextureDimension {
    /// Undefined - default to 2D texture.
    GPUTextureDimension_Undefined = 0,
    /// One-dimensional Texture.
    GPUTextureDimension_1D = 1,
    /// Two-dimensional Texture.
    GPUTextureDimension_2D = 2,
    /// Three-dimensional Texture.
    GPUTextureDimension_3D = 3,
    /// Cubemap Texture.
    GPUTextureDimension_Cube = 4,

    _GPUTextureDimension_Force32 = 0x7FFFFFFF
} GPUTextureDimension;

typedef enum GPUIndexType {
    GPUIndexType_Uint16 = 0,
    GPUIndexType_Uint32 = 1,

    _GPUIndexType_Count,
    _GPUIndexType_Force32 = 0x7FFFFFFF
} GPUIndexType;

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
} GPUBlendFactor;

typedef enum GPUBlendOperation {
    GPUBlendOperation_Undefined = 0,
    GPUBlendOperation_Add,
    GPUBlendOperation_Subtract,
    GPUBlendOperation_ReverseSubtract,
    GPUBlendOperation_Min,
    GPUBlendOperation_Max,

    _GPUBlendOperation_Count,
    _GPUBlendOperation_Force32 = 0x7FFFFFFF
} GPUBlendOperation;

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
} GPUStencilOperation;

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
    GPUShaderStage_Mesh,
    GPUShaderStage_Amplification,

    _GPUShaderStage_Count,
    _GPUShaderStage_Force32 = 0x7FFFFFFF
} GPUShaderStage;

typedef enum GPUVertexStepMode {
    GPUVertexStepMode_Vertex = 0,
    GPUVertexStepMode_Instance = 1,

    _GPUVertexStepMode_Force32 = 0x7FFFFFFF
} GPUVertexStepMode;

typedef enum GPUFillMode {
    _GPUFillMode_Default = 0,
    GPUFillMode_Solid,
    GPUFillMode_Wireframe,

    _GPUFillMode_Force32 = 0x7FFFFFFF
} GPUFillMode;

typedef enum GPUCullMode {
    _GPUCullMode_Default = 0,
    GPUCullMode_None,
    GPUCullMode_Front,
    GPUCullMode_Back,

    _GPUCullMode_Force32 = 0x7FFFFFFF
} GPUCullMode;

typedef enum GPUFrontFace {
    _GPUFrontFace_Default = 0,
    GPUFrontFace_CounterClockwise,
    GPUFrontFace_Clockwise,

    _GPUFrontFace_Force32 = 0x7FFFFFFF
} GPUFrontFace;

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
    GPUAdapterType_DiscreteGpu,
    GPUAdapterType_IntegratedGpu,
    GPUAdapterType_VirtualGpu,
    GPUAdapterType_Cpu,
    GPUAdapterType_Other,

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

typedef enum GPUVariableShadingRateTier {
    GPUVariableShadingRateTier_NotSupported = 0,
    GPUVariableShadingRateTier_1 = 1,
    GPUVariableShadingRateTier_2 = 2,

    _GPUVariableShadingRateTier_Force32 = 0x7FFFFFFF
} GPUVariableShadingRateTier;

typedef enum GPURayTracingTier {
    GPURayTracingTier_NotSupported = 0,
    GPURayTracingTier_1 = 1,
    GPURayTracingTier_2 = 2,

    _GPURayTracingTier_Force32 = 0x7FFFFFFF
} GPURayTracingTier;

typedef enum GPUMeshShaderTier {
    GPUMeshShaderTier_NotSupported = 0,
    GPUMeshShaderTier_1 = 1,

    _GPUMeshShaderTier_Force32 = 0x7FFFFFFF
} GPUMeshShaderTier;

typedef enum GPUFeature {
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
    GPUFeature_CopyQueueTimestampQuery,
    GPUFeature_CacheCoherentUMA,
    GPUFeature_ShaderOutputViewportIndex,
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

typedef enum GPUQueryType {
    /// Create a heap that contains timestamp queries
    GPUQueryType_Timestamp,
    /// Create a heap that contains timestamp queries for copy queue
    GPUQueryType_TimestampCopyQueue,
    /// Create a heap that contains occlusion queries
    GPUQueryType_Occlusion,
    /// Create a heap that contains binary occlusion queries
    GPUQueryType_BinaryOcclusion,
    /// Create a heap to contain a structure of `PipelineStatistics`
    GPUQueryType_PipelineStatistics,

    _GPUQueryType_Force32 = 0x7FFFFFFF
} GPUQueryType;

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
    GPUTextureDimension dimension DEFAULT_INITIALIZER(GPUTextureDimension_2D);
    GPUPixelFormat format DEFAULT_INITIALIZER(GPUPixelFormat_RGBA8Unorm);
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

typedef struct GPUShaderModuleDesc
{
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUShaderStage stage DEFAULT_INITIALIZER(GPUShaderStage_Undefined);
    size_t byteCodeSize DEFAULT_INITIALIZER(0);
    const void* byteCode DEFAULT_INITIALIZER(nullptr);
    const char* entryPoint DEFAULT_INITIALIZER("main");
} GPUShaderModuleDesc;

typedef struct GPUComputePipelineDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout layout;
    GPUShaderModule shader;
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

typedef struct GPUVertexLayout {
    uint32_t bufferCount DEFAULT_INITIALIZER(0);
    const GPUVertexBufferLayout* buffers DEFAULT_INITIALIZER(nullptr);
} GPUVertexLayout;

typedef struct GPURasterizerState {
    GPUFillMode fillMode DEFAULT_INITIALIZER(GPUFillMode_Solid);
    GPUCullMode cullMode DEFAULT_INITIALIZER(GPUCullMode_Back);
    GPUFrontFace frontFace DEFAULT_INITIALIZER(GPUFrontFace_CounterClockwise);
    float depthBias DEFAULT_INITIALIZER(0.0f);
    float depthBiasSlopeScale DEFAULT_INITIALIZER(0.0f);
    float depthBiasClamp DEFAULT_INITIALIZER(0.0f);
    GPUDepthClipMode depthClipMode DEFAULT_INITIALIZER(GPUDepthClipMode_Clip);
    GPUBool conservativeRasterEnable DEFAULT_INITIALIZER(false);
} GPURasterizerState;

typedef struct GPUStencilFaceState {
    GPUCompareFunction compareFunction DEFAULT_INITIALIZER(GPUCompareFunction_Always);
    GPUStencilOperation failOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
    GPUStencilOperation depthFailOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
    GPUStencilOperation passOperation DEFAULT_INITIALIZER(GPUStencilOperation_Keep);
} GPUStencilFaceState;

typedef struct GPUDepthStencilState {
    GPUBool depthWriteEnabled DEFAULT_INITIALIZER(false);
    GPUCompareFunction depthCompareFunction DEFAULT_INITIALIZER(GPUCompareFunction_Always);
    uint8_t stencilReadMask DEFAULT_INITIALIZER(0xFF);
    uint8_t stencilWriteMask DEFAULT_INITIALIZER(0xFF);
    GPUStencilFaceState frontFace;
    GPUStencilFaceState backFace;
    GPUBool depthBoundsTestEnable DEFAULT_INITIALIZER(false);  /* Only if GPUFeature_DepthBoundsTest is supported */
} GPUDepthStencilState;

typedef struct GPUMultisampleState {
    uint32_t count;
    uint32_t mask;
    GPUBool alphaToCoverageEnabled;
} GPUMultisampleState;

typedef struct GPURenderPipelineColorAttachmentDesc {
    GPUPixelFormat      format DEFAULT_INITIALIZER(GPUPixelFormat_Undefined);
    GPUBlendFactor      srcColorBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_One);
    GPUBlendFactor      destColorBlendFactor  DEFAULT_INITIALIZER(GPUBlendFactor_Zero);
    GPUBlendOperation   colorBlendOperation DEFAULT_INITIALIZER(GPUBlendOperation_Add);
    GPUBlendFactor      srcAlphaBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_One);
    GPUBlendFactor      destAlphaBlendFactor DEFAULT_INITIALIZER(GPUBlendFactor_Zero);
    GPUBlendOperation   alphaBlendOperation DEFAULT_INITIALIZER(GPUBlendOperation_Add);
    GPUColorWriteMask   colorWriteMask DEFAULT_INITIALIZER(GPUColorWriteMask_All);
} GPURenderPipelineColorAttachmentDesc;

typedef struct GPURenderPipelineDesc {
    const char*                             label DEFAULT_INITIALIZER(nullptr);
    GPUPipelineLayout                       layout;

    GPUShaderModule                         vertexShader DEFAULT_INITIALIZER(nullptr);
    GPUShaderModule                         fragmentShader DEFAULT_INITIALIZER(nullptr);
    GPUShaderModule                         meshShader DEFAULT_INITIALIZER(nullptr);
    GPUShaderModule                         amplificationShader DEFAULT_INITIALIZER(nullptr);

    GPURasterizerState                      rasterizerState;
    GPUDepthStencilState                    depthStencilState;
    const GPUVertexLayout*                  vertexLayout DEFAULT_INITIALIZER(nullptr);
    GPUPrimitiveTopology                    primitiveTopology DEFAULT_INITIALIZER(GPUPrimitiveTopology_TriangleList);
    GPUMultisampleState                     multisample;
    uint32_t                                colorAttachmentCount DEFAULT_INITIALIZER(0);
    GPURenderPipelineColorAttachmentDesc    colorAttachments[GPU_MAX_COLOR_ATTACHMENTS];
    GPUPixelFormat                          depthStencilAttachmentFormat DEFAULT_INITIALIZER(GPUPixelFormat_Undefined);
} GPURenderPipelineDesc;

typedef struct GPUQueryHeapDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    /// ie: Timestamp, Occlusion, PipelineStatistics
    GPUQueryType queryType DEFAULT_INITIALIZER(GPUQueryType_Timestamp);
    /// Total size of the heap in number of queries.
    uint32_t count DEFAULT_INITIALIZER(1);
} GPUQueryHeapDesc;

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
    uint32_t colorAttachmentCount DEFAULT_INITIALIZER(0);
    const GPURenderPassColorAttachment* colorAttachments DEFAULT_INITIALIZER(nullptr);
    const GPURenderPassDepthStencilAttachment* depthStencilAttachment DEFAULT_INITIALIZER(nullptr);
    GPUTexture shadingRateTexture DEFAULT_INITIALIZER(nullptr);
} GPURenderPassDesc;

typedef struct GPUDeviceDesc {
    const char* label DEFAULT_INITIALIZER(nullptr);
    uint32_t maxFramesInFlight DEFAULT_INITIALIZER(2);
} GPUDeviceDesc;

typedef struct GPUAdapterInfo {
    char deviceName[GPU_MAX_ADAPTER_NAME_SIZE];
    uint16_t driverVersion[4];
    const char* driverDescription;
    GPUAdapterType adapterType;
    GPUAdapterVendor vendor;
    uint32_t vendorID;
    uint32_t deviceID;
} GPUAdapterInfo;

typedef struct GPUDeviceLimits {
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

    /* ConservativeRasterization tier */
    GPUConservativeRasterizationTier conservativeRasterizationTier;

    /* VariableShadingRate tier */
    GPUVariableShadingRateTier variableShadingRateTier;
    uint32_t variableShadingRateImageTileSize;
    GPUBool isAdditionalVariableShadingRatesSupported;

    /* Ray tracing */
    GPURayTracingTier rayTracingTier;
    uint32_t rayTracingShaderGroupIdentifierSize;
    uint32_t rayTracingShaderTableAlignment;
    uint32_t rayTracingShaderTableMaxStride;
    uint32_t rayTracingShaderRecursionMaxDepth;
    uint32_t rayTracingMaxGeometryCount;
    uint32_t rayTracingScratchAlignment;

    /* Mesh shader */
    GPUMeshShaderTier meshShaderTier;
} GPUDeviceLimits;

typedef struct GPUSurfaceCapabilities {
    GPUPixelFormat preferredFormat;
    GPUTextureUsage supportedUsage;
    uint32_t formatCount;
    const GPUPixelFormat* formats;
    uint32_t presentModeCount;
    const GPUPresentMode* presentModes;
} GPUSurfaceCapabilities;

typedef struct GPUSurfaceConfig {
    GPUDevice device;
    GPUPixelFormat format;
    uint32_t width;
    uint32_t height;
    GPUPresentMode presentMode;
} GPUSurfaceConfig;

typedef struct GPUFactoryDesc {
    GPUBackendType preferredBackend;
    GPUValidationMode validationMode;
} GPUFactoryDesc;

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

typedef void (*GPULogCallback)(GPULogLevel level, const char* message, void* userData);
ALIMER_GPU_API GPULogLevel agpuGetLogLevel(void);
ALIMER_GPU_API void agpuSetLogLevel(GPULogLevel level);
ALIMER_GPU_API void agpuSetLogCallback(GPULogCallback func, void* userData);

ALIMER_GPU_API bool agpuIsBackendSupport(GPUBackendType backend);
ALIMER_GPU_API GPUFactory agpuCreateFactory(const GPUFactoryDesc* desc);
ALIMER_GPU_API void agpuFactoryDestroy(GPUFactory factory);
ALIMER_GPU_API GPUBackendType agpuFactoryGetBackend(GPUFactory factory);
ALIMER_GPU_API uint32_t agpuFactoryGetAdapterCount(GPUFactory factory);
ALIMER_GPU_API GPUAdapter agpuFactoryGetAdapter(GPUFactory factory, uint32_t index);
ALIMER_GPU_API GPUAdapter agpuFactoryGetBestAdapter(GPUFactory factory);
ALIMER_GPU_API GPUSurface agpuFactoryCreateSurface(GPUFactory factory, GPUSurfaceSource source);

/* Adapter */
ALIMER_GPU_API void agpuAdapterGetInfo(GPUAdapter adapter, GPUAdapterInfo* info);
ALIMER_GPU_API GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter, const GPUDeviceDesc* desc);

/* SurfaceSource */
ALIMER_GPU_API GPUSurfaceSource agpuSurfaceSourceCreateFromWin32(void* hwnd);
ALIMER_GPU_API GPUSurfaceSource agpuSurfaceSourceCreateFromAndroid(void* window);
ALIMER_GPU_API GPUSurfaceSource agpuSurfaceSourceCreateFromMetalLayer(void* metalLayer);
ALIMER_GPU_API GPUSurfaceSource agpuSurfaceSourceCreateFromWaylandSurface(void* display, void* surface);
ALIMER_GPU_API GPUSurfaceSource agpuSurfaceSourceCreateFromXlibWindow(void* display, uint64_t window);
ALIMER_GPU_API void agpuSurfaceSourceDestroy(GPUSurfaceSource surfaceSource);

/* Surface */
ALIMER_GPU_API void agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter adapter, GPUSurfaceCapabilities* capabilities);
ALIMER_GPU_API bool agpuSurfaceConfigure(GPUSurface surface, const GPUSurfaceConfig* config);
ALIMER_GPU_API void agpuSurfaceUnconfigure(GPUSurface surface);
ALIMER_GPU_API uint32_t agpuSurfaceAddRef(GPUSurface surface);
ALIMER_GPU_API uint32_t agpuSurfaceRelease(GPUSurface surface);

/* Device */
ALIMER_GPU_API void agpuDeviceSetLabel(GPUDevice device, const char* label);
ALIMER_GPU_API uint32_t agpuDeviceAddRef(GPUDevice device);
ALIMER_GPU_API uint32_t agpuDeviceRelease(GPUDevice device);
ALIMER_GPU_API void agpuDeviceGetLimits(GPUDevice device, GPUDeviceLimits* limits);
ALIMER_GPU_API bool agpuDeviceHasFeature(GPUDevice device, GPUFeature feature);
ALIMER_GPU_API GPUCommandQueue agpuDeviceGetCommandQueue(GPUDevice device, GPUCommandQueueType type);
ALIMER_GPU_API void agpuDeviceWaitIdle(GPUDevice device);
ALIMER_GPU_API uint64_t agpuDeviceGetTimestampFrequency(GPUDevice device);

/// Commit the current frame and advance to next frame
ALIMER_GPU_API uint64_t agpuDeviceCommitFrame(GPUDevice device);

/* Device resource creation methods */
ALIMER_GPU_API GPUBuffer agpuDeviceCreateBuffer(GPUDevice device, const GPUBufferDesc* desc, const void* pInitialData);
ALIMER_GPU_API GPUTexture agpuDeviceCreateTexture(GPUDevice device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData);

/* CommandQueue */
ALIMER_GPU_API GPUCommandQueueType agpuCommandQueueGetType(GPUCommandQueue queue);
ALIMER_GPU_API void agpuCommandQueueWaitIdle(GPUCommandQueue queue);
ALIMER_GPU_API GPUCommandBuffer agpuCommandQueueAcquireCommandBuffer(GPUCommandQueue queue, const GPUCommandBufferDesc* desc);
ALIMER_GPU_API void agpuCommandQueueSubmit(GPUCommandQueue queue, uint32_t numCommandBuffers, GPUCommandBuffer* commandBuffers);

/* CommandBuffer */
ALIMER_GPU_API void agpuCommandBufferPushDebugGroup(GPUCommandBuffer commandBuffer, const char* groupLabel);
ALIMER_GPU_API void agpuCommandBufferPopDebugGroup(GPUCommandBuffer commandBuffer);
ALIMER_GPU_API void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer commandBuffer, const char* markerLabel);
ALIMER_GPU_API GPUAcquireSurfaceResult agpuCommandBufferAcquireSurfaceTexture(GPUCommandBuffer commandBuffer, GPUSurface surface, GPUTexture* surfaceTexture);
ALIMER_GPU_API GPUComputePassEncoder agpuCommandBufferBeginComputePass(GPUCommandBuffer commandBuffer, const GPUComputePassDesc* desc);
ALIMER_GPU_API GPURenderPassEncoder agpuCommandBufferBeginRenderPass(GPUCommandBuffer commandBuffer, const GPURenderPassDesc* desc);

/* ComputePassEncoder */
ALIMER_GPU_API void agpuComputePassEncoderSetPipeline(GPUComputePassEncoder computePassEncoder, GPUComputePipeline pipeline);
ALIMER_GPU_API void agpuComputePassEncoderSetPushConstants(GPUComputePassEncoder computePassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size);
ALIMER_GPU_API void agpuComputePassEncoderDispatch(GPUComputePassEncoder computePassEncoder, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
ALIMER_GPU_API void agpuComputePassEncoderDispatchIndirect(GPUComputePassEncoder computePassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_GPU_API void agpuComputePassEncoderEnd(GPUComputePassEncoder computePassEncoder);
ALIMER_GPU_API void agpuComputePassEncoderPushDebugGroup(GPUComputePassEncoder computePassEncoder, const char* groupLabel);
ALIMER_GPU_API void agpuComputePassEncoderPopDebugGroup(GPUComputePassEncoder computePassEncoder);
ALIMER_GPU_API void agpuComputePassEncoderInsertDebugMarker(GPUComputePassEncoder computePassEncoder, const char* markerLabel);

/* RenderCommandEncoder */
ALIMER_GPU_API void agpuRenderPassEncoderSetViewport(GPURenderPassEncoder renderPassEncoder, const GPUViewport* viewport);
ALIMER_GPU_API void agpuRenderPassEncoderSetViewports(GPURenderPassEncoder renderPassEncoder, uint32_t viewportCount, const GPUViewport* viewports);
ALIMER_GPU_API void agpuRenderPassEncoderSetScissorRect(GPURenderPassEncoder renderPassEncoder, const GPUScissorRect* scissorRect);
ALIMER_GPU_API void agpuRenderPassEncoderSetScissorRects(GPURenderPassEncoder renderPassEncoder, uint32_t scissorCount, const GPUScissorRect* scissorRects);
ALIMER_GPU_API void agpuRenderPassEncoderSetBlendColor(GPURenderPassEncoder renderPassEncoder, const GPUColor* color);
ALIMER_GPU_API void agpuRenderPassEncoderSetStencilReference(GPURenderPassEncoder renderPassEncoder, uint32_t reference);
ALIMER_GPU_API void agpuRenderPassEncoderSetVertexBuffer(GPURenderPassEncoder renderPassEncoder, uint32_t slot, GPUBuffer buffer, uint64_t offset);
ALIMER_GPU_API void agpuRenderPassEncoderSetIndexBuffer(GPURenderPassEncoder renderPassEncoder, GPUBuffer buffer, GPUIndexType type, uint64_t offset);
ALIMER_GPU_API void agpuRenderPassEncoderSetPipeline(GPURenderPassEncoder renderPassEncoder, GPURenderPipeline pipeline);
ALIMER_GPU_API void agpuRenderPassEncoderSetPushConstants(GPURenderPassEncoder renderPassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size);
ALIMER_GPU_API void agpuRenderPassEncoderDraw(GPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
ALIMER_GPU_API void agpuRenderPassEncoderDrawIndexed(GPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
ALIMER_GPU_API void agpuRenderPassEncoderDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_GPU_API void agpuRenderPassEncoderDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset);
ALIMER_GPU_API void agpuRenderPassEncoderMultiDrawIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset);
ALIMER_GPU_API void agpuRenderPassEncoderMultiDrawIndexedIndirect(GPURenderPassEncoder renderPassEncoder, GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer drawCountBuffer, uint64_t drawCountBufferOffset);
ALIMER_GPU_API void agpuRenderPassEncoderSetShadingRate(GPURenderPassEncoder renderPassEncoder, GPUShadingRate rate);
ALIMER_GPU_API void agpuRenderPassEncoderEnd(GPURenderPassEncoder renderPassEncoder);
ALIMER_GPU_API void agpuRenderPassEncoderPushDebugGroup(GPURenderPassEncoder renderPassEncoder, const char* groupLabel);
ALIMER_GPU_API void agpuRenderPassEncoderPopDebugGroup(GPURenderPassEncoder renderPassEncoder);
ALIMER_GPU_API void agpuRenderPassEncoderInsertDebugMarker(GPURenderPassEncoder renderPassEncoder, const char* markerLabel);

/* Buffer */
ALIMER_GPU_API void agpuBufferSetLabel(GPUBuffer buffer, const char* label);
ALIMER_GPU_API uint32_t agpuBufferAddRef(GPUBuffer buffer);
ALIMER_GPU_API uint32_t agpuBufferRelease(GPUBuffer buffer);
ALIMER_GPU_API uint64_t agpuBufferGetSize(GPUBuffer buffer);
ALIMER_GPU_API GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer buffer);

/* Texture */
ALIMER_GPU_API void agpuTextureSetLabel(GPUTexture texture, const char* label);
ALIMER_GPU_API GPUTextureDimension agpuTextureGetDimension(GPUTexture texture);
ALIMER_GPU_API GPUPixelFormat agpuTextureGetFormat(GPUTexture texture);
ALIMER_GPU_API GPUTextureUsage agpuTextureGetUsage(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetWidth(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetHeight(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetDepthOrArrayLayers(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetMipLevelCount(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetSampleCount(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureGetLevelWidth(GPUTexture texture, uint32_t mipLevel);
ALIMER_GPU_API uint32_t agpuTextureGetLevelHeight(GPUTexture texture, uint32_t mipLevel);
ALIMER_GPU_API uint32_t agpuTextureAddRef(GPUTexture texture);
ALIMER_GPU_API uint32_t agpuTextureRelease(GPUTexture texture);

/* Sampler */
ALIMER_GPU_API GPUSampler agpuCreateSampler(GPUDevice device, const GPUSamplerDesc* desc);
ALIMER_GPU_API void agpuSamplerSetLabel(GPUSampler sampler, const char* label);
ALIMER_GPU_API uint32_t agpuSamplerAddRef(GPUSampler sampler);
ALIMER_GPU_API uint32_t agpuSamplerRelease(GPUSampler sampler);

/* PipelineLayout */
ALIMER_GPU_API GPUPipelineLayout agpuCreatePipelineLayout(GPUDevice device, const GPUPipelineLayoutDesc* desc);
ALIMER_GPU_API void agpuPipelineLayoutSetLabel(GPUPipelineLayout pipelineLayout, const char* label);
ALIMER_GPU_API uint32_t agpuPipelineLayoutAddRef(GPUPipelineLayout pipelineLayout);
ALIMER_GPU_API uint32_t agpuPipelineLayoutRelease(GPUPipelineLayout pipelineLayout);

/* ShaderModule */
ALIMER_GPU_API GPUShaderModule agpuCreateShaderModule(GPUDevice device, const GPUShaderModuleDesc* desc);
ALIMER_GPU_API void agpuShaderModuleSetLabel(GPUShaderModule shaderModule, const char* label);
ALIMER_GPU_API uint32_t agpuShaderModuleAddRef(GPUShaderModule shaderModule);
ALIMER_GPU_API uint32_t agpuShaderModuleRelease(GPUShaderModule shaderModule);

/* ComputePipeline */
ALIMER_GPU_API GPUComputePipeline agpuCreateComputePipeline(GPUDevice device, const GPUComputePipelineDesc* desc);
ALIMER_GPU_API void agpuComputePipelineSetLabel(GPUComputePipeline computePipeline, const char* label);
ALIMER_GPU_API uint32_t agpuComputePipelineAddRef(GPUComputePipeline computePipeline);
ALIMER_GPU_API uint32_t agpuComputePipelineRelease(GPUComputePipeline computePipeline);

/* RenderPipeline */
ALIMER_GPU_API GPURenderPipeline agpuCreateRenderPipeline(GPUDevice device, const GPURenderPipelineDesc* desc);
ALIMER_GPU_API void agpuRenderPipelineSetLabel(GPURenderPipeline renderPipeline, const char* label);
ALIMER_GPU_API uint32_t agpuRenderPipelineAddRef(GPURenderPipeline renderPipeline);
ALIMER_GPU_API uint32_t agpuRenderPipelineRelease(GPURenderPipeline renderPipeline);

/* QueryHeap */
ALIMER_GPU_API GPUQueryHeap agpuCreateQueryHeap(GPUDevice device, const GPUQueryHeapDesc* desc);
ALIMER_GPU_API void agpuQueryHeapSetLabel(GPUQueryHeap queryHeap, const char* label);
ALIMER_GPU_API uint32_t agpuQueryHeapAddRef(GPUQueryHeap queryHeap);
ALIMER_GPU_API uint32_t agpuQueryHeapRelease(GPUQueryHeap queryHeap);

/* Other */
ALIMER_GPU_API uint32_t agpuGetVertexFormatByteSize(GPUVertexFormat format);
ALIMER_GPU_API uint32_t agpuGetVertexFormatComponentCount(GPUVertexFormat format);
ALIMER_GPU_API GPUAdapterVendor agpuGPUAdapterVendorFromID(uint32_t vendorId);
ALIMER_GPU_API uint32_t agpuGPUAdapterVendorToID(GPUAdapterVendor vendor);

#endif /* ALIMER_GPU_H_ */
