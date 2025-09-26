// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandBuffer : RenderContext
{
    private const uint MaxBarrierCount = 16;

    private readonly VulkanCommandQueue _queue;
    private readonly VkInstanceApi _instanceApi;
    private readonly VkDeviceApi _deviceApi;
    private readonly VkCommandPool[] _commandPools = new VkCommandPool[MaxFramesInFlight];
    private readonly VkCommandBuffer[] _commandBuffers = new VkCommandBuffer[MaxFramesInFlight];
    private VkCommandBuffer _commandBuffer; // recording command buffer

    private uint _memoryBarrierCount;
    private uint _bufferBarrierCount;
    private uint _imageBarrierCount;
    private readonly VkMemoryBarrier2[] _memoryBarriers = new VkMemoryBarrier2[MaxBarrierCount];
    private readonly VkImageMemoryBarrier2[] _imageBarriers = new VkImageMemoryBarrier2[MaxBarrierCount];
    private readonly VkBufferMemoryBarrier2[] _bufferBarriers = new VkBufferMemoryBarrier2[MaxBarrierCount];

    private VulkanPipeline? _currentPipeline;
    private VulkanPipelineLayout? _currentPipelineLayout;
    private RenderPassDescription _currentRenderPass;

    private bool _bindGroupsDirty;
    private uint _numBoundBindGroups;
    private readonly VkDescriptorSet[] _descriptorSets = new VkDescriptorSet[MaxBindGroups];

    public VulkanCommandBuffer(VulkanCommandQueue queue)
    {
        _queue = queue;
        _instanceApi = queue.VkDevice.InstanceApi;
        _deviceApi = queue.VkDevice.DeviceApi;

        for (uint i = 0; i < MaxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = new()
            {
                flags = VkCommandPoolCreateFlags.Transient,
                queueFamilyIndex = queue.VkDevice.GetQueueFamily(queue.QueueType)
            };

            _deviceApi.vkCreateCommandPool(queue.VkDevice.Handle, &poolInfo, null, out _commandPools[i]).CheckResult();

            VkCommandBufferAllocateInfo commandBufferInfo = new()
            {
                commandPool = _commandPools[i],
                level = VkCommandBufferLevel.Primary,
                commandBufferCount = 1
            };
            _deviceApi.vkAllocateCommandBuffer(queue.VkDevice.Handle, &commandBufferInfo, out _commandBuffers[i]).CheckResult();

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
            _deviceApi.vkDestroyCommandPool(_queue.VkDevice.Handle, _commandPools[i]);
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
        _currentPipeline = default;
        _currentPipelineLayout = default;
        _currentRenderPass = default;
        _memoryBarrierCount = 0;
        _bufferBarrierCount = 0;
        _imageBarrierCount = 0;
        Array.Clear(_memoryBarriers, 0, _memoryBarriers.Length);
        Array.Clear(_bufferBarriers, 0, _bufferBarriers.Length);
        Array.Clear(_imageBarriers, 0, _imageBarriers.Length);
        _bindGroupsDirty = false;
        _numBoundBindGroups = 0;
        Array.Clear(_descriptorSets, 0, _descriptorSets.Length);

        _deviceApi.vkResetCommandPool(_queue.VkDevice.Handle, _commandPools[frameIndex], 0).CheckResult();
        _commandBuffer = _commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo = new()
        {
            flags = VkCommandBufferUsageFlags.OneTimeSubmit,
            pInheritanceInfo = null // Optional
        };
        _deviceApi.vkBeginCommandBuffer(_commandBuffer, &beginInfo).CheckResult();

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
            _deviceApi.vkCmdSetScissor(_commandBuffer, 0, 16, scissors);

            _deviceApi.vkCmdSetBlendConstants(_commandBuffer, 1.0f, 1.0f, 1.0f, 1.0f);
            _deviceApi.vkCmdSetStencilReference(_commandBuffer, VkStencilFaceFlags.FrontAndBack, ~0u);

            if (_queue.VkDevice.VkAdapter.Features2.features.depthBounds == true)
            {
                _deviceApi.vkCmdSetDepthBounds(_commandBuffer, 0.0f, 1.0f);
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
        if (!_queue.VkDevice.DebugUtils)
            return;

        byte* pLabelName = VkStringInterop.ConvertToUnmanaged(groupLabel);
        VkDebugUtilsLabelEXT label = new()
        {
            pLabelName = pLabelName
        };
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        _queue.VkDevice.InstanceApi.vkCmdBeginDebugUtilsLabelEXT(_commandBuffer, &label);
        VkStringInterop.Free(pLabelName);
    }

    public override void PopDebugGroup()
    {
        if (!_queue.VkDevice.DebugUtils)
            return;

        _instanceApi.vkCmdEndDebugUtilsLabelEXT(_commandBuffer);
    }

    public override void InsertDebugMarker(string debugLabel)
    {
        if (!_queue.VkDevice.DebugUtils)
            return;

        byte* pLabelName = VkStringInterop.ConvertToUnmanaged(debugLabel);
        VkDebugUtilsLabelEXT label = new()
        {
            pLabelName = pLabelName
        };
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        _instanceApi.vkCmdInsertDebugUtilsLabelEXT(_commandBuffer, &label);
        VkStringInterop.Free(pLabelName);
    }

    public void BufferBarrier(VulkanBuffer buffer, BufferStates newState)
    {
        if (buffer.CurrentState == newState)
            return;

        VkBufferStateMapping before = ConvertBufferState(buffer.CurrentState);
        VkBufferStateMapping after = ConvertBufferState(newState);

        ref VkBufferMemoryBarrier2 barrier = ref _bufferBarriers[_bufferBarrierCount++];
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.srcStageMask = before.StageFlags;
        barrier.srcAccessMask = before.AccessMask;
        barrier.dstStageMask = after.StageFlags;
        barrier.dstAccessMask = after.AccessMask;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = buffer.Handle;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        if (_bufferBarrierCount == MaxBarrierCount)
        {
            CommitBarriers();
        }

        buffer.CurrentState = newState;
    }

    public void TextureBarrier(VulkanTexture texture, TextureLayout newLayout,
        uint baseMiplevel,
        uint levelCount,
        uint baseArrayLayer,
        uint layerCount,
        TextureAspect aspect = TextureAspect.All)
    {
        TextureLayout currentLayout = texture.GetTextureLayout(baseMiplevel, baseArrayLayer);
        if (currentLayout == newLayout)
            return;

        bool depthOnlyFormat = texture.Format.IsDepthOnlyFormat();
        VkImageLayoutMapping mappingBefore = ConvertImageLayout(currentLayout, depthOnlyFormat);
        VkImageLayoutMapping mappingAfter = ConvertImageLayout(newLayout, depthOnlyFormat);

        VkImageMemoryBarrier2 barrier = new()
        {
            srcStageMask = mappingBefore.StageFlags,
            srcAccessMask = mappingBefore.AccessMask,
            dstStageMask = mappingAfter.StageFlags,
            dstAccessMask = mappingAfter.AccessMask,
            oldLayout = mappingBefore.Layout,
            newLayout = mappingAfter.Layout,
            srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            image = texture.Handle
        };
        barrier.subresourceRange.aspectMask = texture.VkFormat.GetImageAspectFlags(aspect);
        barrier.subresourceRange.baseMipLevel = baseMiplevel;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount = layerCount;
        _imageBarriers[_imageBarrierCount++] = barrier;

        if (_imageBarrierCount == MaxBarrierCount)
        {
            CommitBarriers();
        }

        texture.SetTextureLayout(newLayout, baseMiplevel, levelCount, baseArrayLayer, layerCount);
    }

    public void CommitBarriers()
    {
        if (_memoryBarrierCount > 0
            || _bufferBarrierCount > 0
            || _imageBarrierCount > 0)
        {
            fixed (VkMemoryBarrier2* pMemoryBarriers = _memoryBarriers)
            {
                fixed (VkBufferMemoryBarrier2* pBufferMemoryBarriers = _bufferBarriers)
                {
                    fixed (VkImageMemoryBarrier2* pImageMemoryBarriers = _imageBarriers)
                    {
                        VkDependencyInfo dependencyInfo = new()
                        {
                            memoryBarrierCount = _memoryBarrierCount,
                            pMemoryBarriers = pMemoryBarriers,
                            bufferMemoryBarrierCount = _bufferBarrierCount,
                            pBufferMemoryBarriers = pBufferMemoryBarriers,
                            imageMemoryBarrierCount = _imageBarrierCount,
                            pImageMemoryBarriers = pImageMemoryBarriers
                        };
                        _deviceApi.vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
                    }
                }
            }

            _memoryBarrierCount = 0;
            _bufferBarrierCount = 0;
            _imageBarrierCount = 0;
        }
    }

    #region ComputeContext Methods
    protected override void SetPipelineCore(Pipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        VulkanPipeline newPipeline = (VulkanPipeline)pipeline;

        _deviceApi.vkCmdBindPipeline(_commandBuffer, newPipeline.BindPoint, newPipeline.Handle);
        _currentPipeline = newPipeline;
        _currentPipelineLayout = (VulkanPipelineLayout)newPipeline.Layout;
    }

    protected override void SetBindGroupCore(uint groupIndex, BindGroup bindGroup)
    {
        var backendBindGroup = (VulkanBindGroup)bindGroup;
        if (_descriptorSets[groupIndex] != backendBindGroup.Handle)
        {
            _bindGroupsDirty = true;
            _descriptorSets[groupIndex] = backendBindGroup.Handle;
            _numBoundBindGroups = Math.Max(groupIndex + 1, _numBoundBindGroups);
        }
    }

    protected override void SetPushConstantsCore(uint pushConstantIndex, void* data, uint size)
    {
        Debug.Assert(size <= _queue.Device.Adapter.Limits.MaxPushConstantsSize);
        Debug.Assert(_currentPipelineLayout != null);

        ref readonly VkPushConstantRange range = ref _currentPipelineLayout.GetPushConstantRange(pushConstantIndex);
        _deviceApi.vkCmdPushConstants(_commandBuffer, _currentPipelineLayout.Handle, range.stageFlags, range.offset, size, data);
    }

    private void PrepareDispatch()
    {
        FlushBindGroups();
    }

    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDispatch();

        _deviceApi.vkCmdDispatch(_commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDispatch();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDispatchIndirect(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset);
    }
    #endregion ComputeContext Methods

    #region RenderContext Methods
    protected override void BeginRenderPassCore(in RenderPassDescription renderPass)
    {
        if (!string.IsNullOrEmpty(renderPass.Label))
        {
            PushDebugGroup(renderPass.Label);
        }

        VkRect2D renderArea = new(0, 0,
            _queue.VkDevice.VkAdapter.Properties2.properties.limits.maxFramebufferWidth,
            _queue.VkDevice.VkAdapter.Properties2.properties.limits.maxFramebufferHeight
            );

        VkRenderingInfo renderingInfo = new()
        {
            layerCount = 1,
            viewMask = 0
        };

        VkRenderingAttachmentInfo* colorAttachments = stackalloc VkRenderingAttachmentInfo[MaxColorAttachments];
        VkRenderingAttachmentInfo depthAttachment = new();
        VkRenderingAttachmentInfo stencilAttachment = new();

        PixelFormat depthStencilFormat = renderPass.DepthStencilAttachment.Texture != null ? renderPass.DepthStencilAttachment.Texture.Format : PixelFormat.Undefined;
        bool hasDepthOrStencil = depthStencilFormat != PixelFormat.Undefined;

        for (int slot = 0; slot < renderPass.ColorAttachments.Length; slot++)
        {
            ref readonly RenderPassColorAttachment attachment = ref renderPass.ColorAttachments[slot];
            Guard.IsTrue(attachment.Texture is not null);

            VulkanTexture texture = (VulkanTexture)attachment.Texture;
            uint mipLevel = attachment.MipLevel;
            uint slice = attachment.Slice;
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
                clearValue = new VkClearValue(attachment.ClearColor.Red, attachment.ClearColor.Green, attachment.ClearColor.Blue, attachment.ClearColor.Alpha)
            };

            TextureBarrier(texture, TextureLayout.RenderTarget, mipLevel, 1u, slice, 1u);
        }

        if (hasDepthOrStencil)
        {
            RenderPassDepthStencilAttachment attachment = renderPass.DepthStencilAttachment;

            VulkanTexture texture = (VulkanTexture)attachment.Texture!;
            uint mipLevel = attachment.MipLevel;
            uint slice = attachment.Slice;

            renderArea.extent.width = Math.Min(renderArea.extent.width, texture.GetWidth(mipLevel));
            renderArea.extent.height = Math.Min(renderArea.extent.height, texture.GetHeight(mipLevel));

            depthAttachment.imageView = texture.GetView(mipLevel, slice);
            depthAttachment.imageLayout = VkImageLayout.DepthAttachmentOptimal; //  //desc.depthStencilAttachment.depthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VkResolveModeFlags.None;
            depthAttachment.loadOp = attachment.DepthLoadAction.ToVk();
            depthAttachment.storeOp = attachment.DepthStoreAction.ToVk();
            depthAttachment.clearValue.depthStencil = new(attachment.DepthClearValue, attachment.StencilClearValue);
            renderingInfo.pDepthAttachment = &depthAttachment;

            if (depthStencilFormat.IsStencilFormat())
            {
                stencilAttachment.imageView = depthAttachment.imageView;
                stencilAttachment.imageLayout = VkImageLayout.StencilAttachmentOptimal; //  //desc.depthStencilAttachment.depthReadOnly ? VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment.resolveMode = VkResolveModeFlags.None;
                stencilAttachment.loadOp = attachment.StencilLoadAction.ToVk();
                stencilAttachment.storeOp = attachment.StencilStoreAction.ToVk();
                stencilAttachment.clearValue.depthStencil = new(attachment.DepthClearValue, attachment.StencilClearValue);
                renderingInfo.pStencilAttachment = &stencilAttachment;
            }

            TextureBarrier(texture, TextureLayout.DepthWrite, mipLevel, 1u, slice, 1u);
        }
        CommitBarriers();

        renderingInfo.renderArea = renderArea;
        renderingInfo.pColorAttachments = renderingInfo.colorAttachmentCount > 0 ? colorAttachments : null;

        _deviceApi.vkCmdBeginRendering(_commandBuffer, &renderingInfo);

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
        _deviceApi.vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

        VkRect2D scissorRect = new(0, 0, renderArea.extent.width, renderArea.extent.height);
        _deviceApi.vkCmdSetScissor(_commandBuffer, 0, 1, &scissorRect);

        _currentRenderPass = renderPass;
    }

    protected override void EndRenderPassCore()
    {
        _deviceApi.vkCmdEndRendering(_commandBuffer);

        if (!string.IsNullOrEmpty(_currentRenderPass.Label))
        {
            PopDebugGroup();
        }
    }

    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
        VulkanBuffer vulkanBuffer = (VulkanBuffer)buffer;
        VkBuffer vkBuffer = vulkanBuffer.Handle;

        _deviceApi.vkCmdBindVertexBuffers(_commandBuffer, slot, 1, &vkBuffer, &offset);
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0)
    {
        VulkanBuffer vulkanBuffer = (VulkanBuffer)buffer;
        VkIndexType vkIndexType = (indexType == IndexType.Uint16) ? VkIndexType.Uint16 : VkIndexType.Uint32;

        _deviceApi.vkCmdBindIndexBuffer(_commandBuffer, vulkanBuffer.Handle, offset, vkIndexType);
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
        _deviceApi.vkCmdSetViewport(_commandBuffer, 0, 1, &vkViewport);
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

        _deviceApi.vkCmdSetViewport(_commandBuffer, firstViewport: 0, 1, vkViewports);
    }

    public override void SetScissorRect(in System.Drawing.Rectangle rect)
    {
        VkRect2D vkRect = new(rect.X, rect.Y, (uint)rect.Width, (uint)rect.Height);
        _deviceApi.vkCmdSetScissor(_commandBuffer, 0, 1, &vkRect);
    }

    public override void SetStencilReference(uint reference)
    {
        _deviceApi.vkCmdSetStencilReference(_commandBuffer, VkStencilFaceFlags.FrontAndBack, reference);
    }

    public override void SetBlendColor(in Color color)
    {
        fixed (Color* colorPtr = &color)
            _deviceApi.vkCmdSetBlendConstants(_commandBuffer, (float*)colorPtr);
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

    public override void SetDepthBounds(float minBounds, float maxBounds)
    {
        if (_queue.VkDevice.VkAdapter.Features2.features.depthBounds)
        {
            _deviceApi.vkCmdSetDepthBounds(_commandBuffer, minBounds, maxBounds);
        }
    }

    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
    {
        PrepareDraw();

        _deviceApi.vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance)
    {
        PrepareDraw();

        _deviceApi.vkCmdDrawIndexed(_commandBuffer, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawIndirect(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDrawIndirectCommand));
    }

    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;

        _deviceApi.vkCmdDrawIndirectCount(_commandBuffer,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDrawIndirectCommand)
            );
    }

    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawIndexedIndirect(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDrawIndexedIndirectCommand));
    }

    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;

        _deviceApi.vkCmdDrawIndexedIndirectCount(_commandBuffer,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDrawIndexedIndirectCommand)
            );
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDraw();

        _deviceApi.vkCmdDrawMeshTasksEXT(_commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDrawMeshTasksIndirectEXT(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDispatchIndirectCommand));
    }

    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        VulkanBuffer backendIndirectBuffer = (VulkanBuffer)indirectBuffer;
        VulkanBuffer backendCountBuffer = (VulkanBuffer)countBuffer;
        _deviceApi.vkCmdDrawMeshTasksIndirectCountEXT(_commandBuffer,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset,
            maxCount, (uint)sizeof(VkDispatchIndirectCommand)
            );
    }

    private void PrepareDraw()
    {
        FlushBindGroups();
    }

    private void FlushBindGroups()
    {
        Debug.Assert(_currentPipelineLayout != null);
        Debug.Assert(_currentPipeline != null);

        if (!_bindGroupsDirty)
            return;

        _deviceApi.vkCmdBindDescriptorSets(
            _commandBuffer,
            _currentPipeline.BindPoint,
            _currentPipelineLayout.Handle,
            0u,
            (uint)_currentPipelineLayout.BindGroupLayoutCount,
            (VkDescriptorSet*)Unsafe.AsPointer(ref MemoryMarshal.GetArrayDataReference(_descriptorSets))
        );
        _bindGroupsDirty = false;
    }

    public override void Present(SwapChain swapChain)
    {
        VulkanSwapChain backendSwapChain = (VulkanSwapChain)swapChain;

        TextureBarrier(backendSwapChain.CurrentTexture, TextureLayout.Present, 0, 1, 0, 1);
        _queue.QueuePresent(backendSwapChain);
        backendSwapChain.NeedAcquire = true;
    }
    #endregion RenderContext Methods
}
