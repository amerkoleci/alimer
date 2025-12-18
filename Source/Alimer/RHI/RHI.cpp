// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/RHI/RHI.h"
#include "Alimer/Core/BitOperations.h"
#include "Alimer/Core/Hash.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Math/Matrix4x4.h"
#include "Alimer/IO/FileSystem.h"
//#include "Alimer/Assets/AssetManager.h"
#include <ShaderMake/ShaderBlob.h>

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#endif

namespace Alimer
{
    namespace
    {
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

        static const char* GetEntryPointName(ShaderStages stage)
        {
            switch (stage)
            {
                case ShaderStages::Vertex:
                    return "vertexMain";
                case ShaderStages::Fragment:
                    return "fragmentMain";
                case ShaderStages::Compute:
                    return "computeMain";
                case ShaderStages::Amplification:
                    ALIMER_UNREACHABLE();
                    break;
                case ShaderStages::Mesh:
                    ALIMER_UNREACHABLE();
                    break;
                default:
                    return "Main";
            }

            return "Main";
        }

        std::string GetShadersPath()
        {
            return "CompiledShaders";
        }

        std::string GetCompiledShadersPath(bool dxil)
        {
            if (dxil)
                return Path::Combine(GetShadersPath(), "dxil");
            else
                return Path::Combine(GetShadersPath(), "spirv");
        }
    }

    RHITextureView* RHITexture::GetDefaultView() const
    {
        if (!defaultView)
        {
            defaultView = GetView(nullptr);
        }

        return defaultView.Get();
    }

    RHITextureView* RHITexture::GetView(const TextureViewDesc* desc) const
    {
        if (!CheckBitsAny(usage, TextureUsage::ShaderRead | TextureUsage::ShaderWrite | TextureUsage::RenderTarget))
        {
            LOGE("Cannot create TextureView for texture without ShaderRead, ShaderWrite or RenderTarget usage");
        }

        TextureViewDesc creationDesc = {};
        if (desc)
            creationDesc = *desc;
        //else
        //    creationDesc.label = "Default";

        if (creationDesc.format == PixelFormat::Undefined)
        {
            // TODO: Use TextureAspect (like dawn)
            creationDesc.format = format;
        }

        creationDesc.baseMipLevel = Min(creationDesc.baseMipLevel, mipLevelCount);
        if (creationDesc.mipLevelCount == kMipLevelCountUndefined)
        {
            creationDesc.mipLevelCount = mipLevelCount - creationDesc.baseMipLevel;
        }
        else
        {
            creationDesc.mipLevelCount = Min(creationDesc.mipLevelCount, mipLevelCount - creationDesc.baseMipLevel);
        }

        const uint32_t textureArrayLayerCount = GetArrayLayers() * (type == TextureType::TextureCube ? 6 : 1);
        creationDesc.baseArrayLayer = Min(creationDesc.baseArrayLayer, textureArrayLayerCount);
        creationDesc.arrayLayerCount = Min(creationDesc.arrayLayerCount, textureArrayLayerCount - creationDesc.baseArrayLayer);

        size_t hash = 0;

        HashCombine(hash, (uint32_t)creationDesc.format);
        HashCombine(hash, (uint32_t)creationDesc.aspect);
        HashCombine(hash, creationDesc.baseMipLevel);
        HashCombine(hash, creationDesc.mipLevelCount);
        HashCombine(hash, creationDesc.baseArrayLayer);
        HashCombine(hash, creationDesc.arrayLayerCount);

        auto it = views.find(hash);
        if (it == views.end())
        {
            RHITextureViewRef newView = CreateView(creationDesc);
            if (!newView)
            {
                LOGE("Failed to create TextureView");
                return nullptr;
            }

            views[hash] = newView;
            return newView.Get();
        }

        return it->second.Get();
    }


    /* RHISurface */
    RHISurface::RHISurface(Type type_)
        : type(type_)
    {

    }

    ANativeWindow* RHISurface::GetAndroidNativeWindow() const
    {
        ALIMER_ASSERT(type == Type::AndroidWindow);
        return androidNativeWindow;
    }

    void* RHISurface::GetMetalLayer() const
    {
        ALIMER_ASSERT(type == Type::MetalLayer);
        return metalLayer;
    }

    wl_display* RHISurface::GetWaylandDisplay() const
    {
        ALIMER_ASSERT(type == Type::WaylandSurface);
        return waylandDisplay;
    }

    wl_surface* RHISurface::GetWaylandSurface() const
    {
        ALIMER_ASSERT(type == Type::WaylandSurface);
        return waylandSurface;
    }

    void* RHISurface::GetXDisplay() const
    {
        ALIMER_ASSERT(type == Type::XlibWindow);
        return xDisplay;
    }

    uint64_t RHISurface::GetXWindow() const
    {
        ALIMER_ASSERT(type == Type::XlibWindow);
        return xWindow;
    }

    HWND RHISurface::GetHWND() const
    {
        ALIMER_ASSERT(type == Type::WindowsHWND);
        return hwnd;
    }

    RHISurfaceRef RHISurface::CreateAndroid(ANativeWindow* window)
    {
        RHISurfaceRef surface(new RHISurface(Type::AndroidWindow));
        surface->androidNativeWindow = window;
        return surface;
    }

