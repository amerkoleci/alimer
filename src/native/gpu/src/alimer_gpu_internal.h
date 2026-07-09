// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_INTERNAL_H_
#define ALIMER_GPU_INTERNAL_H_

//#include "alimer_internal.h"
#include "alimer_gpu.h"
#include <string.h>         /* for memset, memcpy */
#include <algorithm>
#include <atomic>
#include <functional>

#if defined(_WIN32)
#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// We don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// We dont' need <mcx.h>
#define NOMCX

// We dont' need <winsvc.h>
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP
#include <windows.h>

#ifndef __IUnknown_FWD_DEFINED__
#define __IUnknown_FWD_DEFINED__
typedef struct IUnknown IUnknown;
#endif 	/* __IUnknown_FWD_DEFINED__ */
#endif

/* Compiler defines */
#if defined(__clang__)
#define ALIMER_THREADLOCAL _Thread_local
#define ALIMER_DEPRECATED __attribute__(deprecated)
#define ALIMER_FORCE_INLINE inline __attribute__((__always_inline__))
#define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ALIMER_UNREACHABLE() __builtin_unreachable()
#define ALIMER_DEBUG_BREAK() __builtin_trap()
// CLANG ENABLE/DISABLE WARNING DEFINITION
#define ALIMER_DISABLE_WARNINGS() \
    _Pragma("clang diagnostic push")\
	_Pragma("clang diagnostic ignored \"-Wall\"") \
	_Pragma("clang diagnostic ignored \"-Wextra\"") \
	_Pragma("clang diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-completeness\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-extension\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-function\"") \
    _Pragma("clang diagnostic ignored \"-Wtypedef-redefinition\"") \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")

#define ALIMER_ENABLE_WARNINGS() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
#define ALIMER_THREADLOCAL __thread
#define ALIMER_DEPRECATED __attribute__(deprecated)
#define ALIMER_FORCE_INLINE inline __attribute__((__always_inline__))
#define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ALIMER_UNREACHABLE() __builtin_unreachable()
#define ALIMER_DEBUG_BREAK() __builtin_trap()
// GCC ENABLE/DISABLE WARNING DEFINITION
#	define ALIMER_DISABLE_WARNINGS() \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wall\"") \
	_Pragma("GCC diagnostic ignored \"-Wextra\"") \
	_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-result\"")

#define ALIMER_ENABLE_WARNINGS() _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define ALIMER_THREADLOCAL __declspec(thread)
#define ALIMER_DEPRECATED __declspec(deprecated)
#define ALIMER_FORCE_INLINE __forceinline
#define ALIMER_LIKELY(x) (x)
#define ALIMER_UNLIKELY(x) (x)
#define ALIMER_UNREACHABLE() __assume(false)
#define ALIMER_DEBUG_BREAK() __debugbreak()
#define ALIMER_DISABLE_WARNINGS() __pragma(warning(push, 0))
#define ALIMER_ENABLE_WARNINGS() __pragma(warning(pop))
#endif

#define ALIMER_UNUSED(x) (void)(x)
#define ALIMER_COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))
#define _ALIMER_DEF(val, def) (((val) == 0) ? (def) : (val))
#define _ALIMER_DEF_FLT(val, def) (((val) == 0.0f) ? (def) : (val))

#ifdef ALIMER_ENABLE_ASSERTS
#   include <assert.h>
#   define ALIMER_ASSERT(c) assert(c)
#else
#   define ALIMER_ASSERT(...) ((void)0)
#endif

// Forward declaration for native handle
struct ANativeWindow;
struct wl_display;
struct wl_surface;

enum class TextureLayout : uint8_t
{
    Undefined,
    CopySource,
    CopyDest,
    ResolveSource,
    ResolveDest,
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
    {}

private:
    std::atomic_uint32_t refCount = 1;
};

struct GPUBufferImpl : public GPUResource
{
    GPUBufferDesc desc;
    virtual GPUDeviceAddress GetDeviceAddress() const = 0;
};

struct GPUTextureImpl : public GPUResource
{
    GPUTextureDesc desc;
};

struct GPUSamplerImpl : public GPUResource
{

};

struct GPUQueryHeapImpl : public GPUResource
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

