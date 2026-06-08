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

        static const char* GetEntryPointName(RHIShaderStages stage)
        {
            switch (stage)
            {
                case RHIShaderStages::Vertex:
                    return "vertexMain";
                case RHIShaderStages::Fragment:
                    return "fragmentMain";
                case RHIShaderStages::Compute:
                    return "computeMain";
                case RHIShaderStages::Mesh:
                    return "meshMain";
                case RHIShaderStages::Amplification:
                    return "amplificationMain";
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

    /* RHITexture */
    RHITexture::RHITexture(const RHITextureDesc& desc, RHITextureLayout initialLayout)
        : dimension(desc.dimension)
        , format(desc.format)
        , width(desc.width)
        , height(desc.height)
        , depthOrArrayLayers(desc.depthOrArrayLayers)
        , mipLevelCount(desc.mipLevelCount)
        , sampleCount(desc.sampleCount)
        , usage(desc.usage)
    {
        const uint32_t numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
        subresourceLayouts.resize(numSubResources);
        for (uint32_t i = 0; i < numSubResources; i++)
        {
            subresourceLayouts[i] = initialLayout;
        }
    }

    RHITextureView* RHITexture::GetView(const RHITextureViewDesc* desc) const
    {
        if (!CheckBitsAny(usage, RHITextureUsage::ShaderRead | RHITextureUsage::ShaderWrite | RHITextureUsage::RenderTarget))
        {
            LOGE("Cannot create TextureView for texture without ShaderRead, ShaderWrite or RenderTarget usage");
        }

        RHITextureViewDesc creationDesc = {};
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

        const uint32_t textureArrayLayerCount = GetArrayLayers() * (dimension == RHITextureDimension::TextureCube ? 6 : 1);
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

    uint32_t RHITexture::GetSubresourceIndex(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice) const
    {
        return mipLevel + arrayLayer * mipLevelCount + planeSlice * mipLevelCount * depthOrArrayLayers;
    }

    RHITextureLayout RHITexture::GetLayout(uint32_t mipLevel, uint32_t arrayLayer, uint32_t placeSlice) const
    {
        ALIMER_ASSERT(mipLevel < mipLevelCount);
        ALIMER_ASSERT(arrayLayer < depthOrArrayLayers);

        uint32_t subresource = GetSubresourceIndex(mipLevel, arrayLayer, placeSlice);
        return subresourceLayouts[subresource];
    }

    RHITextureLayout RHITexture::GetLayout(uint32_t subresource) const
    {
        ALIMER_ASSERT(subresource < subresourceLayouts.size());

        return subresourceLayouts[subresource];
    }

    void RHITexture::SetLayout(RHITextureLayout newLayout) const
    {
        for (size_t i = 0; i < subresourceLayouts.size(); i++)
        {
            subresourceLayouts[i] = newLayout;
        }
    }

    void RHITexture::SetLayout(uint32_t subresource, RHITextureLayout newLayout) const
    {
        ALIMER_ASSERT(subresource < subresourceLayouts.size());

        subresourceLayouts[subresource] = newLayout;
    }

    void RHITexture::SetLayout(RHITextureLayout newLayout, uint32_t mipLevel, uint32_t arrayLayer, uint32_t placeSlice) const
    {
        ALIMER_ASSERT(mipLevel < mipLevelCount);
        ALIMER_ASSERT(arrayLayer < depthOrArrayLayers);

        const uint32_t subresource = GetSubresourceIndex(mipLevel, arrayLayer, placeSlice);
        subresourceLayouts[subresource] = newLayout;
    }

    void RHITexture::SetLayout(RHITextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
    {
        for (uint32_t arrayLayer = baseArrayLayer; arrayLayer < (baseArrayLayer + layerCount); arrayLayer++)
        {
            for (uint32_t mipLevel = baseMiplevel; mipLevel < (baseMiplevel + levelCount); mipLevel++)
            {
                uint32_t subresource = GetSubresourceIndex(mipLevel, arrayLayer);
                subresourceLayouts[subresource] = newLayout;
            }
        }
    }

    /* RHITextureView */
    uint32_t RHITextureView::GetSubresourceIndex(uint32_t planeSlice) const
    {
        return texture->GetSubresourceIndex(baseMipLevel, baseArrayLayer, planeSlice);
    }

    /* RHISurfaceSource */
    RHISurfaceSource::RHISurfaceSource(Type type_)
        : type(type_)
    {

    }

    RHISurfaceSourceRef RHISurfaceSource::CreateWin32(void* hwnd)
    {
#if defined(_WIN32)
        if (!IsWindow(static_cast<HWND>(hwnd)))
        {
            LOGE("Cannot create surface source from invalid window handle");
            return nullptr;
        }
#endif

        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::WindowsHWND));
        surface->hwnd = hwnd;
        return surface;
    }

    RHISurfaceSourceRef RHISurfaceSource::CreateSwapChainPanel(void* swapChainPanel)
    {
        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::SwapChainPanel));
        surface->idCompositionVisualOrSwapChainPanel = swapChainPanel;
        return surface;
    }

    RHISurfaceSourceRef RHISurfaceSource::CreateAndroid(void* window)
    {
        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::AndroidWindow));
        surface->androidWindow = window;
        return surface;
    }

    RHISurfaceSourceRef RHISurfaceSource::CreateWayland(void* waylandDisplay, void* waylandSurface)
    {
        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::WaylandSurface));
        surface->waylandDisplay = waylandDisplay;
        surface->waylandSurface = waylandSurface;
        return surface;
    }

    RHISurfaceSourceRef RHISurfaceSource::CreateXlib(void* display, uint64_t window)
    {
        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::XlibWindow));
        surface->xDisplay = display;
        surface->xWindow = window;
        return surface;
    }

    RHISurfaceSourceRef RHISurfaceSource::CreateMetalLayer(void* layer)
    {
        RHISurfaceSourceRef surface(new RHISurfaceSource(Type::MetalLayer));
        surface->metalLayer = layer;
        return surface;
    }

    void* RHISurfaceSource::GetHWND() const
    {
        ALIMER_ASSERT(type == Type::WindowsHWND);

        return hwnd;
    }

    void* RHISurfaceSource::GetSwapChainPanel() const
    {
        ALIMER_ASSERT(type == Type::SwapChainPanel);

        return idCompositionVisualOrSwapChainPanel;
    }

    void* RHISurfaceSource::GetAndroidWindow() const
    {
        ALIMER_ASSERT(type == Type::AndroidWindow);

        return androidWindow;
    }

    void* RHISurfaceSource::GetWaylandDisplay() const
    {
        ALIMER_ASSERT(type == Type::WaylandSurface);

        return waylandDisplay;
    }

    void* RHISurfaceSource::GetWaylandSurface() const
    {
        ALIMER_ASSERT(type == Type::WaylandSurface);

        return waylandSurface;
    }

    void* RHISurfaceSource::GetXDisplay() const
    {
        ALIMER_ASSERT(type == Type::XlibWindow);

        return xDisplay;
    }

    uint64_t RHISurfaceSource::GetXWindow() const
    {
        ALIMER_ASSERT(type == Type::XlibWindow);

        return xWindow;
    }

    void* RHISurfaceSource::GetMetalLayer() const
    {
        ALIMER_ASSERT(type == Type::MetalLayer);

        return metalLayer;
    }

    /* RHISurface */
    RHISurface::RHISurface(RHISurfaceSource* source_)
        : source(source_)
    {

    }

    RHISurface::~RHISurface()
    {
        configured = false;
    }

    void RHISurface::Configure(RHIDevice* device, const RHISurfaceConfig& config)
    {
        if (configured)
        {
            Unconfigure();
        }

        width = Max(config.width, 1u);
        height = Max(config.height, 1u);
        format = config.format;
        alphaMode = config.alphaMode;
        presentMode = config.presentMode;
        ConfigureCore(device);
        configured = true;
    }

    void RHISurface::Unconfigure()
    {
        if (!configured)
            return;

        width = 0;
        height = 0;
        format = PixelFormat::Undefined;
        alphaMode = RHICompositeAlphaMode::Auto;
        presentMode = RHIPresentMode::Fifo;
        UnconfigureCore();
        configured = false;
    }

    void RHISurface::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        if (width == newWidth && height == newHeight)
            return;

        width = Max(newWidth, 1u);
        height = Max(newHeight, 1u);
        ResizeCore();
    }

    /* CommandEncoder */
    void RHICommandEncoder::SetConstantBuffer(uint32_t slot, RHIBuffer* buffer, uint64_t offset)
    {
        ALIMER_ASSERT(slot < kDynamicConstantBufferCount);

        if (table.CBV[slot] != buffer)
        {
            table.CBV[slot].Reset(buffer);
            //binder.dirty |= DescriptorBinder::DIRTY_DESCRIPTOR;
        }

        if (table.CBV_offset[slot] != offset)
        {
            table.CBV_offset[slot] = offset;
        }
    }

    void RHICommandEncoder::SetPushConstants(const void* data, uint32_t size, uint32_t offset)
    {
        if (size > kMaxPushConstantsSize)
        {
            LOGF("Push constant limit of {} exceeded (pushing {} bytes)", kMaxPushConstantsSize, size);
            return;
        }

        SetPushConstantsCore(data, size, offset);
    }

    /* RHIComputePassEncoder */
    GPUAllocation RHIComputePassEncoder::AllocateGPU(uint64_t size)
    {
        if (size == 0)
        {
            return {};
        }

        RHIDevice* device = GetCommandBuffer()->GetDevice();
        RHILinearAllocator& allocator = device->GetFrameAllocator();
        const uint64_t alignment = Max(device->GetLimits().minConstantBufferOffsetAlignment, device->GetLimits().minStorageBufferOffsetAlignment);

        const uint64_t bufferSize = (allocator.buffer == nullptr) ? 0 : allocator.buffer->GetSize();
        const uint64_t freeSpace = bufferSize - allocator.offset;
        if (size > freeSpace)
        {
            RHIBufferDesc bufferDesc;
            bufferDesc.size = AlignUp((bufferSize + size) * 2, alignment);
            bufferDesc.usage = RHIBufferUsage::Vertex | RHIBufferUsage::Index | RHIBufferUsage::Constant | RHIBufferUsage::ShaderRead;
            bufferDesc.memoryType = RHIMemoryType::Upload;

            allocator.buffer = device->CreateBuffer(bufferDesc);
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

    void RHIComputePassEncoder::UploadBufferData(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size)
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

    void RHIComputePassEncoder::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    void RHIComputePassEncoder::Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), 1u, 1u);
    }

    void RHIComputePassEncoder::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), DivideByMultiple(threadCountY, groupSizeX), 1u);
    }

    void RHIComputePassEncoder::Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeY),
            DivideByMultiple(threadCountZ, groupSizeZ)
        );
    }

    void RHIComputePassEncoder::DispatchIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
