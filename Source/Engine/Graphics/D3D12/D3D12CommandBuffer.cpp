// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12CommandBuffer.h"
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "D3D12Pipeline.h"
#include "D3D12Graphics.h"
#include "directx/d3dx12.h"
#include <unordered_set>
#include <pix.h>

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace Alimer
{
    namespace
    {
        static_assert(sizeof(Alimer::Viewport) == sizeof(D3D12_VIEWPORT), "Size mismatch");
        static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D12_VIEWPORT, Width), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D12_VIEWPORT, Height), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "Layout mismatch");


        [[nodiscard]] constexpr D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToD3D12(LoadAction action)
        {
            switch (action) {
                case LoadAction::Load:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                case LoadAction::Clear:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                case LoadAction::Discard:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
            }
        }

        [[nodiscard]] constexpr D3D12_RENDER_PASS_ENDING_ACCESS_TYPE  ToD3D12(StoreAction action)
        {
            switch (action) {
                case StoreAction::Store:
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                case StoreAction::Discard:
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            }
        }

        constexpr DXGI_FORMAT ToD3D12(IndexType type)
        {
            switch (type)
            {
                case IndexType::UInt16:
                    return DXGI_FORMAT_R16_UINT;
                case IndexType::UInt32:
                    return DXGI_FORMAT_R32_UINT;
                default:
                    ALIMER_UNREACHABLE();
                    return DXGI_FORMAT_UNKNOWN;
            }
        }
    }

    D3D12CommandBuffer::D3D12CommandBuffer(D3D12CommandQueue& queue_)
        : device(queue_.GetDevice())
        , queue(queue_)
        , supportsRenderPass(queue_.GetDevice().supportsRenderPass)
    {
        supportsRenderPass = true;
        _currentAllocator = queue.RequestAllocator();
        ThrowIfFailed(queue.GetDevice().GetHandle()->CreateCommandList(1,
            ToD3D12(queue.GetQueueType()),
            _currentAllocator,
            nullptr,
            IID_PPV_ARGS(&handle))
        );

        D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
        if (SUCCEEDED(queue.GetDevice().GetHandle()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS,
            &options,
            sizeof(options))))
        {
            typedUAVLoadAdditionalFormats = options.TypedUAVLoadAdditionalFormats != 0;
            standardSwizzle64KBSupported = options.StandardSwizzle64KBSupported != 0;
        }

        ID3D12DescriptorHeap* heaps[2] = {
            device.GetResourceDescriptorHeap(),
            device.GetSamplerDescriptorHeap()
        };
        handle->SetDescriptorHeaps(_countof(heaps), heaps);
    }

    D3D12CommandBuffer::~D3D12CommandBuffer()
    {
    }

    void D3D12CommandBuffer::Reset(uint32_t frameIndex_)
    {
        CommandBuffer::Reset(frameIndex_);

        ALIMER_ASSERT(handle != nullptr && _currentAllocator == nullptr);
        _currentAllocator = queue.RequestAllocator();
        ThrowIfFailed(handle->Reset(_currentAllocator, nullptr));

        ID3D12DescriptorHeap* heaps[2] = {
            device.GetResourceDescriptorHeap(),
            device.GetSamplerDescriptorHeap()
        };
        handle->SetDescriptorHeaps(_countof(heaps), heaps);

        numBarriersToFlush = 0;
        primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        // Reset state
        bindingState.Reset();
        pushConstants = {};
    }

    ID3D12GraphicsCommandList4* D3D12CommandBuffer::FinishAndReturn(uint64_t fenceValue)
    {
        // Make sure we transition all swapchain to present state
        for (auto swapChainTexture : swapChainTextures)
        {
            TransitionResource(swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
        }
        swapChainTextures.clear();

        FlushResourceBarriers();

        ThrowIfFailed(handle->Close());

        queue.DiscardAllocator(fenceValue, _currentAllocator);
        _currentAllocator = nullptr;

        return handle.Get();
    }

    void D3D12CommandBuffer::PushDebugGroup(const std::string& name)
    {
        auto wideName = ToUtf16(name);
        PIXBeginEvent(handle.Get(), PIX_COLOR_DEFAULT, wideName.c_str());
    }

    void D3D12CommandBuffer::PopDebugGroup()
    {
        PIXEndEvent(handle.Get());
    }

    void D3D12CommandBuffer::InsertDebugMarker(const std::string& name)
    {
        auto wideName = ToUtf16(name);
        PIXSetMarker(handle.Get(), PIX_COLOR_DEFAULT, wideName.c_str());
    }

    void D3D12CommandBuffer::UpdateBufferCore(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size)
    {
        //auto stagingBuffer = _device.CreateBuffer(BufferUsage::MapWrite, size, data);
        //CopyBufferCore(stagingBuffer, 0, buffer, offset, size);

        auto allocation = Allocate(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        memcpy(allocation.data, data, size);

        CopyBufferCore(allocation.buffer, allocation.offset, buffer, offset, size);
    }

    void D3D12CommandBuffer::CopyBufferCore(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size)
    {
        const D3D12Buffer* d3d12Source = ToD3D12(source);
        const D3D12Buffer* d3d12Destination = ToD3D12(destination);

        TransitionResource(d3d12Source, D3D12_RESOURCE_STATE_COPY_SOURCE);
        TransitionResource(d3d12Destination, D3D12_RESOURCE_STATE_COPY_DEST);
        FlushResourceBarriers();

        handle->CopyBufferRegion(d3d12Destination->GetHandle(), destinationOffset, d3d12Source->GetHandle(), sourceOffset, size);
    }

    void D3D12CommandBuffer::BeginRenderPassCore(const RenderPassInfo& info)
    {
        uint32_t width = UINT32_MAX;
        uint32_t height = UINT32_MAX;
        const D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;

        uint32_t colorAttachmentCount = 0;
        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = info.colorAttachments[i];
            if (attachment.view == nullptr)
                break;

            D3D12Texture* sourceTexture = static_cast<D3D12Texture*>(attachment.view->GetTexture());
            D3D12TextureView* view = static_cast<D3D12TextureView*>(attachment.view);

            if (sourceTexture->IsSwapChainTexture())
            {
                swapChainTextures.push_back(sourceTexture);
                queue.QueuePresent(sourceTexture->GetSwapChain());
            }

            const uint32_t mipLevel = view->GetBaseMipLevel();

            width = Min(width, sourceTexture->GetWidth(mipLevel));
            height = Min(height, sourceTexture->GetHeight(mipLevel));

            TransitionResource(sourceTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

            if (supportsRenderPass)
            {
                rtvDescs[i].cpuDescriptor = view->GetRTV();

                switch (attachment.loadAction)
                {
                    default:
                    case LoadAction::Load:
                        rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                        break;

                    case LoadAction::Clear:
                        rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                        rtvDescs[i].BeginningAccess.Clear.ClearValue.Format = sourceTexture->GetDXGIFormat();
                        rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor.r;
                        rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor.g;
                        rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor.b;
                        rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor.a;
                        break;

                    case LoadAction::Discard:
                        rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                        break;
                }

                switch (attachment.storeAction)
                {
                    default:
                    case StoreAction::Store:
                        rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                        break;

                    case StoreAction::Discard:
                        rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                        break;
                }

                if (attachment.resolveView != nullptr)
                {
                    D3D12Texture* destTexture = static_cast<D3D12Texture*>(attachment.resolveView->GetTexture());

                    TransitionResource(sourceTexture, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, false);
                    TransitionResource(destTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST, false);
                    FlushResourceBarriers();

                    rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
                    rtvDescs[i].EndingAccess.Resolve.pSrcResource = sourceTexture->GetHandle();
                    rtvDescs[i].EndingAccess.Resolve.pDstResource = destTexture->GetHandle();
                    rtvDescs[i].EndingAccess.Resolve.SubresourceCount = 1;

                    subresourceParameters[i].SrcSubresource = sourceTexture->GetSubresourceIndex(mipLevel, attachment.view->GetBaseArrayLayer());
                    subresourceParameters[i].DstSubresource = destTexture->GetSubresourceIndex(mipLevel, attachment.view->GetBaseArrayLayer());
                    subresourceParameters[i].DstX = 0;
                    subresourceParameters[i].DstY = 0;
                    subresourceParameters[i].SrcRect = { 0, 0, 0, 0 };

                    rtvDescs[i].EndingAccess.Resolve.pSubresourceParameters = &subresourceParameters[i];
                    rtvDescs[i].EndingAccess.Resolve.Format = destTexture->GetDXGIFormat();

                    // RESOLVE_MODE_AVERAGE is only valid for non-integer formats.
                    switch (GetFormatKind(destTexture->GetFormat()))
                    {
                        case PixelFormatKind::Integer:
                        //case PixelFormatKind::Integer:
                            rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_MAX;
                            break;
                        default:
                            rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
                            break;
                    }

                    rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;

                    // Clear or preserve the resolve source.
                    if (attachment.storeAction == StoreAction::Discard)
                    {
                        rtvDescs[i].EndingAccess.Resolve.PreserveResolveSource = false;
                    }
                    else if (attachment.storeAction == StoreAction::Store)
                    {
                        rtvDescs[i].EndingAccess.Resolve.PreserveResolveSource = true;
                    }
                }
            }
            else
            {
                colorRTVHandles[i] = view->GetRTV();

                switch (attachment.loadAction)
                {
                    case LoadAction::Clear:
                        handle->ClearRenderTargetView(colorRTVHandles[i], attachment.clearColor.data, 0, nullptr);
                        break;

                    default:
                        break;
                }
            }

            colorAttachmentCount++;
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsvDesc{};
        D3D12_CPU_DESCRIPTOR_HANDLE dsv{};
        bool hasDepthStencil = false;
        if (info.depthStencilAttachment.view != nullptr)
        {
            hasDepthStencil = true;
            const RenderPassDepthStencilAttachment& attachment = info.depthStencilAttachment;

            D3D12TextureView* view = static_cast<D3D12TextureView*>(attachment.view);
            D3D12Texture* texture = static_cast<D3D12Texture*>(view->GetTexture());
            const uint32_t mipLevel = view->GetBaseMipLevel();

            width = Min(width, texture->GetWidth(mipLevel));
            height = Min(height, texture->GetHeight(mipLevel));

            dsv = view->GetDSV(attachment.depthReadOnly, attachment.stencilReadOnly);

            TransitionResource(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

            if (supportsRenderPass)
            {
                dsvDesc.cpuDescriptor = dsv;

                switch (attachment.depthLoadAction)
                {
                    default:
                    case LoadAction::Load:
                        dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                        break;

                    case LoadAction::Clear:
                        dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                        dsvDesc.DepthBeginningAccess.Clear.ClearValue.Format = texture->GetDXGIFormat();
                        dsvDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = attachment.clearDepth;
                        break;

                    case LoadAction::Discard:
                        dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                        break;
                }

                switch (attachment.depthStoreAction)
                {
                    default:
                    case StoreAction::Store:
                        dsvDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                        break;

                    case StoreAction::Discard:
                        dsvDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                        break;
                }

                switch (attachment.stencilLoadAction)
                {
                    default:
                    case LoadAction::Load:
                        dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                        break;

                    case LoadAction::Clear:
                        dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                        dsvDesc.StencilBeginningAccess.Clear.ClearValue.Format = texture->GetDXGIFormat();
                        dsvDesc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = attachment.clearStencil;
                        break;

                    case LoadAction::Discard:
                        dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                        break;
                }

                switch (attachment.stencilStoreAction)
                {
                    default:
                    case StoreAction::Store:
                        dsvDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                        break;

                    case StoreAction::Discard:
                        dsvDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                        break;
                }
            }
            else
            {
                D3D12_CLEAR_FLAGS clearFlags = {};
                float clearDepth = 0.0f;
                uint8_t clearStencil = 0u;
                if (attachment.depthLoadAction == LoadAction::Clear)
                {
                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                    clearDepth = attachment.clearDepth;
                }

                if (attachment.stencilLoadAction == LoadAction::Clear
                    && IsStencilFormat(view->GetFormat()))
                {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                    clearStencil = attachment.clearStencil;
                }

                if (clearFlags)
                {
                    handle->ClearDepthStencilView(dsv, clearFlags, clearDepth, clearStencil, 0, nullptr);
                }
            }
        }

        if (supportsRenderPass)
        {
            handle->BeginRenderPass(colorAttachmentCount, rtvDescs, hasDepthStencil ? &dsvDesc : nullptr, renderPassFlags);
        }
        else
        {
            handle->OMSetRenderTargets(colorAttachmentCount, colorRTVHandles, FALSE, hasDepthStencil ? &dsv : nullptr);
        }

        // Set the default value for the dynamic state
        {
            //vkCmdSetLineWidth(handle, 1.0f);
            //vkCmdSetDepthBounds(handle, 0.0f, 1.0f);
            SetStencilReference(0);
            float blendColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            SetBlendColor(blendColor);

            // The viewport and scissor default to cover all of the attachments
            SetViewport(Viewport((float)width, (float)height));
            //SetScissorRect(Rect(width, height));
        }

        // Reset state
        bindingState.Reset();
        pushConstants = {};
    }

    void D3D12CommandBuffer::EndRenderPassCore()
    {
        if (supportsRenderPass)
        {
            handle->EndRenderPass();
        }

        // TODO: Ending barriers?
        // TODO: Query flush?
    }

    //void D3D12CommandBuffer::SetViewport(const Rect& rect)
    //{
    //    SetViewport(Viewport(rect));
    //}

    void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
    {
        handle->RSSetViewports(1, (D3D12_VIEWPORT*)&viewport);
    }

    void D3D12CommandBuffer::SetViewports(const Viewport* viewports, uint32_t count)
    {
        handle->RSSetViewports(count, (D3D12_VIEWPORT*)viewports);
    }

    //void D3D12CommandBuffer::SetScissorRect(const Rect& rect)
    //{
    //    D3D12_RECT d3dScissorRect;
    //    d3dScissorRect.left = LONG(rect.x);
    //    d3dScissorRect.top = LONG(rect.y);
    //    d3dScissorRect.right = LONG(rect.x + rect.width);
    //    d3dScissorRect.bottom = LONG(rect.y + rect.height);
    //    handle->RSSetScissorRects(1, &d3dScissorRect);
    //}

    //void D3D12CommandBuffer::SetScissorRects(const Rect* rects, uint32_t count)
    //{
    //    ALIMER_ASSERT(count <= kMaxViewportsAndScissors);
    //
    //    D3D12_RECT d3dScissorRects[kMaxViewportsAndScissors];
    //    for (uint32_t i = 0; i < count; i += 1)
    //    {
    //        d3dScissorRects[i].left = LONG(rects[i].x);
    //        d3dScissorRects[i].top = LONG(rects[i].y);
    //        d3dScissorRects[i].right = LONG(rects[i].x + rects[i].width);
    //        d3dScissorRects[i].bottom = LONG(rects[i].y + rects[i].height);
    //    }
    //    handle->RSSetScissorRects(count, d3dScissorRects);
    //}

    void D3D12CommandBuffer::SetStencilReference(uint32_t value)
    {
        handle->OMSetStencilRef(value);
    }

    void D3D12CommandBuffer::SetBlendColor(const Color& color)
    {
        handle->OMSetBlendFactor(color.data);
    }

    void D3D12CommandBuffer::SetBlendColor(const float blendColor[4])
    {
        handle->OMSetBlendFactor(blendColor);
    }

    void D3D12CommandBuffer::SetVertexBuffersCore(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets)
    {
        for (uint32_t i = startSlot; i < count; ++i)
        {
            auto d3d12Buffer = ToD3D12(buffers[i]);

            vboViews[i].BufferLocation = d3d12Buffer->GetGpuVirtualAddress();
            vboViews[i].SizeInBytes = static_cast<UINT>(buffers[i]->GetSize());

            if (offsets != nullptr)
            {
                vboViews[i].BufferLocation += offsets[i];
                vboViews[i].SizeInBytes -= (UINT)offsets[i];
            }
        }
    }

    void D3D12CommandBuffer::SetIndexBufferCore(const Buffer* buffer, IndexType indexType, uint64_t offset)
    {
        auto d3d12Buffer = ToD3D12(buffer);
        D3D12_INDEX_BUFFER_VIEW view = {};
        view.BufferLocation = d3d12Buffer->GetGpuVirtualAddress() + offset;
        view.SizeInBytes = (UINT)(buffer->GetSize() - offset);
        view.Format = (indexType == IndexType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
        handle->IASetIndexBuffer(&view);
    }

    void D3D12CommandBuffer::BindBufferCore(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range)
    {
        bindingState.BindBuffer(buffer, offset, range, set, binding);
    }

    void D3D12CommandBuffer::SetTextureCore(uint32_t set, uint32_t binding, const TextureView* texture)
    {
        bindingState.SetTexture(set, binding + 1000, texture);
    }

    void D3D12CommandBuffer::PushConstants(const void* data, uint32_t size)
    {
        memcpy(pushConstants.data, data, size);
        pushConstants.size = size;
    }

    void D3D12CommandBuffer::SetPipeline(const Pipeline* pipeline)
    {
        boundPipeline = ToD3D12(pipeline);
        handle->SetGraphicsRootSignature(boundPipeline->GetRootSignature());
        handle->SetPipelineState(boundPipeline->GetHandle());

        if (primitiveTopology != boundPipeline->GetPrimitiveTopology())
        {
            primitiveTopology = boundPipeline->GetPrimitiveTopology();
            handle->IASetPrimitiveTopology(primitiveTopology);
        }
    }

    void D3D12CommandBuffer::FlushDraw()
    {
        uint32_t startSlot = 0;
        uint32_t endSlot = 0;
        for (uint32_t slot = 0; slot < boundPipeline->GetVertexBufferSlotsUsed(); ++slot)
        {
            startSlot = Min(startSlot, slot);
            endSlot = Max(endSlot, slot + 1);
            vboViews[slot].StrideInBytes = boundPipeline->GetVertexBufferStride(slot);
        }

        handle->IASetVertexBuffers(startSlot, endSlot, vboViews.data());

        FlushDescriptorState(true);

        FlushPushConstants();
    }

    void D3D12CommandBuffer::FlushDescriptorState(bool graphics)
    {
        // Check if a descriptor set needs to be updated
        if (!bindingState.IsDirty())
            return;

        bindingState.ClearDirty();

        // Iterate over all of the resource sets bound by the command buffer
        for (auto& resourceSetIt : bindingState.GetSets())
        {
            uint32_t descriptorSetId = resourceSetIt.first;
            auto& resourceSet = resourceSetIt.second;

            // Don't update resource set if it's not in the update list OR its state hasn't changed
            if (!resourceSet.IsDirty())
            {
                continue;
            }

            // Clear dirty flag for resource set
            bindingState.ClearDirty(descriptorSetId);

            for (auto& binding_it : resourceSet.GetResourceBindings())
            {
                const uint32_t bindingIndex = binding_it.first;
                const D3D12ResourceInfo& resourceInfo = binding_it.second;

                // Pointer references
                auto& buffer = resourceInfo.buffer;

                // Get buffer info
                if (buffer != nullptr)
                {
                    uint32_t rootParameterIndex = boundPipeline->GetDescriptorCBVParameterIndex();
                    if (graphics)
                    {
                        handle->SetGraphicsRootConstantBufferView(rootParameterIndex + bindingIndex, ToD3D12(buffer)->GetGpuVirtualAddress() + resourceInfo.offset);
                    }
                    else
                    {
                        handle->SetComputeRootConstantBufferView(rootParameterIndex + bindingIndex, ToD3D12(buffer)->GetGpuVirtualAddress() + resourceInfo.offset);
                    }
                }
                else if (resourceInfo.texture != nullptr)
                {
                    LOGE("Only bindless is supported for textures");
                    //CD3DX12_CPU_DESCRIPTOR_HANDLE dst(device.GetCPUDescriptorHeapStart(), resourceIndex++, device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    //device.GetHandle()->CopyDescriptorsSimple(1, dst, ToD3D12(resourceInfo.texture)->GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                }
            }
        }

        if (boundPipeline->GetDescriptorTableRootParameterIndex() != -1)
        {
            const D3D12_GPU_DESCRIPTOR_HANDLE& gpuStart = device.GetResourceDescriptorHeapGPUStart();
            handle->SetGraphicsRootDescriptorTable(boundPipeline->GetDescriptorTableRootParameterIndex(), gpuStart);
        }
    }

    void D3D12CommandBuffer::FlushPushConstants()
    {
        if (pushConstants.size > 0 && boundPipeline->GetPushConstantNum32BitValues() > 0)
        {
            const uint32_t rootParameterIndex = boundPipeline->GetDescriptorPushConstantParameterIndex();
            handle->SetGraphicsRoot32BitConstants(
                rootParameterIndex,
                boundPipeline->GetPushConstantNum32BitValues(),
                pushConstants.data,
                0
            );

            pushConstants.size = 0;
        }
    }

    void D3D12CommandBuffer::DrawCore(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
    {
        FlushDraw();

        handle->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void D3D12CommandBuffer::DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        FlushDraw();

        handle->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void D3D12CommandBuffer::TransitionResource(const D3D12GpuResource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
    {
        if (resource->fixedResourceState)
        {
            // D3D12_RESOURCE_STATE_COPY_DEST and D3D12_RESOURCE_STATE_GENERIC_READ are fixed
            return;
        }

        D3D12_RESOURCE_STATES oldState = resource->state;

        if (queue.GetQueueType() == CommandQueueType::Compute)
        {
            ALIMER_ASSERT((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
            ALIMER_ASSERT((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
        }

        if (oldState != newState)
        {
            ALIMER_ASSERT_MSG(numBarriersToFlush < kMaxResourceBarriers, "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers[numBarriersToFlush++];

            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource->GetHandle();
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldState;
            barrierDesc.Transition.StateAfter = newState;

            // Check to see if we already started the transition
            if (newState == resource->transitioningState)
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                resource->transitioningState = (D3D12_RESOURCE_STATES)-1;
            }
            else
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            }

            resource->state = newState;
        }
        else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            InsertUAVBarrier(resource, flushImmediate);
        }

        if (flushImmediate || numBarriersToFlush == kMaxResourceBarriers)
        {
            FlushResourceBarriers();
        }
    }

    void D3D12CommandBuffer::BeginResourceTransition(const D3D12GpuResource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
    {
        // If it's already transitioning, finish that transition
        if (resource->transitioningState != (D3D12_RESOURCE_STATES)-1)
            TransitionResource(resource, resource->transitioningState);

        D3D12_RESOURCE_STATES oldState = resource->state;

        if (oldState != newState)
        {
            ALIMER_ASSERT_MSG(numBarriersToFlush < kMaxResourceBarriers, "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers[numBarriersToFlush++];

            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource->GetHandle();
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldState;
            barrierDesc.Transition.StateAfter = newState;
            barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

            resource->transitioningState = newState;
        }

        if (flushImmediate || numBarriersToFlush == kMaxResourceBarriers)
        {
            FlushResourceBarriers();
        }
    }

    void D3D12CommandBuffer::InsertUAVBarrier(const D3D12GpuResource* resource, bool flushImmediate)
    {
        ALIMER_ASSERT_MSG(numBarriersToFlush < kMaxResourceBarriers, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers[numBarriersToFlush++];
        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.UAV.pResource = resource->GetHandle();

        if (flushImmediate)
            FlushResourceBarriers();
    }

    void D3D12CommandBuffer::InsertAliasBarrier(const D3D12GpuResource* before, const D3D12GpuResource* after, bool flushImmediate)
    {
        ALIMER_ASSERT_MSG(numBarriersToFlush < kMaxResourceBarriers, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers[numBarriersToFlush++];

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.Aliasing.pResourceBefore = before->GetHandle();
        barrierDesc.Aliasing.pResourceAfter = after->GetHandle();

        if (flushImmediate)
            FlushResourceBarriers();
    }

    void D3D12CommandBuffer::FlushResourceBarriers()
    {
        if (numBarriersToFlush > 0)
        {
            handle->ResourceBarrier(numBarriersToFlush, resourceBarriers);
            numBarriersToFlush = 0;
        }
    }

    /* D3D12ResourceBindingState */
    void D3D12ResourceSet::Reset()
    {
        ClearDirty();

        bindings.clear();
    }

    bool D3D12ResourceSet::IsDirty() const
    {
        return dirty;
    }

    void D3D12ResourceSet::ClearDirty()
    {
        dirty = false;
    }

    void D3D12ResourceSet::ClearDirty(uint32_t binding)
    {
        bindings[binding].dirty = false;
    }

    bool D3D12ResourceSet::SetBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t binding)
    {
        if (bindings[binding].buffer == buffer &&
            bindings[binding].offset == offset &&
            bindings[binding].range == range)
        {
            return false;
        }

        bindings[binding].dirty = true;
        bindings[binding].buffer = buffer;
        bindings[binding].offset = offset;
        bindings[binding].range = range;
        dirty = true;
        return true;
    }

    bool D3D12ResourceSet::SetTexture(uint32_t binding, const TextureView* texture)
    {
        if (bindings[binding].texture == texture)
        {
            return false;
        }

        bindings[binding].dirty = true;
        bindings[binding].texture = texture;
        dirty = true;
        return true;
    }

    void D3D12ResourceBindingState::Reset()
    {
        ClearDirty();
        sets.clear();
    }

    bool D3D12ResourceBindingState::IsDirty()
    {
        return dirty;
    }

    void D3D12ResourceBindingState::ClearDirty()
    {
        dirty = false;
    }

    void D3D12ResourceBindingState::ClearDirty(uint32_t set)
    {
        sets[set].ClearDirty();
    }

    void D3D12ResourceBindingState::BindBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t set, uint32_t binding)
    {
        if (sets[set].SetBuffer(buffer, offset, range, binding))
        {
            dirty = true;
        }
    }

    void D3D12ResourceBindingState::SetTexture(uint32_t set, uint32_t binding, const TextureView* texture)
    {
        if (sets[set].SetTexture(binding, texture))
        {
            dirty = true;
        }
    }
}