struct GPUShaderModuleImpl : public GPUResource
{

};

struct GPUComputePipelineImpl : public GPUResource
{

};

struct GPURenderPipelineImpl : public GPUResource
{

};

struct GPUCommandEncoder : public GPUResource
{
    virtual void EndEncoding() = 0;
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;
};

struct GPUComputePassEncoderImpl : public GPUCommandEncoder
{
    virtual void SetPipeline(GPUComputePipeline pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void DispatchIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset) = 0;
};

struct GPURenderPassEncoderImpl : public GPUCommandEncoder
{
    virtual void SetViewport(const GPUViewport* viewport) = 0;
    virtual void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) = 0;
    virtual void SetScissorRect(const GPUScissorRect* scissorRect) = 0;
    virtual void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) = 0;
    virtual void SetBlendColor(const GPUColor* color) = 0;
    virtual void SetStencilReference(uint32_t reference) = 0;

    virtual void SetVertexBuffer(uint32_t slot, GPUBuffer buffer, uint64_t offset) = 0;
    virtual void SetIndexBuffer(GPUBuffer buffer, GPUIndexType type, uint64_t offset) = 0;
    virtual void SetPipeline(GPURenderPipeline pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset) = 0;
    virtual void DrawIndexedIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset) = 0;

    virtual void MultiDrawIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;
    virtual void MultiDrawIndexedIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;

    virtual void SetShadingRate(GPUShadingRate rate) = 0;
};

struct GPUCommandBufferImpl : public GPUResource
{
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;

    virtual GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface surface, GPUTexture* surfaceTexture) = 0;
    virtual GPUComputePassEncoder BeginComputePass(const GPUComputePassDesc& desc) = 0;
    virtual GPURenderPassEncoder BeginRenderPass(const GPURenderPassDesc& desc) = 0;
};

struct GPUCommandQueueImpl : public GPUResource
{
    virtual GPUCommandQueueType GetType() const = 0;

    virtual void WaitIdle() = 0;
    virtual GPUCommandBuffer AcquireCommandBuffer(const GPUCommandBufferDesc* desc) = 0;
    virtual void Submit(uint32_t numCommandBuffers, GPUCommandBuffer* commandBuffers) = 0;
};

struct GPUDeviceImpl : public GPUResource
{
    virtual bool HasFeature(GPUFeature feature) const = 0;
    virtual GPUCommandQueue GetQueue(GPUCommandQueueType type) = 0;
    virtual void WaitIdle() = 0;
    virtual uint64_t CommitFrame() = 0;

    virtual uint64_t GetTimestampFrequency() const = 0;

    /* Resource creation */
    virtual GPUBuffer CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) = 0;
    virtual GPUTexture CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) = 0;
    virtual GPUSampler CreateSampler(const GPUSamplerDesc& desc) = 0;
    virtual GPUBindGroupLayout CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) = 0;
    virtual GPUPipelineLayout CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) = 0;

    virtual GPUShaderModule CreateShaderModule(const GPUShaderModuleDesc* desc) = 0;
    virtual GPUComputePipeline CreateComputePipeline(const GPUComputePipelineDesc& desc) = 0;
    virtual GPURenderPipeline CreateRenderPipeline(const GPURenderPipelineDesc& desc) = 0;
    virtual GPUQueryHeap CreateQueryHeap(const GPUQueryHeapDesc& desc) = 0;

};

struct GPUSurfaceSourceImpl final
{
    enum class Type
    {
        Invalid,
        AndroidWindow,
        MetalLayer,
        WindowsHWND,
        IDCompositionVisual,
        SwapChainPanel,
        SurfaceHandle,
        WaylandSurface,
        XlibWindow,
    } type = Type::Invalid;

    // MetalLayer
    void* metalLayer = nullptr;
    // ANativeWindow
    ANativeWindow* androidNativeWindow = nullptr;

    // Wayland
    wl_display* waylandDisplay = nullptr;
    wl_surface* waylandSurface = nullptr;
    // Xlib
    void* xDisplay = nullptr;
    uint64_t xWindow = 0;

    // WindowsHwnd
    HWND hwnd = nullptr;