    RHISurfaceRef RHISurface::CreateMetalLayer(/*CAMetalLayer*/void* layer)
    {
        RHISurfaceRef surface(new RHISurface(Type::MetalLayer));
        surface->metalLayer = layer;
        return surface;
    }

    RHISurfaceRef RHISurface::CreateWayland(wl_display* waylandDisplay, wl_surface* waylandSurface)
    {
        RHISurfaceRef surface(new RHISurface(Type::WaylandSurface));
        surface->waylandDisplay = waylandDisplay;
        surface->waylandSurface = waylandSurface;
        return surface;
    }

    RHISurfaceRef RHISurface::CreateXlib(/*Display*/void* display, /*Window*/uint64_t window)
    {
        RHISurfaceRef surface(new RHISurface(Type::XlibWindow));
        surface->xDisplay = display;
        surface->xWindow = window;
        return surface;
    }

    RHISurfaceRef RHISurface::CreateWin32([[maybe_unused]] HWND hwnd)
    {
#if defined(_WIN32)
        ALIMER_ASSERT(IsWindow(hwnd));

        RHISurfaceRef surface(new RHISurface(Type::WindowsHWND));
        surface->hwnd = hwnd;
        return surface;
#else
        LOGE("Win32 surface can be created only on Windows platforms");
        return nullptr;
#endif
    }

    RHISurfaceRef RHISurface::CreateIDCompositionVisual(IUnknown* visual)
    {
        RHISurfaceRef surface(new RHISurface(Type::IDCompositionVisual));
        surface->idCompositionVisualOrSwapChainPanel = visual;
        return surface;
    }

    RHISurfaceRef RHISurface::CreateSwapChainPanel(IUnknown* swapChainPanel)
    {
        RHISurfaceRef surface(new RHISurface(Type::SwapChainPanel));
        surface->idCompositionVisualOrSwapChainPanel = swapChainPanel;
        return surface;
    }

    IUnknown* RHISurface::GetIDCompositionVisual() const
    {
        ALIMER_ASSERT(type == Type::IDCompositionVisual);
        return idCompositionVisualOrSwapChainPanel;
    }

    IUnknown* RHISurface::GetSwapChainPanel() const
    {
        ALIMER_ASSERT(type == Type::SwapChainPanel);
        return idCompositionVisualOrSwapChainPanel;
    }

    void* RHISurface::GetSurfaceHandle() const
    {
        ALIMER_ASSERT(type == Type::SurfaceHandle);
        return surfaceHandle;
    }

    RHISwapChain::RHISwapChain(RHISurface* surface_)
        : surface(surface_)
    {

    }

    GPUAllocation CopyContext::AllocateGPU(uint64_t size)
    {
        if (size == 0)
        {
            return {};
        }

        GPULinearAllocator& allocator = frameAllocators[GRHIDevice->GetFrameIndex()];
        const uint64_t alignment = Max(GRHIDevice->GetAdapterLimits().minConstantBufferOffsetAlignment, GRHIDevice->GetAdapterLimits().minStorageBufferOffsetAlignment);

        const uint64_t bufferSize = (allocator.buffer == nullptr) ? 0 : allocator.buffer->GetSize();
        const uint64_t freeSpace = bufferSize - allocator.offset;
        if (size > freeSpace)
        {
            BufferDesc bufferDesc;
            bufferDesc.size = AlignUp((bufferSize + size) * 2, alignment);
            bufferDesc.usage = BufferUsage::Vertex | BufferUsage::Index | BufferUsage::Constant | BufferUsage::ShaderRead;
            bufferDesc.memoryType = MemoryType::Upload;

            allocator.buffer = GRHIDevice->CreateBuffer(bufferDesc);
            allocator.offset = 0;
        }

        GPUAllocation allocation;
        allocation.buffer = allocator.buffer;
        allocation.offset = allocator.offset;
        allocation.data = (void*)((size_t)allocator.buffer->GetMappedData() + allocator.offset);

        allocator.offset += AlignUp(size, alignment);

        assert(allocation.IsValid());
        return allocation;
    }

    void CopyContext::UpdateBuffer(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size)
    {
        if (buffer == nullptr || data == nullptr)
            return;

        size = Alimer::Min(buffer->GetSize(), size);
        if (size == 0)
            return;

        GPUAllocation allocation = AllocateGPU(size);
        std::memcpy(allocation.data, data, size);
        CopyBufferToBuffer(allocation.buffer.Get(), allocation.offset, buffer, offset, size);
    }

    /* ComputeContext */
    void ComputeContext::PreDispatchValidation()
    {
#if defined(_DEBUG)
        ALIMER_ASSERT_MSG(!insideRenderPass, "Dispatch needs to happen Outside render pass.");
#endif
    }

    void ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        PreDispatchValidation();

        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    /* GraphicsContext */
    void GraphicsContext::Reset(uint32_t frameIndex)
    {
        insideRenderPass = false;
        currentShadingRate = ShadingRate::Invalid;
        frameAllocators[frameIndex].Reset();
    }

    void GraphicsContext::BeginRenderPass(const RenderPassDesc& desc)
    {
        if (insideRenderPass)
        {
            LOGE("Cannot begin render pass while inside render pass");
            return;
        }

        BeginRenderPassCore(desc);
        insideRenderPass = true;
    }

