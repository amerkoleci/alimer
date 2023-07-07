// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE_COMBINER;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_ENDING_ACCESS_TYPE;
using Alimer.Graphics.D3D;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandBuffer : RenderContext
{
    private const int MaxBarriers = 16;

    /// <summary>
    /// Allowed states for <see cref="QueueType.Compute"/>
    /// </summary>
    private static readonly D3D12_RESOURCE_STATES s_ValidComputeResourceStates =
       D3D12_RESOURCE_STATE_UNORDERED_ACCESS
       | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
       | D3D12_RESOURCE_STATE_COPY_DEST
       | D3D12_RESOURCE_STATE_COPY_SOURCE;

    /// <summary>
    /// Allowed states for <see cref="QueueType.Copy"/>
    /// </summary>
    private static readonly D3D12_RESOURCE_STATES s_ValidCopyResourceStates = D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;

    private readonly D3D12CommandQueue _queue;
    private readonly ComPtr<ID3D12CommandAllocator>[] _commandAllocators = new ComPtr<ID3D12CommandAllocator>[MaxFramesInFlight];
    private readonly ComPtr<ID3D12GraphicsCommandList6> _commandList;
    private readonly D3D12_RESOURCE_BARRIER[] _resourceBarriers = new D3D12_RESOURCE_BARRIER[MaxBarriers];
    private uint _numBarriersToFlush;

    private readonly D3D12_VERTEX_BUFFER_VIEW[] _vboViews = new D3D12_VERTEX_BUFFER_VIEW[MaxVertexBufferBindings];

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
           queue.Device.Handle->CreateCommandList1(0, queue.CommandListType, D3D12_COMMAND_LIST_FLAG_NONE,
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

        if (_queue.QueueType != QueueType.Copy)
        {
            //ID3D12DescriptorHeap* heaps[2] = {
            //    device->resourceDescriptorHeap.handle,
            //    device->samplerDescriptorHeap.handle
            //};
            //_commandList.Get()->SetDescriptorHeaps(ALIMER_STATIC_ARRAY_SIZE(heaps), heaps);
        }

        if (_queue.QueueType == QueueType.Graphics)
        {
            for (int i = 0; i < MaxVertexBufferBindings; ++i)
            {
                _vboViews[i] = default;
            }

            RECT* scissorRects = stackalloc RECT[D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX + 1];
            for (int i = 0; i < D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX; ++i)
            {
                scissorRects[i].bottom = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].left = D3D12_VIEWPORT_BOUNDS_MIN;
                scissorRects[i].right = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].top = D3D12_VIEWPORT_BOUNDS_MIN;
            }
            _commandList.Get()->RSSetScissorRects(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX, scissorRects);
        }
    }

    public void TransitionResource(ID3D12GpuResource resource, ResourceStates newState, bool flushImmediate = false)
    {
        ResourceStates oldState = resource.State;
        D3D12_RESOURCE_STATES oldStateD3D12 = resource.State.ToD3D12();
        D3D12_RESOURCE_STATES newStateD3D12 = newState.ToD3D12();

        if (_queue.QueueType == QueueType.Compute)
        {
            Guard.IsTrue((oldStateD3D12 & s_ValidComputeResourceStates) == oldStateD3D12);
            Guard.IsTrue((newStateD3D12 & s_ValidComputeResourceStates) == newStateD3D12);
        }

        if (oldState != newState)
        {
            Guard.IsTrue(_numBarriersToFlush < MaxBarriers, "Exceeded arbitrary limit on buffered barriers");
            ref D3D12_RESOURCE_BARRIER barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];

            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource.Handle;
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldStateD3D12;
            barrierDesc.Transition.StateAfter = newStateD3D12;

            // Check to see if we already started the transition
            if (newState == resource.TransitioningState)
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                resource.TransitioningState = (ResourceStates)uint.MaxValue;
            }
            else
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
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
        ref D3D12_RESOURCE_BARRIER barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.UAV.pResource = resource.Handle;

        if (flushImmediate)
            FlushResourceBarriers();
    }

    public void FlushResourceBarriers()
    {
        if (_numBarriersToFlush > 0)
        {
            fixed (D3D12_RESOURCE_BARRIER* pBarriers = _resourceBarriers)
            {
                _commandList.Get()->ResourceBarrier(_numBarriersToFlush, pBarriers);
            }

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
            _commandList.Get()->SetGraphicsRootSignature(newPipeline.RootSignature);
            _commandList.Get()->IASetPrimitiveTopology(newPipeline.PrimitiveTopology);
        }
        else
        {
            _commandList.Get()->SetComputeRootSignature(newPipeline.RootSignature);
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
        D3D12_RENDER_PASS_RENDER_TARGET_DESC* RTVs = stackalloc D3D12_RENDER_PASS_RENDER_TARGET_DESC[(int)D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC DSV = default;
        D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE;

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
                    RTVs[slot].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction.Clear:
                    RTVs[slot].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[0] = attachment.ClearColor.R;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[1] = attachment.ClearColor.G;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[2] = attachment.ClearColor.B;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[3] = attachment.ClearColor.A;
                    break;
                case LoadAction.Discard:
                    RTVs[slot].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.StoreAction)
            {
                default:
                case StoreAction.Store:
                    RTVs[slot].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    break;
                case StoreAction.Discard:
                    RTVs[slot].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
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

            renderArea.Width = Math.Min(renderArea.Width, (int)texture.GetWidth(mipLevel));
            renderArea.Height = Math.Min(renderArea.Height, (int)texture.GetHeight(mipLevel));

            DSV.cpuDescriptor = texture.GetDSV(mipLevel, slice/*, attachment.depthReadOnly, attachment.stencilReadOnly*/);
            DSV.DepthBeginningAccess.Clear.ClearValue.Format = texture.DxgiFormat;
            DSV.StencilBeginningAccess.Clear.ClearValue.Format = texture.DxgiFormat;

            switch (attachment.DepthLoadAction)
            {
                default:
                case LoadAction.Load:
                    DSV.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction.Clear:
                    DSV.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    DSV.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = attachment.ClearDepth;
                    break;
                case LoadAction.Discard:
                    DSV.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.DepthStoreAction)
            {
                default:
                case StoreAction.Store:
                    DSV.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    break;
                case StoreAction.Discard:
                    DSV.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                    break;
            }

            if (!texture.Format.IsDepthOnlyFormat())
            {
                switch (attachment.StencilLoadAction)
                {
                    default:
                    case LoadAction.Load:
                        DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                        break;

                    case LoadAction.Clear:
                        DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                        DSV.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = (byte)attachment.ClearStencil;
                        break;
                    case LoadAction.Discard:
                        DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                        break;
                }

                switch (attachment.StencilStoreAction)
                {
                    default:
                    case StoreAction.Store:
                        DSV.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                        break;
                    case StoreAction.Discard:
                        DSV.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                        break;
                }
            }
        }

        _commandList.Get()->BeginRenderPass(numRTVS, RTVs,
             hasDepthOrStencil ? &DSV : null,
             renderPassFlags
         );

        // The viewport and scissor default to cover all of the attachments
        D3D12_VIEWPORT viewport = new(0.0f, 0.0f, (float)renderArea.Width, (float)renderArea.Height);
        RECT scissorRect = new(0, 0, renderArea.Width, renderArea.Height);
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
        D3D12Buffer d3d12Buffer = (D3D12Buffer)buffer;

        _vboViews[slot].BufferLocation = d3d12Buffer.GpuAddress + offset;
        _vboViews[slot].SizeInBytes = (uint)(d3d12Buffer.Size - offset);
        _vboViews[slot].StrideInBytes = 0;
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0)
    {
        D3D12Buffer d3d12Buffer = (D3D12Buffer)buffer;

        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = d3d12Buffer.GpuAddress + offset;
        view.SizeInBytes = (uint)(d3d12Buffer.Size - offset);
        view.Format = indexType.ToDxgiFormat();
        _commandList.Get()->IASetIndexBuffer(&view);
    }

    public override void SetViewport(in Viewport viewport)
    {
        D3D12_VIEWPORT d3d12Viewport = new(viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
        _commandList.Get()->RSSetViewports(1, &d3d12Viewport);
    }

    public override void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0)
    {
        if (count == 0)
        {
            count = viewports.Length;
        }
        D3D12_VIEWPORT* d3d12Viewports = stackalloc D3D12_VIEWPORT[count];

        for (int i = 0; i < count; i++)
        {
            ref readonly Viewport viewport = ref viewports[(int)i];

            d3d12Viewports[i] = new D3D12_VIEWPORT(viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
        }
        _commandList.Get()->RSSetViewports((uint)count, d3d12Viewports);
    }

    public override void SetScissorRect(in Rectangle rect)
    {
        RECT scissorRect = new(rect.Left, rect.Top, rect.Right, rect.Bottom);
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

            D3D12_SHADING_RATE d3dRate = D3D12_SHADING_RATE_1X1;
            _queue.Device.WriteShadingRateValue(rate, &d3dRate);

            var combiners = stackalloc D3D12_SHADING_RATE_COMBINER[2]
            {
                D3D12_SHADING_RATE_COMBINER_MAX,
                D3D12_SHADING_RATE_COMBINER_MAX,
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

    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandList.Get()->ExecuteIndirect(_queue.Device.DrawIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandList.Get()->ExecuteIndirect(_queue.Device.DrawIndirectCommandSignature, maxCount,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset);
    }

    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandList.Get()->ExecuteIndirect(_queue.Device.DrawIndexedIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandList.Get()->ExecuteIndirect(_queue.Device.DrawIndexedIndirectCommandSignature, maxCount,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset);
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDraw();

        _commandList.Get()->DispatchMesh(groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer backendBuffer = (D3D12Buffer)indirectBuffer;
        _commandList.Get()->ExecuteIndirect(_queue.Device.DispatchMeshIndirectCommandSignature, 1, backendBuffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandList.Get()->ExecuteIndirect(_queue.Device.DispatchMeshIndirectCommandSignature,
            maxCount,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset);
    }

    private void PrepareDraw()
    {
        if (_currentPipeline!.NumVertexBindings > 0)
        {
            for (uint i = 0; i < _currentPipeline.NumVertexBindings; ++i)
            {
                _vboViews[i].StrideInBytes = _currentPipeline.GetStride(i);
            }

            fixed (D3D12_VERTEX_BUFFER_VIEW* pViews = _vboViews)
            {
                _commandList.Get()->IASetVertexBuffers(0, _currentPipeline.NumVertexBindings, pViews);
            }
        }
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
