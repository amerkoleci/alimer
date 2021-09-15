// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Graphics.h"
#include "directx/d3dx12.h"

namespace Alimer
{
    namespace
    {
        [[nodiscard]] constexpr D3D12_RESOURCE_DIMENSION ToD3D12(TextureType type)
        {
            switch (type)
            {
                case TextureType::Texture1D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

                case TextureType::Texture2D:
                case TextureType::TextureCube:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

                case TextureType::Texture3D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_RESOURCE_DIMENSION_UNKNOWN;
            }
        }

        [[nodiscard]] constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(PixelFormat format)
        {
            switch (format)
            {
                case PixelFormat::Depth16Unorm:
                    return DXGI_FORMAT_R16_TYPELESS;
                case PixelFormat::Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;
                case PixelFormat::Depth24UnormStencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case PixelFormat::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    ALIMER_ASSERT(IsDepthFormat(format) == false);
                    return ToDXGIFormat(format);
            }
        }
    }

    D3D12Texture::D3D12Texture(D3D12Graphics& device, const TextureCreateInfo& info, const void* initialData, ID3D12Resource* existingHandle, PixelFormat viewFormat)
        : Texture(info)
        , device{ device }
        , D3D12GpuResource(existingHandle, D3D12_RESOURCE_STATE_COMMON)
    {
        if (existingHandle == nullptr)
        {
            D3D12MA::ALLOCATION_DESC allocationDesc{};
            allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = ToD3D12(info.type);
            resourceDesc.Alignment = 0;
            resourceDesc.Width = info.width;
            resourceDesc.Height = info.height;
            resourceDesc.MipLevels = info.mipLevels;
            resourceDesc.Format = ToDXGIFormat(info.format);
            resourceDesc.SampleDesc.Count = D3D12SampleCount(info.sampleCount);
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (info.type == TextureType::TextureCube)
            {
                resourceDesc.DepthOrArraySize = info.depthOrArraySize * 6;
            }
            else
            {
                resourceDesc.DepthOrArraySize = info.depthOrArraySize;
            }

            if (CheckBitsAny(usage, TextureUsage::Storage))
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            D3D12_CLEAR_VALUE clearValue = {};
            D3D12_CLEAR_VALUE* pClearValue = nullptr;

            if (CheckBitsAny(usage, TextureUsage::RenderTarget))
            {
                // Render targets and Depth/Stencil targets are always committed resources
                allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;

                clearValue.Format = resourceDesc.Format;

                if (IsDepthStencilFormat(format))
                {
                    state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                    if (!CheckBitsAny(usage, TextureUsage::Sampled))
                    {
                        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                    }

                    clearValue.DepthStencil.Depth = 1.0f;
                }
                else
                {
                    state = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }

                pClearValue = &clearValue;
            }

            // If depth and either ua or sr, set to typeless
            if (IsDepthFormat(format) && CheckBitsAny(usage, (TextureUsage::Sampled | TextureUsage::Storage)))
            {
                resourceDesc.Format = GetTypelessFormatFromDepthFormat(format);
                pClearValue = nullptr;
            }

            HRESULT result = device.GetAllocator()->CreateResource(&allocationDesc,
                &resourceDesc,
                state,
                pClearValue,
                &allocation,
                IID_PPV_ARGS(&handle)
            );

            if (FAILED(result))
            {
                LOGE("Direct3D12: Failed to create texture: {}", std::to_string(result));
                return;
            }

            dxgiFormat = resourceDesc.Format;

            // Upload data.
            if (initialData != nullptr)
            {
                resourceDesc = handle->GetDesc();
                const uint32_t numSubResources = resourceDesc.MipLevels * resourceDesc.DepthOrArraySize;

                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubResources);
                std::vector<UINT64> rowSizesInBytes(numSubResources);
                std::vector<UINT> numRows(numSubResources);
                device.GetHandle()->GetCopyableFootprints(&resourceDesc, 0, numSubResources, 0, layouts.data(), numRows.data(), rowSizesInBytes.data(), &allocatedSize);

                D3D12UploadContext uploadContext = device.ResourceUploadBegin(allocatedSize);
                uint8_t* uploadMemory = reinterpret_cast<uint8_t*>(uploadContext.CPUAddress);

                const uint8_t* srcMem = reinterpret_cast<const uint8_t*>(initialData);
                const uint64_t srcTexelSize = GetFormatBlockSize(info.format);

                for (uint64_t arrayIdx = 0; arrayIdx < resourceDesc.DepthOrArraySize; ++arrayIdx)
                {
                    uint64_t mipWidth = resourceDesc.Width;
                    for (uint64_t mipIdx = 0; mipIdx < resourceDesc.MipLevels; ++mipIdx)
                    {
                        const uint64_t subResourceIdx = mipIdx + (arrayIdx * resourceDesc.MipLevels);

                        const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& subResourceLayout = layouts[subResourceIdx];
                        const uint64_t subResourceHeight = numRows[subResourceIdx];
                        const uint64_t subResourcePitch = subResourceLayout.Footprint.RowPitch;
                        const uint64_t subResourceDepth = subResourceLayout.Footprint.Depth;
                        const uint64_t srcPitch = mipWidth * srcTexelSize;
                        uint8_t* dstSubResourceMem = uploadMemory + subResourceLayout.Offset;

                        for (uint64_t z = 0; z < subResourceDepth; ++z)
                        {
                            for (uint64_t y = 0; y < subResourceHeight; ++y)
                            {
                                memcpy(dstSubResourceMem, srcMem, Min(subResourcePitch, srcPitch));
                                dstSubResourceMem += subResourcePitch;
                                srcMem += srcPitch;
                            }
                        }

                        mipWidth = Max(mipWidth / 2, 1ull);
                    }
                }

                for (uint32_t subResourceIdx = 0; subResourceIdx < numSubResources; ++subResourceIdx)
                {
                    D3D12_TEXTURE_COPY_LOCATION dst = {};
                    dst.pResource = handle;
                    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    dst.SubresourceIndex = subResourceIdx;

                    D3D12_TEXTURE_COPY_LOCATION src = {};
                    src.pResource = uploadContext.resource;
                    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    src.PlacedFootprint = layouts[subResourceIdx];
                    src.PlacedFootprint.Offset += uploadContext.resourceOffset;

                    uploadContext.commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
                }

                device.ResourceUploadEnd(uploadContext);
            }
        }
        else
        {
            handle->AddRef();
            dxgiFormat = handle->GetDesc().Format;
        }

