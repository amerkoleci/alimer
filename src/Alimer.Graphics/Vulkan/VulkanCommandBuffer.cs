// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandBuffer : CommandBuffer
{
    private readonly VulkanCommandQueue _queue;
    private readonly VkCommandPool[] _commandPools = new VkCommandPool[MaxFramesInFlight];
    private readonly VkCommandBuffer[] _commandBuffers = new VkCommandBuffer[MaxFramesInFlight];
    private VkCommandBuffer _commandBuffer; // recording command buffer
    private RenderPassDescription _currentRenderPass;

    public VulkanCommandBuffer(VulkanCommandQueue queue)
        : base(queue.Device)
    {
        _queue = queue;

        for (uint i = 0; i < MaxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = new()
            {
                flags = VkCommandPoolCreateFlags.Transient,
                queueFamilyIndex = queue.Device.GetQueueFamily(queue.QueueType)
            };

            vkCreateCommandPool(queue.Device.Handle, &poolInfo, null, out _commandPools[i]).DebugCheckResult();

            VkCommandBufferAllocateInfo commandBufferInfo = new()
            {
                commandPool = _commandPools[i],
                level = VkCommandBufferLevel.Primary,
                commandBufferCount = 1
            };
            vkAllocateCommandBuffer(queue.Device.Handle, &commandBufferInfo, out _commandBuffers[i]).DebugCheckResult();

            //binderPools[i].Init(device);
        }
    }

    /// <inheritdoc />
    public override QueueType Queue => _queue.QueueType;

    public void Destroy()
    {
        for (int i = 0; i < _commandPools.Length; ++i)
        {
            //vkFreeCommandBuffers(Queue.Device.Handle, _commandPools[i], 1, &commandBuffers[i]);
            vkDestroyCommandPool(_queue.Device.Handle, _commandPools[i]);
            //binderPools[i].Shutdown();
        }
    }

    public void Begin(uint frameIndex, string? label = null)
    {
        base.Reset(frameIndex);
        //currentPipeline.Reset();
        _currentRenderPass = default;

        vkResetCommandPool(_queue.Device.Handle, _commandPools[frameIndex], 0).DebugCheckResult();
        _commandBuffer = _commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo = new()
        {
            flags = VkCommandBufferUsageFlags.OneTimeSubmit,
            pInheritanceInfo = null // Optional
        };
        vkBeginCommandBuffer(_commandBuffer, &beginInfo).DebugCheckResult();

        if (_queue.QueueType == QueueType.Graphics)
        {
            VkRect2D* scissors = stackalloc VkRect2D[16];
            for (uint i = 0; i < 16; ++i)
            {
                scissors[i].offset.x = 0;
                scissors[i].offset.y = 0;
                scissors[i].extent.width = 65535;
                scissors[i].extent.height = 65535;
            }
            vkCmdSetScissor(_commandBuffer, 0, 16, scissors);

            vkCmdSetBlendConstants(_commandBuffer, 1.0f, 1.0f, 1.0f, 1.0f);
            vkCmdSetStencilReference(_commandBuffer, VkStencilFaceFlags.FrontAndBack, ~0u);

            if (_queue.Device.PhysicalDeviceFeatures2.features.depthBounds == true)
            {
                vkCmdSetDepthBounds(_commandBuffer, 0.0f, 1.0f);
            }

            // Silence validation about uninitialized stride:
            //const VkDeviceSize zero = {};
            //vkCmdBindVertexBuffers2(commandBuffer, 0, 1, &nullBuffer, &zero, &zero, &zero);
        }

        if (!string.IsNullOrEmpty(label))
        {
            _hasLabel = true;
            PushDebugGroup(label);
        }
    }

    public override void PushDebugGroup(string groupLabel)
    {
        if (!_queue.Device.DebugUtils)
            return;

        fixed (sbyte* pLabelName = groupLabel.GetUtf8Span())
        {
            VkDebugUtilsLabelEXT label = new()
            {
                pLabelName = pLabelName
            };
            label.color[0] = 0.0f;
            label.color[1] = 0.0f;
            label.color[2] = 0.0f;
            label.color[3] = 1.0f;
            vkCmdBeginDebugUtilsLabelEXT(_commandBuffer, &label);
        }
    }

    public override void PopDebugGroup()
    {
        if (!_queue.Device.DebugUtils)
            return;

        vkCmdEndDebugUtilsLabelEXT(_commandBuffer);
    }

    public override void InsertDebugMarker(string debugLabel)
    {
        if (!_queue.Device.DebugUtils)
            return;

        fixed (sbyte* pLabelName = debugLabel.GetUtf8Span())
        {
            VkDebugUtilsLabelEXT label = new()
            {
                pLabelName = pLabelName
            };
            label.color[0] = 0.0f;
            label.color[1] = 0.0f;
            label.color[2] = 0.0f;
            label.color[3] = 1.0f;
            vkCmdInsertDebugUtilsLabelEXT(_commandBuffer, &label);
        }
    }

    protected override void BeginRenderPassCore(in RenderPassDescription renderPass)
    {
        if (!string.IsNullOrEmpty(renderPass.Label))
        {
            PushDebugGroup(renderPass.Label);
        }

        VkRect2D renderArea = new(_queue.Device.PhysicalDeviceProperties.properties.limits.maxFramebufferWidth, _queue.Device.PhysicalDeviceProperties.properties.limits.maxFramebufferHeight);

        if (_queue.Device.PhysicalDeviceFeatures1_3.dynamicRendering)
        {
            VkRenderingInfo renderingInfo = new();
            renderingInfo.layerCount = 1;
            renderingInfo.viewMask = 0;

            VkRenderingAttachmentInfo* colorAttachments = stackalloc VkRenderingAttachmentInfo[MaxColorAttachments];
            VkRenderingAttachmentInfo depthAttachment = new();
            VkRenderingAttachmentInfo stencilAttachment = new();

            PixelFormat depthStencilFormat = PixelFormat.Undefined; //desc.depthStencilAttachment.texture != nullptr ? desc.depthStencilAttachment.texture->GetFormat() : PixelFormat::Undefined;
            bool hasDepthOrStencil = false; // desc.depthStencilAttachment.texture != nullptr;

            for (int slot = 0; slot < renderPass.ColorAttachments.Length; slot++)
            {
                ref RenderPassColorAttachment attachment = ref renderPass.ColorAttachments[slot];
                Guard.IsTrue(attachment.Texture is not null);

                VulkanTexture texture = (VulkanTexture)attachment.Texture;
                int mipLevel = attachment.MipLevel;
                int slice = attachment.Slice;
                VkImageView imageView = texture.GetView(mipLevel, slice);

                renderArea.extent.width = Math.Min(renderArea.extent.width, texture.GetWidth(mipLevel));
                renderArea.extent.height = Math.Min(renderArea.extent.height, texture.GetHeight(mipLevel));

                ref VkRenderingAttachmentInfo attachmentInfo = ref colorAttachments[renderingInfo.colorAttachmentCount++];
                attachmentInfo = new()
                {
                    imageView = imageView,
                    imageLayout = VkImageLayout.ColorAttachmentOptimal,
                    loadOp = attachment.LoadAction.ToVk(),
                    storeOp = attachment.StoreAction.ToVk(),
                    clearValue = new VkClearValue(attachment.ClearColor.R, attachment.ClearColor.G, attachment.ClearColor.B, attachment.ClearColor.A)
                };
            }

            if (hasDepthOrStencil)
            {
                RenderPassDepthStencilAttachment attachment = renderPass.DepthStencilAttachment.Value;

                VulkanTexture texture = (VulkanTexture)attachment.Texture;
                int mipLevel = attachment.MipLevel;
                int slice = attachment.Slice;

                renderArea.extent.width = Math.Min(renderArea.extent.width, texture.GetWidth(mipLevel));
                renderArea.extent.height = Math.Min(renderArea.extent.height, texture.GetHeight(mipLevel));

                //depthAttachment.imageView = texture->GetView(mipLevel, slice);
                //depthAttachment.imageLayout = desc.depthStencilAttachment.depthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                //depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
                //depthAttachment.loadOp = ToVk(desc.depthStencilAttachment.depthLoadAction);
                //depthAttachment.storeOp = ToVk(desc.depthStencilAttachment.depthStoreAction);
                //depthAttachment.clearValue.depthStencil.depth = desc.depthStencilAttachment.clearDepth;
            }

            renderingInfo.renderArea = renderArea;
            renderingInfo.pColorAttachments = renderingInfo.colorAttachmentCount > 0 ? colorAttachments : null;
            renderingInfo.pDepthAttachment = hasDepthOrStencil ? &depthAttachment : null;
            renderingInfo.pStencilAttachment = !depthStencilFormat.IsDepthOnlyFormat() ? &depthAttachment : null;

            vkCmdBeginRendering(_commandBuffer, &renderingInfo);

            // The viewport and scissor default to cover all of the attachments
            VkViewport viewport = new()
            {
                x = 0.0f,
                y = (float)(renderArea.extent.height),
                width = (float)(renderArea.extent.width),
                height = -(float)(renderArea.extent.height),
                minDepth = 0.0f,
                maxDepth = 1.0f
            };
            vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

            VkRect2D scissorRect = new(renderArea.extent.width, renderArea.extent.height);
            vkCmdSetScissor(_commandBuffer, 0, 1, &scissorRect);
        }

        _currentRenderPass = renderPass;
    }

    protected override void EndRenderPassCore()
    {
        if (_queue.Device.PhysicalDeviceFeatures1_3.dynamicRendering)
        {
            vkCmdEndRendering(_commandBuffer);
        }


        if (!string.IsNullOrEmpty(_currentRenderPass.Label))
        {
            PopDebugGroup();
        }
    }

    public override Texture? AcquireSwapChainTexture(SwapChain swapChain)
    {
        VulkanSwapChain vulkanSwapChain = (VulkanSwapChain)swapChain;

        VkResult result = VkResult.Success;
        uint imageIndex = 0;
        lock (vulkanSwapChain.LockObject)
        {
            result = vkAcquireNextImageKHR(
                _queue.Device.Handle,
                vulkanSwapChain.Handle,
                ulong.MaxValue,
                vulkanSwapChain.AcquireSemaphore,
                VkFence.Null,
                out imageIndex);
        }

        if (result != VkResult.Success)
        {
            // Handle outdated error in acquire
            if (result == VkResult.SuboptimalKHR || result == VkResult.ErrorOutOfDateKHR)
            {
                _queue.Device.WaitIdle();
                //_queue.Device.UpdateSwapChain(vulkanSwapChain);
                return AcquireSwapChainTexture(swapChain);
            }
        }

        vulkanSwapChain.AcquiredImageIndex = imageIndex;
        _queue.QueuePresent(vulkanSwapChain);

        // TextureBarrier(swapChainTexture, ResourceStates.RenderTarget);
        return vulkanSwapChain.CurrentTexture;
    }

    public override void Commit()
    {
        //for (auto & swapChain : presentSwapChains)
        //{
        //    VulkanTexture* swapChainTexture = swapChain->backbufferTextures[swapChain->imageIndex].Get();
        //    TextureBarrier(swapChainTexture, ResourceStates::Present);
        //}

        if (_hasLabel)
        {
            PopDebugGroup();
        }

        vkEndCommandBuffer(_commandBuffer).DebugCheckResult();
        _queue.Commit(_commandBuffer);
    }
}
