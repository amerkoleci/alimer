// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using CommunityToolkit.Diagnostics;
using System.Diagnostics;
using System.Drawing;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandBuffer : RenderContext
{
    private readonly VulkanCommandQueue _queue;
    private readonly VkCommandPool[] _commandPools = new VkCommandPool[MaxFramesInFlight];
    private readonly VkCommandBuffer[] _commandBuffers = new VkCommandBuffer[MaxFramesInFlight];
    private VkCommandBuffer _commandBuffer; // recording command buffer
    private RenderPassDescription _currentRenderPass;
    private bool _synchronization2;

    public VulkanCommandBuffer(VulkanCommandQueue queue)
    {
        _queue = queue;
        _synchronization2 = queue.Device.PhysicalDeviceFeatures1_3.synchronization2;

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
    public override GraphicsDevice Device => _queue.Device;

    public void Destroy()
    {
        for (int i = 0; i < _commandPools.Length; ++i)
        {
            //vkFreeCommandBuffers(Queue.Device.Handle, _commandPools[i], 1, &commandBuffers[i]);
            vkDestroyCommandPool(_queue.Device.Handle, _commandPools[i]);
            //binderPools[i].Shutdown();
        }
    }

    public override void Flush(bool waitForCompletion = false)
    {
        if (_hasLabel)
        {
            PopDebugGroup();
        }

        _queue.Commit(this, _commandBuffer);
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

    public void TextureBarrier(VulkanTexture texture, ResourceStates newState)
    {
        if (texture.CurrentState == newState)
            return;

        // Transition from undefined -> render target
        VkImageSubresourceRange range = new()
        {
            aspectMask = texture.VkFormat.GetVkImageAspectFlags(),
            baseMipLevel = 0,
            levelCount = VK_REMAINING_MIP_LEVELS,
            baseArrayLayer = 0,
            layerCount = VK_REMAINING_ARRAY_LAYERS
        };
        ImageMemoryBarrier(texture.Handle, texture.CurrentState, newState, range);
        texture.CurrentState = newState;
    }

    public void ImageMemoryBarrier(VkImage image, ResourceStates before, ResourceStates after, VkImageSubresourceRange range)
    {
        if (_synchronization2)
        {
            ResourceStateMapping mappingBefore = ConvertResourceState(before);
            ResourceStateMapping mappingAfter = ConvertResourceState(after);

            Debug.Assert(mappingAfter.ImageLayout != VkImageLayout.Undefined);

            VkImageMemoryBarrier2 barrier = new()
            {
                srcStageMask = mappingBefore.StageFlags,
                srcAccessMask = mappingBefore.AccessMask,
                oldLayout = mappingBefore.ImageLayout,

                dstStageMask = mappingAfter.StageFlags,
                dstAccessMask = mappingAfter.AccessMask,
                newLayout = mappingAfter.ImageLayout,

                srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                image = image,
                subresourceRange = range
            };

            VkDependencyInfo dependencyInfo = new()
            {
                imageMemoryBarrierCount = 1,
                pImageMemoryBarriers = &barrier
            };

            vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
        }
        else
        {

        }
    }

    #region ComputeContext Methods
    public override void SetPipeline(Pipeline pipeline)
    {
        throw new NotImplementedException();
    }

    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        vkCmdDispatch(_commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    #endregion ComputeContext Methods

    #region RenderContext Methods
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

            var depthStencilFormat = renderPass.DepthStencilAttachment.Texture != null ? renderPass.DepthStencilAttachment.Texture.Format : PixelFormat.Undefined;
            var hasDepthOrStencil = depthStencilFormat != PixelFormat.Undefined;

            for (var slot = 0; slot < renderPass.ColorAttachments.Length; slot++)
            {
                ref readonly RenderPassColorAttachment attachment = ref renderPass.ColorAttachments[slot];
                Guard.IsTrue(attachment.Texture is not null);

                var texture = (VulkanTexture)attachment.Texture;
                var mipLevel = attachment.MipLevel;
                var slice = attachment.Slice;
                var imageView = texture.GetView(mipLevel, slice);

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
                var attachment = renderPass.DepthStencilAttachment;

                var texture = (VulkanTexture)attachment.Texture!;
                var mipLevel = attachment.MipLevel;
                var slice = attachment.Slice;

                renderArea.extent.width = Math.Min(renderArea.extent.width, texture.GetWidth(mipLevel));
                renderArea.extent.height = Math.Min(renderArea.extent.height, texture.GetHeight(mipLevel));

                depthAttachment.imageView = texture.GetView(mipLevel, slice);
                depthAttachment.imageLayout = VkImageLayout.DepthAttachmentOptimal; //  //desc.depthStencilAttachment.depthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                depthAttachment.resolveMode = VkResolveModeFlags.NoneKHR;
                depthAttachment.loadOp = attachment.DepthLoadAction.ToVk();
                depthAttachment.storeOp = attachment.DepthStoreAction.ToVk();
                depthAttachment.clearValue.depthStencil = new(attachment.ClearDepth, attachment.ClearStencil);
                renderingInfo.pDepthAttachment = &depthAttachment;

                if (depthStencilFormat.IsStencilFormat())
                {
                    stencilAttachment.imageView = depthAttachment.imageView;
                    stencilAttachment.imageLayout = VkImageLayout.StencilAttachmentOptimal; //  //desc.depthStencilAttachment.depthReadOnly ? VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                    stencilAttachment.resolveMode = VkResolveModeFlags.NoneKHR;
                    stencilAttachment.loadOp = attachment.StencilLoadAction.ToVk();
                    stencilAttachment.storeOp = attachment.StencilStoreAction.ToVk();
                    stencilAttachment.clearValue.depthStencil = new(attachment.ClearDepth, attachment.ClearStencil);
                    renderingInfo.pStencilAttachment = &stencilAttachment;
                }
            }

            renderingInfo.renderArea = renderArea;
            renderingInfo.pColorAttachments = renderingInfo.colorAttachmentCount > 0 ? colorAttachments : null;

            vkCmdBeginRendering(_commandBuffer, &renderingInfo);

            // The viewport and scissor default to cover all of the attachments
            VkViewport viewport = new()
            {
                x = 0.0f,
                y = renderArea.extent.height,
                width = renderArea.extent.width,
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

    public override void SetViewport(in Viewport viewport)
    {
        // Flip viewport to match DirectX coordinate system
        VkViewport vkViewport = new()
        {
            x = viewport.X,
            y = viewport.Height - viewport.Y,
            width = viewport.Width,
            height = -viewport.Height,
            minDepth = viewport.MinDepth,
            maxDepth = viewport.MaxDepth
        };
        vkCmdSetViewport(_commandBuffer, 0, 1, &vkViewport);
    }

    public override void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0)
    {
        if (count == 0)
        {
            count = viewports.Length;
        }
        VkViewport* vkViewports = stackalloc VkViewport[count];

        for (int i = 0; i < count; i++)
        {
            ref readonly Viewport viewport = ref viewports[(int)i];

            vkViewports[i] = new()
            {
                x = viewport.X,
                y = viewport.Height - viewport.Y,
                width = viewport.Width,
                height = -viewport.Height,
                minDepth = viewport.MinDepth,
                maxDepth = viewport.MaxDepth
            };
        }

        vkCmdSetViewport(_commandBuffer, firstViewport: 0, 1, vkViewports);
    }

    public override void SetScissorRect(in Rectangle rect)
    {
        VkRect2D vkRect = new(rect.X, rect.Y, rect.Width, rect.Height);
        vkCmdSetScissor(_commandBuffer, 0, 1, &vkRect);
    }

    public override void SetStencilReference(uint reference)
    {
        vkCmdSetStencilReference(_commandBuffer, VkStencilFaceFlags.FrontAndBack, reference);
    }

    public override void SetBlendColor(Numerics.Color color)
    {
        vkCmdSetBlendConstants(_commandBuffer, (float*)&color);
    }

    public override void SetShadingRate(ShadingRate rate)
    {
        if (_queue.Device.QueryFeatureSupport(Feature.VariableRateShading) && _currentShadingRate != rate)
        {
            _currentShadingRate = rate;

            VkExtent2D fragmentSize;
            switch (rate)
            {
                case ShadingRate.Rate1x1:
                    fragmentSize.width = 1;
                    fragmentSize.height = 1;
                    break;
                case ShadingRate.Rate1x2:
                    fragmentSize.width = 1;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate.Rate2x1:
                    fragmentSize.width = 2;
                    fragmentSize.height = 1;
                    break;
                case ShadingRate.Rate2x2:
                    fragmentSize.width = 2;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate.Rate2x4:
                    fragmentSize.width = 2;
                    fragmentSize.height = 4;
                    break;
                case ShadingRate.Rate4x2:
                    fragmentSize.width = 4;
                    fragmentSize.height = 2;
                    break;
                case ShadingRate.Rate4x4:
                    fragmentSize.width = 4;
                    fragmentSize.height = 4;
                    break;
                default:
                    break;
            }

            var combiner = stackalloc VkFragmentShadingRateCombinerOpKHR[2]
            {
                VkFragmentShadingRateCombinerOpKHR.Keep,
                VkFragmentShadingRateCombinerOpKHR.Keep
            };

            //if (_queue.Device.fragmentShadingRateProperties.fragmentShadingRateNonTrivialCombinerOps == VK_TRUE)
            //{
            //    if (device->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
            //    {
            //        combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
            //    }
            //    if (device->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
            //    {
            //        combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
            //    }
            //}
            //else
            //{
            //    if (device->fragmentShadingRateFeatures.primitiveFragmentShadingRate == VK_TRUE)
            //    {
            //        combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
            //    }
            //    if (device->fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
            //    {
            //        combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
            //    }
            //}

            vkCmdSetFragmentShadingRateKHR(_commandBuffer, &fragmentSize, combiner);
        }
    }

    public override void SetDepthBounds(float minBounds, float maxBounds)
    {
        if (_queue.Device.PhysicalDeviceFeatures2.features.depthBounds)
        {
            vkCmdSetDepthBounds(_commandBuffer, minBounds, maxBounds);
        }
    }

    public override Texture? AcquireSwapChainTexture(SwapChain swapChain)
    {
        var vulkanSwapChain = (VulkanSwapChain)swapChain;

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

        TextureBarrier(vulkanSwapChain.CurrentTexture, ResourceStates.RenderTarget);
        return vulkanSwapChain.CurrentTexture;
    }
    #endregion RenderContext Methods
}
