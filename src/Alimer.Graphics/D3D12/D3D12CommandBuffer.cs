// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using CommunityToolkit.Diagnostics;
using Win32;
using Win32.Graphics.Direct3D12;
using static Alimer.Graphics.Constants;
using static Win32.Apis;
using static Win32.Graphics.Direct3D12.Apis;
using D3DResourceStates = Win32.Graphics.Direct3D12.ResourceStates;
using D3DShadingRate = Win32.Graphics.Direct3D12.ShadingRate;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandBuffer : RenderContext
{
    private const int MaxBarriers = 16;

    /// <summary>
    /// Allowed states for <see cref="QueueType.Compute"/>
    /// </summary>
    private static readonly D3DResourceStates s_ValidComputeResourceStates =
       D3DResourceStates.UnorderedAccess
       | D3DResourceStates.NonPixelShaderResource
       | D3DResourceStates.CopyDest
       | D3DResourceStates.CopySource;

    /// <summary>
    /// Allowed states for <see cref="QueueType.Copy"/>
    /// </summary>
    private static readonly D3DResourceStates s_ValidCopyResourceStates = D3DResourceStates.CopyDest | D3DResourceStates.CopySource;

    private readonly D3D12CommandQueue _queue;
    private readonly ComPtr<ID3D12CommandAllocator>[] _commandAllocators = new ComPtr<ID3D12CommandAllocator>[MaxFramesInFlight];
    private readonly ComPtr<ID3D12GraphicsCommandList6> _commandList;
    private readonly ResourceBarrier[] _resourceBarriers = new ResourceBarrier[MaxBarriers];
    private int _numBarriersToFlush;

    private D3D12Pipeline? _currentPipeline;
    private RenderPassDescription _currentRenderPass;

    public D3D12CommandBuffer(D3D12CommandQueue queue)
    {
        _queue = queue;

        for (uint i = 0; i < MaxFramesInFlight; ++i)
        {
            ThrowIfFailed(
                queue.Device.Handle->CreateCommandAllocator(queue.CommandListType,
                __uuidof<ID3D12CommandAllocator>(), _commandAllocators[i].GetVoidAddressOf())
            );
        }

        ThrowIfFailed(
           queue.Device.Handle->CreateCommandList1(0, queue.CommandListType, CommandListFlags.None,
            __uuidof<ID3D12GraphicsCommandList6>(), _commandList.GetVoidAddressOf()
        ));
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _queue.Device;

    public ID3D12GraphicsCommandList6* CommandList => _commandList;

    public void Destroy()
    {
        for (int i = 0; i < _commandAllocators.Length; ++i)
        {
            _commandAllocators[i].Dispose();
        }

        _commandList.Dispose();
    }

    public override void Flush(bool waitForCompletion = false)
    {
        if (_hasLabel)
        {
            PopDebugGroup();
        }

        _ = _queue.Commit(this);
    }

    public void Begin(uint frameIndex, string? label = null)
    {
        base.Reset(frameIndex);
        _currentPipeline = default;
        _currentRenderPass = default;

        // Start the command list in a default state:
        ThrowIfFailed(_commandAllocators[frameIndex].Get()->Reset());
        ThrowIfFailed(_commandList.Get()->Reset(_commandAllocators[frameIndex].Get(), null));

        if (!string.IsNullOrEmpty(label))
        {
            _hasLabel = true;
            PushDebugGroup(label);
        }
    }

    public void TransitionResource(ID3D12GpuResource resource, ResourceStates newState, bool flushImmediate = false)
    {
        ResourceStates oldState = resource.State;
        D3DResourceStates oldStateD3D12 = resource.State.ToD3D12();
        D3DResourceStates newStateD3D12 = newState.ToD3D12();

        if (_queue.QueueType == QueueType.Compute)
        {
            Guard.IsTrue((oldStateD3D12 & s_ValidComputeResourceStates) == oldStateD3D12);
            Guard.IsTrue((newStateD3D12 & s_ValidComputeResourceStates) == newStateD3D12);
        }

        if (oldState != newState)
        {
            Guard.IsTrue(_numBarriersToFlush < MaxBarriers, "Exceeded arbitrary limit on buffered barriers");
            ref ResourceBarrier barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];

            barrierDesc.Type = ResourceBarrierType.Transition;
            barrierDesc.Transition.pResource = resource.Handle;
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldStateD3D12;
            barrierDesc.Transition.StateAfter = newStateD3D12;

            // Check to see if we already started the transition
            if (newState == resource.TransitioningState)
            {
                barrierDesc.Flags = ResourceBarrierFlags.EndOnly;
                resource.TransitioningState = (ResourceStates)uint.MaxValue;
            }
            else
            {
                barrierDesc.Flags = ResourceBarrierFlags.None;
            }

            resource.State = newState;
        }
        else if (newState == ResourceStates.UnorderedAccess)
        {
            InsertUAVBarrier(resource, flushImmediate);
        }

        if (flushImmediate || _numBarriersToFlush == MaxBarriers)
        {
            FlushResourceBarriers();
        }
    }

    public void InsertUAVBarrier(ID3D12GpuResource resource, bool flushImmediate = false)
    {
        Guard.IsTrue(_numBarriersToFlush < _resourceBarriers.Length, "Exceeded arbitrary limit on buffered barriers");
        ref ResourceBarrier barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];

        barrierDesc.Type = ResourceBarrierType.Uav;
        barrierDesc.Flags = ResourceBarrierFlags.None;
        barrierDesc.UAV.pResource = resource.Handle;

        if (flushImmediate)
            FlushResourceBarriers();
    }

    public void FlushResourceBarriers()
    {
        if (_numBarriersToFlush > 0)
        {
            _commandList.Get()->ResourceBarrier(_numBarriersToFlush, _resourceBarriers);

            _numBarriersToFlush = 0;
        }
    }

    public override void PushDebugGroup(string groupLabel)
    {
        // TODO: Use Pix3 (WinPixEventRuntime)

        var bufferSize = PixHelpers.CalculateNoArgsEventSize(groupLabel);
        var buffer = stackalloc byte[bufferSize];
        PixHelpers.FormatNoArgsEventToBuffer(buffer, PixHelpers.PixEventType.PIXEvent_BeginEvent_NoArgs, 0, groupLabel);
        _commandList.Get()->BeginEvent(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
    }

    public override void PopDebugGroup()
    {
        _commandList.Get()->EndEvent();
    }

    public override void InsertDebugMarker(string debugLabel)
    {
        var bufferSize = PixHelpers.CalculateNoArgsEventSize(debugLabel);
        var buffer = stackalloc byte[bufferSize];
        PixHelpers.FormatNoArgsEventToBuffer(buffer, PixHelpers.PixEventType.PIXEvent_SetMarker_NoArgs, 0, debugLabel);
        _commandList.Get()->SetMarker(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
    }

    #region ComputeContext Methods
    protected override void SetPipelineCore(Pipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        D3D12Pipeline newPipeline = (D3D12Pipeline)pipeline;

        _commandList.Get()->SetPipelineState(newPipeline.Handle);

        if (newPipeline.PipelineType == PipelineType.Render)
        {
            //_commandList.Get()->SetGraphicsRootSignature(newPipelineLayout->handle);
            //_commandList.Get()->IASetPrimitiveTopology(newPipeline.primitiveTopology);
        }
        else
        {
            //_commandList.Get()->SetComputeRootSignature(newPipelineLayout->handle);
        }

        _currentPipeline = newPipeline;
    }

    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        _commandList.Get()->Dispatch(groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandList.Get()->ExecuteIndirect(_queue.Device.DispatchIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }
    #endregion ComputeContext Methods

    #region RenderContext Methods
    protected override void BeginRenderPassCore(in RenderPassDescription renderPass)
    {
        if (!string.IsNullOrEmpty(renderPass.Label))
        {
            PushDebugGroup(renderPass.Label);
        }

        Size renderArea = new(int.MaxValue, int.MaxValue);
        uint numRTVS = 0;
        RenderPassRenderTargetDescription* RTVs = stackalloc RenderPassRenderTargetDescription[(int)D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
        RenderPassDepthStencilDescription DSV = default;
        RenderPassFlags renderPassFlags = RenderPassFlags.None;

        bool hasDepthOrStencil = renderPass.DepthStencilAttachment.Texture != null;
        PixelFormat depthStencilFormat = renderPass.DepthStencilAttachment.Texture != null ? renderPass.DepthStencilAttachment.Texture.Format : PixelFormat.Undefined;

        for (int slot = 0; slot < renderPass.ColorAttachments.Length; slot++)
        {
            ref readonly RenderPassColorAttachment attachment = ref renderPass.ColorAttachments[slot];
            Guard.IsTrue(attachment.Texture is not null);

            D3D12Texture texture = (D3D12Texture)attachment.Texture;
            int mipLevel = attachment.MipLevel;
            int slice = attachment.Slice;

            renderArea.Width = Math.Min(renderArea.Width, (int)texture.GetWidth(mipLevel));
            renderArea.Height = Math.Min(renderArea.Height, (int)texture.GetHeight(mipLevel));

            RTVs[slot].cpuDescriptor = texture.GetRTV(mipLevel, slice, texture.DxgiFormat);
            RTVs[slot].BeginningAccess.Clear.ClearValue.Format = texture.DxgiFormat;

            switch (attachment.LoadAction)
            {
                default:
                case LoadAction.Load:
                    RTVs[slot].BeginningAccess.Type = RenderPassBeginningAccessType.Preserve;
                    break;

                case LoadAction.Clear:
                    RTVs[slot].BeginningAccess.Type = RenderPassBeginningAccessType.Clear;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[0] = attachment.ClearColor.R;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[1] = attachment.ClearColor.G;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[2] = attachment.ClearColor.B;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[3] = attachment.ClearColor.A;
                    break;
                case LoadAction.Discard:
                    RTVs[slot].BeginningAccess.Type = RenderPassBeginningAccessType.Discard;
                    break;
            }

            switch (attachment.StoreAction)
            {
                default:
                case StoreAction.Store:
                    RTVs[slot].EndingAccess.Type = RenderPassEndingAccessType.Preserve;
                    break;
                case StoreAction.Discard:
                    RTVs[slot].EndingAccess.Type = RenderPassEndingAccessType.Discard;
                    break;
            }

            // Barrier -> D3D12_RESOURCE_STATE_RENDER_TARGET
            TransitionResource(texture, ResourceStates.RenderTarget, true);

            ++numRTVS;
        }

        if (hasDepthOrStencil)
        {
            RenderPassDepthStencilAttachment attachment = renderPass.DepthStencilAttachment;

            var texture = (D3D12Texture)attachment.Texture!;
            int mipLevel = attachment.MipLevel;
            int slice = attachment.Slice;

            //renderArea.Width = Math.Min(renderArea.Width, texture.GetWidth(mipLevel));
            //renderArea.Height = Math.Min(renderArea.Height, texture.GetHeight(mipLevel));
        }

        _commandList.Get()->BeginRenderPass(numRTVS, RTVs,
             hasDepthOrStencil ? &DSV : null,
             renderPassFlags
         );

        // The viewport and scissor default to cover all of the attachments
        Win32.Numerics.Viewport viewport = new((float)renderArea.Width, (float)renderArea.Height);
        Win32.Numerics.Rect scissorRect = new(0, 0, renderArea.Width, renderArea.Height);
        _commandList.Get()->RSSetViewports(1, &viewport);
        _commandList.Get()->RSSetScissorRects(1, &scissorRect);

        _currentRenderPass = renderPass;
    }

    protected override void EndRenderPassCore()
    {
        if (!string.IsNullOrEmpty(_currentRenderPass.Label))
        {
            PopDebugGroup();
        }

        _commandList.Get()->EndRenderPass();
    }

    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0)
    {
    }

    public override void SetViewport(in Viewport viewport)
    {
        Win32.Numerics.Viewport d3d12Viewport = new(viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
        _commandList.Get()->RSSetViewports(1, &d3d12Viewport);
    }

    public override void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0)
    {
        if (count == 0)
        {
            count = viewports.Length;
        }
        Win32.Numerics.Viewport* d3d12Viewports = stackalloc Win32.Numerics.Viewport[count];

        for (int i = 0; i < count; i++)
        {
            ref readonly Viewport viewport = ref viewports[(int)i];

            d3d12Viewports[i] = new Win32.Numerics.Viewport(viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
        }
        _commandList.Get()->RSSetViewports((uint)count, d3d12Viewports);
    }

    public override void SetScissorRect(in Rectangle rect)
    {
        Win32.Numerics.Rect scissorRect = new(rect.Left, rect.Top, rect.Right, rect.Bottom);
        _commandList.Get()->RSSetScissorRects(1, &scissorRect);
    }

    public override void SetStencilReference(uint reference)
    {
        _commandList.Get()->OMSetStencilRef(reference);
    }

    public override void SetBlendColor(Numerics.Color color)
    {
        _commandList.Get()->OMSetBlendFactor((float*)&color);
    }

    public override void SetShadingRate(ShadingRate rate)
    {
        if (_queue.Device.QueryFeatureSupport(Feature.VariableRateShading) && _currentShadingRate != rate)
        {
            _currentShadingRate = rate;

            D3DShadingRate d3dRate = D3DShadingRate.Rate1x1;
            _queue.Device.WriteShadingRateValue(rate, &d3dRate);

            var combiners = stackalloc ShadingRateCombiner[2]
            {
                ShadingRateCombiner.Max,
                ShadingRateCombiner.Max,
            };
            _commandList.Get()->RSSetShadingRate(d3dRate, combiners);
        }
    }

    public override void SetDepthBounds(float minBounds, float maxBounds)
    {
        if (_queue.Device.QueryFeatureSupport(Feature.DepthBoundsTest))
        {
            _commandList.Get()->OMSetDepthBounds(minBounds, maxBounds);
        }
    }

    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
    {
        PrepareDraw();

        _commandList.Get()->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance)
    {
        PrepareDraw();

        _commandList.Get()->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDraw();

        //vkCmdDrawMeshTasksEXT(_commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer vulkanBuffer = (D3D12Buffer)indirectBuffer;
        //vkCmdDrawMeshTasksIndirectEXT(_commandBuffer, vulkanBuffer.Handle, indirectBufferOffset, 1, (uint)sizeof(VkDispatchIndirectCommand));
    }

    private void PrepareDraw()
    {
    }

    public override Texture? AcquireSwapChainTexture(SwapChain swapChain)
    {
        var d3dSwapChain = (D3D12SwapChain)swapChain;

        D3D12Texture swapChainTexture = d3dSwapChain.CurrentBackBufferTexture;

        _queue.QueuePresent(d3dSwapChain);
        return swapChainTexture;
    }
    #endregion RenderContext Methods
}
