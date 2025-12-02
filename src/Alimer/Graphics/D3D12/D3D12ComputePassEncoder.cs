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
    protected override void SetBindGroupCore(int groupIndex, BindGroup bindGroup)
    {
        _commandBuffer.SetBindGroup(groupIndex, bindGroup);
    }

    /// <inheritdoc/>
    protected override void SetPushConstantsCore(uint pushConstantIndex, void* data, int size)
    {
        Debug.Assert(_currentPipeline != null);

        uint rootParameterIndex = _currentPipeline.D3DLayout.PushConstantsBaseIndex + pushConstantIndex;
        int num32BitValuesToSet = size / 4;

        _commandBuffer.CommandList->SetComputeRoot32BitConstants(rootParameterIndex, (uint)num32BitValuesToSet, data, 0
        );
    }

    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PrepareDispatch();

        _commandBuffer.CommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
    }

    protected override void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset)
    {
        PrepareDispatch();

        D3D12Buffer d3d12Buffer = (D3D12Buffer)indirectBuffer;
        _commandBuffer.CommandList->ExecuteIndirect(_commandBuffer.D3DDevice.DispatchIndirectCommandSignature, 1, d3d12Buffer.Handle, indirectBufferOffset, null, 0);
    }

    private void PrepareDispatch()
    {
        _commandBuffer.FlushBindGroups(graphics: false);
    }
}
