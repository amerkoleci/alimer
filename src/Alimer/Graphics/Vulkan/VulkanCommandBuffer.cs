// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandBuffer : CommandBuffer
{
    private const uint MaxBarrierCount = 16;

    private readonly VulkanCommandQueue _queue;
    private readonly VulkanBindlessManager _bindlessManager;
    private readonly VkInstanceApi _instanceApi;
    private readonly VkDeviceApi _deviceApi;
    private readonly VkCommandPool[] _commandPools;
    private readonly VkCommandBuffer[] _commandBuffers;
    private VkCommandBuffer _commandBuffer; // recording command buffer

    private uint _memoryBarrierCount;
    private uint _bufferBarrierCount;
    private uint _imageBarrierCount;
    private readonly VkMemoryBarrier2[] _memoryBarriers = new VkMemoryBarrier2[MaxBarrierCount];
    private readonly VkImageMemoryBarrier2[] _imageBarriers = new VkImageMemoryBarrier2[MaxBarrierCount];
    private readonly VkBufferMemoryBarrier2[] _bufferBarriers = new VkBufferMemoryBarrier2[MaxBarrierCount];

    protected readonly DescriptorBindingTable _bindingTable = new();
    private DescriptorBindingDirtyFlags _bindingDirtyFlags;
    private int _numBoundBindGroups;
    private readonly VkDescriptorSet[] _descriptorSets = new VkDescriptorSet[8];
    private readonly VulkanRenderPassEncoder _renderPassEncoder;
    private readonly VulkanComputePassEncoder _computePassEncoder;

    public VulkanCommandBuffer(VulkanCommandQueue queue)
    {
        _queue = queue;
        _bindlessManager = queue.VkDevice.BindlessManager;
        _instanceApi = queue.VkDevice.InstanceApi;
        _deviceApi = queue.VkDevice.DeviceApi;

        _commandPools = new VkCommandPool[queue.VkDevice.MaxFramesInFlight];
        _commandBuffers = new VkCommandBuffer[queue.VkDevice.MaxFramesInFlight];
        for (uint i = 0; i < queue.VkDevice.MaxFramesInFlight; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = new()
            {
                flags = VkCommandPoolCreateFlags.Transient,
                queueFamilyIndex = queue.VkDevice.GetQueueFamilyIndex(queue.QueueType)
            };

            _deviceApi.vkCreateCommandPool(&poolInfo, null, out _commandPools[i]).CheckResult();

            VkCommandBufferAllocateInfo commandBufferInfo = new()
            {
                commandPool = _commandPools[i],
                level = VkCommandBufferLevel.Primary,
                commandBufferCount = 1
            };
            _deviceApi.vkAllocateCommandBuffer(&commandBufferInfo, out _commandBuffers[i]).CheckResult();

            //binderPools[i].Init(device);
        }

        _renderPassEncoder = new VulkanRenderPassEncoder(this, _deviceApi);
        _computePassEncoder = new VulkanComputePassEncoder(this, _deviceApi);
        DynamicContantBufferCount = Math.Min(Constants.DynamicContantBufferCount, queue.VkDevice.VkAdapter.Properties2.properties.limits.maxDescriptorSetUniformBuffersDynamic);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _queue.VkDevice;
    public VulkanGraphicsDevice VkDevice => _queue.VkDevice;
    public VkCommandBuffer Handle => _commandBuffer;
    public uint DynamicContantBufferCount { get; }

    public void Destroy()
    {
        for (int i = 0; i < _commandPools.Length; ++i)
        {
            //vkFreeCommandBuffers(Queue.Device.Handle, _commandPools[i], 1, &commandBuffers[i]);
            _deviceApi.vkDestroyCommandPool(_commandPools[i]);
            //binderPools[i].Shutdown();
        }
    }

    public VkCommandBuffer End()
    {
        //for (auto & surface : presentSurfaces)
        //{
        //    VulkanTexture* swapChainTexture = surface->backbufferTextures[surface->backBufferIndex];
        //    TextureBarrier(swapChainTexture, TextureLayout::Present, 0, 1, 0, 1);
        //}
        CommitBarriers();

        if (_hasLabel)
        {
            PopDebugGroup();
        }

        _deviceApi.vkEndCommandBuffer(_commandBuffer).CheckResult();
        return _commandBuffer;
    }

    public void Begin(uint frameIndex, Utf8ReadOnlyString label = default)
    {
        base.Reset(frameIndex);
        _memoryBarrierCount = 0;
        _bufferBarrierCount = 0;
        _imageBarrierCount = 0;
        Array.Clear(_memoryBarriers, 0, _memoryBarriers.Length);
        Array.Clear(_bufferBarriers, 0, _bufferBarriers.Length);
        Array.Clear(_imageBarriers, 0, _imageBarriers.Length);
        _bindingDirtyFlags = DescriptorBindingDirtyFlags.All;
        _numBoundBindGroups = 0;
        Array.Clear(_descriptorSets, 0, _descriptorSets.Length);

        _deviceApi.vkResetCommandPool(_commandPools[frameIndex], 0).CheckResult();
        _commandBuffer = _commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo = new()
        {
            flags = VkCommandBufferUsageFlags.OneTimeSubmit,
            pInheritanceInfo = null // Optional
        };
        _deviceApi.vkBeginCommandBuffer(_commandBuffer, &beginInfo).CheckResult();

        if (_queue.QueueType == CommandQueueType.Graphics)
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
            _deviceApi.vkCmdSetStencilReference(_commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, ~0u);

            if (_queue.VkDevice.VkAdapter.Features2.features.depthBounds == true)
            {
                _deviceApi.vkCmdSetDepthBounds(_commandBuffer, 0.0f, 1.0f);
            }

            // Silence validation about uninitialized stride:
            //const VkDeviceSize zero = {};
            //vkCmdBindVertexBuffers2(commandBuffer, 0, 1, &nullBuffer, &zero, &zero, &zero);

            if (_queue.VkDevice.VkAdapter.FragmentShadingRateFeatures.pipelineFragmentShadingRate)
            {
                VkExtent2D fragmentSize = new(1, 1);
                VkFragmentShadingRateCombinerOpKHR* combiner = stackalloc VkFragmentShadingRateCombinerOpKHR[2];
                combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
                combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

                _deviceApi.vkCmdSetFragmentShadingRateKHR(_commandBuffer, &fragmentSize, combiner);
            }
        }

        if (!label.IsEmpty)
        {
            _hasLabel = true;
            PushDebugGroup(label);
        }
    }

    public override void PushDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        if (!_queue.VkDevice.DebugUtils)
            return;

        VkDebugUtilsLabelEXT label = new()
        {
            pLabelName = (byte*)groupLabel
        };
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        _queue.VkDevice.InstanceApi.vkCmdBeginDebugUtilsLabelEXT(_commandBuffer, &label);
    }

    public override void PopDebugGroup()
    {
        if (!_queue.VkDevice.DebugUtils)
            return;

        _instanceApi.vkCmdEndDebugUtilsLabelEXT(_commandBuffer);
    }

    public override void InsertDebugMarker(Utf8ReadOnlyString debugLabel)
    {
        if (!_queue.VkDevice.DebugUtils)
            return;

        VkDebugUtilsLabelEXT label = new()
        {
            pLabelName = (byte*)debugLabel
        };
        label.color[0] = 0.0f;
        label.color[1] = 0.0f;
        label.color[2] = 0.0f;
        label.color[3] = 1.0f;
        _instanceApi.vkCmdInsertDebugUtilsLabelEXT(_commandBuffer, &label);
    }

    protected override ComputePassEncoder BeginComputePassCore(in ComputePassDescriptor descriptor)
    {
        _computePassEncoder.Begin(in descriptor);
        return _computePassEncoder;
    }

    protected override RenderPassEncoder BeginRenderPassCore(in RenderPassDescriptor descriptor)
    {
        _renderPassEncoder.Begin(in descriptor);
        return _renderPassEncoder;
    }

    public override void Present(SwapChain swapChain)
    {
        VulkanSwapChain backendSwapChain = (VulkanSwapChain)swapChain;

        TextureBarrier(backendSwapChain.CurrentTexture, TextureLayout.Present, 0, 1, 0, 1);
        _queue.QueuePresent(backendSwapChain);
        backendSwapChain.NeedAcquire = true;
    }

    public void EndEncoding()
    {
        _encoderActive = false;
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

    public void TextureBarrier(VulkanTextureView view, TextureLayout newLayout)
    {
        VulkanTexture backendTexture = (VulkanTexture)view.Texture;
        TextureBarrier(backendTexture, newLayout,
            view.BaseMipLevel, view.MipLevelCount,
            view.BaseArrayLayer, view.ArrayLayerCount,
            view.Aspect
        );
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

    public void SetBindGroup(int groupIndex, BindGroup bindGroup)
    {
        VulkanBindGroup backendBindGroup = (VulkanBindGroup)bindGroup;
        if (_descriptorSets[groupIndex] != backendBindGroup.Handle)
        {
            _bindingDirtyFlags = DescriptorBindingDirtyFlags.All;
            _descriptorSets[groupIndex] = backendBindGroup.Handle;
            _numBoundBindGroups = Math.Max(groupIndex + 1, _numBoundBindGroups);
        }
    }

    public void SetConstantBuffer(uint slot, GPUBuffer buffer, ulong offset)
    {
        if (_bindingTable.ConstantBuffer[slot] != buffer)
        {
            _bindingTable.ConstantBuffer[slot] = buffer;
            _bindingDirtyFlags |= DescriptorBindingDirtyFlags.Descriptor;
        }

        if (_bindingTable.ConstantBufferOffset[slot] != offset)
        {
            _bindingTable.ConstantBufferOffset[slot] = offset;
            if (slot < DynamicContantBufferCount)
            {
                _bindingDirtyFlags |= DescriptorBindingDirtyFlags.SetOrOffset;
            }
            else
            {
                _bindingDirtyFlags |= DescriptorBindingDirtyFlags.Descriptor;
            }
        }
    }

    public void SetPushConstants(void* data, uint size, uint offset)
    {
        Debug.Assert(size <= _queue.VkDevice.VkAdapter.Properties2.properties.limits.maxPushConstantsSize);

        _deviceApi.vkCmdPushConstants(_commandBuffer.Handle,
            _bindlessManager.PipelineLayout,
            VK_SHADER_STAGE_ALL,
            offset,
            size,
            data);
    }

    public void FlushBindGroups(VkPipelineBindPoint bindPoint)
    {
        if (_bindingDirtyFlags == DescriptorBindingDirtyFlags.None)
            return;

        // Bind bindless descriptor sets
        Span<uint> uniformBufferDynamicOffsets = stackalloc uint[(int)DynamicContantBufferCount];

        if ((_bindingDirtyFlags & DescriptorBindingDirtyFlags.Descriptor) != 0)
        {
            VkDescriptorBufferInfo* bufferInfo = stackalloc VkDescriptorBufferInfo[(int)DynamicContantBufferCount];
            VkWriteDescriptorSet* bufferInfoWrite = stackalloc VkWriteDescriptorSet[(int)DynamicContantBufferCount];

            VkDescriptorBufferInfo nullBufferInfo = new()
            {
                buffer = _queue.VkDevice.NullBuffer,
                range = VK_WHOLE_SIZE
            };

            for (uint i = 0; i < _bindingTable.ConstantBuffer.Length; ++i)
            {
                GPUBuffer? buffer = _bindingTable.ConstantBuffer[i];
                ref VkWriteDescriptorSet write = ref bufferInfoWrite[i];
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write.dstBinding = VulkanRegisterShift.ContantBuffer + i;
                write.descriptorCount = 1;
                write.dstSet = _bindlessManager.BindingsDescriptorSet;
                write.pBufferInfo = &bufferInfo[i];

                if (buffer is not null)
                {
                    VulkanBuffer backendBuffer = buffer.ToVk();
                    bufferInfo[i].buffer = backendBuffer.Handle;
                    bufferInfo[i].offset = i < DynamicContantBufferCount ? 0 : _bindingTable.ConstantBufferOffset[i];
                    bufferInfo[i].range = MathUtilities.AlignUp(buffer.Size - _bindingTable.ConstantBufferOffset[i], _queue.VkDevice.Limits.MinConstantBufferOffsetAlignment);
                }
                else
                {
                    write.pBufferInfo = &nullBufferInfo;
                }
            }

            _deviceApi.vkUpdateDescriptorSets(
                (uint)_bindingTable.ConstantBuffer.Length,
                bufferInfoWrite,
                0, null
            );
        }

        _bindlessManager.Bind(_commandBuffer, bindPoint, uniformBufferDynamicOffsets);
        _bindingDirtyFlags = DescriptorBindingDirtyFlags.None;
    }

    protected override void BeginQueryCore(QueryHeap queryHeap, uint index)
    {
        VulkanQueryHeap backendQueryHeap = queryHeap.ToVk();

        switch (queryHeap.Type)
        {
            case QueryType.Timestamp:
                break;

            case QueryType.Occlusion:
                _deviceApi.vkCmdBeginQuery(_commandBuffer, backendQueryHeap.Handle, index, VK_QUERY_CONTROL_PRECISE_BIT);
                break;

            default:
                _deviceApi.vkCmdBeginQuery(_commandBuffer, backendQueryHeap.Handle, index, 0);
                break;
        }
    }

    protected override void EndQueryCore(QueryHeap queryHeap, uint index)
    {
        VulkanQueryHeap backendQueryHeap = queryHeap.ToVk();

        switch (queryHeap.Type)
        {
            case QueryType.Timestamp:
                _deviceApi.vkCmdWriteTimestamp2(_commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, backendQueryHeap.Handle, index);
                break;

            default:
                _deviceApi.vkCmdEndQuery(_commandBuffer, backendQueryHeap.Handle, index);
                break;
        }
    }

    protected override void ResolveQueryCore(QueryHeap queryHeap, uint index, uint count, GPUBuffer destinationBuffer, ulong destinationOffset)
    {
        VulkanQueryHeap backendQueryHeap = queryHeap.ToVk();
        VulkanBuffer backendDestBuffer = destinationBuffer.ToVk();

        VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

        switch (backendQueryHeap.Type)
        {
            case QueryType.BinaryOcclusion:
                flags |= VK_QUERY_RESULT_PARTIAL_BIT;
                break;
            default:
                break;
        }

        // Need to be called outside Render Pass Scope and Video Coding Scope
        _deviceApi.vkCmdCopyQueryPoolResults(
            _commandBuffer,
            backendQueryHeap.Handle,
            index,
            count,
            backendDestBuffer.Handle,
            destinationOffset,
            backendQueryHeap.QueryResultSize,
            flags
        );
    }

    protected override void ResetQueryCore(QueryHeap queryHeap, uint index, uint count)
    {
        VulkanQueryHeap backendQueryHeap = queryHeap.ToVk();
        // Need to be called outside Render Pass Scope and Video Coding Scope
        _deviceApi.vkCmdResetQueryPool(_commandBuffer, backendQueryHeap.Handle, index, count);
    }

    [Flags]
    enum DescriptorBindingDirtyFlags : uint
    {
        None = 0,
        Descriptor = 1 << 1,
        SetOrOffset = 1 << 2,

        All = uint.MaxValue,
    }
}