    void GraphicsContext::EndRenderPass()
    {
        if (!insideRenderPass)
        {
            LOGE("Cannot end render without BeginRenderPass first");
            return;
        }

        EndRenderPassCore();
        insideRenderPass = false;
    }

#if defined(ALIMER_RHI_VULKAN)
    extern bool Vulkan_IsSupported();
    extern RHIDevice* Vulkan_CreateDevice(const std::string& appName, const RHIDeviceDesc& desc);
#endif
#if defined(ALIMER_RHI_D3D12) && defined(TODO)
    extern bool D3D12_IsSupported();
    extern RHIDevice* D3D12_CreateDevice(const std::string& appName, const RHIDeviceDesc& desc);
#endif

    bool RHIIsSupported(GraphicsAPI backend)
    {
        switch (backend)
        {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
            case GraphicsAPI::D3D12:
                return D3D12_IsSupported();
#endif

#if defined(ALIMER_RHI_VULKAN)
            case GraphicsAPI::Vulkan:
                return Vulkan_IsSupported();
#endif

#if defined(ALIMER_RHI_METAL)
            case GraphicsAPI::Metal:
                return false;
#endif

            default:
                return false;
        }
    }

    RHIDevice* GRHIDevice = nullptr;

    RHIBufferRef RHIDevice::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        if (desc.size > limits.maxBufferSize)
        {
            LOGE("Buffer size too large: {}, limit: {}", desc.size, limits.maxBufferSize);
            return nullptr;
        }