    // IDCompositionVisual/SwapChainPanel
    IUnknown* idCompositionVisualOrSwapChainPanel = nullptr;
    // SurfaceHandle
    void* surfaceHandle = nullptr;
};

struct GPUSurfaceImpl : public GPUResource
{
    virtual void GetCapabilities(GPUAdapter adapter, GPUSurfaceCapabilities* capabilities) const = 0;
    virtual bool Configure(const GPUSurfaceConfig* config_) = 0;
    virtual void Unconfigure() = 0;

    GPUSurfaceConfig config;
};

struct GPUAdapterImpl : public GPUResource
{
    virtual GPUAdapterType GetType() const = 0;
    virtual void GetInfo(GPUAdapterInfo* info) const = 0;
    virtual void GetLimits(GPUAdapterLimits* limits) const = 0;
    virtual bool HasFeature(GPUFeature feature) const = 0;
    virtual GPUDevice CreateDevice(const GPUDeviceDesc& desc) = 0;
};

struct GPUFactoryImpl : public GPUResource
{
public:
    virtual ~GPUFactoryImpl() = default;

    virtual GPUBackendType GetBackend() const = 0;
    virtual uint32_t GetAdapterCount() const = 0;
    virtual GPUAdapter GetAdapter(uint32_t index) const = 0;
    virtual GPUSurface CreateSurface(GPUSurfaceSource source) = 0;
};


_ALIMER_EXTERN bool agpuShouldLog(GPULogLevel level);
_ALIMER_EXTERN void agpuLogInfo(const char* format, ...);
_ALIMER_EXTERN void agpuLogWarn(const char* format, ...);
_ALIMER_EXTERN void agpuLogError(const char* format, ...);

typedef enum GPUPixelFormatKind {
    /// Unsigned normalized formats
    GPUPixelFormatKind_Unorm,
    /// Unsigned normalized sRGB formats
    GPUPixelFormatKind_UnormSrgb,
    /// Signed normalized formats
    GPUPixelFormatKind_Snorm,
    /// Unsigned integer formats
    GPUPixelFormatKind_Uint,
    /// Unsigned integer formats
    GPUPixelFormatKind_Sint,
    /// Floating-point formats
    GPUPixelFormatKind_Float,

    _GPUPixelFormatKind_Count,
    _GPUPixelFormatKind_Force32 = 0x7FFFFFFF
} GPUPixelFormatKind;

typedef struct GPUPixelFormatInfo {
    GPUPixelFormat format;
    const char* name;
    uint8_t bytesPerBlock;
    uint8_t blockWidth;
    uint8_t blockHeight;
    GPUPixelFormatKind kind;
} GPUPixelFormatInfo;

_ALIMER_EXTERN GPUPixelFormatInfo agpuPixelFormatGetInfo(GPUPixelFormat format);

