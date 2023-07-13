// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12_ROOT_SIGNATURE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_ROOT_PARAMETER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using static TerraFX.Interop.DirectX.D3D12;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12PipelineLayout : PipelineLayout
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12RootSignature> _handle;

    public D3D12PipelineLayout(D3D12GraphicsDevice device, in PipelineLayoutDescription description)
        : base(description)
    {
        _device = device;

        CbvUavSrvRootParameterIndex = ~0u;
        SamplerRootParameterIndex = ~0u;
        PushConstantsBaseIndex = ~0u;

        int rootParameterCount = 0;

        int setLayoutCount = description.BindGroupLayouts.Length;
        D3D12_ROOT_PARAMETER1* rootParameters = stackalloc D3D12_ROOT_PARAMETER1[16];
        for (int i = 0; i < setLayoutCount; i++)
        {
            D3D12BindGroupLayout bindGroupLayout = (D3D12BindGroupLayout)description.BindGroupLayouts[i];
            if (bindGroupLayout._cbvUavSrvDescriptorRanges.Count > 0)
            {
                CbvUavSrvRootParameterIndex = (uint)rootParameterCount;

                uint numDescriptorRanges = (uint)bindGroupLayout._cbvUavSrvDescriptorRanges.Count;
                Span<D3D12_DESCRIPTOR_RANGE1> descriptorRanges = CollectionsMarshal.AsSpan(bindGroupLayout._cbvUavSrvDescriptorRanges);
                foreach (ref D3D12_DESCRIPTOR_RANGE1 range in descriptorRanges)
                {
                    Debug.Assert(range.RegisterSpace == D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
                    range.RegisterSpace = (uint)i;
                }

                rootParameters[rootParameterCount].InitAsDescriptorTable(
                    numDescriptorRanges,
                    (D3D12_DESCRIPTOR_RANGE1*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(descriptorRanges)),
                    D3D12_SHADER_VISIBILITY_ALL
                );

                rootParameterCount++;
            }
        }

        int pushConstantRangeCount = description.PushConstantRanges.Length;
        rootParameterCount += pushConstantRangeCount;
        int staticSamplerDescCount = 0;

        D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = stackalloc D3D12_STATIC_SAMPLER_DESC[staticSamplerDescCount];

        if (pushConstantRangeCount > 0)
        {
            PushConstantsBaseIndex = (uint)rootParameterCount;

            for (int i = 0; i < pushConstantRangeCount; i++)
            {
                ref readonly PushConstantRange pushConstantRange = ref description.PushConstantRanges[i];

                rootParameters[rootParameterCount].InitAsConstants(pushConstantRange.Size / 4, pushConstantRange.ShaderRegister);
                rootParameterCount++;
            }
        }

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC.Init_1_1(
            out D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc,
            (uint)rootParameterCount,
            rootParameters,
            (uint)staticSamplerDescCount,
            staticSamplerDescs,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        using ComPtr<ID3DBlob> rootSignatureBlob = default;
        using ComPtr<ID3DBlob> rootSignatureErrorBlob = default;

        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc,
            device.D3D12Features.RootSignatureHighestVersion,
            rootSignatureBlob.GetAddressOf(),
            rootSignatureErrorBlob.GetAddressOf());
        if (hr.FAILED)
        {
            return;
        }

        hr = device.Handle->CreateRootSignature(0,
            rootSignatureBlob.Get()->GetBufferPointer(),
            rootSignatureBlob.Get()->GetBufferSize(),
            __uuidof<ID3D12RootSignature>(),
            _handle.GetVoidAddressOf());
        if (hr.FAILED)
        {
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12PipelineLayout" /> class.
    /// </summary>
    ~D3D12PipelineLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12RootSignature* Handle => _handle;

    public uint CbvUavSrvRootParameterIndex { get; }
    public uint SamplerRootParameterIndex { get; }

    public uint PushConstantsBaseIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        fixed (char* pName = newLabel)
        {
            _ = _handle.Get()->SetName((ushort*)pName);
        }
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }
}
