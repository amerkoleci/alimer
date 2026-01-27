// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE_COMBINER;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_ENDING_ACCESS_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOLVE_MODE;
using CommunityToolkit.Diagnostics;
using System.Diagnostics;
using Alimer.Utilities;
using System.Runtime.CompilerServices;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12RenderPassEncoder : RenderPassEncoder
{
    private readonly D3D12CommandBuffer _commandBuffer;
    private D3D12RenderPipeline? _currentPipeline;
    private readonly D3D12_VERTEX_BUFFER_VIEW[] _vboViews = new D3D12_VERTEX_BUFFER_VIEW[MaxVertexBufferBindings];
    private int _subresourceParamsCount = 0;
    private readonly D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS[] _subresourceParams = new D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];

    public D3D12RenderPassEncoder(D3D12CommandBuffer commandBuffer)
    {
        _commandBuffer = commandBuffer;
    }

    public override GraphicsDevice Device => _commandBuffer.Device;

    public void Begin(in RenderPassDescriptor descriptor)
    {
        for (int i = 0; i < MaxVertexBufferBindings; ++i)
        {
            _vboViews[i] = default;
        }

        _currentPipeline = default;
        _subresourceParamsCount = 0;

        if (!descriptor.Label.IsEmpty)
        {
            _commandBuffer.PushDebugGroup(descriptor.Label);
            _hasLabel = true;
        }
        else
        {
            _hasLabel = false;
        }


        SizeI renderArea = new(int.MaxValue, int.MaxValue);
        uint numRTVS = 0;
        D3D12_RENDER_PASS_RENDER_TARGET_DESC* RTVs = stackalloc D3D12_RENDER_PASS_RENDER_TARGET_DESC[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC DSV = default;
        D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE;

        bool hasDepthOrStencil = descriptor.DepthStencilAttachment.View != null;
        PixelFormat depthStencilFormat = descriptor.DepthStencilAttachment.View != null ? descriptor.DepthStencilAttachment.View.Format : PixelFormat.Undefined;

        for (int slot = 0; slot < descriptor.ColorAttachments.Length; slot++)
        {
            ref readonly RenderPassColorAttachment attachment = ref descriptor.ColorAttachments[slot];
            Guard.IsTrue(attachment.View is not null);

            D3D12TextureView view = (D3D12TextureView)attachment.View;

            renderArea.Width = Math.Min(renderArea.Width, (int)view.Width);
            renderArea.Height = Math.Min(renderArea.Height, (int)view.Height);

            RTVs[slot].cpuDescriptor = view.RTV;
            RTVs[slot].BeginningAccess.Clear.ClearValue.Format = view.RTVFormat;

            switch (attachment.LoadAction)
            {
                default:
                case LoadAction.Load:
                    RTVs[slot].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction.Clear:
                    RTVs[slot].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[0] = attachment.ClearColor.Red;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[1] = attachment.ClearColor.Green;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[2] = attachment.ClearColor.Blue;
                    RTVs[slot].BeginningAccess.Clear.ClearValue.Color[3] = attachment.ClearColor.Alpha;
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

            if (attachment.ResolveView is not null)
            {
                D3D12TextureView resolveView = (D3D12TextureView)attachment.ResolveView;

                RTVs[slot].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;

                RTVs[slot].EndingAccess.Resolve.pSrcResource = ((D3D12Texture)view.Texture).Handle;
                RTVs[slot].EndingAccess.Resolve.pDstResource = ((D3D12Texture)resolveView.Texture).Handle;
                RTVs[slot].EndingAccess.Resolve.SubresourceCount = 1u;

                _subresourceParams[_subresourceParamsCount++] = EndingAccessResolveSubresourceParameters(resolveView);

                RTVs[slot].EndingAccess.Resolve.pSubresourceParameters = (D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS*)Unsafe.AsPointer(ref UnsafeUtilities.GetReferenceUnsafe(_subresourceParams, _subresourceParamsCount));
                RTVs[slot].EndingAccess.Resolve.Format = resolveView.RTVFormat;
                RTVs[slot].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;

                // Clear or preserve the resolve source.
                if (attachment.StoreAction == StoreAction.Discard)
                {
                    RTVs[slot].EndingAccess.Resolve.PreserveResolveSource = false;
                }
                else if (attachment.StoreAction == StoreAction.Store)
                {
                    RTVs[slot].EndingAccess.Resolve.PreserveResolveSource = true;
                }
            }

            // Barrier -> D3D12_RESOURCE_STATE_RENDER_TARGET
            _commandBuffer.TextureBarrier(view, TextureLayout.RenderTarget);

            ++numRTVS;
        }

        if (hasDepthOrStencil)
        {
            RenderPassDepthStencilAttachment attachment = descriptor.DepthStencilAttachment;

            D3D12TextureView view = (D3D12TextureView)attachment.View!;


            renderArea.Width = Math.Min(renderArea.Width, (int)view.Width);
            renderArea.Height = Math.Min(renderArea.Height, (int)view.Height);

            DSV.cpuDescriptor = (attachment.DepthReadOnly || attachment.StencilReadOnly) ? view.GetDSV(attachment.DepthReadOnly, attachment.StencilReadOnly) : view.DSV;
            DSV.DepthBeginningAccess.Clear.ClearValue.Format = view.DSVFormat;

            switch (attachment.DepthLoadAction)
            {
                default:
                case LoadAction.Load:
                    DSV.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction.Clear:
                    DSV.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    DSV.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = attachment.DepthClearValue;
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

            if (!view.Format.IsDepthOnlyFormat())
            {
                DSV.StencilBeginningAccess.Clear.ClearValue.Format = view.DSVFormat;

                switch (attachment.StencilLoadAction)
                {
                    default:
                    case LoadAction.Load:
                        DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                        break;

                    case LoadAction.Clear:
                        DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                        DSV.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = attachment.StencilClearValue;
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
            else
            {
                DSV.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                DSV.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            }

            if (attachment.DepthReadOnly)
            {
                renderPassFlags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
            }

            if (attachment.StencilReadOnly)
            {
                renderPassFlags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
            }

            // View barrier
            _commandBuffer.TextureBarrier(view, (attachment.DepthReadOnly || attachment.StencilReadOnly) ? TextureLayout.DepthRead : TextureLayout.DepthWrite);
        }
        _commandBuffer.CommitBarriers();

        _commandBuffer.CommandList->BeginRenderPass(numRTVS, RTVs,
             hasDepthOrStencil ? &DSV : null,
             renderPassFlags
         );

        // The viewport and scissor default to cover all of the attachments
        D3D12_VIEWPORT viewport = new(0.0f, 0.0f, (float)renderArea.Width, (float)renderArea.Height);
        RECT scissorRect = new(0, 0, renderArea.Width, renderArea.Height);
        _commandBuffer.CommandList->RSSetViewports(1, &viewport);
        _commandBuffer.CommandList->RSSetScissorRects(1, &scissorRect);
    }

    public override void EndEncoding()
    {
        _commandBuffer.CommandList->EndRenderPass();

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

    // <inheritdoc/>
    protected override void SetPipelineCore(RenderPipeline pipeline)
    {
        if (_currentPipeline == pipeline)
            return;

        D3D12RenderPipeline backendPipeline = (D3D12RenderPipeline)pipeline;
        _commandBuffer.SetPipelineLayout(backendPipeline.D3DLayout);

        _commandBuffer.CommandList->SetPipelineState(backendPipeline.Handle);
        _commandBuffer.CommandList->SetGraphicsRootSignature(backendPipeline.RootSignature);
        _commandBuffer.CommandList->IASetPrimitiveTopology(backendPipeline.D3DPrimitiveTopology);

        _currentPipeline = backendPipeline;
    }

    /// <inheritdoc/>
    protected override void SetBindGroupCore(int groupIndex, BindGroup bindGroup, Span<uint> dynamicBufferOffsets)
    {
        _commandBuffer.SetBindGroup(groupIndex, bindGroup, dynamicBufferOffsets);
    }

    /// <inheritdoc/>
    protected override void SetPushConstantsCore(uint pushConstantIndex, void* data, int size)
    {
        Debug.Assert(_currentPipeline != null);

        uint rootParameterIndex = _currentPipeline.D3DLayout.PushConstantsBaseIndex + pushConstantIndex;
        int num32BitValuesToSet = size / 4;

        _commandBuffer.CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, (uint)num32BitValuesToSet, data, 0);
    }


    public override void SetViewport(in Viewport viewport)
    {
        D3D12_VIEWPORT d3d12Viewport = new(viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
        _commandBuffer.CommandList->RSSetViewports(1, &d3d12Viewport);
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
        _commandBuffer.CommandList->RSSetViewports((uint)count, d3d12Viewports);
    }

    public override void SetScissorRect(in RectI rect)
    {
        RECT scissorRect = new(rect.Left, rect.Top, rect.Right, rect.Bottom);
        _commandBuffer.CommandList->RSSetScissorRects(1, &scissorRect);
    }

    public override void SetStencilReference(uint reference)
    {
        _commandBuffer.CommandList->OMSetStencilRef(reference);
    }

    public override void SetBlendColor(in Color color)
    {
        fixed (Color* colorPtr = &color)
            _commandBuffer.CommandList->OMSetBlendFactor((float*)colorPtr);
    }

    public override void SetShadingRate(ShadingRate rate)
    {
        if (_commandBuffer.D3DDevice.QueryFeatureSupport(Feature.VariableRateShading)
            && _currentShadingRate != rate)
        {
            _currentShadingRate = rate;

            D3D12_SHADING_RATE d3dRate = D3D12_SHADING_RATE_1X1;
            _commandBuffer.D3DDevice.WriteShadingRateValue(rate, &d3dRate);

            var combiners = stackalloc D3D12_SHADING_RATE_COMBINER[2]
            {
                D3D12_SHADING_RATE_COMBINER_MAX,
                D3D12_SHADING_RATE_COMBINER_MAX,
            };
            _commandBuffer.CommandList->RSSetShadingRate(d3dRate, combiners);
        }
    }

    protected override void SetDepthBoundsCore(float minBounds, float maxBounds)
    {
        _commandBuffer.CommandList->OMSetDepthBounds(minBounds, maxBounds);
    }

    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
        D3D12Buffer d3d12Buffer = (D3D12Buffer)buffer;

        _vboViews[slot].BufferLocation = d3d12Buffer.GpuAddress + offset;
        _vboViews[slot].SizeInBytes = (uint)(d3d12Buffer.Size - offset);
        _vboViews[slot].StrideInBytes = 0;
    }

    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexFormat format, ulong offset = 0)
    {
        D3D12Buffer d3d12Buffer = (D3D12Buffer)buffer;

        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = d3d12Buffer.GpuAddress + offset;
        view.SizeInBytes = (uint)(d3d12Buffer.Size - offset);
        view.Format = (format == IndexFormat.UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        _commandBuffer.CommandList->IASetIndexBuffer(&view);
    }

    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
    {
        PrepareDraw();

        _commandBuffer.CommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance)
    {
        PrepareDraw();

        _commandBuffer.CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DrawIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DrawIndirectCommandSignature, maxCount,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset);
    }


    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DrawIndexedIndirectCommandSignature, 1,
            d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DrawIndexedIndirectCommandSignature, maxCount,
            backendIndirectBuffer.Handle, indirectBufferOffset,
            backendCountBuffer.Handle, countBufferOffset);
    }

    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDraw();

        _commandBuffer.CommandList->DispatchMesh(groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDraw();

        D3D12Buffer backendBuffer = (D3D12Buffer)indirectBuffer;
        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DispatchMeshIndirectCommandSignature, 1, backendBuffer.Handle, indirectBufferOffset, null, 0);
    }

    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount)
    {
        PrepareDraw();

        D3D12Buffer backendIndirectBuffer = (D3D12Buffer)indirectBuffer;
        D3D12Buffer backendCountBuffer = (D3D12Buffer)countBuffer;

        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DispatchMeshIndirectCommandSignature,
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
                _commandBuffer.CommandList->IASetVertexBuffers(0, _currentPipeline.NumVertexBindings, pViews);
            }
        }

        _commandBuffer.FlushBindGroups(graphics: true);
    }

    private static D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS EndingAccessResolveSubresourceParameters(D3D12TextureView resolveDestination)
    {
        D3D12Texture resolveDestinationTexture = (D3D12Texture)resolveDestination.Texture;
        //Debug.Assert(resolveDestinationTexture->GetFormat().aspects == Aspect::Color);

        D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS subresourceParameters = new()
        {
            SrcSubresource = 0,
            DstSubresource = resolveDestinationTexture.GetSubresourceIndex(resolveDestination.BaseMipLevel, resolveDestination.BaseArrayLayer/*, TextureAspect.Color*/),
            DstX = 0,
            DstY = 0,
            SrcRect = new RECT()
            {
                left = 0,
                top = 0,
                right = (int)resolveDestinationTexture.Width,
                bottom = (int)resolveDestinationTexture.Height
            }
        };

        return subresourceParameters;
    }
}
