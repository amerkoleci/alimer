// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_FLAGS;
using Alimer.Utilities;
using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12ComputePassEncoder : ComputePassEncoder
{
    private readonly D3D12CommandBuffer _commandBuffer;
    private D3D12ComputePipeline? _currentPipeline;

    public D3D12ComputePassEncoder(D3D12CommandBuffer commandBuffer)
    {
        _commandBuffer = commandBuffer;
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

        D3D12ComputePipeline backendPipeline = (D3D12ComputePipeline)pipeline;
        _commandBuffer.SetPipelineLayout(backendPipeline.D3DLayout);

        _commandBuffer.CommandList->SetPipelineState(backendPipeline.Handle);
        _commandBuffer.CommandList->SetComputeRootSignature(backendPipeline.RootSignature);

        _currentPipeline = backendPipeline;
    }

    /// <inheritdoc/>
    protected override void SetBindGroupCore(int groupIndex, BindGroup bindGroup, Span<uint> dynamicBufferOffsets)
    {
        _commandBuffer.SetBindGroup(groupIndex, bindGroup, dynamicBufferOffsets);
    }

    /// <inheritdoc/>
    protected override void SetPushConstantsCore(void* data, uint size, uint offset)
    {
        Debug.Assert(_currentPipeline != null);

        uint rootParameterIndex = _currentPipeline.D3DLayout.PushConstantsBaseIndex;
        uint num32BitValuesToSet = size / sizeof(uint);
        uint destOffsetIn32BitValues = offset / sizeof(uint);

        _commandBuffer.CommandList->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, data, destOffsetIn32BitValues);
    }

    /// <inheritdoc/>
    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDispatch();

        _commandBuffer.CommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
    }

    /// <inheritdoc/>
    protected override void DispatchIndirectCore(GpuBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDispatch();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DispatchIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    /// <inheritdoc/>
    protected override void CopyBufferToBufferCore(GpuBuffer sourceBuffer, GpuBuffer destinationBuffer)
    {
        D3D12Buffer backendSrcBuffer = sourceBuffer.ToD3D12();
        D3D12Buffer backendDestBuffer = destinationBuffer.ToD3D12();

        _commandBuffer.BufferBarrier(backendSrcBuffer, BufferStates.CopySource);
        _commandBuffer.BufferBarrier(backendDestBuffer, BufferStates.CopyDest);
        _commandBuffer.CommitBarriers();

        // Note: D3D12 inverts the order of source and destination parameters
        _commandBuffer.CommandList->CopyResource(backendDestBuffer.Handle, backendSrcBuffer.Handle);
    }

    /// <inheritdoc/>
    protected override void CopyBufferToBufferCore(GpuBuffer sourceBuffer, ulong sourceOffset, GpuBuffer destinationBuffer, ulong destinationOffset, ulong size)
    {
        D3D12Buffer backendSrcBuffer = sourceBuffer.ToD3D12();
        D3D12Buffer backendDestBuffer = destinationBuffer.ToD3D12();

        _commandBuffer.BufferBarrier(backendSrcBuffer, BufferStates.CopySource);
        _commandBuffer.BufferBarrier(backendDestBuffer, BufferStates.CopyDest);
        _commandBuffer.CommitBarriers();

        // Note: D3D12 inverts the order of source and destination parameters
        _commandBuffer.CommandList->CopyBufferRegion(
            backendDestBuffer.Handle, destinationOffset,
            backendSrcBuffer.Handle, sourceOffset,
            size);
    }

    private void PrepareDispatch()
    {
        _commandBuffer.FlushBindGroups(graphics: false);
    }
}
