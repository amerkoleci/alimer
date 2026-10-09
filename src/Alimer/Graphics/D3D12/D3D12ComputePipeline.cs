// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Vortice.Win32;
using Vortice.Win32.Graphics;
using static Vortice.Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12ComputePipeline : ComputePipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12PipelineState> _handle;

    public D3D12ComputePipeline(D3D12GraphicsDevice device, in ComputePipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        D3D12ShaderModule computeShader = (D3D12ShaderModule)descriptor.ComputeShader;

        ComputePipelineStateStream stream = new()
        {
            pRootSignature = device.BindlessManager.UniversalRootSignature,
            CS = new(computeShader.ByteCode),
            NodeMask = 0
        };

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = new()
        {
            pPipelineStateSubobjectStream = &stream,
            SizeInBytes = (nuint)sizeof(ComputePipelineStateStream)
        };

        HRESULT hr = device.Device->CreatePipelineState(&streamDesc, __uuidof<ID3D12PipelineState>(), (void**)_handle.GetAddressOf());
        if (hr.Failure)
        {
            Log.Error("D3D12: Failed to create Compute Pipeline.");
            return;
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12PipelineState* Handle => _handle;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _handle.Get()->SetName(newLabel);
    }

    [StructLayout(LayoutKind.Sequential)]
    struct ComputePipelineStateStream
    {
        public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK NodeMask;
        public CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        public CD3DX12_PIPELINE_STATE_STREAM_CS CS;
    }
}
