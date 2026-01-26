// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanRenderPassEncoder : RenderPassEncoder
{
    private readonly VulkanCommandBuffer _commandBuffer;
    private readonly VkDeviceApi _deviceApi;
    private VulkanRenderPipeline? _currentPipeline;

    public VulkanRenderPassEncoder(VulkanCommandBuffer commandBuffer, VkDeviceApi deviceApi)
    {
        _commandBuffer = commandBuffer;
        _deviceApi = deviceApi;
    }

    public override GraphicsDevice Device => _commandBuffer.Device;

    public void Begin(in RenderPassDescriptor descriptor)
    {
        _currentPipeline = default;

        if (!descriptor.Label.IsEmpty)
        {
            _commandBuffer.PushDebugGroup(descriptor.Label);
            _hasLabel = true;
        }
        else
        {
            _hasLabel = false;
        }

        VkRect2D renderArea = new(0, 0,
            _commandBuffer.VkDevice.VkAdapter.Properties2.properties.limits.maxFramebufferWidth,
            _commandBuffer.VkDevice.VkAdapter.Properties2.properties.limits.maxFramebufferHeight
            );

        uint layerCount = _commandBuffer.VkDevice.VkAdapter.Properties2.properties.limits.maxFramebufferLayers;

        VkRenderingInfo renderingInfo = new()
        {
            layerCount = 1,
            viewMask = 0
        };

        VkRenderingAttachmentInfo* colorAttachments = stackalloc VkRenderingAttachmentInfo[MaxColorAttachments];
        VkRenderingAttachmentInfo depthAttachment = new();
        VkRenderingAttachmentInfo stencilAttachment = new();

        PixelFormat depthStencilFormat = descriptor.DepthStencilAttachment.View != null ? descriptor.DepthStencilAttachment.View.Format : PixelFormat.Undefined;
        bool hasDepthOrStencil = depthStencilFormat != PixelFormat.Undefined;

        for (int slot = 0; slot < descriptor.ColorAttachments.Length; slot++)
        {
            ref readonly RenderPassColorAttachment attachment = ref descriptor.ColorAttachments[slot];
            Guard.IsTrue(attachment.View is not null);

            VulkanTextureView view = (VulkanTextureView)attachment.View;
            VkImageView imageView = view.Handle;

            renderArea.extent.width = Math.Min(renderArea.extent.width, view.Width);
            renderArea.extent.height = Math.Min(renderArea.extent.height, view.Height);
            layerCount = Math.Min(layerCount, view.ArrayLayerCount);

            ref VkRenderingAttachmentInfo attachmentInfo = ref colorAttachments[renderingInfo.colorAttachmentCount++];
            attachmentInfo = new()
            {
                imageView = imageView,
                imageLayout = VkImageLayout.ColorAttachmentOptimal,
                loadOp = attachment.LoadAction.ToVk(),
                storeOp = attachment.StoreAction.ToVk(),
                clearValue = new VkClearValue(attachment.ClearColor.Red, attachment.ClearColor.Green, attachment.ClearColor.Blue, attachment.ClearColor.Alpha)
            };

            if (attachment.ResolveView is not null)
            {
                VulkanTextureView resolveView = (VulkanTextureView)attachment.ResolveView;
                attachmentInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                attachmentInfo.resolveImageView = resolveView.Handle;
                attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            _commandBuffer.TextureBarrier(view, TextureLayout.RenderTarget);
        }

        if (hasDepthOrStencil)
        {
            RenderPassDepthStencilAttachment attachment = descriptor.DepthStencilAttachment;

            VulkanTextureView view = (VulkanTextureView)attachment.View!;

            renderArea.extent.width = Math.Min(renderArea.extent.width, view.Width);
            renderArea.extent.height = Math.Min(renderArea.extent.height, view.Height);
            layerCount = Math.Min(layerCount, view.ArrayLayerCount);

            depthAttachment.imageView = view.Handle;
            depthAttachment.imageLayout = attachment.DepthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.loadOp = attachment.DepthLoadAction.ToVk();
            depthAttachment.storeOp = attachment.DepthStoreAction.ToVk();
            depthAttachment.clearValue.depthStencil = new(attachment.DepthClearValue, attachment.StencilClearValue);
            renderingInfo.pDepthAttachment = &depthAttachment;

            if (depthStencilFormat.IsStencilFormat())
            {
                stencilAttachment.imageView = depthAttachment.imageView;
                stencilAttachment.imageLayout = attachment.StencilReadOnly ? VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment.resolveMode = VkResolveModeFlags.None;
                stencilAttachment.loadOp = attachment.StencilLoadAction.ToVk();
                stencilAttachment.storeOp = attachment.StencilStoreAction.ToVk();
                stencilAttachment.clearValue.depthStencil = new(attachment.DepthClearValue, attachment.StencilClearValue);
                renderingInfo.pStencilAttachment = &stencilAttachment;
            }

            _commandBuffer.TextureBarrier(view,
                (attachment.DepthReadOnly || attachment.StencilReadOnly) ? TextureLayout.DepthRead : TextureLayout.DepthWrite
                );
        }
        _commandBuffer.CommitBarriers();

        renderingInfo.layerCount = layerCount;
        renderingInfo.renderArea = renderArea;
        renderingInfo.pColorAttachments = renderingInfo.colorAttachmentCount > 0 ? colorAttachments : null;

        _deviceApi.vkCmdBeginRendering(_commandBuffer.Handle, &renderingInfo);

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
        _deviceApi.vkCmdSetViewport(_commandBuffer.Handle, 0, 1, &viewport);

        VkRect2D scissorRect = new(0, 0, renderArea.extent.width, renderArea.extent.height);
        _deviceApi.vkCmdSetScissor(_commandBuffer.Handle, 0, 1, &scissorRect);
    }

    public override void EndEncoding()
    {
        _deviceApi.vkCmdEndRendering(_commandBuffer.Handle);

        if (_hasLabel)
        {
            PopDebugGroup();
            _hasLabel = false;
        }

        _commandBuffer.EndEncoding();
        Reset();
    }

    /// <inheritdoc/>
    public override void PushDebugGroup(Utf8ReadOnlyString groupLabel) => _commandBuffer.PushDebugGroup(groupLabel);

    /// <inheritdoc/>
    public override void PopDebugGroup() => _commandBuffer.PopDebugGroup();

    /// <inheritdoc/>
    public override void InsertDebugMarker(Utf8ReadOnlyString debugLabel) => _commandBuffer.InsertDebugMarker(debugLabel);

    /// <inheritdoc/>
    protected override void SetPipelineCore(RenderPipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        VulkanRenderPipeline backendPipeline = (VulkanRenderPipeline)pipeline;
        _commandBuffer.SetPipelineLayout(backendPipeline.VkLayout);

        _deviceApi.vkCmdBindPipeline(_commandBuffer.Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, backendPipeline.Handle);
        _currentPipeline = backendPipeline;
    }

    /// <inheritdoc/>
    protected override void SetBindGroupCore(int groupIndex, BindGroup bindGroup)
    {
        _commandBuffer.SetBindGroup(groupIndex, bindGroup);
    }

    /// <inheritdoc/>
    protected override void SetPushConstantsCore(uint pushConstantIndex, void* data, int size)
    {
        _commandBuffer.SetPushConstants(pushConstantIndex, data, size);
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
        _deviceApi.vkCmdSetViewport(_commandBuffer.Handle, 0, 1, &vkViewport);
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

        _deviceApi.vkCmdSetViewport(_commandBuffer.Handle, firstViewport: 0, 1, vkViewports);
    }

    public override void SetScissorRect(in RectI rect)
    {
        VkRect2D vkRect = new(rect.X, rect.Y, (uint)rect.Width, (uint)rect.Height);
        _deviceApi.vkCmdSetScissor(_commandBuffer.Handle, 0, 1, &vkRect);
    }

    public override void SetStencilReference(uint reference)
    {
        _deviceApi.vkCmdSetStencilReference(_commandBuffer.Handle, VkStencilFaceFlags.FrontAndBack, reference);
    }

    public override void SetBlendColor(in Color color)
    {
        fixed (Color* colorPtr = &color)
            _deviceApi.vkCmdSetBlendConstants(_commandBuffer.Handle, (float*)colorPtr);
    }

    public override void SetShadingRate(ShadingRate rate)
    {
#if TODO
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

            if (_queue.Device.FragmentShadingRateProperties.fragmentShadingRateNonTrivialCombinerOps)
            {
                if (_queue.Device.FragmentShadingRateFeatures.primitiveFragmentShadingRate)
                {
                    combiner[0] = VkFragmentShadingRateCombinerOpKHR.Max;
                }
                if (_queue.Device.FragmentShadingRateFeatures.attachmentFragmentShadingRate)
                {
                    combiner[1] = VkFragmentShadingRateCombinerOpKHR.Max;
                }
            }
            else
            {
                if (_queue.Device.FragmentShadingRateFeatures.primitiveFragmentShadingRate)
                {
                    combiner[0] = VkFragmentShadingRateCombinerOpKHR.Replace;
                }
                if (_queue.Device.FragmentShadingRateFeatures.attachmentFragmentShadingRate)
                {
                    combiner[1] = VkFragmentShadingRateCombinerOpKHR.Replace;
                }
            }

            _deviceApi.vkCmdSetFragmentShadingRateKHR(_commandBuffer, &fragmentSize, combiner);
        } 
#endif
    }

    protected override void SetDepthBoundsCore(float minBounds, float maxBounds)
    {
        _deviceApi.vkCmdSetDepthBounds(_commandBuffer.Handle, minBounds, maxBounds);
    }

    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
        VulkanBuffer vulkanBuffer = (VulkanBuffer)buffer;
        VkBuffer vkBuffer = vulkanBuffer.Handle;

        _deviceApi.vkCmdBindVertexBuffers(_commandBuffer.Handle, slot, 1, &vkBuffer, &offset);
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexFormat format, ulong offset = 0)
    {
        VulkanBuffer vulkanBuffer = (VulkanBuffer)buffer;
        VkIndexType vkIndexType = (format == IndexFormat.UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

        _deviceApi.vkCmdBindIndexBuffer(_commandBuffer.Handle, vulkanBuffer.Handle, offset, vkIndexType);
    }
    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
    {
        PrepareDraw();

        _deviceApi.vkCmdDraw(_commandBuffer.Handle, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance)
    {
        PrepareDraw();

        _deviceApi.vkCmdDrawIndexed(_commandBuffer.Handle, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawIndirect(_commandBuffer.Handle, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDrawIndirectCommand));
    }

    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;

        _deviceApi.vkCmdDrawIndirectCount(_commandBuffer.Handle,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDrawIndirectCommand)
            );
    }

    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawIndexedIndirect(_commandBuffer.Handle, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDrawIndexedIndirectCommand));
    }

    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;

        _deviceApi.vkCmdDrawIndexedIndirectCount(_commandBuffer.Handle,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDrawIndexedIndirectCommand)
            );
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDraw();

        _deviceApi.vkCmdDrawMeshTasksEXT(_commandBuffer.Handle, groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawMeshTasksIndirectEXT(_commandBuffer.Handle, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDispatchIndirectCommand));
    }

    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;
        _deviceApi.vkCmdDrawMeshTasksIndirectCountEXT(_commandBuffer.Handle,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDispatchIndirectCommand)
            );
    }

    private void PrepareDraw()
    {
        _commandBuffer.FlushBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

}
