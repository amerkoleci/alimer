// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Graphics.D3D;
using Alimer.Utilities;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.DirectX.D3D12_INDEX_BUFFER_STRIP_CUT_VALUE;
using static TerraFX.Interop.DirectX.D3D12_INPUT_CLASSIFICATION;
using static TerraFX.Interop.DirectX.D3D12_PRIMITIVE_TOPOLOGY_TYPE;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_MODE;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_TIER;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12ComputePipeline : ComputePipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12PipelineState> _handle;

    public D3D12ComputePipeline(D3D12GraphicsDevice device, in ComputePipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        D3DLayout = (D3D12PipelineLayout)descriptor.Layout;
        D3D12ShaderModule computeShader = (D3D12ShaderModule)descriptor.ComputeShader;

        ComputePipelineStateStream stream = new()
        {
            pRootSignature = D3DLayout.Handle,
            CS = new(computeShader.ByteCode),
            NodeMask = 0
        };

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = new()
        {
            pPipelineStateSubobjectStream = &stream,
            SizeInBytes = (nuint)sizeof(ComputePipelineStateStream)
        };

        HRESULT hr = device.Device->CreatePipelineState(&streamDesc, __uuidof<ID3D12PipelineState>(), (void**)_handle.GetAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Compute Pipeline.");
            return;
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override PipelineLayout Layout => D3DLayout;

    public ID3D12PipelineState* Handle => _handle;
    public D3D12PipelineLayout D3DLayout { get; }
    public ID3D12RootSignature* RootSignature => D3DLayout.Handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12ComputePipeline" /> class.
    /// </summary>
    ~D3D12ComputePipeline() => Dispose(disposing: false);

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
