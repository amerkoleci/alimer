// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.WebGPU.WebGPUUtils;
using CommunityToolkit.Diagnostics;
using System.Diagnostics;
using System.Drawing;
using Alimer.Utilities;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUCommandBuffer : RenderContext
{
    private readonly WebGPUCommandQueue _queue;
    private WGPUCommandEncoder _encoder;

    private WebGPUPipeline? _currentPipeline;
    private WebGPUPipelineLayout? _currentPipelineLayout;
    private RenderPassDescription _currentRenderPass;

    public WebGPUCommandBuffer(WebGPUCommandQueue queue)
    {
        _queue = queue;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _queue.Device;

    public void Destroy()
    {
    }

    public override void Flush(bool waitForCompletion = false)
    {
        if (_hasLabel)
        {
            PopDebugGroup();
        }

        _queue.Commit(_encoder);
        _encoder = default;
    }

    public void Begin(uint frameIndex, string? label = null)
    {
        base.Reset(frameIndex);
        _currentPipeline = default;
        _currentPipelineLayout = default;
        _currentRenderPass = default;

        WGPUCommandEncoderDescriptor commandEncoderDesc = new()
        {
            nextInChain = null,
        };
        _encoder = wgpuDeviceCreateCommandEncoder(_queue.Device.Handle, &commandEncoderDesc);

        if (!string.IsNullOrEmpty(label))
        {
            _hasLabel = true;
            PushDebugGroup(label);
        }
    }

    public override void PushDebugGroup(string groupLabel)
    {
        wgpuCommandEncoderPushDebugGroup(_encoder, groupLabel);
    }

    public override void PopDebugGroup()
    {
        wgpuCommandEncoderPopDebugGroup(_encoder);
    }

    public override void InsertDebugMarker(string debugLabel)
    {
        wgpuCommandEncoderInsertDebugMarker(_encoder, debugLabel);
    }

    #region ComputeContext Methods
    protected override void SetPipelineCore(Pipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        WebGPUPipeline newPipeline = (WebGPUPipeline)pipeline;

        //wgpuRenderPassEncoderSetPipeline(_commandBuffer, newPipeline.BindPoint, newPipeline.Handle);
        _currentPipeline = newPipeline;
        _currentPipelineLayout = (WebGPUPipelineLayout) newPipeline.Layout;
    }

    protected override void SetBindGroupCore(uint groupIndex, BindGroup group)
    {
        Debug.Assert(_currentPipelineLayout != null);
        Debug.Assert(_currentPipeline != null);

        var backendBindGroup = (WebGPUBindGroup)group;
        //wgpuRenderPassEncoderSetBindGroup();
    }

    protected override void SetPushConstantsCore(uint pushConstantIndex, void* data, uint size)
    {
        //Debug.Assert(size <= device->limits.pushConstantsMaxSize);
        Debug.Assert(_currentPipelineLayout != null);

        //ref readonly VkPushConstantRange range = ref _currentPipelineLayout.GetPushConstantRange(pushConstantIndex);
        //wgpuRenderPassEncoderSetPushConstants(_commandBuffer, _currentPipelineLayout.Handle, range.stageFlags, range.offset, size, data);
    }

    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        //wgpuComputePassEncoderDispatchWorkgroups(groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        WebGPUBuffer backendBuffer = (WebGPUBuffer)indirectBuffer;
        //vkCmdDispatchIndirect(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset);
    }
    #endregion ComputeContext Methods

    #region RenderContext Methods
    protected override void BeginRenderPassCore(in RenderPassDescription renderPass)
    {
        if (!string.IsNullOrEmpty(renderPass.Label))
        {
            PushDebugGroup(renderPass.Label);
        }

        _currentRenderPass = renderPass;
    }

    protected override void EndRenderPassCore()
    {
        if (!string.IsNullOrEmpty(_currentRenderPass.Label))
        {
            PopDebugGroup();
        }
    }

    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
        //WebGPUBuffer backendBuffer = (WebGPUBuffer)buffer;
        //vkCmdBindVertexBuffers(_commandBuffer, slot, 1, &vkBuffer, &offset);
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0)
    {
        //WebGPUBuffer backendBuffer = (WebGPUBuffer)buffer;
        //VkIndexType vkIndexType = (indexType == IndexType.Uint16) ? VkIndexType.Uint16 : VkIndexType.Uint32;
        //vkCmdBindIndexBuffer(_commandBuffer, vulkanBuffer.Handle, offset, vkIndexType);
    }

    public override void SetViewport(in Viewport viewport)
    {
    }

    public override void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0)
    {
        if (count == 0)
        {
            count = viewports.Length;
        }
    }

    public override void SetScissorRect(in Rectangle rect)
    {
    }

    public override void SetStencilReference(uint reference)
    {
    }

    public override void SetBlendColor(Numerics.Color color)
    {
    }

    public override void SetShadingRate(ShadingRate rate)
    {
        throw new GraphicsException("WebGPU: SetShadingRate is not supported");
    }

    public override void SetDepthBounds(float minBounds, float maxBounds)
    {
        throw new GraphicsException("WebGPU: SetDepthBounds is not supported");
    }

    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
    {
    }

    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance)
    {
    }

    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
    }

    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
    }

    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
    }

    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
    }

    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
    }

    public override void Present(SwapChain swapChain)
    {
        WebGPUSwapChain backendSwapChain = (WebGPUSwapChain)swapChain;

        _queue.QueuePresent(backendSwapChain);
    }
    #endregion RenderContext Methods
}