namespace
{
    template <typename T>
    void SafeRelease(T& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    /// Check if inV is a power of 2
    template <typename T>
    constexpr bool IsPowerOf2(T value)
    {
        return (value & (value - 1)) == 0;
    }

    template <typename T>
    inline T AlignUp(T val, T alignment)
    {
        ALIMER_ASSERT(IsPowerOf2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }


    // Returns smallest power of 2 greater or equal to v.
    static inline uint32_t NextPow2(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    static inline uint64_t NextPow2(uint64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;
        return v;
    }

    /// @brief Helper function that hashes a single value into ioSeed
    /// Taken from: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
    template <typename T>
    inline void HashCombine(size_t& ioSeed, const T& value)
    {
        std::hash<T> hasher;
        ioSeed ^= hasher(value) + 0x9e3779b9 + (ioSeed << 6) + (ioSeed >> 2);
    }

    constexpr bool alimerPixelFormatIsDepth(GPUPixelFormat format)
    {
        switch (format)
        {
            case GPUPixelFormat_Depth16Unorm:
            case GPUPixelFormat_Depth24UnormStencil8:
            case GPUPixelFormat_Depth32Float:
            case GPUPixelFormat_Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    constexpr bool alimerPixelFormatIsStencil(GPUPixelFormat format)
    {
        switch (format)
        {
            //case PixelFormat_Stencil8:
            case GPUPixelFormat_Depth24UnormStencil8:
            case GPUPixelFormat_Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    constexpr bool alimerPixelFormatIsDepthStencil(GPUPixelFormat format)
    {
        switch (format)
        {
            //case PixelFormat_Stencil8:
            case GPUPixelFormat_Depth16Unorm:
            case GPUPixelFormat_Depth24UnormStencil8:
            case GPUPixelFormat_Depth32Float:
            case GPUPixelFormat_Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    constexpr bool alimerPixelFormatIsDepthOnly(GPUPixelFormat format)
    {
        switch (format)
        {
            case GPUPixelFormat_Depth16Unorm:
            case GPUPixelFormat_Depth32Float:
                return true;
            default:
                return false;
        }
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t mipLevelCount) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount;
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice, uint32_t mipLevelCount, uint32_t arrayLayers) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount + planeSlice * mipLevelCount * arrayLayers;
    }

    inline uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth = 1u, uint32_t minDimension = 1u, uint32_t requiredAlignment = 1u)
    {
        uint32_t mips = 1;
        while (width > minDimension || height > minDimension || depth > minDimension)
        {
            width = std::max(minDimension, width >> 1u);
            height = std::max(minDimension, height >> 1u);
            depth = std::max(minDimension, depth >> 1u);
            if (
                AlignUp(width, requiredAlignment) != width ||
                AlignUp(height, requiredAlignment) != height ||
                AlignUp(depth, requiredAlignment) != depth
                )
                break;
            mips++;
        }
        return mips;
    }

    inline bool BlendEnabled(const GPURenderPipelineColorAttachmentDesc* state)
    {
        return
            state->colorBlendOperation != GPUBlendOperation_Add
            || state->destColorBlendFactor != GPUBlendFactor_Zero
            || state->srcColorBlendFactor != GPUBlendFactor_One
            || state->alphaBlendOperation != GPUBlendOperation_Add
            || state->destAlphaBlendFactor != GPUBlendFactor_Zero
            || state->srcAlphaBlendFactor != GPUBlendFactor_One;
    }

    inline bool StencilTestEnabled(const GPUDepthStencilState& depthStencil)
    {
        return depthStencil.backFace.compareFunction != GPUCompareFunction_Always
            || depthStencil.backFace.failOperation != GPUStencilOperation_Keep
            || depthStencil.backFace.depthFailOperation != GPUStencilOperation_Keep
            || depthStencil.backFace.passOperation != GPUStencilOperation_Keep
            || depthStencil.frontFace.compareFunction != GPUCompareFunction_Always
            || depthStencil.frontFace.failOperation != GPUStencilOperation_Keep
            || depthStencil.frontFace.depthFailOperation != GPUStencilOperation_Keep
            || depthStencil.frontFace.passOperation != GPUStencilOperation_Keep;
    }
}

namespace string
{
    inline void copy_safe(char* dst, size_t dstSize, const char* src)
    {
        if (!dst || !src || dstSize == 0)
        {
            return;
        }

        // Copy characters from src to dst until either (dstSize - 1) is exhausted or we hit a null terminator in src.
        while (dstSize > 1 && *src)
        {
            *dst++ = *src++;
            --dstSize;
        }
        // Fill the rest of dst with null characters to ensure null-termination.
        while (dstSize > 0)
        {
            *dst++ = 0;
            --dstSize;
        }
    }
}

_ALIMER_EXTERN GPUFactory Null_CreateFactory(const GPUFactoryDesc* desc);

#if defined(ALIMER_GPU_VULKAN)
_ALIMER_EXTERN bool Vulkan_IsSupported(void);
_ALIMER_EXTERN GPUFactory Vulkan_CreateFactory(const GPUFactoryDesc* desc);
#endif

#if defined(ALIMER_GPU_D3D12)
_ALIMER_EXTERN bool D3D12_IsSupported(void);
_ALIMER_EXTERN GPUFactory D3D12_CreateFactory(const GPUFactoryDesc* desc);
#endif

#if defined(ALIMER_GPU_WEBGPU)
_ALIMER_EXTERN bool WGPU_IsSupported(void);
_ALIMER_EXTERN GPUFactory WGPU_CreateInstance(const GPUFactoryDesc* desc);
#endif

#endif /* ALIMER_GPU_INTERNAL_H_ */
