// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"

#if defined(ALIMER_RHI_VULKAN)
#include "RHI_Vulkan.h"
#endif

#include "Core/Log.h"

namespace Alimer::rhi
{
    /* Helper methods */
    const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case KnownVendorId_AMD:
                return "AMD";
            case KnownVendorId_ImgTec:
                return "IMAGINATION";
            case KnownVendorId_Nvidia:
                return "Nvidia";
            case KnownVendorId_ARM:
                return "ARM";
            case KnownVendorId_Qualcomm:
                return "Qualcom";
            case KnownVendorId_Intel:
                return "Intel";
            default:
                return "Unknown";
        }
    }

    uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth)
    {
        uint32_t numMips = 0;
        uint32_t size = std::max(std::max(width, height), depth);
        while (1u << numMips <= size)
        {
            ++numMips;
        }

        if (1u << numMips < size)
        {
            ++numMips;
        }

        return numMips;
    }

    /* Implementation */
    TextureHandle IDevice::CreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        return CreateTextureCore(desc, nullptr, initialData);
    }

    TextureHandle IDevice::CreateExternalTexture(const void* handle, const TextureDesc& desc)
    {
        assert(handle);

        return CreateTextureCore(desc, handle, nullptr);
    }

    BufferHandle IDevice::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        assert(desc.size > 0);

        static constexpr uint64_t kMaxBufferSize = 128u * 1024u * 1024u;

        if (desc.size > kMaxBufferSize)
        {
            LOGE("Buffer size too large (size {})", desc.size);
            return nullptr;
        }

        return CreateBufferCore(desc, initialData);
    }

    SwapChainHandle IDevice::CreateSwapChain(void* windowHandle, const SwapChainDesc& desc)
    {
        return CreateSwapChainCore(windowHandle, desc);
    }

    DeviceHandle GRHIDevice = nullptr;
}
