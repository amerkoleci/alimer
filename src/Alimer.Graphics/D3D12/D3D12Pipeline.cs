// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12_INDEX_BUFFER_STRIP_CUT_VALUE;
using static TerraFX.Interop.DirectX.D3D12_PRIMITIVE_TOPOLOGY_TYPE;
using static TerraFX.Interop.DirectX.D3D12_INPUT_CLASSIFICATION;
using static Alimer.Graphics.Constants;
using Alimer.Utilities;
using Alimer.Graphics.D3D;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Pipeline : Pipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12PipelineLayout _layout;
    private readonly ComPtr<ID3D12PipelineState> _handle;

    public D3D12Pipeline(D3D12GraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        _layout = (D3D12PipelineLayout)description.Layout;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc = new();
        d3dDesc.pRootSignature = _layout.Handle;

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
                    d3dDesc.VS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Hull:
                    d3dDesc.HS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Domain:
                    d3dDesc.DS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                case ShaderStages.Fragment:
                    d3dDesc.PS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    break;
                    //case ShaderStages.Amplification:
                    //    d3dDesc.AS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    //    break;
                    //case ShaderStages.Mesh:
                    //    d3dDesc.MS = new(shaderStageBytecodes[i], (nuint)shaderDesc.ByteCode.Length);
                    //    break;
            }
        }

        d3dDesc.BlendState = D3D12_BLEND_DESC.DEFAULT;
        d3dDesc.SampleMask = uint.MaxValue;
        d3dDesc.RasterizerState = D3D12_RASTERIZER_DESC.DEFAULT;
        d3dDesc.DepthStencilState = D3D12_DEPTH_STENCIL_DESC.DEFAULT;

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

                //pipeline->numVertexBindings = std::max(attribute->bufferIndex + 1, pipeline->numVertexBindings);
                //pipeline->strides[attribute->bufferIndex] = layout->stride;

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

        d3dDesc.InputLayout = inputLayoutDesc;

        d3dDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        d3dDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        d3dDesc.NumRenderTargets = 1u;
        d3dDesc.RTVFormats[0] = (DXGI_FORMAT)description.ColorFormats[0].ToDxgiFormat();
        d3dDesc.DSVFormat = (DXGI_FORMAT)description.DepthStencilFormat.ToDxgiFormat();
        d3dDesc.SampleDesc = new(1, 0);
        d3dDesc.NodeMask = 0;

        HRESULT hr = device.Handle->CreateGraphicsPipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Render Pipeline.");
            return;
        }

        PrimitiveTopology = description.PrimitiveTopology.ToD3DPrimitiveTopology();
    }

    public D3D12Pipeline(D3D12GraphicsDevice device, in ComputePipelineDescription description)
        : base(PipelineType.Compute, description.Label)
    {
        _device = device;
        _layout = (D3D12PipelineLayout)description.Layout;

        fixed (byte* pByteCode = description.ComputeShader.ByteCode)
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC d3dDesc = new()
            {
                pRootSignature = _layout.Handle,
                CS = new(pByteCode, (nuint)description.ComputeShader.ByteCode.Length)
            };
            HRESULT hr = device.Handle->CreateComputePipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
            if (hr.FAILED)
            {
                Log.Error("D3D12: Failed to create Compute Pipeline.");
                return;
            }
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12PipelineState* Handle => _handle;
    public ID3D12RootSignature* RootSignature => _layout.Handle;

    public D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology { get; }

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
            _ = _handle.Get()->SetName((ushort*)pName);
        }
    }
}