        // Create default view.
        TextureViewCreateInfo viewInfo;
        viewInfo.format = viewFormat;
        const size_t hash = Hash(viewInfo);
        defaultView = CreateView(viewInfo);
        views[hash] = defaultView;

        OnCreated();
    }

    D3D12Texture::~D3D12Texture()
    {
        Destroy();
    }

    void D3D12Texture::Destroy()
    {
        DestroyViews();
        device.DeferDestroy(handle, allocation);
        OnDestroyed();
        D3D12GpuResource::Destroy();
    }

    void D3D12Texture::ApiSetName()
    {
        auto wideName = ToUtf16(name);
        handle->SetName(wideName.c_str());
    }

    TextureView* D3D12Texture::CreateView(const TextureViewCreateInfo& createInfo)
    {
        auto result = new D3D12TextureView(this, createInfo);
        return result;
    }

    D3D12TextureView::D3D12TextureView(_In_ D3D12Texture* texture, const TextureViewCreateInfo& info)
        : TextureView(texture, info)
        , device(texture->GetDevice())
    {
        const TextureUsage usage = texture->GetUsage();
        if (CheckBitsAny(usage, TextureUsage::RenderTarget))
        {
            if (!IsDepthStencilFormat(format))
            {
                rtv = CreateRTV(texture->GetHandle());
            }
            else
            {
                readWriteDSV = CreateDSV(texture->GetHandle(), false, false);
            }
        }

        if (CheckBitsAny(usage, TextureUsage::Sampled))
        {
            srv = device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
            srvDesc.Format = ToDXGIFormat(format);
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = texture->GetMipLevels();
            srvDesc.Texture2D.PlaneSlice = 0;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            device.GetHandle()->CreateShaderResourceView(texture->GetHandle(), &srvDesc, srv);

            // Allocate bindless handle
            auto bindlessHandle = device.AllocateSRV();
            device.GetHandle()->CopyDescriptorsSimple(1, bindlessHandle.handle, srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            bindless_srv = bindlessHandle.index;
        }
    }

    D3D12TextureView::~D3D12TextureView()
    {
        if (rtv.ptr)
        {
            device.FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtv);
        }

        if (readWriteDSV.ptr)
        {
            device.FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, readWriteDSV);
        }

        if (srv.ptr)
        {
            device.FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srv);
        }

        // TODO: Free bindless
        if (bindless_srv != kInvalidBindlessIndex)
        {

        }
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& D3D12TextureView::GetDSV(bool readOnlyDepth, bool readOnlyStencil) const
    {
        if (readOnlyDepth)
        {
            if (readOnlyStencil)
            {
                if (!readOnlyDSV.ptr)
                {
                    readOnlyDSV = CreateDSV(static_cast<D3D12Texture*>(texture)->GetHandle(), true, true);
                }

                return readOnlyDSV;
            }
            else
            {
                if (!readOnlyDepthReadWriteStencilView.ptr)
                {
                    readOnlyDepthReadWriteStencilView = CreateDSV(static_cast<D3D12Texture*>(texture)->GetHandle(), true, false);
                }

                return readOnlyDepthReadWriteStencilView;
            }
        }
        else
        {
            if (readOnlyStencil)
            {
                if (!readWriteDepthReadOnlyView.ptr)
                {
                    readWriteDepthReadOnlyView = CreateDSV(static_cast<D3D12Texture*>(texture)->GetHandle(), false, true);
                }

                return readWriteDepthReadOnlyView;
            }

            return readWriteDSV;
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12TextureView::CreateRTV(ID3D12Resource* resource) const
    {
        uint32_t arrayMultiplier = (texture->GetTextureType() == TextureType::TextureCube) ? 6 : 1;

        D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.Format = ToDXGIFormat(format);

        switch (texture->GetTextureType())
        {
            case TextureType::Texture1D:
                if (texture->GetArraySize() > 1)
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = baseMipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = baseArrayLayer;
                    viewDesc.Texture1DArray.ArraySize = arrayLayerCount;

                }
                else
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = baseMipLevel;
                }
                break;
            case TextureType::Texture2D:
            case TextureType::TextureCube:
                if (texture->GetSampleCount() != SampleCount::Count1)
                {
                    if (texture->GetArraySize() > 1)
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = baseArrayLayer;
                        viewDesc.Texture2DMSArray.ArraySize = arrayLayerCount;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    }
                }
                else
                {
                    if (texture->GetArraySize() * arrayMultiplier > 1)
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = baseMipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = baseArrayLayer * arrayMultiplier;
                        viewDesc.Texture2DArray.ArraySize = arrayLayerCount * arrayMultiplier;
                        viewDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    else
                    {

                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = baseMipLevel;
                        viewDesc.Texture2D.PlaneSlice = 0;
                    }
                }
                break;
            case TextureType::Texture3D:
                viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MipSlice = baseMipLevel;
                viewDesc.Texture3D.FirstWSlice = 0;
                viewDesc.Texture3D.WSize = texture->GetDepth(baseMipLevel);
                break;
            default:
                ALIMER_UNREACHABLE();
                break;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        device.GetHandle()->CreateRenderTargetView(resource, &viewDesc, handle);
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12TextureView::CreateDSV(ID3D12Resource* resource, bool readOnlyDepth, bool readOnlyStencil) const
    {
        uint32_t arrayMultiplier = (texture->GetTextureType() == TextureType::TextureCube) ? 6 : 1;

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.Format = ToDXGIFormat(format);

        if (readOnlyDepth)
            viewDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

        const bool hasStencil = viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT
            || viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT;

        if (readOnlyStencil && hasStencil)
            viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

        switch (texture->GetTextureType())
        {
            case TextureType::Texture1D:
                if (texture->GetArraySize() > 1)
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = baseMipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = baseArrayLayer;
                    viewDesc.Texture1DArray.ArraySize = arrayLayerCount;

                }
                else
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = baseMipLevel;
                }
                break;
            case TextureType::Texture2D:
            case TextureType::TextureCube:
                if (texture->GetSampleCount() != SampleCount::Count1)
                {
                    if (texture->GetArraySize() > 1)
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = baseArrayLayer;
                        viewDesc.Texture2DMSArray.ArraySize = arrayLayerCount;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    }
                }
                else
                {
                    if (texture->GetArraySize() * arrayMultiplier > 1)
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = baseMipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = baseArrayLayer * arrayMultiplier;
                        viewDesc.Texture2DArray.ArraySize = arrayLayerCount * arrayMultiplier;
                    }
                    else
                    {

                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = baseMipLevel;
                    }
                }
                break;
            case TextureType::Texture3D:
                viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Texture2DArray.MipSlice = baseMipLevel;
                viewDesc.Texture2DArray.FirstArraySlice = 0;
                viewDesc.Texture2DArray.ArraySize = texture->GetDepth(baseMipLevel);
                break;
            default:
                ALIMER_UNREACHABLE();
                break;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        device.GetHandle()->CreateDepthStencilView(resource, &viewDesc, handle);
        return handle;
    }
}

