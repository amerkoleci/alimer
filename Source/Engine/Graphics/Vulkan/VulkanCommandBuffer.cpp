// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanSampler.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphics.h"
#include <unordered_set>

namespace Alimer
{
    namespace
    {
        static_assert(sizeof(Alimer::Viewport) == sizeof(VkViewport), "Size mismatch");
        static_assert(offsetof(Alimer::Viewport, x) == offsetof(VkViewport, x), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, y) == offsetof(VkViewport, y), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, width) == offsetof(VkViewport, width), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, height) == offsetof(VkViewport, height), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(VkViewport, minDepth), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(VkViewport, maxDepth), "Layout mismatch");

        constexpr VkIndexType ToVulkan(IndexType type)
        {
            switch (type)
            {
                case IndexType::UInt16:
                    return VK_INDEX_TYPE_UINT16;
                case IndexType::UInt32:
                    return VK_INDEX_TYPE_UINT32;
                default:
                    ALIMER_UNREACHABLE();
                    return VK_INDEX_TYPE_MAX_ENUM;
            }
        }
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanGraphics& device, QueueType queueType, uint8_t index)
        : device{ device }
        , queue{ queueType }
        , index{ index }
        , debugUtilsSupported(device.DebugUtilsSupported())
    {
        for(uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            switch (queue)
            {
            case QueueType::Graphics:
                poolInfo.queueFamilyIndex = device.graphicsQueueFamily;
                break;
            case QueueType::Compute:
                poolInfo.queueFamilyIndex = device.computeQueueFamily;
                break;
            default:
                assert(0); // queue type not handled
                break;
            }
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VK_CHECK(vkCreateCommandPool(device.GetHandle(), &poolInfo, nullptr, &commandPools[i]));

            VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocateInfo.commandPool = commandPools[i];
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(device.GetHandle(), &allocateInfo, &commandBuffers[i]));
        }

        handle = commandBuffers[0];

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            // Create descriptor pool:
            VkDescriptorPoolSize poolSizes[9] = {};
            uint32_t count = 0;

            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = kUniformBufferCount * poolSize;
            count++;

            poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            poolSizes[1].descriptorCount = kSRVCount * poolSize;
            count++;

            poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[2].descriptorCount = kSRVCount * poolSize;
            count++;

            poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSizes[3].descriptorCount = kUAVCount * poolSize;
            count++;

            poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLER;
            poolSizes[4].descriptorCount = kSamplerCount * poolSize;
            count++;

            VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
            poolInfo.poolSizeCount = count;
            poolInfo.pPoolSizes = poolSizes;
            poolInfo.maxSets = poolSize;
            //poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            VK_CHECK(vkCreateDescriptorPool(device.GetHandle(), &poolInfo, nullptr, &descriptors[i].descriptorPool));
        }
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            vkFreeCommandBuffers(device.GetHandle(), commandPools[i], 1, &commandBuffers[i]);
            vkDestroyCommandPool(device.GetHandle(), commandPools[i], nullptr);
        }

        handle = VK_NULL_HANDLE;

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            device.DeferDestroy(descriptors[i].descriptorPool);
        }
    }

    void VulkanCommandBuffer::Reset(uint32_t frameIndex_)
    {
        CommandBuffer::Reset(frameIndex_);

        VK_CHECK(vkResetCommandPool(device.GetHandle(), commandPools[frameIndex], 0));

        handle = commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));

        colorAttachmentCount = 0;
        boundRenderPass = VK_NULL_HANDLE;
        boundPipeline = nullptr;

        // Reset descriptor allocators.
        descriptors[frameIndex].dirty = true;

        if (descriptors[frameIndex].descriptorPool != VK_NULL_HANDLE)
        {
            VK_CHECK(vkResetDescriptorPool(device.GetHandle(), descriptors[frameIndex].descriptorPool, 0));
        }

        // Reset state
        bindingState.Reset();
        pushConstants = {};
        swapChains.clear();
    }

    void VulkanCommandBuffer::End()
    {
        for (auto swapChainTexture : swapChainTextures)
        {
            //queue.AddSignalSemaphore(swapChainTexture->GetSignalSemaphore());
            swapChainTexture->TransitionImageLayout(handle,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }

        swapChainTextures.clear();

        VK_CHECK(vkEndCommandBuffer(handle));
    }

    void VulkanCommandBuffer::PushDebugGroup(const std::string_view& name)
    {
        if (!debugUtilsSupported)
            return;

        VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        vkCmdBeginDebugUtilsLabelEXT(handle, &label);
    }

    void VulkanCommandBuffer::PopDebugGroup()
    {
        if (!debugUtilsSupported)
            return;

        vkCmdEndDebugUtilsLabelEXT(handle);
    }

    void VulkanCommandBuffer::InsertDebugMarker(const std::string_view& name)
    {
        if (!debugUtilsSupported)
            return;

        VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name.data();
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        vkCmdInsertDebugUtilsLabelEXT(handle, &label);
    }

    void VulkanCommandBuffer::UpdateBufferCore(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size)
    {
        GPUAllocation allocation = Allocate(size, gGraphics().GetCaps().limits.minConstantBufferOffsetAlignment);
        memcpy(allocation.data, data, size);
        CopyBufferCore(allocation.buffer, allocation.offset, buffer, offset, size);
    }

    void VulkanCommandBuffer::CopyBufferCore(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size)
    {
        const VulkanBuffer* vkSource = ToVulkan(source);
        const VulkanBuffer* vkDestination = ToVulkan(destination);

        //vkSource->Barrier(handle, VulkanBufferState::CopySource);
        //vkDestination->Barrier(handle, VulkanBufferState::CopyDest);

        VkBufferCopy region;
        region.srcOffset = sourceOffset;
        region.dstOffset = destinationOffset;
        region.size = size;
        vkCmdCopyBuffer(handle, vkSource->GetHandle(), vkDestination->GetHandle(), 1, &region);
    }

    void VulkanCommandBuffer::BeginRenderPassCore(_In_ SwapChain* swapChain, const Color& clearColor)
    {
        VulkanSwapChain* vulkanSwapChain = checked_cast<VulkanSwapChain*>(swapChain);

        swapChains.push_back(vulkanSwapChain);
        auto textureView = vulkanSwapChain->GetCurrentTextureView();

        RenderPassInfo info{};
        info.colorAttachments[0].view = textureView;
        info.colorAttachments[0].loadAction = LoadAction::Clear;
        info.colorAttachments[0].storeAction = StoreAction::Store;
        info.colorAttachments[0].clearColor = clearColor;

        BeginRenderPassCore(info);
    }

    void VulkanCommandBuffer::BeginRenderPassCore(const RenderPassInfo& info)
    {
        VulkanRenderPassKey renderPassKey;
        renderPassKey.colorAttachmentCount = 0;

        VulkanFboKey fboKey;
        fboKey.attachmentCount = 0;
        fboKey.width = UINT32_MAX;
        fboKey.height = UINT32_MAX;
        fboKey.layers = 1u;

        size_t framebufferHash = 0;

        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = info.colorAttachments[i];
            if (attachment.view == nullptr)
                break;

            VulkanTextureView* view = static_cast<VulkanTextureView*>(attachment.view);
            VulkanTexture* texture = static_cast<VulkanTexture*>(view->GetTexture());

            if (texture->IsSwapChainTexture() && swapChainTextures.find(texture) == swapChainTextures.end())
            {
                swapChainTextures.insert(texture);
                //queue.AddWaitSemaphore(texture->GetWaitSemaphore(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT);
                //queue.QueuePresent(texture->GetSwapChain());
            }

            const uint32_t mipLevel = view->GetBaseMipLevel();
            fboKey.width = Min(fboKey.width, texture->GetWidth(mipLevel));
            fboKey.height = Min(fboKey.height, texture->GetHeight(mipLevel));

            // Hash
            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].format = texture->GetFormat();
            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].loadAction = attachment.loadAction;
            renderPassKey.colorAttachments[renderPassKey.colorAttachmentCount].storeAction = attachment.storeAction;

            HashCombine(framebufferHash, (uint64_t)view->GetHandle());

            // Transition to render target.
            texture->TransitionImageLayout(handle,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

            memcpy(&renderPassClearValues[fboKey.attachmentCount].color.float32[i], &attachment.clearColor.r, 4 * sizeof(float));

            renderPassKey.colorAttachmentCount++;

            fboKey.attachments[fboKey.attachmentCount] = view->GetHandle();
            fboKey.attachmentCount++;
        }

        bool hasDepthStencil = false;
        if (info.depthStencilAttachment.view != nullptr)
        {
            hasDepthStencil = true;

            const RenderPassDepthStencilAttachment& attachment = info.depthStencilAttachment;

            VulkanTextureView* view = static_cast<VulkanTextureView*>(attachment.view);
            VulkanTexture* texture = static_cast<VulkanTexture*>(view->GetTexture());

            renderPassKey.depthStencilAttachment.format = texture->GetFormat();
            renderPassKey.depthStencilAttachment.loadAction = attachment.depthLoadAction;
            renderPassKey.depthStencilAttachment.storeAction = attachment.depthStoreAction;

            const uint32_t mipLevel = view->GetBaseMipLevel();

            fboKey.width = Min(fboKey.width, texture->GetWidth(mipLevel));
            fboKey.height = Min(fboKey.height, texture->GetHeight(mipLevel));

            HashCombine(framebufferHash, (uint64_t)view->GetHandle());

            // Transition to depth stencil write.
            texture->TransitionImageLayout(handle, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

            renderPassClearValues[fboKey.attachmentCount].depthStencil.depth = attachment.clearDepth;
            renderPassClearValues[fboKey.attachmentCount].depthStencil.stencil = attachment.clearStencil;

            fboKey.attachments[fboKey.attachmentCount] = view->GetHandle();
            fboKey.attachmentCount++;
        }

        fboKey.renderPass = device.GetVkRenderPass(renderPassKey);

        HashCombine(framebufferHash, fboKey.attachmentCount);
        HashCombine(framebufferHash, (uint64_t)fboKey.renderPass);

        VkFramebuffer framebuffer = device.GetVkFramebuffer(framebufferHash, fboKey);

        VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        beginInfo.pNext = nullptr;
        beginInfo.renderPass = fboKey.renderPass;
        beginInfo.framebuffer = framebuffer;
        beginInfo.renderArea.offset.x = 0;
        beginInfo.renderArea.offset.y = 0;
        beginInfo.renderArea.extent.width = fboKey.width;
        beginInfo.renderArea.extent.height = fboKey.height;
        beginInfo.clearValueCount = fboKey.attachmentCount;
        beginInfo.pClearValues = renderPassClearValues.data();

        vkCmdBeginRenderPass(handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set the default value for the dynamic state
        {
            vkCmdSetLineWidth(handle, 1.0f);
            vkCmdSetDepthBounds(handle, 0.0f, 1.0f);
            SetStencilReference(0);
            float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            SetBlendColor(blendColor);

            // The viewport and scissor default to cover all of the attachments
            Viewport viewport(static_cast<float>(fboKey.width), static_cast<float>(fboKey.height));
            SetViewport(viewport);

            SetScissorRect(Rect(static_cast<float>(fboKey.width), static_cast<float>(fboKey.height)));
        }

        boundRenderPass = beginInfo.renderPass;
        colorAttachmentCount = renderPassKey.colorAttachmentCount;

        // Reset state
        bindingState.Reset();
        pushConstants = {};
    }

    void VulkanCommandBuffer::EndRenderPassCore()
    {
        vkCmdEndRenderPass(handle);
        boundRenderPass = VK_NULL_HANDLE;
        colorAttachmentCount = 0;
    }

    void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
    {
        // Flip viewport to match DirectX coordinate system
        VkViewport vkViewport;
        vkViewport.x = viewport.x;
        vkViewport.y = viewport.y + viewport.height;
        vkViewport.width = viewport.width;
        vkViewport.height = -viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;

        vkCmdSetViewport(handle, 0, 1, &vkViewport);
    }

    void VulkanCommandBuffer::SetViewports(const Viewport* viewports, uint32_t count)
    {
        // Flip viewport to match DirectX coordinate system
        VkViewport vkViewports[kMaxViewportsAndScissors];
        for (uint32_t i = 0; i < count; ++i)
        {
            vkViewports[i].x = viewports[i].x;
            vkViewports[i].y = viewports[i].y + viewports[i].height;
            vkViewports[i].width = viewports[i].width;
            vkViewports[i].height = -viewports[i].height;
            vkViewports[i].minDepth = viewports[i].minDepth;
            vkViewports[i].maxDepth = viewports[i].maxDepth;
        }

        vkCmdSetViewport(handle, 0, count, (const VkViewport*)viewports);
    }

    void VulkanCommandBuffer::SetScissorRect(const Rect& rect)
    {
        VkRect2D vkRect;
        vkRect.offset.x = (int32_t)rect.x;
        vkRect.offset.y = (int32_t)rect.y;
        vkRect.extent.width = (uint32_t)rect.width;
        vkRect.extent.height = (uint32_t)rect.height;
        vkCmdSetScissor(handle, 0, 1, &vkRect);
    }
    
    void VulkanCommandBuffer::SetScissorRects(const Rect* rects, uint32_t count)
    {
        VkRect2D vkRects[kMaxViewportsAndScissors];
        for (uint32_t i = 0; i < count; i += 1)
        {
            vkRects[i].offset.x = (int32_t)rects[i].x;
            vkRects[i].offset.y = (int32_t)rects[i].y;
            vkRects[i].extent.width = (uint32_t)rects[i].width;
            vkRects[i].extent.height = (uint32_t)rects[i].height;
        }
    
        vkCmdSetScissor(handle, 0, count, vkRects);
    }

    void VulkanCommandBuffer::SetStencilReference(uint32_t value)
    {
        vkCmdSetStencilReference(handle, VK_STENCIL_FRONT_AND_BACK, value);
    }

    void VulkanCommandBuffer::SetBlendColor(const Color& color)
    {
        vkCmdSetBlendConstants(handle, &color.r);
    }

    void VulkanCommandBuffer::SetBlendColor(const float blendColor[4])
    {
        vkCmdSetBlendConstants(handle, blendColor);
    }

    void VulkanCommandBuffer::SetVertexBuffersCore(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets)
    {
        VkBuffer vbuffers[kMaxVertexBufferBindings];
        for (uint32_t i = startSlot; i < count; ++i)
        {
            vbuffers[i] = ToVulkan(buffers[i])->GetHandle();
        }

        vkCmdBindVertexBuffers(handle, startSlot, count, vbuffers, offsets);
    }

    void VulkanCommandBuffer::SetIndexBufferCore(const Buffer* buffer, IndexType indexType, uint64_t offset)
    {
        vkCmdBindIndexBuffer(handle, ToVulkan(buffer)->GetHandle(), offset, ToVulkan(indexType));
    }

    void VulkanCommandBuffer::BindBufferCore(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range)
    {
        bindingState.BindBuffer(buffer, offset, range, set, binding);
    }

    void VulkanCommandBuffer::SetTextureCore(uint32_t set, uint32_t binding, const TextureView* texture)
    {
        bindingState.SetTexture(set, binding + kVulkanBindingShift_SRV, texture);
    }

    void VulkanCommandBuffer::PushConstants(const void* data, uint32_t size)
    {
        memcpy(pushConstants.data, data, size);
        pushConstants.size = size;
    }

    void VulkanCommandBuffer::SetPipeline(const Pipeline* pipeline)
    {
        boundPipeline = ToVulkan(pipeline);
        vkCmdBindPipeline(handle, boundPipeline->GetBindPoint(), boundPipeline->GetHandle());
    }

    void VulkanCommandBuffer::DrawCore(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
    {
        Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);

        vkCmdDraw(handle, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void VulkanCommandBuffer::DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);

        vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void VulkanCommandBuffer::Flush(VkPipelineBindPoint bindPoint)
    {
        FlushDescriptorState(bindPoint);

        FlushPushConstants();
    }

    void VulkanCommandBuffer::FlushDescriptorState(VkPipelineBindPoint bindPoint)
    {
        if (!bindingState.IsDirty())
            return;

        bindingState.ClearDirty();

        std::unordered_set<uint32_t> updateDescriptorSets;

        const VulkanPipelineLayout* pipelineLayout = boundPipeline->GetPipelineLayout();

        for (auto& descriptorSetLayout : pipelineLayout->GetDescriptorSetLayouts())
        {
            const uint32_t set = descriptorSetLayout->GetIndex();
            bindingState.ClearDirty(set);

            const auto resourceSet = bindingState.GetSet(set);
            const BindingMap<VulkanResourceInfo>& bindings = resourceSet.GetResourceBindings();

            // Allocate new DescriptorSet
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            if (set == 0)
            {
                auto setLayoutHandle = descriptorSetLayout->GetHandle();
                VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
                allocateInfo.descriptorPool = descriptors[frameIndex].descriptorPool;
                allocateInfo.descriptorSetCount = 1;
                allocateInfo.pSetLayouts = &setLayoutHandle;

                VkResult result = vkAllocateDescriptorSets(device.GetHandle(), &allocateInfo, &descriptorSet);
                while (result == VK_ERROR_OUT_OF_POOL_MEMORY)
                {
                    poolSize *= 2;
                    device.DeferDestroy(descriptors[frameIndex].descriptorPool);
                    //init(device);
                    allocateInfo.descriptorPool = descriptors[frameIndex].descriptorPool;
                    result = vkAllocateDescriptorSets(device.GetHandle(), &allocateInfo, &descriptorSet);
                }
                ALIMER_ASSERT(result == VK_SUCCESS);
            }

            std::vector<VkWriteDescriptorSet> descriptorWrites;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkBufferView> texelBufferViews;
            std::vector<uint32_t> dynamicOffsets;

            // Iterate over all resource bindings
            for (auto& binding_it : descriptorSetLayout->GetLayoutBindings())
            {
                const uint32_t bindingIndex = binding_it.binding;
                auto boundResouceInfoIt = bindings.find(bindingIndex);

                VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                writeDescriptorSet.dstBinding = bindingIndex;
                writeDescriptorSet.descriptorType = binding_it.descriptorType;
                writeDescriptorSet.dstSet = descriptorSet;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorCount = 1;

                switch (binding_it.descriptorType)
                {
                    default:
                        ALIMER_UNREACHABLE();
                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        imageInfos.emplace_back();
                        writeDescriptorSet.pImageInfo = &imageInfos.back();
                        imageInfos.back() = {};

                        if (boundResouceInfoIt != bindings.end())
                        {
                            imageInfos.back().sampler = ToVulkan(boundResouceInfoIt->second.sampler)->GetHandle();
                        }
                        else
                        {
                            imageInfos.back().sampler = device.nullSampler;
                        }

                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    {
                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.sampler = VK_NULL_HANDLE;

                        if (boundResouceInfoIt != bindings.end())
                        {
                            imageInfo.imageView = ToVulkan(boundResouceInfoIt->second.texture)->GetHandle();
                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        }
                        else
                        {
                            imageInfo.imageView = device.nullImageView2D;
                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        }

                        imageInfos.push_back(imageInfo);
                        writeDescriptorSet.pImageInfo = &imageInfo;
                        break;
                    }

                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    {
                        VkDescriptorBufferInfo bufferInfo{};

                        if (boundResouceInfoIt != bindings.end())
                        {
                            bufferInfo.buffer = ToVulkan(boundResouceInfoIt->second.buffer)->GetHandle();
                            bufferInfo.offset = boundResouceInfoIt->second.offset;
                            bufferInfo.range = boundResouceInfoIt->second.range;

                            if (binding_it.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
                            {
                                dynamicOffsets.push_back(static_cast<uint32_t>(bufferInfo.offset));
                                bufferInfo.offset = 0;
                            }
                        }
                        else
                        {
                            bufferInfo.buffer = VK_NULL_HANDLE;
                            bufferInfo.range = VK_WHOLE_SIZE;
                        }

                        bufferInfos.push_back(bufferInfo);
                        writeDescriptorSet.pBufferInfo = &bufferInfo;
                        break;
                    }
                }

                descriptorWrites.push_back(writeDescriptorSet);
            }

            // Update 
            if (descriptorWrites.size())
            {
                vkUpdateDescriptorSets(
                    device.GetHandle(),
                    (uint32_t)descriptorWrites.size(),
                    descriptorWrites.data(),
                    0,
                    nullptr
                );
            }

            // Bind descriptor set
            vkCmdBindDescriptorSets(handle,
                bindPoint,
                pipelineLayout->GetHandle(),
                set,
                1, &descriptorSet,
                static_cast<uint32_t>(dynamicOffsets.size()),
                dynamicOffsets.data()
            );
        }

        // TODO: Actually detect which bindless sets are used
        if (pipelineLayout->GetBindless())
        {
            auto bindlessSet = device.GetBindlessSampledImageDescriptorSet();
            vkCmdBindDescriptorSets(
                handle,
                bindPoint,
                pipelineLayout->GetHandle(),
                1,
                1, &bindlessSet,
                0, nullptr
            );
        }
    }

    void VulkanCommandBuffer::FlushPushConstants()
    {
        if (pushConstants.size > 0
            && boundPipeline->GetPipelineLayout()->GetPushConstantStage())
        {
            vkCmdPushConstants(
                handle,
                boundPipeline->GetPipelineLayout()->GetHandle(),
                boundPipeline->GetPipelineLayout()->GetPushConstantStage(),
                0,
                pushConstants.size,
                pushConstants.data
            );

            pushConstants.size = 0;
        }
    }

    /* VulkanResourceBindingState */
    void VulkanResourceSet::Reset()
    {
        ClearDirty();

        bindings.clear();
    }

    bool VulkanResourceSet::IsDirty() const
    {
        return dirty;
    }

    void VulkanResourceSet::ClearDirty()
    {
        dirty = false;
    }

    void VulkanResourceSet::ClearDirty(uint32_t binding)
    {
        bindings[binding].dirty = false;
    }

    bool VulkanResourceSet::SetBuffer(uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range)
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

    bool VulkanResourceSet::SetTexture(uint32_t binding, const TextureView* texture)
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

    bool VulkanResourceSet::SetSampler(uint32_t binding, const Sampler* sampler)
    {
        if (bindings[binding].sampler == sampler)
        {
            return false;
        }

        bindings[binding].dirty = true;
        bindings[binding].sampler = sampler;
        dirty = true;
        return true;
    }

    void VulkanResourceBindingState::Reset()
    {
        ClearDirty();
        sets.clear();
    }

    bool VulkanResourceBindingState::IsDirty()
    {
        return dirty;
    }

    void VulkanResourceBindingState::ClearDirty()
    {
        dirty = false;
    }

    void VulkanResourceBindingState::ClearDirty(uint32_t set)
    {
        sets[set].ClearDirty();
    }

    void VulkanResourceBindingState::BindBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t set, uint32_t binding)
    {
        if (sets[set].SetBuffer(binding, buffer, offset, range))
        {
            dirty = true;
        }
    }

    void VulkanResourceBindingState::SetTexture(uint32_t set, uint32_t binding, const TextureView* texture)
    {
        if (sets[set].SetTexture(binding, texture))
        {
            dirty = true;
        }
    }

    void VulkanResourceBindingState::SetSampler(uint32_t set, uint32_t binding, const Sampler* sampler)
    {
        if (sets[set].SetSampler(binding, sampler))
        {
            dirty = true;
        }
    }
}
