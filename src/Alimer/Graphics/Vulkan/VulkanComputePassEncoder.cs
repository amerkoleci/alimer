// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanComputePassEncoder : ComputePassEncoder
{
    private readonly VulkanCommandBuffer _commandBuffer;
    private readonly VkDeviceApi _deviceApi;
    private VulkanComputePipeline? _currentPipeline;

    public VulkanComputePassEncoder(VulkanCommandBuffer commandBuffer, VkDeviceApi deviceApi)
    {
        _commandBuffer = commandBuffer;
        _deviceApi = deviceApi;
    }

    public override GraphicsDevice Device => _commandBuffer.Device;

    public void Begin(in ComputePassDescriptor descriptor)
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
    }

    public override void EndEncoding()
    {
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
    protected override void SetPipelineCore(ComputePipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        VulkanComputePipeline backendPipeline = (VulkanComputePipeline)pipeline;

        _deviceApi.vkCmdBindPipeline(_commandBuffer.Handle, VK_PIPELINE_BIND_POINT_COMPUTE, backendPipeline.Handle);
        _currentPipeline = backendPipeline;
    }

    /// <inheritdoc/>
    protected override void SetConstantBufferCore(uint slot, GPUBuffer buffer, ulong offset)
    {
        _commandBuffer.SetConstantBuffer(slot, buffer, offset);
    }

    /// <inheritdoc/>
    protected override void SetPushConstantsCore(void* data, uint size, uint offset)
    {
        _commandBuffer.SetPushConstants(data, size, offset);
    }

    private void PrepareDispatch()
    {
        _commandBuffer.FlushBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    /// <inheritdoc/>
    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDispatch();

        _deviceApi.vkCmdDispatch(_commandBuffer.Handle, groupCountX, groupCountY, groupCountZ);
    }

    /// <inheritdoc/>
    protected override void DispatchIndirectCore(GPUBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDispatch();

        VulkanBuffer vulkanBuffer = (VulkanBuffer)indirectBuffer;
        _deviceApi.vkCmdDispatchIndirect(_commandBuffer.Handle, vulkanBuffer.Handle, indirectBufferOffset);
    }

    /// <inheritdoc/>
    protected override void CopyBufferToBufferCore(GPUBuffer sourceBuffer, GPUBuffer destinationBuffer)
    {
        VulkanBuffer backendSrcBuffer = sourceBuffer.ToVk();
        VulkanBuffer backendDestBuffer = destinationBuffer.ToVk();

        _commandBuffer.BufferBarrier(backendSrcBuffer, BufferStates.CopySource);
        _commandBuffer.BufferBarrier(backendDestBuffer, BufferStates.CopyDest);
        _commandBuffer.CommitBarriers();

        VkBufferCopy copy = new()
        {
            srcOffset = 0,
            dstOffset = 0,
            size = Math.Min(backendSrcBuffer.Size, backendDestBuffer.Size)
        };

        _deviceApi.vkCmdCopyBuffer(_commandBuffer.Handle, backendSrcBuffer.Handle, backendDestBuffer.Handle, 1, &copy);
    }

    /// <inheritdoc/>
    protected override void CopyBufferToBufferCore(GPUBuffer sourceBuffer, ulong sourceOffset, GPUBuffer destinationBuffer, ulong destinationOffset, ulong size)
    {

        VulkanBuffer backendSrcBuffer = sourceBuffer.ToVk();
        VulkanBuffer backendDestBuffer = destinationBuffer.ToVk();

        _commandBuffer.BufferBarrier(backendSrcBuffer, BufferStates.CopySource);
        _commandBuffer.BufferBarrier(backendDestBuffer, BufferStates.CopyDest);
        _commandBuffer.CommitBarriers();

        VkBufferCopy copy = new()
        {
            srcOffset = sourceOffset,
            dstOffset = destinationOffset,
            size = size
        };

        _deviceApi.vkCmdCopyBuffer(_commandBuffer.Handle, backendSrcBuffer.Handle, backendDestBuffer.Handle, 1, &copy);
    }
}
