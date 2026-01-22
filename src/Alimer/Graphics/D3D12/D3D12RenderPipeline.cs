// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Graphics.D3D;
using Alimer.Utilities;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.DirectX.D3D12_DEPTH_WRITE_MASK;
using static TerraFX.Interop.DirectX.D3D12_LOGIC_OP;
using static TerraFX.Interop.DirectX.D3D12_INDEX_BUFFER_STRIP_CUT_VALUE;
using static TerraFX.Interop.DirectX.D3D12_INPUT_CLASSIFICATION;
using static TerraFX.Interop.DirectX.D3D12_PRIMITIVE_TOPOLOGY_TYPE;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_MODE;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_TIER;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12RenderPipeline : RenderPipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12PipelineState> _handle;
    private readonly uint _numVertexBindings = 0;
    private readonly uint[] _strides = new uint[MaxVertexBufferBindings];

    public D3D12RenderPipeline(D3D12GraphicsDevice device, in RenderPipelineDescriptor descriptor)
        : base(descriptor.Label)
    {
        _device = device;
        D3DLayout = (D3D12PipelineLayout)descriptor.Layout;

        GraphicsPipelineStateStream stream = new();
        stream.stream1.pRootSignature = D3DLayout.Handle;

        // ShaderStages
        // Mesh Pipeline (D3DX12_MESH_SHADER_PIPELINE_STATE_DESC)
        if (descriptor.MeshShader is not null)
        {
            stream.stream2.MS = new(((D3D12ShaderModule)descriptor.MeshShader).ByteCode);

            if (descriptor.AmplificationShader != null)
            {
                stream.stream2.AS = new(((D3D12ShaderModule)descriptor.AmplificationShader).ByteCode);
            }
        }
        else
        {
            stream.stream1.VS = new(((D3D12ShaderModule)descriptor.VertexShader!).ByteCode);
        }

        if (descriptor.FragmentShader != null)
        {
            stream.stream1.PS = ((D3D12ShaderModule)descriptor.FragmentShader).ByteCode;
        }


        // Input Layout
        ReadOnlySpan<byte> semanticName = "ATTRIBUTE"u8;
        D3D12_INPUT_ELEMENT_DESC* inputElements = stackalloc D3D12_INPUT_ELEMENT_DESC[MaxVertexAttributes];
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = new()
        {
            pInputElementDescs = inputElements
        };
        for (uint binding = 0; binding < descriptor.VertexBufferLayouts.Length; binding++)
        {
            ref readonly VertexBufferLayout layout = ref descriptor.VertexBufferLayouts[binding];

            if (layout.Stride == 0)
                continue;

            for (int i = 0; i < layout.Attributes.Length; i++)
            {
                ref readonly VertexAttribute attribute = ref layout.Attributes[i];

                ref D3D12_INPUT_ELEMENT_DESC element = ref inputElements[inputLayoutDesc.NumElements++];
                element.SemanticName = (sbyte*)UnsafeUtilities.GetPointerUnsafe(semanticName);
                element.SemanticIndex = attribute.ShaderLocation;
                element.Format = attribute.Format.ToDxgiFormat();
                element.InputSlot = binding;
                element.AlignedByteOffset = attribute.Offset;

                _numVertexBindings = Math.Max(binding + 1, _numVertexBindings);
                _strides[binding] = layout.Stride;

                if (layout.StepMode == VertexStepMode.Vertex)
                {
                    element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    element.InstanceDataStepRate = 0;
                }
                else
                {
                    element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                    element.InstanceDataStepRate = 1;
                }
            }
        }
        stream.stream1.InputLayout = inputLayoutDesc;

        // RasterizerState
        {
            D3D12_RASTERIZER_DESC1 rasterizerState = D3D12_RASTERIZER_DESC1.DEFAULT;
            rasterizerState.FillMode = descriptor.RasterizerState.FillMode.ToD3D12();
            rasterizerState.CullMode = descriptor.RasterizerState.CullMode.ToD3D12();
            rasterizerState.FrontCounterClockwise = (descriptor.RasterizerState.FrontFace == FrontFace.CounterClockwise) ? TRUE : FALSE;
            rasterizerState.DepthBias = descriptor.RasterizerState.DepthBias;
            rasterizerState.DepthBiasClamp = descriptor.RasterizerState.DepthBiasClamp;
            rasterizerState.SlopeScaledDepthBias = descriptor.RasterizerState.DepthBiasSlopeScale;
            rasterizerState.DepthClipEnable = (descriptor.RasterizerState.DepthClipMode == DepthClipMode.Clip) ? TRUE : FALSE;
            rasterizerState.MultisampleEnable = descriptor.SampleCount > TextureSampleCount.Count1 ? TRUE : FALSE;
            rasterizerState.AntialiasedLineEnable = FALSE;
            rasterizerState.ForcedSampleCount = 0;
            rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            if (descriptor.RasterizerState.ConservativeRasterEnable
                && device.DxAdapter.Features.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED)
            {
                rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
            }
            stream.stream1.RasterizerState = rasterizerState;
        }

        // Handle index strip
        if (descriptor.PrimitiveTopology != PrimitiveTopology.TriangleStrip
            && descriptor.PrimitiveTopology != PrimitiveTopology.LineStrip)
        {
            stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        }
        else
        {
            if (descriptor.StripIndexFormat == IndexFormat.UInt16)
            {
                stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
            }
            else
            {
                stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
            }
        }

        // PrimitiveTopologyType
        switch (descriptor.PrimitiveTopology)
        {
            case PrimitiveTopology.PointList:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                break;
            case PrimitiveTopology.LineList:
            case PrimitiveTopology.LineStrip:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                break;
            case PrimitiveTopology.TriangleList:
            case PrimitiveTopology.TriangleStrip:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                break;
                //case PrimitiveTopology.PatchList:
                //    stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
                //    break;
        }

        // Color Attachments + RTV
        D3D12_RT_FORMAT_ARRAY RTVFormats = new()
        {
            NumRenderTargets = 0
        };

        D3D12_BLEND_DESC blendState = new();
        blendState.AlphaToCoverageEnable = descriptor.BlendState.AlphaToCoverageEnabled ? TRUE : FALSE;
        blendState.IndependentBlendEnable = descriptor.BlendState.IndependentBlendEnable ? TRUE : FALSE;
        for (int i = 0; i < descriptor.ColorFormats.Length; i++)
        {
            if (descriptor.ColorFormats[i] == PixelFormat.Undefined)
                continue;

            ref readonly RenderTargetBlendState attachment = ref descriptor.BlendState.RenderTargets[i];

            blendState.RenderTarget[i].BlendEnable = GraphicsUtilities.BlendEnabled(in attachment) ? TRUE : FALSE;
            blendState.RenderTarget[i].LogicOpEnable = FALSE;
            blendState.RenderTarget[i].SrcBlend = attachment.SourceColorBlendFactor.ToD3D12(false);
            blendState.RenderTarget[i].DestBlend = attachment.DestinationColorBlendFactor.ToD3D12(false);
            blendState.RenderTarget[i].BlendOp = attachment.ColorBlendOperation.ToD3D12();
            blendState.RenderTarget[i].SrcBlendAlpha = attachment.SourceAlphaBlendFactor.ToD3D12(true);
            blendState.RenderTarget[i].DestBlendAlpha = attachment.DestinationColorBlendFactor.ToD3D12(true);
            blendState.RenderTarget[i].BlendOpAlpha = attachment.AlphaBlendOperation.ToD3D12();
            blendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
            blendState.RenderTarget[i].RenderTargetWriteMask = (byte)attachment.ColorWriteMask.ToD3D12();

            // RTV
            RTVFormats.RTFormats[(int)RTVFormats.NumRenderTargets] = (DXGI_FORMAT)descriptor.ColorFormats[i].ToDxgiFormat();
            RTVFormats.NumRenderTargets++;
        }
        stream.stream1.RTVFormats = RTVFormats;
        stream.stream1.BlendState = blendState;

        // DepthStencilState + DSVFormat
        D3D12_DEPTH_STENCIL_DESC1 d3dDepthStencilState = default;
        if (descriptor.DepthStencilFormat != PixelFormat.Undefined)
        {
            // Depth
            DepthStencilState depthStencilState = descriptor.DepthStencilState;
            d3dDepthStencilState.DepthEnable = (depthStencilState.DepthCompare != CompareFunction.Always || depthStencilState.DepthWriteEnabled) ? TRUE : FALSE;
            d3dDepthStencilState.DepthWriteMask = depthStencilState.DepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            d3dDepthStencilState.DepthFunc = depthStencilState.DepthCompare.ToD3D12();

            d3dDepthStencilState.StencilEnable = GraphicsUtilities.StencilTestEnabled(in depthStencilState) ? TRUE : FALSE;
            d3dDepthStencilState.StencilReadMask = (byte)depthStencilState.StencilReadMask;
            d3dDepthStencilState.StencilWriteMask = (byte)depthStencilState.StencilWriteMask;

            d3dDepthStencilState.FrontFace = depthStencilState.StencilFront.ToD3D12();
            d3dDepthStencilState.BackFace = depthStencilState.StencilBack.ToD3D12();

            if (device.DxAdapter.Features.DepthBoundsTestSupported)
            {
                d3dDepthStencilState.DepthBoundsTestEnable = descriptor.DepthStencilState.DepthBoundsTestEnable ? TRUE : FALSE;
            }
            else
            {
                d3dDepthStencilState.DepthBoundsTestEnable = FALSE;
            }
        }
        else
        {
            d3dDepthStencilState = D3D12_DEPTH_STENCIL_DESC1.DEFAULT;
        }
        stream.stream1.DepthStencilState = d3dDepthStencilState;
        stream.stream1.DSVFormat = (DXGI_FORMAT)descriptor.DepthStencilFormat.ToDxgiFormat();

        // SampleDesc and SampleMask
        stream.stream1.SampleDesc = new(new DXGI_SAMPLE_DESC(descriptor.SampleCount.ToSampleCount(), 0));
        stream.stream1.SampleMask = 0;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = new()
        {
            pPipelineStateSubobjectStream = &stream,
            SizeInBytes = (nuint)sizeof(GraphicsPipelineStateStream1)
        };
        if (descriptor.MeshShader is not null
            && device.QueryFeatureSupport(Feature.MeshShader))
        {
            streamDesc.SizeInBytes += (nuint)sizeof(GraphicsPipelineStateStream2);
        }

        HRESULT hr = device.Device->CreatePipelineState(&streamDesc, __uuidof<ID3D12PipelineState>(), (void**)_handle.GetAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Render Pipeline.");
            return;
        }

        D3DPrimitiveTopology = descriptor.PrimitiveTopology.ToD3DPrimitiveTopology();
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override PipelineLayout Layout => D3DLayout;

    public ID3D12PipelineState* Handle => _handle;
    public D3D12PipelineLayout D3DLayout { get; }
    public ID3D12RootSignature* RootSignature => D3DLayout.Handle;

    public D3D_PRIMITIVE_TOPOLOGY D3DPrimitiveTopology { get; }
    public uint NumVertexBindings => _numVertexBindings;
    public uint GetStride(uint slot) => _strides[slot];

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12RenderPipeline" /> class.
    /// </summary>
    ~D3D12RenderPipeline() => Dispose(disposing: false);

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

    public struct GraphicsPipelineStateStream1
    {
        public CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        public CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        public CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE IBStripCutValue;
        public CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        public CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        public CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        public CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
        public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 DepthStencilState;
        public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        public CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER1 RasterizerState;
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
            PS = new();
            BlendState = new();
            DepthStencilState = new();
            DSVFormat = new();
            RasterizerState = new();
            RTVFormats = new();
            SampleDesc = new();
            SampleMask = new();
            NodeMask = new();
        }
    }

    struct GraphicsPipelineStateStream2
    {
        public CD3DX12_PIPELINE_STATE_STREAM_MS MS;
        public CD3DX12_PIPELINE_STATE_STREAM_AS AS;

        public GraphicsPipelineStateStream2()
        {
            MS = new();
            AS = new();
        }
    }

    struct GraphicsPipelineStateStream
    {
        public GraphicsPipelineStateStream1 stream1;
        public GraphicsPipelineStateStream2 stream2;

        public GraphicsPipelineStateStream()
        {
            stream1 = new();
            stream2 = new();
        }
    }
}