#if defined(_DEBUG)
        if (!CheckBitsAny(indirectBuffer->GetUsage(), RHIBufferUsage::Indirect))
        {
            LOGE("DispatchIndirect: indirectBuffer parameter must have been created with Indirect buffer usage");
            return;
        }

        if ((indirectBufferOffset % 4) != 0)
        {
            LOGE("DispatchIndirect: indirectBufferOffset must be a multiple of 4.");
            return;
        }
#endif

        DispatchIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    /* RHICommandBuffer */
    void RHICommandBuffer::Reset(uint32_t frameIndex)
    {
        _frameIndex = frameIndex;
        _encoderActive = false;
    }

    void RHICommandBuffer::SetTextureLayout(const RHITexture* texture, uint32_t subresource, RHITextureLayout newLayout) const
    {
        texture->SetLayout(subresource, newLayout);
    }

    void RHICommandBuffer::SetTextureLayout(const RHITexture* texture, RHITextureLayout newLayout, uint32_t baseMiplevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
    {
        texture->SetLayout(newLayout, baseMiplevel, levelCount, baseArrayLayer, layerCount);
    }

    RHIComputePassEncoder* RHICommandBuffer::BeginComputePass(const RHIComputePassDesc& desc)
    {
        if (_encoderActive)
        {
            LOGE("Cannot begin compute pass while another CommandEncoder already active");
            return nullptr;
        }

        RHIComputePassEncoder* computePassEncoder = BeginComputePassCore(desc);
        _encoderActive = true;
        return computePassEncoder;
    }

    RHIRenderPassEncoder* RHICommandBuffer::BeginRenderPass(const RHIRenderPassDesc& desc)
    {
        if (_encoderActive)
        {
            LOGE("Cannot begin render pass while another CommandEncoder already active");
            return nullptr;
        }

        RHIRenderPassEncoder* renderPassEncoder = BeginRenderPassCore(desc);
        _encoderActive = true;
        return renderPassEncoder;
    }

#if defined(ALIMER_RHI_VULKAN)
    extern bool Vulkan_IsSupported();
    extern RHIFactoryRef Vulkan_CreateFactory(const RHIFactoryDesc& desc);
#endif

#if defined(ALIMER_RHI_D3D12)
    extern bool D3D12_IsSupported();
    extern RHIFactoryRef D3D12_CreateFactory(const RHIFactoryDesc& desc);
#endif

#if defined(ALIMER_RHI_METAL)
    extern bool Metal_IsSupported();
    extern RHIFactoryRef Metal_CreateFactory(const RHIFactoryDesc& desc);
#endif

    RHIBufferRef RHIDevice::CreateBuffer(const RHIBufferDesc& desc, const void* initialData)
    {
        if (desc.size > _limits.maxBufferSize)
        {
            LOGE("Buffer size too large: {}, limit: {}", desc.size, _limits.maxBufferSize);
            return nullptr;
        }

        RHINativeHandle handle(nullptr);
        return CreateBufferCore(desc, handle, initialData);
    }

    void RHIDevice::InitResources()
    {
        RHISamplerDesc samplerDesc{};

        // SamplerPointClamp
        samplerDesc.label = "SamplerPointClamp";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.mipFilter = RHISamplerMipFilter::Point;
        samplerDesc.addressModeU = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeV = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeW = RHISamplerAddressMode::Clamp;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerPointWrap
        samplerDesc.label = "SamplerPointWrap";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.mipFilter = RHISamplerMipFilter::Point;
        samplerDesc.addressModeU = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeV = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeW = RHISamplerAddressMode::Wrap;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerPointMirror
        samplerDesc.label = "SamplerPointMirror";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Point;
        samplerDesc.mipFilter = RHISamplerMipFilter::Point;
        samplerDesc.addressModeU = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeV = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeW = RHISamplerAddressMode::Mirror;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearClamp
        samplerDesc.label = "SamplerLinearClamp";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeV = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeW = RHISamplerAddressMode::Clamp;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearWrap
        samplerDesc.label = "SamplerLinearWrap";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeV = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeW = RHISamplerAddressMode::Wrap;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerLinearMirror
        samplerDesc.label = "SamplerLinearMirror";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeV = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeW = RHISamplerAddressMode::Mirror;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicClamp
        samplerDesc.label = "SamplerAnisotropicClamp";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeV = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeW = RHISamplerAddressMode::Clamp;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicWrap
        samplerDesc.label = "SamplerAnisotropicWrap";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeV = RHISamplerAddressMode::Wrap;
        samplerDesc.addressModeW = RHISamplerAddressMode::Wrap;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerAnisotropicMirror
        samplerDesc.label = "SamplerAnisotropicMirror";
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Linear;
        samplerDesc.addressModeU = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeV = RHISamplerAddressMode::Mirror;
        samplerDesc.addressModeW = RHISamplerAddressMode::Mirror;
        samplerDesc.maxAnisotropy = 16;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));

        // SamplerComparisonDepth
        samplerDesc.label = "SamplerComparisonDepth";
        samplerDesc.reductionType = RHISamplerReductionType::Comparison;
        samplerDesc.minFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.magFilter = RHISamplerMinMagFilter::Linear;
        samplerDesc.mipFilter = RHISamplerMipFilter::Point;
        samplerDesc.addressModeU = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeV = RHISamplerAddressMode::Clamp;
        samplerDesc.addressModeW = RHISamplerAddressMode::Clamp;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.compareFunction = RHICompareFunction::GreaterEqual;
        samplerDesc.lodMinClamp = 0;
        samplerDesc.lodMaxClamp = 0;
        staticSamplers.push_back(CreateSamplerCore(samplerDesc));
    }

    bool RHIDevice::ValidateTextureDesc(const RHITextureDesc& desc)
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

        if ((desc.dimension == RHITextureDimension::Texture1D || desc.dimension == RHITextureDimension::Texture3D)
            && desc.sampleCount != RHITextureSampleCount::Count1)
        {
            LOGE("1D and 3D Textures must use TextureSampleCount.Count1.");
            return false;
        }

        return true;
    }

    RHITextureRef RHIDevice::CreateTexture(const RHITextureDesc& desc, const RHITextureData* initialData)
    {
        if (!ValidateTextureDesc(desc))
        {
            return nullptr;
        }

        RHITextureDesc creationDesc = desc;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(desc.width, desc.height, desc.depthOrArrayLayers);
        }

        return CreateTextureCore(creationDesc, nullptr, initialData);
    }

    RHITextureRef RHIDevice::CreateTextureFromNativeHandle(RHINativeHandle handle, const RHITextureDesc& desc)
    {
        if (!ValidateTextureDesc(desc))
        {
            return nullptr;
        }

        RHITextureDesc creationDesc = desc;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(desc.width, desc.height, desc.depthOrArrayLayers);
        }

        return CreateTextureCore(creationDesc, handle, nullptr);
    }

    RHISamplerRef RHIDevice::CreateSampler(const RHISamplerDesc& desc)
    {
        return CreateSamplerCore(desc);
    }

    RHIShaderModuleRef RHIDevice::CreateShaderModule(const RHIShaderModuleDesc& desc)
    {
        if (desc.stage == RHIShaderStages::None)
        {
            LOGE("Invalid shader module stage");
            return nullptr;
        }

        if (desc.byteCodeSize == 0 || desc.byteCode == nullptr)
        {
            LOGE("Invalid shader module byteCode");
            return nullptr;
        }

        if (!desc.entryPoint || strlen(desc.entryPoint) == 0)
        {
            LOGE("Invalid shader module entryPoint");
            return nullptr;
        }

        return CreateShaderModuleCore(desc);
    }

    RHIComputePipelineRef RHIDevice::CreateComputePipeline(const RHIComputePipelineDesc& desc)
    {
        ALIMER_ASSERT(desc.shader != nullptr);

        return CreateComputePipelineCore(desc);
    }

    RHIRenderPipelineRef RHIDevice::CreateRenderPipeline(const RHIRenderPipelineDesc& desc)
    {
        // Vertex shader is necessary when not using mesh shaders
        if (desc.vertexShader == nullptr)
        {
            if (desc.meshShader == nullptr)
            {
                LOGE("Mesh shader is required when creating mesh pipeline");
                return nullptr;
            }
        }

        if (desc.vertexBufferLayoutCount > 0)
        {
            for (uint32_t bufferIndex = 0; bufferIndex < desc.vertexBufferLayoutCount; ++bufferIndex)
            {
                const RHIVertexBufferLayout& vertexBufferLayout = desc.vertexBufferLayouts[bufferIndex];
                for (uint32_t attributeIndex = 0; attributeIndex < vertexBufferLayout.attributeCount; ++attributeIndex)
                {
                    const RHIVertexAttribute& vertexAttribute = vertexBufferLayout.attributes[attributeIndex];
                    ALIMER_ASSERT(vertexAttribute.format != RHIVertexAttributeFormat::Undefined);
                }
            }
        }

        return CreateRenderPipelineCore(desc);
    }

    RHIQueryHeapRef RHIDevice::CreateQueryHeap(const RHIQueryHeapDesc& desc)
    {
        ALIMER_ASSERT(desc.count > 0 && desc.count < kQuerySetMaxQueries);

        if (desc.type == RHIQueryType::PipelineStatistics
            && !QueryFeatureSupport(RHIFeature::PipelineStatisticsQuery))
        {
            LOGE("PipelineStatistics queries are not supported");
            return nullptr;
        }

        return CreateQueryHeapCore(desc);
    }

    /* RHIFactory */
    uint32_t RHIFactory::GetAdapterCount() const
    {
        return (uint32_t)_adapters.size();
    }

    RHIAdapter* RHIFactory::GetAdapter(uint32_t index) const
    {
        ALIMER_ASSERT(index < _adapters.size());
        return _adapters[index];
    }

    RHIAdapter* RHIFactory::GetBestAdapter() const
    {
        RHIAdapter* adapter = nullptr;
        uint32_t kind = (uint32_t)RHIAdapterType::Other + 1;
        for (size_t i = 0, count = _adapters.size(); i < count; ++i)
        {
            RHIAdapter* item = _adapters[i];
            RHIAdapterType type = item->GetProperties().type;

            if ((uint32_t)type < kind)
            {
                adapter = item;
                kind = (uint32_t)type;
            }
        }

        return adapter;
    }

    /* Global methods */
    bool IsBackendSupported(RHIBackend backend)
    {
        switch (backend)
        {
#if defined(ALIMER_RHI_D3D12)
            case RHIBackend::D3D12:
                return D3D12_IsSupported();
#endif

#if defined(ALIMER_RHI_VULKAN)
            case RHIBackend::Vulkan:
                return Vulkan_IsSupported();
#endif

#if defined(ALIMER_RHI_METAL)
            case RHIBackend::Metal:
                return false;
#endif

            default:
                return false;
        }
    }

    RHIBackend GetPlatformPreferredBackend()
    {
#if defined(ALIMER_RHI_D3D12)
        if (D3D12_IsSupported())
        {
            return RHIBackend::D3D12;
        }
#endif
#if defined(ALIMER_RHI_VULKAN)
        if (Vulkan_IsSupported())
        {
            return RHIBackend::Vulkan;
        }
#endif
#if defined(ALIMER_RHI_METAL)
        if (Metal_IsSupported())
        {
            return RHIBackend::Metal;
        }
#endif

        return RHIBackend::Null;
    }

    RHIFactoryRef RHICreateFactory(const RHIFactoryDesc& desc)
    {
        RHIBackend backend = desc.preferredBackend;
        if (backend == RHIBackend::Count)
        {
            backend = GetPlatformPreferredBackend();
        }

        RHIFactoryRef factory;
        switch (backend)
        {
#if defined(ALIMER_RHI_D3D12)
            case RHIBackend::D3D12:
                if (D3D12_IsSupported())
                {
                    factory = D3D12_CreateFactory(desc);
                }
                else
                {
                    LOGF("Direct3D12 is not supported on current OS");
                    return nullptr;
                }
                break;
#endif

#if defined(ALIMER_RHI_VULKAN)
            case RHIBackend::Vulkan:
                if (Vulkan_IsSupported())
                {
                    factory = Vulkan_CreateFactory(desc);
                }
                else
                {
                    LOGF("Vulkan is not supported on current OS");
                    return nullptr;
                }
                break;
#endif

            default:
                ALIMER_UNREACHABLE();
                break;
        }

        return factory;
    }

    RHIBufferRef RHICreateBuffer(RHIDevice* device, uint64_t size, RHIBufferUsage usage, RHIMemoryType memoryType, const void* initialData, const char* label)
    {
        ALIMER_ASSERT(device);

        const RHIBufferDesc bufferDesc{
            .label = label,
            .size = size,
            .usage = usage,
            .memoryType = memoryType
        };
        return device->CreateBuffer(bufferDesc, initialData);
    }

    RHIBufferRef RHICreateBuffer(RHIDevice* device, const RHIBufferDesc& desc, const void* initialData)
    {
        ALIMER_ASSERT(device);

        return device->CreateBuffer(desc, initialData);
    }

    const std::string ToString(RHIBackend type)
    {
        switch (type)
        {
            case RHIBackend::Null:      return "Null";
            case RHIBackend::Vulkan:    return "Vulkan";
            case RHIBackend::D3D12:     return "D3D12";
            case RHIBackend::Metal:     return "Metal";
            default:                    return "<UNKNOWN>";
        }
    }

    const std::string ToString(RHIAdapterType type)
    {
        switch (type)
        {
            case RHIAdapterType::IntegratedGpu:     return "IntegratedGpu";
            case RHIAdapterType::DiscreteGpu:       return "DiscreteGpu";
            case RHIAdapterType::VirtualGpu:        return "VirtualGpu";
            case RHIAdapterType::Cpu:               return "Cpu";
            default:                                return "Other";
        }
    }

    RHIAdapterVendor VendorIDToAdapterVendor(uint32_t vendorID)
    {
        switch (vendorID)
        {
            case (uint32_t)KnownGPUAdapterVendor::AMD:
                return RHIAdapterVendor::AMD;
            case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
                return RHIAdapterVendor::NVIDIA;
            case (uint32_t)KnownGPUAdapterVendor::INTEL:
                return RHIAdapterVendor::Intel;
            case (uint32_t)KnownGPUAdapterVendor::ARM:
                return RHIAdapterVendor::ARM;
            case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
                return RHIAdapterVendor::Qualcomm;
            case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
                return RHIAdapterVendor::ImgTech;
            case (uint32_t)KnownGPUAdapterVendor::MSFT:
                return RHIAdapterVendor::MSFT;
            case (uint32_t)KnownGPUAdapterVendor::APPLE:
                return RHIAdapterVendor::Apple;
            case (uint32_t)KnownGPUAdapterVendor::MESA:
                return RHIAdapterVendor::Mesa;
            case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
                return RHIAdapterVendor::Broadcom;

            default:
                return RHIAdapterVendor::Unknown;
        }
    }

    uint32_t AdapterVendorToVendorID(RHIAdapterVendor vendor)
    {
        switch (vendor)
        {
            case RHIAdapterVendor::AMD:
                return (uint32_t)KnownGPUAdapterVendor::AMD;
            case RHIAdapterVendor::NVIDIA:
                return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
            case RHIAdapterVendor::Intel:
                return (uint32_t)KnownGPUAdapterVendor::INTEL;
            case RHIAdapterVendor::ARM:
                return (uint32_t)KnownGPUAdapterVendor::ARM;
            case RHIAdapterVendor::Qualcomm:
                return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
            case RHIAdapterVendor::ImgTech:
                return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
            case RHIAdapterVendor::MSFT:
                return (uint32_t)KnownGPUAdapterVendor::MSFT;
            case RHIAdapterVendor::Apple:
                return (uint32_t)KnownGPUAdapterVendor::APPLE;
            case RHIAdapterVendor::Mesa:
                return (uint32_t)KnownGPUAdapterVendor::MESA;
            case RHIAdapterVendor::Broadcom:
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

    bool BlendEnabled(const RHIRenderTargetBlendState* state)
    {
        return
            state->colorBlendOp != RHIBlendOperation::Add ||
            state->destColorBlendFactor != RHIBlendFactor::Zero ||
            state->srcColorBlendFactor != RHIBlendFactor::One ||
            state->alphaBlendOp != RHIBlendOperation::Add ||
            state->destAlphaBlendFactor != RHIBlendFactor::Zero ||
            state->srcAlphaBlendFactor != RHIBlendFactor::One
            ;
    }

    bool StencilTestEnabled(const RHIDepthStencilState* depthStencil)
    {
        return
            depthStencil->backFace.compareFunc != RHICompareFunction::Always ||
            depthStencil->backFace.failOp != RHIStencilOperation::Keep ||
            depthStencil->backFace.depthFailOp != RHIStencilOperation::Keep ||
            depthStencil->backFace.passOp != RHIStencilOperation::Keep ||
            depthStencil->frontFace.compareFunc != RHICompareFunction::Always ||
            depthStencil->frontFace.failOp != RHIStencilOperation::Keep ||
            depthStencil->frontFace.depthFailOp != RHIStencilOperation::Keep ||
            depthStencil->frontFace.passOp != RHIStencilOperation::Keep;
    }

    static const RHIVertexAttributeFormatInfo s_VertexFormatTable[] = {
        { RHIVertexAttributeFormat::Undefined,          0, 0,   RHIVertexFormatKind::Float },
        { RHIVertexAttributeFormat::Uint8,              1, 1,   RHIVertexFormatKind::Uint },
        { RHIVertexAttributeFormat::Uint8x2,            2, 2,   RHIVertexFormatKind::Uint },
        { RHIVertexAttributeFormat::Uint8x4,            4, 4,   RHIVertexFormatKind::Uint },
        { RHIVertexAttributeFormat::Sint8,              1, 1,   RHIVertexFormatKind::Sint },
        { RHIVertexAttributeFormat::Sint8x2,            2, 2,   RHIVertexFormatKind::Sint },
        { RHIVertexAttributeFormat::Sint8x4,            4, 4,   RHIVertexFormatKind::Sint },
        { RHIVertexAttributeFormat::Unorm8,             1, 1,   RHIVertexFormatKind::Unorm },
        { RHIVertexAttributeFormat::Unorm8x2,           2, 2,   RHIVertexFormatKind::Unorm },
        { RHIVertexAttributeFormat::Unorm8x4,           4, 4,   RHIVertexFormatKind::Unorm },
        { RHIVertexAttributeFormat::Snorm8,             1, 1,   RHIVertexFormatKind::Snorm },
        { RHIVertexAttributeFormat::Snorm8x2,           2, 2,   RHIVertexFormatKind::Snorm },
        { RHIVertexAttributeFormat::Snorm8x4,           4, 4,   RHIVertexFormatKind::Snorm },

        { RHIVertexAttributeFormat::Uint16,             2, 1,   RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Uint16x2,           4, 2,   RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Uint16x4,           8, 4,   RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Sint8,              4, 1,   RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Sint8x2,            4, 2,   RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Sint8x4,            8, 4,   RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Unorm16,            2, 1,   RHIVertexFormatKind::Unorm},
        { RHIVertexAttributeFormat::Unorm16x2,          4, 2,   RHIVertexFormatKind::Unorm},
        { RHIVertexAttributeFormat::Unorm16x4,          8, 4,   RHIVertexFormatKind::Unorm},
        { RHIVertexAttributeFormat::Snorm16,            2, 1,   RHIVertexFormatKind::Snorm},
        { RHIVertexAttributeFormat::Snorm16x2,          4, 2,   RHIVertexFormatKind::Snorm},
        { RHIVertexAttributeFormat::Snorm16x4,          8, 4,   RHIVertexFormatKind::Snorm},

        { RHIVertexAttributeFormat::Float16,            2, 1,   RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float16x2,          4, 2,   RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float16x4,          8, 4,   RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float32,            4, 1,   RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float32x2,          8, 2,   RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float32x3,          12, 3,  RHIVertexFormatKind::Float},
        { RHIVertexAttributeFormat::Float32x4,          16, 4,  RHIVertexFormatKind::Float},

        { RHIVertexAttributeFormat::Uint32,             4, 1,   RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Uint32x2,           8, 2,   RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Uint32x3,           12, 3,  RHIVertexFormatKind::Uint},
        { RHIVertexAttributeFormat::Uint32x4,           16, 4,  RHIVertexFormatKind::Uint},

        { RHIVertexAttributeFormat::Sint32,             4, 1,   RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Sint32x2,           8, 2,   RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Sint32x3,           12, 3,  RHIVertexFormatKind::Sint},
        { RHIVertexAttributeFormat::Sint32x4,           16, 4,  RHIVertexFormatKind::Sint},

        //new(VertexFormat.Int1010102Normalized,        32, 4, FormatKind.Unorm),
        { RHIVertexAttributeFormat::Unorm10_10_10_2,    4, 4, RHIVertexFormatKind::Unorm},
        { RHIVertexAttributeFormat::Unorm8x4BGRA,       4, 4, RHIVertexFormatKind::Unorm},
        //{VertexFormat::RG11B10Float,                  32, 4,  VertexFormatKind::Float},
        //{VertexFormat::RGB9E5Float,                   32, 4, VertexFormatKind::Float},
    };

    static_assert(
        sizeof(s_VertexFormatTable) / sizeof(RHIVertexAttributeFormatInfo) == size_t(RHIVertexAttributeFormat::Count),
        "The format info table doesn't have the right number of elements"
        );

    const RHIVertexAttributeFormatInfo& GetVertexAttributeFormatInfo(RHIVertexAttributeFormat format)
    {
        ALIMER_ASSERT(format != RHIVertexAttributeFormat::Undefined);
        ALIMER_ASSERT(ecast(format) < ecast(RHIVertexAttributeFormat::Count));
        ALIMER_ASSERT(s_VertexFormatTable[ecast(format)].format == format);

        return s_VertexFormatTable[ecast(format)];
    }

    RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName)
    {
        return RHILoadShader(device, stage, fileName, nullptr, 0);
    }

    RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName, const Vector<RHIShaderMacro>& defines)
    {
        return RHILoadShader(device, stage, fileName, defines.data(), defines.size());
    }

    RHIShaderModuleRef RHILoadShader(RHIDevice* device, RHIShaderStages stage, const char* fileName, const RHIShaderMacro* pDefines, size_t definesCount)
    {
        bool dxil = false;
        std::string shaderExt = ".bin";
        if (device->GetBackend() == RHIBackend::D3D12)
        {
            dxil = true;
        }

        const char* stageName = GetEntryPointName(stage);
        std::vector<ShaderMake::ShaderConstant> constants;
        if (pDefines && definesCount > 0)
        {
            for (size_t i = 0; i < definesCount; ++i)
            {
                const RHIShaderMacro& define = pDefines[i];
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

        const RHIShaderModuleDesc desc{
            .label = shaderFileName.c_str(),
            .stage = stage,
            .byteCodeSize = permutationSize,
            .byteCode = permutationBytecode,
            .entryPoint = stageName,
        };

        RHIShaderModuleRef shaderModule = device->CreateShaderModule(desc);
        return shaderModule;
    }
}
