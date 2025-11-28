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
    private readonly D3D12PipelineLayout _layout;
    private readonly ComPtr<ID3D12PipelineState> _handle;

    
    public D3D12ComputePipeline(D3D12GraphicsDevice device, in ComputePipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        _layout = (D3D12PipelineLayout)descriptor.Layout;

        fixed (byte* pByteCode = descriptor.ComputeShader.ByteCode)
        {
            ComputePipelineStateStream stream = new()
            {
                pRootSignature = _layout.Handle,
                CS = new(pByteCode, (nuint)descriptor.ComputeShader.ByteCode.Length),
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
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override PipelineLayout Layout => _layout;

    public ID3D12PipelineState* Handle => _handle;
    public ID3D12RootSignature* RootSignature => _layout.Handle;

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
    public struct GraphicsPipelineStateStream1
    {
        public CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        public CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        public CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE IBStripCutValue;
        public CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        public CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        public CD3DX12_PIPELINE_STATE_STREAM_HS HS;
        public CD3DX12_PIPELINE_STATE_STREAM_DS DS;
        public CD3DX12_PIPELINE_STATE_STREAM_GS GS;
        public CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        public CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
        public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 DepthStencilState;
        public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        public CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
        public CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
        public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;
        public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK NodeMask;

        public GraphicsPipelineStateStream1()
        {
            pRootSignature = new();
            InputLayout = new();
            IBStripCutValue = new();
            PrimitiveTopologyType = new();
            VS = new();
            HS = new();
            DS = new();
            GS = new();
            PS = new();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GraphicsPipelineStateStream2
    {
        public CD3DX12_PIPELINE_STATE_STREAM_AS AS;
        public CD3DX12_PIPELINE_STATE_STREAM_MS MS;

        public GraphicsPipelineStateStream2()
        {
            AS = new();
            MS = new();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GraphicsPipelineStateStream
    {
        public GraphicsPipelineStateStream1 stream1;
        public GraphicsPipelineStateStream2 stream2;

        public GraphicsPipelineStateStream()
        {
            stream1 = new();
            stream2 = new();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ComputePipelineStateStream
    {
        public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK NodeMask;
        public CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        public CD3DX12_PIPELINE_STATE_STREAM_CS CS;
    }
}
