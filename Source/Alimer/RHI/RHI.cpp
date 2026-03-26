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

    RHITextureView* RHITexture::GetView(const RHITextureViewDesc* desc) const
    {
        if (!CheckBitsAny(usage, TextureUsage::ShaderRead | TextureUsage::ShaderWrite | TextureUsage::RenderTarget))
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

        const uint32_t textureArrayLayerCount = GetArrayLayers() * (dimension == TextureDimension::TextureCube ? 6 : 1);
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

    /* CommandEncoder */
    void CommandEncoder::SetConstantBuffer(uint32_t slot, RHIBuffer* buffer, uint64_t offset)
    {
        ALIMER_ASSERT(slot < kContantBufferCount);

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

    void CommandEncoder::SetPushConstants(const void* data, uint32_t size, uint32_t offset)
    {
        if (size > kMaxPushConstantsSize)
        {
            LOGF("Push constant limit of {} exceeded (pushing {} bytes)", kMaxPushConstantsSize, size);
            return;
        }

        SetPushConstantsCore(data, size, offset);
    }

    /* ComputePassEncoder */
    GPUAllocation ComputePassEncoder::AllocateGPU(uint64_t size)
    {
        if (size == 0)
        {
            return {};
        }

        RHIDevice* device = GetCommandBuffer()->GetDevice();
        GpuLinearAllocator& allocator = device->GetFrameAllocator();
        const uint64_t alignment = Max(device->GetLimits().minConstantBufferOffsetAlignment, device->GetLimits().minStorageBufferOffsetAlignment);

        const uint64_t bufferSize = (allocator.buffer == nullptr) ? 0 : allocator.buffer->GetSize();
        const uint64_t freeSpace = bufferSize - allocator.offset;
        if (size > freeSpace)
        {
            BufferDesc bufferDesc;
            bufferDesc.size = AlignUp((bufferSize + size) * 2, alignment);
            bufferDesc.usage = BufferUsage::Vertex | BufferUsage::Index | BufferUsage::Constant | BufferUsage::ShaderRead;
            bufferDesc.memoryType = MemoryType::Upload;

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

    void ComputePassEncoder::UploadBufferData(const RHIBuffer* buffer, uint64_t offset, const void* data, uint64_t size)
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

    void ComputePassEncoder::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    void ComputePassEncoder::Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), 1u, 1u);
    }

    void ComputePassEncoder::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), DivideByMultiple(threadCountY, groupSizeX), 1u);
    }

    void ComputePassEncoder::Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeY),
            DivideByMultiple(threadCountZ, groupSizeZ)
        );
    }

    void ComputePassEncoder::DispatchIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset)
    {
#if defined(_DEBUG)
        if (!CheckBitsAny(indirectBuffer->GetUsage(), BufferUsage::Indirect))
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

    /* CommandBuffer */
    void CommandBuffer::Reset(uint32_t frameIndex)
    {
        _frameIndex = frameIndex;
        _encoderActive = false;
    }

    ComputePassEncoder* CommandBuffer::BeginComputePass(const ComputePassDescriptor& descriptor)
    {
        if (_encoderActive)
        {
            LOGE("Cannot begin compute pass while another CommandEncoder already active");
            return nullptr;
        }

        ComputePassEncoder* computePassEncoder = BeginComputePassCore(descriptor);
        _encoderActive = true;
        return computePassEncoder;
    }

    RenderPassEncoder* CommandBuffer::BeginRenderPass(const RenderPassDesc& desc)
    {
        if (_encoderActive)
        {
            LOGE("Cannot begin render pass while another CommandEncoder already active");
            return nullptr;
        }

        RenderPassEncoder* renderPassEncoder = BeginRenderPassCore(desc);
        _encoderActive = true;
        return renderPassEncoder;
    }

#if defined(ALIMER_RHI_VULKAN)
    extern bool Vulkan_IsSupported();
    extern Vector<Adapter*> Vulkan_InitAdapters();
    extern RHIFactoryRef Vulkan_CreateFactory(const RHIFactoryDesc& desc);
#endif

#if defined(ALIMER_RHI_D3D12) && defined(TODO)
    extern bool D3D12_IsSupported();
    extern RHIFactoryRef D3D12_CreateFactory(const FactoryDescriptor& desc);
#endif

#if defined(ALIMER_RHI_METAL)
    extern bool Metal_IsSupported();
    extern RHIFactoryRef Metal_CreateFactory(const FactoryDescriptor& desc);
#endif

    RHIBufferRef RHIDevice::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        if (desc.size > _limits.maxBufferSize)
        {
            LOGE("Buffer size too large: {}, limit: {}", desc.size, _limits.maxBufferSize);
            return nullptr;
        }

        return CreateBufferCore(desc, initialData);
    }

    RHIBufferRef RHIDevice::CreateBuffer(uint64_t size, BufferUsage usage, const void* initialData, const char* label)
    {
        if (size > _limits.maxBufferSize)
        {
            LOGE("Buffer size too large: {}, limit: {}", size, _limits.maxBufferSize);
            return nullptr;
        }

        BufferDesc bufferDesc{};
        bufferDesc.label = label;
        bufferDesc.size = size;
        bufferDesc.usage = usage;
        return CreateBufferCore(bufferDesc, initialData);
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

    bool RHIDevice::ValidateTextureDesc(const TextureDescriptor& desc)
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

        if ((desc.dimension == TextureDimension::Texture1D || desc.dimension == TextureDimension::Texture3D)
            && desc.sampleCount != TextureSampleCount::Count1)
        {
            LOGE("1D and 3D Textures must use TextureSampleCount.Count1.");
            return false;
        }

        return true;
    }

    RHITextureRef RHIDevice::CreateTexture(const TextureDescriptor& descriptor, const TextureData* initialData)
    {
        if (!ValidateTextureDesc(descriptor))
        {
            return nullptr;
        }

        TextureDescriptor creationDesc = descriptor;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(descriptor.width, descriptor.height, descriptor.depthOrArrayLayers);
        }

        return CreateTextureCore(creationDesc, initialData);
    }

    RHITextureRef RHIDevice::CreateTextureFromNativeHandle(NativeHandle handle, const TextureDescriptor& descriptor)
    {
        if (!ValidateTextureDesc(descriptor))
        {
            return nullptr;
        }

        TextureDescriptor creationDesc = descriptor;
        if (creationDesc.mipLevelCount == 0)
        {
            creationDesc.mipLevelCount = GetMipLevelCount(descriptor.width, descriptor.height, descriptor.depthOrArrayLayers);
        }

        return CreateTextureFromNativeHandleCore(handle, creationDesc);
    }

    SamplerRef RHIDevice::CreateSampler(const SamplerDesc& desc)
    {
        return CreateSamplerCore(desc);
    }

    ShaderModuleRef RHIDevice::CreateShaderModule(const ShaderModuleDesc& descriptor)
    {
        if (descriptor.byteCodeSize == 0 || descriptor.byteCode == nullptr)
        {
            LOGE("Invalid shader module byteCode");
            return nullptr;
        }

        return CreateShaderModuleCore(descriptor);
    }

    RenderPipelineRef RHIDevice::CreateRenderPipeline(const RenderPipelineDescriptor& descriptor)
    {
        // Vertex shader is necessary when not using mesh shaders
        if (descriptor.vertexShader == nullptr)
        {
            if (descriptor.meshShader == nullptr)
            {
                LOGE("Mesh shader is required when creating mesh pipeline");
                return nullptr;
            }
        }

        if (descriptor.vertexBufferLayoutCount > 0)
        {
            for (uint32_t bufferIndex = 0; bufferIndex < descriptor.vertexBufferLayoutCount; ++bufferIndex)
            {
                const VertexBufferLayout& vertexBufferLayout = descriptor.vertexBufferLayouts[bufferIndex];
                for (uint32_t attributeIndex = 0; attributeIndex < vertexBufferLayout.attributeCount; ++attributeIndex)
                {
                    const VertexAttribute& vertexAttribute = vertexBufferLayout.attributes[attributeIndex];
                    ALIMER_ASSERT(vertexAttribute.format != VertexAttributeFormat::Undefined);
                }
            }
        }

        return CreateRenderPipelineCore(descriptor);
    }

    ComputePipelineRef RHIDevice::CreateComputePipeline(const ComputePipelineDescriptor& descriptor)
    {
        ALIMER_ASSERT(descriptor.shader != nullptr);

        return CreateComputePipelineCore(descriptor);
    }

    QueryHeapRef RHIDevice::CreateQueryHeap(const QueryHeapDescriptor& descriptor)
    {
        ALIMER_ASSERT(descriptor.count > 0 && descriptor.count < kQuerySetMaxQueries);

        if (descriptor.type == QueryType::PipelineStatistics
            && !QueryFeatureSupport(Feature::PipelineStatisticsQuery))
        {
            LOGE("PipelineStatistics queries are not supported");
            return nullptr;
        }

        return CreateQueryHeapCore(descriptor);
    }

    RHISwapChainRef RHIDevice::CreateSwapChain(RHISurface* surface, const RHISwapChainDesc& desc)
    {
        ALIMER_ASSERT(surface);

        return CreateSwapChainCore(surface, desc);
    }

    /* RHIFactory */
    RHIFactoryRef GRHIFactory = nullptr;
    RHIDeviceRef GRHIDevice = nullptr;

    bool IsBackendSupported(BackendType backend)
    {
        switch (backend)
        {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
            case BackendType::D3D12:
                return D3D12_IsSupported();
#endif

#if defined(ALIMER_RHI_VULKAN)
            case BackendType::Vulkan:
                return Vulkan_IsSupported();
#endif

#if defined(ALIMER_RHI_METAL)
            case BackendType::Metal:
                return false;
#endif

            default:
                return false;
        }
    }

    BackendType GetPlatformPreferredBackend()
    {
#if defined(ALIMER_RHI_D3D12)&& defined(TODO)
        if (D3D12_IsSupported())
        {
            return BackendType::D3D12;
        }
#endif
#if defined(ALIMER_RHI_VULKAN)
        if (Vulkan_IsSupported())
        {
            return BackendType::Vulkan;
        }
#endif
#if defined(ALIMER_RHI_METAL)
        if (Metal_IsSupported())
        {
            return BackendType::Metal;
        }
#endif

        return BackendType::Null;
    }

    bool RHIInit(const RHIFactoryDesc& desc)
    {
        if (GRHIFactory)
            return true;

        BackendType backend = desc.preferredBackend;
        if (backend == BackendType::Count)
        {
            backend = GetPlatformPreferredBackend();
        }

        RHIFactoryRef factory;
        switch (backend)
        {
#if defined(ALIMER_RHI_D3D12) && defined(TODO)
            case RHIBackendType::D3D12:
                if (D3D12_IsSupported())
                {
                    factory = D3D12_CreateDevice(appName, desc);
                }
                else
                {
                    LOGF("Direct3D12 is not supported on current OS");
                    return false;
                }
                break;
#endif

#if defined(ALIMER_RHI_VULKAN)
            case BackendType::Vulkan:
                if (Vulkan_IsSupported())
                {
                    factory = Vulkan_CreateFactory(desc);
                }
                else
                {
                    LOGF("Vulkan is not supported on current OS");
                    return false;
                }
                break;
#endif

            default:
                ALIMER_UNREACHABLE();
                break;
        }

        if (factory == nullptr)
        {
            return false;
        }

        GRHIFactory = factory;
        return true;
    }

    void RHIShutdown()
    {
        GRHIFactory.Reset();
    }

    Adapter* RHIFactory::GetBestAdapter() const
    {
        Adapter* adapter = nullptr;
        uint32_t kind = (uint32_t)AdapterType::Other + 1;
        for (size_t i = 0, count = _adapters.size(); i < count; ++i)
        {
            Adapter* item = _adapters[i];
            AdapterType type = item->GetType();

            if ((uint32_t)type < kind)
            {
                adapter = item;
                kind = (uint32_t)type;
            }
        }

        return adapter;
    }

    uint32_t RHIFactory::GetAdapterCount() const
    {
        return (uint32_t)_adapters.size();
    }

    Adapter* RHIFactory::GetAdapter(uint32_t index) const
    {
        ALIMER_ASSERT(index < _adapters.size());
        return _adapters[index];
    }

    const std::string ToString(BackendType type)
    {
        switch (type)
        {
            case BackendType::Null:         return "Null";
            case BackendType::Vulkan:       return "Vulkan";
            case BackendType::D3D12:        return "D3D12";
            case BackendType::Metal:        return "Metal";
            default:                        return "<UNKNOWN>";
        }
    }

    const std::string ToString(AdapterType type)
    {
        switch (type)
        {
            case AdapterType::IntegratedGpu:     return "IntegratedGpu";
            case AdapterType::DiscreteGpu:       return "DiscreteGpu";
            case AdapterType::VirtualGpu:        return "VirtualGpu";
            case AdapterType::Cpu:               return "Cpu";
            default:                            return "Other";
        }
    }

    AdapterVendor VendorIdToAdapterVendor(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case (uint32_t)KnownGPUAdapterVendor::AMD:
                return AdapterVendor::AMD;
            case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
                return AdapterVendor::NVIDIA;
            case (uint32_t)KnownGPUAdapterVendor::INTEL:
                return AdapterVendor::Intel;
            case (uint32_t)KnownGPUAdapterVendor::ARM:
                return AdapterVendor::ARM;
            case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
                return AdapterVendor::Qualcomm;
            case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
                return AdapterVendor::ImgTech;
            case (uint32_t)KnownGPUAdapterVendor::MSFT:
                return AdapterVendor::MSFT;
            case (uint32_t)KnownGPUAdapterVendor::APPLE:
                return AdapterVendor::Apple;
            case (uint32_t)KnownGPUAdapterVendor::MESA:
                return AdapterVendor::Mesa;
            case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
                return AdapterVendor::Broadcom;

            default:
                return AdapterVendor::Unknown;
        }
    }

    uint32_t AdapterVendorToVendorId(AdapterVendor vendor)
    {
        switch (vendor)
        {
            case AdapterVendor::AMD:
                return (uint32_t)KnownGPUAdapterVendor::AMD;
            case AdapterVendor::NVIDIA:
                return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
            case AdapterVendor::Intel:
                return (uint32_t)KnownGPUAdapterVendor::INTEL;
            case AdapterVendor::ARM:
                return (uint32_t)KnownGPUAdapterVendor::ARM;
            case AdapterVendor::Qualcomm:
                return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
            case AdapterVendor::ImgTech:
                return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
            case AdapterVendor::MSFT:
                return (uint32_t)KnownGPUAdapterVendor::MSFT;
            case AdapterVendor::Apple:
                return (uint32_t)KnownGPUAdapterVendor::APPLE;
            case AdapterVendor::Mesa:
                return (uint32_t)KnownGPUAdapterVendor::MESA;
            case AdapterVendor::Broadcom:
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

    static const VertexAttributeFormatInfo s_VertexFormatTable[] = {
        {VertexAttributeFormat::Undefined,          0, 0,   VertexFormatKind::Float},
        {VertexAttributeFormat::Uint8,              1, 1,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint8x2,            2, 2,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint8x4,            4, 4,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Sint8,              1, 1,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint8x2,            2, 2,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint8x4,            4, 4,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Unorm8,             1, 1,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Unorm8x2,           2, 2,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Unorm8x4,           4, 4,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Snorm8,             1, 1,   VertexFormatKind::Snorm},
        {VertexAttributeFormat::Snorm8x2,           2, 2,   VertexFormatKind::Snorm},
        {VertexAttributeFormat::Snorm8x4,           4, 4,   VertexFormatKind::Snorm},

        {VertexAttributeFormat::Uint16,             2, 1,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint16x2,           4, 2,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint16x4,           8, 4,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Sint8,              4, 1,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint8x2,            4, 2,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint8x4,            8, 4,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Unorm16,            2, 1,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Unorm16x2,          4, 2,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Unorm16x4,          8, 4,   VertexFormatKind::Unorm},
        {VertexAttributeFormat::Snorm16,            2, 1,   VertexFormatKind::Snorm},
        {VertexAttributeFormat::Snorm16x2,          4, 2,   VertexFormatKind::Snorm},
        {VertexAttributeFormat::Snorm16x4,          8, 4,   VertexFormatKind::Snorm},

        {VertexAttributeFormat::Float16,            2, 1,   VertexFormatKind::Float},
        {VertexAttributeFormat::Float16x2,          4, 2,   VertexFormatKind::Float},
        {VertexAttributeFormat::Float16x4,          8, 4,   VertexFormatKind::Float},
        {VertexAttributeFormat::Float32,            4, 1,   VertexFormatKind::Float},
        {VertexAttributeFormat::Float32x2,          8, 2,   VertexFormatKind::Float},
        {VertexAttributeFormat::Float32x3,          12, 3,  VertexFormatKind::Float},
        {VertexAttributeFormat::Float32x4,          16, 4,  VertexFormatKind::Float},

        {VertexAttributeFormat::Uint32,             4, 1,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint32x2,           8, 2,   VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint32x3,           12, 3,  VertexFormatKind::Uint},
        {VertexAttributeFormat::Uint32x4,           16, 4,  VertexFormatKind::Uint},

        {VertexAttributeFormat::Sint32,             4, 1,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint32x2,           8, 2,   VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint32x3,           12, 3,  VertexFormatKind::Sint},
        {VertexAttributeFormat::Sint32x4,           16, 4,  VertexFormatKind::Sint},

        //new(VertexFormat.Int1010102Normalized,    32, 4, FormatKind.Unorm),
        {VertexAttributeFormat::Unorm10_10_10_2, 4, 4, VertexFormatKind::Unorm},
        {VertexAttributeFormat::Unorm8x4BGRA, 4, 4, VertexFormatKind::Unorm},
        //{VertexFormat::RG11B10Float,   32, 4,  VertexFormatKind::Float},
        //{VertexFormat::RGB9E5Float,   32, 4, VertexFormatKind::Float},
    };

    static_assert(
        sizeof(s_VertexFormatTable) / sizeof(VertexAttributeFormatInfo) == size_t(VertexAttributeFormat::Count),
        "The format info table doesn't have the right number of elements"
        );

    const VertexAttributeFormatInfo& GetVertexAttributeFormatInfo(VertexAttributeFormat format)
    {
        ALIMER_ASSERT(format != VertexAttributeFormat::Undefined);
        ALIMER_ASSERT(ecast(format) < ecast(VertexAttributeFormat::Count));
        ALIMER_ASSERT(s_VertexFormatTable[ecast(format)].format == format);

        return s_VertexFormatTable[ecast(format)];
    }

    ShaderModuleRef RHILoadShader(RHIDevice* device, ShaderStages stage, const char* fileName, const Vector<ShaderMacro>* pDefines)
    {
        bool dxil = false;
        std::string shaderExt = ".bin";
        if (device->GetBackend() == BackendType::D3D12)
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
        desc.byteCodeSize = permutationSize;
        desc.byteCode = permutationBytecode;

        ShaderModuleRef shaderModule = device->CreateShaderModule(desc);
        return shaderModule;
    }
}
