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
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Pipeline : Pipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12PipelineLayout _layout;
    private readonly ComPtr<ID3D12PipelineState> _handle;
    private readonly uint _numVertexBindings = 0;
    private readonly uint[] _strides = new uint[MaxVertexBufferBindings];

    public D3D12Pipeline(D3D12GraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        _layout = (D3D12PipelineLayout)description.Layout;

        GraphicsPipelineStateStream stream = new();
        stream.stream1.pRootSignature = _layout.Handle;

        // ShaderStages
        int shaderStageCount = description.ShaderStages.Length;
        void** shaderStageBytecodes = stackalloc void*[shaderStageCount];

        for (int i = 0; i < shaderStageCount; i++)
        {
            ref ShaderStageDescription shaderDesc = ref description.ShaderStages[i];
            shaderStageBytecodes[i] = Unsafe.AsPointer(ref shaderDesc.ByteCode[0]);

            switch (shaderDesc.Stage)
            {
                case ShaderStages.Vertex:
                    stream.stream1.VS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Hull:
                    stream.stream1.HS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Domain:
                    stream.stream1.DS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Geometry:
                    stream.stream1.DS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Fragment:
                    stream.stream1.PS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Amplification:
                    stream.stream2.AS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Mesh:
                    stream.stream2.MS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
            }
        }

        stream.stream1.BlendState = D3D12_BLEND_DESC.DEFAULT;
        stream.stream1.SampleMask = uint.MaxValue;
        // RasterizerState
        D3D12_RASTERIZER_DESC rasterizerState = D3D12_RASTERIZER_DESC.DEFAULT;
        rasterizerState.FillMode = description.RasterizerState.FillMode.ToD3D12();
        rasterizerState.CullMode = description.RasterizerState.CullMode.ToD3D12();
        rasterizerState.FrontCounterClockwise = description.RasterizerState.FrontFaceCounterClockwise ? TRUE : FALSE;
        //rasterizerState.DepthBias = static_cast<INT>(desc.rasterizerState.depthBias);
        //rasterizerState.DepthBiasClamp = desc.rasterizerState.depthBiasClamp;
        //rasterizerState.SlopeScaledDepthBias = desc.rasterizerState.slopeScaledDepthBias;
        rasterizerState.DepthClipEnable = (description.RasterizerState.DepthClipMode == DepthClipMode.Clip) ? TRUE : FALSE;
        //rasterizerState.MultisampleEnable = desc.sampleCount > TextureSampleCount::Count1 ? TRUE : FALSE;
        rasterizerState.AntialiasedLineEnable = FALSE;
        rasterizerState.ForcedSampleCount = 0;
        rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        stream.stream1.RasterizerState = rasterizerState;

        stream.stream1.DepthStencilState = D3D12_DEPTH_STENCIL_DESC1.DEFAULT;

        // Input Layout
        ReadOnlySpan<byte> semanticName = "ATTRIBUTE"u8;
        D3D12_INPUT_ELEMENT_DESC* inputElements = stackalloc D3D12_INPUT_ELEMENT_DESC[MaxVertexAttributes];
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = new();
        inputLayoutDesc.pInputElementDescs = inputElements;
        for (uint binding = 0; binding < description.VertexBufferLayouts.Length; binding++)
        {
            ref readonly VertexBufferLayout layout = ref description.VertexBufferLayouts[binding];

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

        // Handle index strip
        if (description.PrimitiveTopology != PrimitiveTopology.TriangleStrip &&
            description.PrimitiveTopology != PrimitiveTopology.LineStrip)
        {
            stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        }
        else
        {
            stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        }

        // PrimitiveTopologyType
        switch (description.PrimitiveTopology)
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
            case PrimitiveTopology.PatchList:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
                break;
        }

        // Color Attachments + RTV
        D3D12_RT_FORMAT_ARRAY RTVFormats = new();
        RTVFormats.NumRenderTargets = 1;
        RTVFormats.RTFormats[0] = (DXGI_FORMAT)description.ColorFormats[0].ToDxgiFormat();

        stream.stream1.RTVFormats = RTVFormats;
        stream.stream1.DSVFormat = (DXGI_FORMAT)description.DepthStencilFormat.ToDxgiFormat();
        stream.stream1.SampleDesc = new(1, 0);
        stream.stream1.NodeMask = 0;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = new()
        {
            pPipelineStateSubobjectStream = &stream,
            SizeInBytes = (nuint)sizeof(GraphicsPipelineStateStream1)
        };
        if (device.QueryFeatureSupport(Feature.MeshShader))
        {
            streamDesc.SizeInBytes += (nuint)sizeof(GraphicsPipelineStateStream2);
        }

        HRESULT hr = device.Handle->CreatePipelineState(&streamDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Render Pipeline.");
            return;
        }

        D3DPrimitiveTopology = description.PrimitiveTopology.ToD3DPrimitiveTopology(description.PatchControlPoints);
    }

    public D3D12Pipeline(D3D12GraphicsDevice device, in ComputePipelineDescription description)
        : base(PipelineType.Compute, description.Label)
    {
        _device = device;
        _layout = (D3D12PipelineLayout)description.Layout;

        fixed (byte* pByteCode = description.ComputeShader.ByteCode)
        {
            ComputePipelineStateStream stream = new()
            {
                pRootSignature = _layout.Handle,
                CS = new(pByteCode, (nuint)description.ComputeShader.ByteCode.Length),
                NodeMask = 0
            };

            D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = new()
            {
                pPipelineStateSubobjectStream = &stream,
                SizeInBytes = (nuint)sizeof(ComputePipelineStateStream)
            };

            HRESULT hr = device.Handle->CreatePipelineState(&streamDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
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

    public D3D_PRIMITIVE_TOPOLOGY D3DPrimitiveTopology { get; }
    public uint NumVertexBindings => _numVertexBindings;
    public uint GetStride(uint slot) => _strides[slot];

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Pipeline" /> class.
    /// </summary>
    ~D3D12Pipeline() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        fixed (char* pName = newLabel)
        {
            _ = _handle.Get()->SetName(pName);
        }
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
