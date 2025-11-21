// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_GPU_INTERNAL_H_
#define ALIMER_GPU_INTERNAL_H_

#include "alimer_internal.h"
#include "alimer_gpu.h"
#include <atomic>

// Forward declaration for native handle
typedef struct HINSTANCE__* HINSTANCE;
typedef struct HWND__* HWND;
struct IUnknown;
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

struct GPUSampler : public GPUResource
{

};

struct GPUQueryHeap : public GPUResource
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

struct GPUComputePipeline : public GPUResource
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

struct GPUComputePassEncoder : public GPUCommandEncoder
{
    virtual void SetPipeline(GPUComputePipeline* pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void DispatchIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
};

struct GPURenderPassEncoder : public GPUCommandEncoder
{
    virtual void SetViewport(const GPUViewport* viewport) = 0;
    virtual void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) = 0;
    virtual void SetScissorRect(const GPUScissorRect* scissorRect) = 0;
    virtual void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) = 0;
    virtual void SetBlendColor(const Color* color) = 0;
    virtual void SetStencilReference(uint32_t reference) = 0;

    virtual void SetVertexBuffer(uint32_t slot, GPUBuffer* buffer, uint64_t offset) = 0;
    virtual void SetIndexBuffer(GPUBuffer* buffer, GPUIndexType type, uint64_t offset) = 0;
    virtual void SetPipeline(GPURenderPipeline pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
    virtual void DrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;

    virtual void MultiDrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;
    virtual void MultiDrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;

    virtual void SetShadingRate(GPUShadingRate rate) = 0;
};

struct GPUCommandBuffer : public GPUResource
{
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;

    virtual GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface* surface, GPUTexture** surfaceTexture) = 0;
    virtual GPUComputePassEncoder* BeginComputePass(const GPUComputePassDesc& desc) = 0;
    virtual GPURenderPassEncoder* BeginRenderPass(const GPURenderPassDesc& desc) = 0;
};

struct GPUCommandQueue : public GPUResource
{
    virtual GPUCommandQueueType GetType() const = 0;

    virtual void WaitIdle() = 0;
    virtual GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) = 0;
    virtual void Submit(uint32_t numCommandBuffers, GPUCommandBuffer** commandBuffers) = 0;
};

struct GPUDevice : public GPUResource
{
    virtual bool HasFeature(GPUFeature feature) const = 0;
    virtual GPUCommandQueue* GetQueue(GPUCommandQueueType type) = 0;
    virtual void WaitIdle() = 0;
    virtual uint64_t CommitFrame() = 0;

    virtual uint64_t GetTimestampFrequency() const = 0;

    /* Resource creation */
    virtual GPUBuffer* CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) = 0;
    virtual GPUTexture* CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) = 0;
    virtual GPUSampler* CreateSampler(const GPUSamplerDesc& desc) = 0;
    virtual GPUBindGroupLayoutImpl* CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) = 0;
    virtual GPUPipelineLayoutImpl* CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) = 0;
    virtual GPUComputePipeline* CreateComputePipeline(const GPUComputePipelineDesc& desc) = 0;
    virtual GPURenderPipelineImpl* CreateRenderPipeline(const GPURenderPipelineDesc& desc) = 0;
    virtual GPUQueryHeap* CreateQueryHeap(const GPUQueryHeapDesc& desc) = 0;

};

struct GPUSurfaceHandle
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
    HINSTANCE hinstance = nullptr;
    HWND hwnd = nullptr;

    // IDCompositionVisual/SwapChainPanel
    IUnknown* idCompositionVisualOrSwapChainPanel = nullptr;
    // SurfaceHandle
    void* surfaceHandle = nullptr;
};

struct GPUSurface : public GPUResource
{
    virtual void GetCapabilities(GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities) const = 0;
    virtual bool Configure(const GPUSurfaceConfig* config_) = 0;
    virtual void Unconfigure() = 0;

    GPUSurfaceConfig config;
};

struct GPUAdapter : public GPUResource
{
    virtual GPUAdapterType GetType() const = 0;
    virtual void GetInfo(GPUAdapterInfo* info) const = 0;
    virtual void GetLimits(GPUAdapterLimits* limits) const = 0;
    virtual bool HasFeature(GPUFeature feature) const = 0;
    virtual GPUDevice* CreateDevice(const GPUDeviceDesc& desc) = 0;
};

struct GPUFactory : public GPUResource
{
public:
    virtual ~GPUFactory() = default;

    virtual GPUBackendType GetBackend() const = 0;
    virtual uint32_t GetAdapterCount() const = 0;
    virtual GPUAdapter* GetAdapter(uint32_t index) const = 0;
    virtual GPUSurface* CreateSurface(GPUSurfaceHandle* surfaceHandle) = 0;
};

namespace
{
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

_ALIMER_EXTERN GPUFactory* Null_CreateInstance(const GPUFactoryDesc* desc);

#if defined(ALIMER_GPU_VULKAN)
_ALIMER_EXTERN bool Vulkan_IsSupported(void);
_ALIMER_EXTERN GPUFactory* Vulkan_CreateInstance(const GPUFactoryDesc* desc);
#endif

#if defined(ALIMER_GPU_D3D12)
_ALIMER_EXTERN bool D3D12_IsSupported(void);
_ALIMER_EXTERN GPUFactory* D3D12_CreateInstance(const GPUFactoryDesc* desc);
#endif

#if defined(ALIMER_GPU_WEBGPU)
_ALIMER_EXTERN bool WGPU_IsSupported(void);
_ALIMER_EXTERN GPUFactory* WGPU_CreateInstance(const GPUFactoryDesc* desc);
#endif

#endif /* ALIMER_GPU_INTERNAL_H_ */
