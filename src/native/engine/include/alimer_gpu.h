// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_H_
#define ALIMER_GPU_H_

#include "alimer.h"

/* Forward declarations */
typedef struct GPUAdapterImpl*          GPUAdapter;
typedef struct GPUSurfaceImpl*          GPUSurface;
typedef struct GPUDeviceImpl*           GPUDevice;
typedef struct GPUQueueImpl*            GPUQueue;
typedef struct GPUCommandBufferImpl*    GPUCommandBuffer;
typedef struct GPUBufferImpl*           GPUBuffer;
typedef struct GPUTextureImpl*          GPUTexture;
typedef struct GPUTextureViewImpl*      GPUTextureView;
typedef struct GPUShaderModuleImpl*     GPUShaderModule;

/* Constants */
#define GPU_MAX_INFLIGHT_FRAMES (2u)

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
static const GPUBufferUsage GPUBufferUsage_None = 0x0000000000000000;
static const GPUBufferUsage GPUBufferUsage_Vertex = 0x0000000000000001;
static const GPUBufferUsage GPUBufferUsage_Index = 0x0000000000000002;
/// Supports Constant buffer access.
static const GPUBufferUsage GPUBufferUsage_Constant = 0x0000000000000004;
static const GPUBufferUsage GPUBufferUsage_ShaderRead = 0x0000000000000008;
static const GPUBufferUsage GPUBufferUsage_ShaderWrite = 0x0000000000000010;
/// Supports indirect buffer access for indirect draw/dispatch.
static const GPUBufferUsage GPUBufferUsage_Indirect = 0x0000000000000020;
/// Supports predication access for conditional rendering.
static const GPUBufferUsage GPUBufferUsage_Predication = 0x0000000000000040;
/// Supports ray tracing acceleration structure usage.
static const GPUBufferUsage GPUBufferUsage_RayTracing = 0x0000000000000080;

typedef uint64_t GPUTextureUsage;
static const GPUTextureUsage GPUTextureUsage_None = 0x0000000000000000;

typedef enum GPUBackendType {
    GPUBackendType_Undefined = 0,
    GPUBackendType_Null,
    GPUBackendType_Vulkan,
    GPUBackendType_D3D12,
    GPUBackendType_Metal,
    GPUBackendType_WebGPU,

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
    //GPUQueueType_VideoDecode,

    GPUQueueType_Count,
    _GPUQueueType_Force32 = 0x7FFFFFFF
} GPUQueueType;

/* Structs */
typedef struct GPULimits {
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    uint32_t maxTextureDimensionCube;
    uint32_t maxTextureArrayLayers;
} GPULimits;

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

typedef struct GPURequestAdapterOptions {
    GPUSurface compatibleSurface;
    GPUPowerPreference powerPreference;
} GPURequestAdapterOptions;

typedef struct GPUConfig {
    GPUBackendType preferredBackend;
    GPUValidationMode validationMode;
} GPUConfig;

ALIMER_API bool agpuIsBackendSupport(GPUBackendType backend);
ALIMER_API bool agpuInit(const GPUConfig* config);
ALIMER_API void agpuShutdown(void);
ALIMER_API GPUSurface agpuCreateSurface(Window* window);
ALIMER_API GPUAdapter agpuRequestAdapter(const GPURequestAdapterOptions* options);

/* Surface */
ALIMER_API uint32_t agpuSurfaceAddRef(GPUSurface surface);
ALIMER_API uint32_t agpuSurfaceRelease(GPUSurface surface);

/* Adapter */
ALIMER_API GPUResult agpuAdapterGetLimits(GPUAdapter adapter, GPULimits* limits);
ALIMER_API GPUDevice agpuAdapterCreateDevice(GPUAdapter adapter);

/* Device */
ALIMER_API void agpuDeviceRelease(GPUDevice device);
ALIMER_API GPUQueue agpuDeviceGetQueue(GPUDevice device, GPUQueueType type);
ALIMER_API bool agpuDeviceWaitIdle(GPUDevice device);

/// Commit the current frame and advance to next frame
ALIMER_API uint64_t agpuDeviceCommitFrame(GPUDevice device);

/* Queue */
ALIMER_API GPUCommandBuffer agpuQueueCreateCommandBuffer(GPUQueue queue, const GPUCommandBufferDesc* desc);

/* CommandBuffer */

/* Buffer */
ALIMER_API GPUBuffer agpuCreateBuffer(GPUDevice device, const GPUBufferDesc* desc, const void* pInitialData);
ALIMER_API uint32_t agpuBufferAddRef(GPUBuffer buffer);
ALIMER_API uint32_t agpuBufferRelease(GPUBuffer buffer);

/* Texture */
ALIMER_API GPUTexture agpuCreateTexture(GPUDevice device, const GPUTextureDesc* desc);
ALIMER_API uint32_t agpuTextureAddRef(GPUTexture texture);
ALIMER_API uint32_t agpuTextureRelease(GPUTexture texture);

#endif /* ALIMER_GPU_H_ */