        return CreateBufferCore(desc, initialData);
    }

    void RHIDevice::InitResources()
    {
        SamplerDesc samplerDesc{};

        // SamplerPointClamp
        samplerDesc.label = "SamplerPointClamp";
        samplerDesc.minFilter = SamplerMinMagFilter::Point;
        samplerDesc.magFilter = SamplerMinMagFilter::Point;
        samplerDesc.mipFilter = SamplerMipFilter::Point;
        samplerDesc.addressModeU = SamplerAddressMode::Clamp;
        samplerDesc.addressModeV = SamplerAddressMode::Clamp;
        samplerDesc.addressModeW = SamplerAddressMode::Clamp;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerPointWrap
        samplerDesc.label = "SamplerPointWrap";
        samplerDesc.minFilter = SamplerMinMagFilter::Point;
        samplerDesc.magFilter = SamplerMinMagFilter::Point;
        samplerDesc.mipFilter = SamplerMipFilter::Point;
        samplerDesc.addressModeU = SamplerAddressMode::Wrap;
        samplerDesc.addressModeV = SamplerAddressMode::Wrap;
        samplerDesc.addressModeW = SamplerAddressMode::Wrap;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerPointMirror
        samplerDesc.label = "SamplerPointMirror";
        samplerDesc.minFilter = SamplerMinMagFilter::Point;
        samplerDesc.magFilter = SamplerMinMagFilter::Point;
        samplerDesc.mipFilter = SamplerMipFilter::Point;
        samplerDesc.addressModeU = SamplerAddressMode::Mirror;
        samplerDesc.addressModeV = SamplerAddressMode::Mirror;
        samplerDesc.addressModeW = SamplerAddressMode::Mirror;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearClamp
        samplerDesc.label = "SamplerLinearClamp";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Clamp;
        samplerDesc.addressModeV = SamplerAddressMode::Clamp;
        samplerDesc.addressModeW = SamplerAddressMode::Clamp;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearWrap
        samplerDesc.label = "SamplerLinearWrap";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Wrap;
        samplerDesc.addressModeV = SamplerAddressMode::Wrap;
        samplerDesc.addressModeW = SamplerAddressMode::Wrap;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearMirror
        samplerDesc.label = "SamplerLinearMirror";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Mirror;
        samplerDesc.addressModeV = SamplerAddressMode::Mirror;
        samplerDesc.addressModeW = SamplerAddressMode::Mirror;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicClamp
        samplerDesc.label = "SamplerAnisotropicClamp";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Clamp;
        samplerDesc.addressModeV = SamplerAddressMode::Clamp;
        samplerDesc.addressModeW = SamplerAddressMode::Clamp;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicWrap
        samplerDesc.label = "SamplerAnisotropicWrap";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Wrap;
        samplerDesc.addressModeV = SamplerAddressMode::Wrap;
        samplerDesc.addressModeW = SamplerAddressMode::Wrap;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicMirror
        samplerDesc.label = "SamplerAnisotropicMirror";
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Linear;
        samplerDesc.addressModeU = SamplerAddressMode::Mirror;
        samplerDesc.addressModeV = SamplerAddressMode::Mirror;
        samplerDesc.addressModeW = SamplerAddressMode::Mirror;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerComparisonDepth
        samplerDesc.label = "SamplerComparisonDepth";
        samplerDesc.reductionType = SamplerReductionType::Comparison;
        samplerDesc.minFilter = SamplerMinMagFilter::Linear;
        samplerDesc.magFilter = SamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = SamplerMipFilter::Point;
        samplerDesc.addressModeU = SamplerAddressMode::Clamp;
        samplerDesc.addressModeV = SamplerAddressMode::Clamp;
        samplerDesc.addressModeW = SamplerAddressMode::Clamp;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.compareFunction = CompareFunction::GreaterEqual;
        samplerDesc.lodMinClamp = 0;
        samplerDesc.lodMaxClamp = 0;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));
    }

    bool RHIDevice::ValidateTextureDesc(const TextureDesc& desc)
    {
        if (desc.width == 0 || desc.height == 0 || desc.depthOrArrayLayers == 0)
        {
            LOGE("Texture width, height and depthOrArrayLayers must be non-zero.");
            return false;
        }

        if (desc.format == PixelFormat::Undefined)
        {
            LOGE("Texture format must be different than Undefined.");
            return false;
        }

        if ((desc.type == TextureType::Texture1D || desc.type == TextureType::Texture3D)
            && desc.sampleCount != TextureSampleCount::Count1)
        {
            LOGE("1D and 3D Textures must use TextureSampleCount.Count1.");
            return false;
        }

        return true;
    }

    RHITextureRef RHIDevice::CreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        if (!ValidateTextureDesc(desc))
        {
            return nullptr;
        }

        TextureDesc creationDesc = desc;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(desc.width, desc.height, desc.depthOrArrayLayers);
        }

        return CreateTextureCore(creationDesc, initialData);
    }

    RHITextureRef RHIDevice::CreateTextureFromNativeHandle(RHINativeHandle handle, const TextureDesc& desc)
    {
        if (!ValidateTextureDesc(desc))
        {
            return nullptr;
        }

        TextureDesc creationDesc = desc;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(desc.width, desc.height, desc.depthOrArrayLayers);
        }

        return CreateTextureFromNativeHandleCore(handle, creationDesc);
    }

    RHISamplerRef RHIDevice::CreateSampler(const SamplerDesc& desc)
    {
        return CreateSamplerCore(desc);
    }

    RHIShaderModuleRef RHIDevice::CreateShaderModule(const ShaderModuleDesc& desc)
    {
        if (desc.bytecodeSize == 0 || desc.bytecode == nullptr)
        {
            LOGE("Invalid shader module bytecode");
            return nullptr;
        }

        return CreateShaderModuleCore(desc);
    }

    RHIBindGroupLayoutRef RHIDevice::CreateBindGroupLayout(const BindGroupLayoutDesc& desc)
    {
        return CreateBindGroupLayoutCore(desc);
    }

    RHIPipelineLayoutRef RHIDevice::CreatePipelineLayout(const PipelineLayoutDesc& desc)
    {
        return CreatePipelineLayoutCore(desc);
    }

    RHIBindGroupRef RHIDevice::CreateBindGroup(RHIBindGroupLayout* layout, const BindGroupDesc& desc)
    {
        ALIMER_ASSERT(layout);
        ALIMER_ASSERT(desc.entryCount > 0);
        ALIMER_ASSERT(desc.entries != nullptr);

        return CreateBindGroupCore(layout, desc);
    }

    RHIPipelineRef RHIDevice::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        // Vertex shader is necessary when not using mesh shaders
        if (desc.vertexShader == nullptr)
        {
            if (desc.amplificationShader == nullptr && desc.meshShader == nullptr)
            {
                LOGE("Vertex shader is required when creating mesh pipeline");
                return nullptr;
            }
        }

        if (desc.vertexInput)
        {
            const VertexInputDesc* vertexInput = desc.vertexInput;
            for (uint32_t attributeIndex = 0; attributeIndex < vertexInput->attributeCount; ++attributeIndex)
            {
                const VertexAttribute& attribute = vertexInput->attributes[attributeIndex];
                ALIMER_ASSERT(attribute.format != VertexFormat::Undefined);
            }
        }

        return CreateRenderPipelineCore(desc);
    }

    RHIPipelineRef RHIDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
    {
        ALIMER_ASSERT(desc.layout != nullptr);
        ALIMER_ASSERT(desc.shader != nullptr);

        return CreateComputePipelineCore(desc);
    }

    RHIPipelineRef RHIDevice::CreateRayTracingPipeline(const RayTracingPipelineDesc& desc)
    {
        ALIMER_ASSERT(desc.layout != nullptr);

        return CreateRayTracingPipelineCore(desc);
    }

    RHIQueryHeapRef RHIDevice::CreateQueryHeap(const QueryHeapDesc& desc)
    {
        ALIMER_ASSERT(desc.count > 0 && desc.count < kQuerySetMaxQueries);

        if (desc.type == QueryType::PipelineStatistics
            && !QueryFeatureSupport(RHIFeature::PipelineStatisticsQuery))
        {
            LOGE("PipelineStatistics queries are not supported");
            return nullptr;
        }

        return CreateQueryHeapCore(desc);
    }

    RHISwapChainRef RHIDevice::CreateSwapChain(RHISurface* surface, const RHISwapChainDesc& desc)
    {
        ALIMER_ASSERT(surface);

        return CreateSwapChainCore(surface, desc);
    }

    GraphicsAPI RHIGetPlatformPreferredApi()
    {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
        if (D3D12_IsSupported())
        {
            return GraphicsAPI::D3D12;
        }
#endif
#if defined(ALIMER_RHI_VULKAN)
        if (Vulkan_IsSupported())
        {
            return GraphicsAPI::Vulkan;
        }
#endif
#if defined(ALIMER_RHI_METAL)
        if (Metal_IsSupported())
        {
            return GraphicsAPI::Metal;
        }
#endif
#if defined(ALIMER_RHI_WEBGPU)
        if (WGPU_IsSupported())
        {
            return GraphicsAPI::WebGPU;
        }
#endif

        return GraphicsAPI::Null;
    }

    bool RHIInit(const std::string& appName, const RHIDeviceDesc& desc)
    {
        if (GRHIDevice != nullptr)
            return true;

        GraphicsAPI api = desc.preferredApi;
        if (api == GraphicsAPI::Count)
        {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
            if (D3D12_IsSupported())
            {
                api = GraphicsAPI::D3D12;
            }
#endif

#if defined(ALIMER_RHI_VULKAN)
            if (Vulkan_IsSupported())
            {
                api = GraphicsAPI::Vulkan;
            }
#endif
        }

        switch (api)
        {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
            case GraphicsAPI::D3D12:
                if (D3D12_IsSupported())
                {
                    GRHIDevice = D3D12_CreateDevice(appName, desc);
                }
                else
                {
                    LOGF("Direct3D12 is not supported on current OS");
                    return false;
                }
                break;
#endif

#if defined(ALIMER_RHI_VULKAN)
            case GraphicsAPI::Vulkan:
                if (Vulkan_IsSupported())
                {
                    GRHIDevice = Vulkan_CreateDevice(appName, desc);
                }
                else
                {
                    LOGF("Vulkan is not supported on current OS");
                    return false;
                }
                break;
#endif


#if defined(ALIMER_RHI_WEBGPU)
            case GraphicsAPI::WebGPU:
                if (WGPU_IsSupported())
                {
                    GRHIDevice = WGPU_CreateDevice(appName, desc);
                }
                else
                {
                    LOGF("WGPU is not supported on current OS");
                    return false;
                }
                break;
#endif

            default:
                ALIMER_UNREACHABLE();
                break;
        }

        return GRHIDevice != nullptr;
    }

    void RHIShutdown()
    {
        if (GRHIDevice != nullptr)
        {
            delete GRHIDevice;
            GRHIDevice = nullptr;
        }
    }

    RHIBufferRef RHICreateBuffer(uint64_t size, BufferUsage usage, const void* initialData, const char* label)
    {
        ALIMER_ASSERT(GRHIDevice);

        BufferDesc bufferDesc{};
        bufferDesc.label = label;
        bufferDesc.size = size;
        bufferDesc.usage = usage;
        return GRHIDevice->CreateBuffer(bufferDesc, initialData);
    }

    RHIBufferRef RHICreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateBuffer(desc, initialData);
    }

    RHITextureRef RHICreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateTexture(desc, initialData);
    }

    RHISamplerRef RHICreateSampler(const SamplerDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateSampler(desc);
    }

    RHIShaderModuleRef RHICreateShaderModule(const ShaderModuleDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateShaderModule(desc);
    }

    RHIBindGroupLayoutRef RHICreateBindGroupLayout(const BindGroupLayoutDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateBindGroupLayout(desc);
    }

    RHIPipelineLayoutRef RHICreatePipelineLayout(const PipelineLayoutDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreatePipelineLayout(desc);
    }

    RHIBindGroupRef RHICreateBindGroup(RHIBindGroupLayout* layout, const BindGroupDesc& desc)
    {
        ALIMER_ASSERT(layout);
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateBindGroup(layout, desc);
    }

    RHIPipelineRef RHICreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateRenderPipeline(desc);
    }

    RHIPipelineRef RHICreateComputePipeline(const ComputePipelineDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateComputePipeline(desc);
    }

    RHIPipelineRef RHICreateRayTracingPipeline(const RayTracingPipelineDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateRayTracingPipeline(desc);
    }

    RHIQueryHeapRef RHICreateQueryHeap(const QueryHeapDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);

        return GRHIDevice->CreateQueryHeap(desc);
    }

    RHISwapChainRef RHICreateSwapChain(RHISurface* surface, const RHISwapChainDesc& desc)
    {
        ALIMER_ASSERT(GRHIDevice);
        ALIMER_ASSERT(surface);

        return GRHIDevice->CreateSwapChain(surface, desc);
    }

    const std::string ToString(GraphicsAPI type)
    {
        switch (type)
        {
            case GraphicsAPI::Null:         return "Null";
            case GraphicsAPI::Vulkan:       return "Vulkan";
            case GraphicsAPI::D3D12:        return "D3D12";
            default:                        return "<UNKNOWN>";
        }
    }

    const std::string ToString(AdapterType type)
    {
        switch (type)
        {
            case AdapterType::IntegratedGpu:    return "IntegratedGpu";
            case AdapterType::DiscreteGpu:      return "DiscreteGpu";
            case AdapterType::VirtualGpu:       return "VirtualGpu";
            case AdapterType::Cpu:              return "Cpu";
            default:                            return "Other";
        }
    }

    GPUAdapterVendor VendorIdToAdapterVendor(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case (uint32_t)KnownGPUAdapterVendor::AMD:
                return GPUAdapterVendor::AMD;
            case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
                return GPUAdapterVendor::NVIDIA;
            case (uint32_t)KnownGPUAdapterVendor::INTEL:
                return GPUAdapterVendor::Intel;
            case (uint32_t)KnownGPUAdapterVendor::ARM:
                return GPUAdapterVendor::ARM;
            case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
                return GPUAdapterVendor::Qualcomm;
            case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
                return GPUAdapterVendor::ImgTech;
            case (uint32_t)KnownGPUAdapterVendor::MSFT:
                return GPUAdapterVendor::MSFT;
            case (uint32_t)KnownGPUAdapterVendor::APPLE:
                return GPUAdapterVendor::Apple;
            case (uint32_t)KnownGPUAdapterVendor::MESA:
                return GPUAdapterVendor::Mesa;
            case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
                return GPUAdapterVendor::Broadcom;

            default:
                return GPUAdapterVendor::Unknown;
        }
    }

    uint32_t AdapterVendorToVendorId(GPUAdapterVendor vendor)
    {
        switch (vendor)
        {
            case GPUAdapterVendor::AMD:
                return (uint32_t)KnownGPUAdapterVendor::AMD;
            case GPUAdapterVendor::NVIDIA:
                return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
            case GPUAdapterVendor::Intel:
                return (uint32_t)KnownGPUAdapterVendor::INTEL;
            case GPUAdapterVendor::ARM:
                return (uint32_t)KnownGPUAdapterVendor::ARM;
            case GPUAdapterVendor::Qualcomm:
                return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
            case GPUAdapterVendor::ImgTech:
                return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
            case GPUAdapterVendor::MSFT:
                return (uint32_t)KnownGPUAdapterVendor::MSFT;
            case GPUAdapterVendor::Apple:
                return (uint32_t)KnownGPUAdapterVendor::APPLE;
            case GPUAdapterVendor::Mesa:
                return (uint32_t)KnownGPUAdapterVendor::MESA;
            case GPUAdapterVendor::Broadcom:
                return (uint32_t)KnownGPUAdapterVendor::BROADCOM;

            default:
                return 0;
        }
    }

    uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth, uint32_t minDimension, uint32_t requiredAlignment)
    {
        uint32_t mips = 1;
        while (width > minDimension || height > minDimension || depth > minDimension)
        {
            width = Max(minDimension, width >> 1u);
            height = Max(minDimension, height >> 1u);
            depth = Max(minDimension, depth >> 1u);
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

    bool BlendEnabled(const RenderTargetBlendState* state)
    {
        return
            state->colorBlendOp != BlendOperation::Add ||
            state->destColorBlendFactor != BlendFactor::Zero ||
            state->srcColorBlendFactor != BlendFactor::One ||
            state->alphaBlendOp != BlendOperation::Add ||
            state->destAlphaBlendFactor != BlendFactor::Zero ||
            state->srcAlphaBlendFactor != BlendFactor::One
            ;
    }

    bool StencilTestEnabled(const DepthStencilState* depthStencil)
    {
        return
            depthStencil->backFace.compareFunc != CompareFunction::Always ||
            depthStencil->backFace.failOp != StencilOperation::Keep ||
            depthStencil->backFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->backFace.passOp != StencilOperation::Keep ||
            depthStencil->frontFace.compareFunc != CompareFunction::Always ||
            depthStencil->frontFace.failOp != StencilOperation::Keep ||
            depthStencil->frontFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->frontFace.passOp != StencilOperation::Keep;
    }

    DescriptorType GetDescriptorType(const BindGroupLayoutEntry& entry)
    {
        if (entry.buffer.type != BufferBindingType::Undefined)
        {
            return DescriptorType::Buffer;
        }
        else if (entry.sampler.type != SamplerBindingType::Undefined || entry.sampler.staticSampler != nullptr)
        {
            return DescriptorType::Sampler;
        }
        else if (entry.texture.sampleType != TextureSampleType::Undefined)
        {
            return DescriptorType::Texture;
        }
        else if (entry.storageTexture.access != StorageTextureAccess::Undefined)
        {
            return DescriptorType::StorageTexture;
        }
        else
        {
            return DescriptorType::Undefined;
        }
    }

    static BlendState BlendStateDescs[ecast(CommonBlendState::Count)] = {};
    static RasterizerState RasterizerStateDescc[ecast(CommonRasterizerState::Count)] = {};
    static DepthStencilState DepthStateDescs[ecast(CommonDepthStencilState::Count)] = {};

    static void InitCommonStates()
    {
        // https://github.com/microsoft/DirectXTK12/blob/main/Src/CommonStates.cpp
        static bool initialized = false;
        if (!initialized)
        {
            // Blend state initialization
            {
                BlendState blendDesc;
                BlendStateDescs[ecast(CommonBlendState::Opaque)] = blendDesc;

                blendDesc.renderTargets[0].srcColorBlendFactor = BlendFactor::SourceAlpha;
                blendDesc.renderTargets[0].destColorBlendFactor = BlendFactor::OneMinusSourceAlpha;
                blendDesc.renderTargets[0].colorBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].srcAlphaBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].destAlphaBlendFactor = BlendFactor::Zero;
                blendDesc.renderTargets[0].alphaBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::All;
                BlendStateDescs[ecast(CommonBlendState::Transparent)] = blendDesc;

                //blendDesc.renderTargets[0].blendEnable = true;
                blendDesc.renderTargets[0].srcColorBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].destColorBlendFactor = BlendFactor::OneMinusSourceAlpha;
                blendDesc.renderTargets[0].colorBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].srcAlphaBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].destAlphaBlendFactor = BlendFactor::OneMinusSourceAlpha;
                blendDesc.renderTargets[0].alphaBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::All;
                BlendStateDescs[ecast(CommonBlendState::Premultiplied)] = blendDesc;

                //blendDesc.renderTargets[0].blendEnable = true;
                blendDesc.renderTargets[0].srcColorBlendFactor = BlendFactor::SourceAlpha;
                blendDesc.renderTargets[0].destColorBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].colorBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].srcAlphaBlendFactor = BlendFactor::Zero;
                blendDesc.renderTargets[0].destAlphaBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].alphaBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::All;
                BlendStateDescs[ecast(CommonBlendState::Additive)] = blendDesc;

                blendDesc = {};
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::None;
                BlendStateDescs[ecast(CommonBlendState::ColorWriteDisable)] = blendDesc;

                //blendDesc.renderTargets[0].blendEnable = true;
                blendDesc.renderTargets[0].srcColorBlendFactor = BlendFactor::DestinationColor;
                blendDesc.renderTargets[0].destColorBlendFactor = BlendFactor::Zero;
                blendDesc.renderTargets[0].colorBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].srcAlphaBlendFactor = BlendFactor::DestinationAlpha;
                blendDesc.renderTargets[0].destAlphaBlendFactor = BlendFactor::Zero;
                blendDesc.renderTargets[0].alphaBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::All;
                BlendStateDescs[ecast(CommonBlendState::Multiply)] = blendDesc;

                //blendDesc.renderTargets[0].blendEnable = true;
                blendDesc.renderTargets[0].srcColorBlendFactor = BlendFactor::Zero;
                blendDesc.renderTargets[0].destColorBlendFactor = BlendFactor::SourceColor;
                blendDesc.renderTargets[0].colorBlendOp = BlendOperation::Add;
                blendDesc.renderTargets[0].srcAlphaBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].destAlphaBlendFactor = BlendFactor::One;
                blendDesc.renderTargets[0].alphaBlendOp = BlendOperation::Max;
                blendDesc.renderTargets[0].colorWriteMask = ColorWriteMask::All;
                BlendStateDescs[ecast(CommonBlendState::TransparentShadow)] = blendDesc;
            }

            // RasterizerState initialization
            {
                RasterizerState rasterizerState;
                rasterizerState.fillMode = FillMode::Solid;
                rasterizerState.cullMode = CullMode::None;
                RasterizerStateDescc[ecast(CommonRasterizerState::CullNone)] = rasterizerState;

                rasterizerState.fillMode = FillMode::Solid;
                rasterizerState.cullMode = CullMode::Front;
                RasterizerStateDescc[ecast(CommonRasterizerState::CullFront)] = rasterizerState;

                rasterizerState.fillMode = FillMode::Solid;
                rasterizerState.cullMode = CullMode::Back;
                RasterizerStateDescc[ecast(CommonRasterizerState::CullBack)] = rasterizerState;

                rasterizerState.fillMode = FillMode::Wireframe;
                rasterizerState.cullMode = CullMode::None;
                RasterizerStateDescc[ecast(CommonRasterizerState::Wireframe)] = rasterizerState;
            }

            // DepthStencilState initialization
            {
                DepthStencilState& dsDesc = DepthStateDescs[ecast(CommonDepthStencilState::None)];
                dsDesc.depthWriteEnabled = false;
                dsDesc.depthCompare = CompareFunction::Always;
            }

            {
                DepthStencilState& dsDesc = DepthStateDescs[ecast(CommonDepthStencilState::Default)];
                dsDesc.depthWriteEnabled = true;
                dsDesc.depthCompare = CompareFunction::LessEqual;
            }

            {
                DepthStencilState& dsDesc = DepthStateDescs[ecast(CommonDepthStencilState::Read)];
                dsDesc.depthWriteEnabled = false;
                dsDesc.depthCompare = CompareFunction::LessEqual;
            }

            {
                DepthStencilState& dsDesc = DepthStateDescs[ecast(CommonDepthStencilState::ReverseZ)];
                dsDesc.depthWriteEnabled = true;
                dsDesc.depthCompare = CompareFunction::GreaterEqual;
            }

            {
                DepthStencilState& dsDesc = DepthStateDescs[ecast(CommonDepthStencilState::ReadReverseZ)];
                dsDesc.depthWriteEnabled = false;
                dsDesc.depthCompare = CompareFunction::GreaterEqual;
            }

            initialized = true;
        }
    }

    BlendState GetBlendState(CommonBlendState state)
    {
        InitCommonStates();

        return BlendStateDescs[ecast(state)];
    }

    RasterizerState GetRasterizerState(CommonRasterizerState state)
    {
        InitCommonStates();

        return RasterizerStateDescc[ecast(state)];
    }


    DepthStencilState GetDepthStencilState(CommonDepthStencilState state)
    {
        InitCommonStates();

        return DepthStateDescs[ecast(state)];
    }

    static const VertexFormatInfo s_VertexFormatTable[] = {
        {VertexFormat::Undefined,           0, 0,   VertexFormatKind::Float},
        {VertexFormat::UByte,               1, 1,   VertexFormatKind::Uint},
        {VertexFormat::UByte2,              2, 2,   VertexFormatKind::Uint},
        {VertexFormat::UByte4,              4, 4,   VertexFormatKind::Uint},
        {VertexFormat::Byte,                1, 1,   VertexFormatKind::Sint},
        {VertexFormat::Byte2,               2, 2,   VertexFormatKind::Sint},
        {VertexFormat::Byte4,               4, 4,   VertexFormatKind::Sint},
        {VertexFormat::UByteNormalized,     1, 1,   VertexFormatKind::Unorm},
        {VertexFormat::UByte2Normalized,    2, 2,   VertexFormatKind::Unorm},
        {VertexFormat::UByte4Normalized,    4, 4,   VertexFormatKind::Unorm},
        {VertexFormat::ByteNormalized,      2, 2,    VertexFormatKind::Snorm},
        {VertexFormat::Byte2Normalized,     2, 2,   VertexFormatKind::Snorm},
        {VertexFormat::Byte4Normalized,     4, 4,   VertexFormatKind::Snorm},

        {VertexFormat::UShort,              2, 1,   VertexFormatKind::Uint},
        {VertexFormat::UShort2,             4, 2,   VertexFormatKind::Uint},
        {VertexFormat::UShort4,             8, 4,   VertexFormatKind::Uint},
        {VertexFormat::Short,               4, 1,   VertexFormatKind::Sint},
        {VertexFormat::Short2,              4, 2,   VertexFormatKind::Sint},
        {VertexFormat::Short4,              8, 4,   VertexFormatKind::Sint},
        {VertexFormat::UShortNormalized,    2, 1,   VertexFormatKind::Unorm},
        {VertexFormat::UShort2Normalized,   4, 2,   VertexFormatKind::Unorm},
        {VertexFormat::UShort4Normalized,   8, 4,   VertexFormatKind::Unorm},
        {VertexFormat::ShortNormalized,     2, 1,   VertexFormatKind::Snorm},
        {VertexFormat::Short2Normalized,    4, 2,   VertexFormatKind::Snorm},
        {VertexFormat::Short4Normalized,    8, 4,   VertexFormatKind::Snorm},

        {VertexFormat::Half,                2, 1,   VertexFormatKind::Float},
        {VertexFormat::Half2,               4, 2,   VertexFormatKind::Float},
        {VertexFormat::Half4,               8, 4,   VertexFormatKind::Float},
        {VertexFormat::Float,               4, 1,   VertexFormatKind::Float},
        {VertexFormat::Float2,              8, 2,   VertexFormatKind::Float},
        {VertexFormat::Float3,              12, 3,  VertexFormatKind::Float},
        {VertexFormat::Float4,              16, 4,  VertexFormatKind::Float},

        {VertexFormat::UInt,                4, 1,   VertexFormatKind::Uint},
        {VertexFormat::UInt2,               8, 2,   VertexFormatKind::Uint},
        {VertexFormat::UInt3,               12, 3,  VertexFormatKind::Uint},
        {VertexFormat::UInt4,               16, 4,  VertexFormatKind::Uint},

        {VertexFormat::Int,                 4, 1,   VertexFormatKind::Sint},
        {VertexFormat::Int2,                8, 2,   VertexFormatKind::Sint},
        {VertexFormat::Int3,                12, 3,  VertexFormatKind::Sint},
        {VertexFormat::Int4,                16, 4,  VertexFormatKind::Sint},

        {VertexFormat::UInt1010102Normalized, 4, 4, VertexFormatKind::Uint},
        //{VertexFormat::RG11B10Float,   32, 4,  VertexFormatKind::Float},
        //{VertexFormat::RGB9E5Float,   32, 4, VertexFormatKind::Float},
    };

    static_assert(
        sizeof(s_VertexFormatTable) / sizeof(VertexFormatInfo) == size_t(VertexFormat::Count),
        "The format info table doesn't have the right number of elements"
        );

    const VertexFormatInfo& GetVertexFormatInfo(VertexFormat format)
    {
        ALIMER_ASSERT(format != VertexFormat::Undefined);
        ALIMER_ASSERT(ecast(format) < ecast(VertexFormat::Count));
        ALIMER_ASSERT(s_VertexFormatTable[ecast(format)].format == format);

        return s_VertexFormatTable[ecast(format)];
    }

    RHIShaderModuleRef RHILoadShader(ShaderStages stage, const char* fileName, const std::vector<ShaderMacro>* pDefines)
    {
        bool dxil = false;
        std::string shaderExt = ".bin";
        if (GRHIDevice->GetGraphicsAPI() == GraphicsAPI::D3D12)
        {
            dxil = true;
        }

        const char* stageName = GetEntryPointName(stage);
        std::vector<ShaderMake::ShaderConstant> constants;
        if (pDefines)
        {
            for (const ShaderMacro& define : *pDefines)
            {
                constants.push_back(ShaderMake::ShaderConstant{ define.name.c_str(), define.definition.c_str() });
            }
        }

        std::string compiledShadersPath = GetCompiledShadersPath(dxil);
        std::string shaderFileName = compiledShadersPath + "/" + fileName + "_" + stageName + shaderExt;
        Vector<uint8_t> byteCode = File::ReadAllBytes(shaderFileName);
        ALIMER_ASSERT(byteCode.size() > 0);

        const void* permutationBytecode = nullptr;
        size_t permutationSize = 0;
        if (!ShaderMake::FindPermutationInBlob(byteCode.data(), byteCode.size(), constants.data(), uint32_t(constants.size()), &permutationBytecode, &permutationSize))
        {
            const std::string message = ShaderMake::FormatShaderNotFoundMessage(byteCode.data(), byteCode.size(), constants.data(), uint32_t(constants.size()));
            LOGE("{}", message);

            return {};
        }

        ShaderModuleDesc desc{};
        desc.label = shaderFileName.c_str();
        desc.bytecodeSize = permutationSize;
        desc.bytecode = permutationBytecode;

        RHIShaderModuleRef shaderModule = RHICreateShaderModule(desc);
        return shaderModule;
    }
}
