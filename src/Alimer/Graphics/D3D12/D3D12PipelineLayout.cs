// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static Alimer.Utilities.UnsafeUtilities;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_ROOT_SIGNATURE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12PipelineLayout : PipelineLayout
{
    private const uint PushConstantsShaderRegister = 999; // b999 in shader

    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12RootSignature> _handle;
    private readonly uint[] _cbvUavSrvRootParameterIndex;
    private readonly uint[] _samplerRootParameterIndex;

    public D3D12PipelineLayout(D3D12GraphicsDevice device, in PipelineLayoutDescriptor descriptor)
        : base(in descriptor)
    {
        _device = device;

        PushConstantsIndex = ~0u;

        // TODO: Handle dynamic constant buffers
        int setLayoutCount = descriptor.BindGroupLayouts.Length;
        _cbvUavSrvRootParameterIndex = new uint[setLayoutCount];
        _samplerRootParameterIndex = new uint[setLayoutCount];

        // Count root parameter count first
        int rootParameterCount = 0;
        for (int i = 0; i < setLayoutCount; i++)
        {
            _cbvUavSrvRootParameterIndex[i] = ~0u;
            _samplerRootParameterIndex[i] = ~0u;

            D3D12BindGroupLayout bindGroupLayout = (D3D12BindGroupLayout)descriptor.BindGroupLayouts[i];
            if (bindGroupLayout.DescriptorTableSizeCbvUavSrv > 0)
            {
                rootParameterCount++;
            }

            if (bindGroupLayout.DescriptorTableSizeSamplers > 0)
            {
                rootParameterCount++;
            }
        }

        // One for push constants
        rootParameterCount += 1;
        uint rootParameterIndex = 0;
        D3D12_ROOT_PARAMETER1* rootParameters = stackalloc D3D12_ROOT_PARAMETER1[rootParameterCount];

        for (int i = 0; i < setLayoutCount; i++)
        {
            D3D12BindGroupLayout bindGroupLayout = (D3D12BindGroupLayout)descriptor.BindGroupLayouts[i];
            if (bindGroupLayout.DescriptorTableSizeCbvUavSrv > 0)
            {
                _cbvUavSrvRootParameterIndex[i] = rootParameterIndex;

                Span<D3D12_DESCRIPTOR_RANGE1> cbvUavSrvDescriptorRanges = CollectionsMarshal.AsSpan(bindGroupLayout.CbvUavSrvDescriptorRanges);
                foreach (ref D3D12_DESCRIPTOR_RANGE1 range in cbvUavSrvDescriptorRanges)
                {
                    //Debug.Assert(range.RegisterSpace == D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
                    range.RegisterSpace = (uint)i;
                }

                rootParameters[rootParameterIndex].InitAsDescriptorTable(
                    (uint)cbvUavSrvDescriptorRanges.Length,
                    cbvUavSrvDescriptorRanges.GetPointerUnsafe(),
                    D3D12_SHADER_VISIBILITY_ALL
                );

                rootParameterIndex++;
            }

            if (bindGroupLayout.DescriptorTableSizeSamplers > 0)
            {
                _samplerRootParameterIndex[i] = rootParameterIndex;

                Span<D3D12_DESCRIPTOR_RANGE1> samplerDescriptorRanges = CollectionsMarshal.AsSpan(bindGroupLayout.SamplerDescriptorRanges);
                foreach (ref D3D12_DESCRIPTOR_RANGE1 range in samplerDescriptorRanges)
                {
                    Debug.Assert(range.RegisterSpace == D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
                    range.RegisterSpace = (uint)i;
                }

                rootParameters[rootParameterIndex].InitAsDescriptorTable(
                    (uint)samplerDescriptorRanges.Length,
                    samplerDescriptorRanges.GetPointerUnsafe(),
                    D3D12_SHADER_VISIBILITY_ALL
                );

                rootParameterIndex++;
            }
        }

        PushConstantsIndex = rootParameterIndex;
        rootParameters[rootParameterIndex++].InitAsConstants(PushConstantsSize / 4, PushConstantsShaderRegister);

        // Static Samplers
        Span<SamplerDescriptor> staticSamplers =
        [
            SamplerDescriptor.PointClamp,
            SamplerDescriptor.PointWrap,
            SamplerDescriptor.PointMirror,
            SamplerDescriptor.LinearClamp,
            SamplerDescriptor.LinearWrap,
            SamplerDescriptor.LinearMirror,
            SamplerDescriptor.AnisotropicClamp,
            SamplerDescriptor.AnisotropicWrap,
            SamplerDescriptor.AnisotropicMirror,
            SamplerDescriptor.ComparisonDepth
        ];

        D3D12_STATIC_SAMPLER_DESC* d3dStaticSamplers = stackalloc D3D12_STATIC_SAMPLER_DESC[StaticSamplerCount];

        for (int i = 0; i < StaticSamplerCount; ++i)
        {
            uint shaderRegister = StaticSamplerRegisterSpaceBegin + (uint)i;
            D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = D3D12Utils.ToD3D12StaticSamplerDesc(
                shaderRegister,
                staticSamplers[i],
                D3D12_SHADER_VISIBILITY_ALL,
                0
                );

            d3dStaticSamplers[i] = staticSamplerDesc;
        }

        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (device.BindlessManager.UltimateBindless)
        {
            flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
            flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
        }

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = new(
            (uint)rootParameterCount,
            rootParameters,
            (uint)StaticSamplerCount,
            d3dStaticSamplers,
            flags
        );

        using ComPtr<ID3DBlob> rootSignatureBlob = default;
        using ComPtr<ID3DBlob> rootSignatureErrorBlob = default;

        HRESULT hr = D3DX12SerializeVersionedRootSignature(&versionedRootSignatureDesc,
            device.DxAdapter.Features.RootSignatureHighestVersion,
            rootSignatureBlob.GetAddressOf(),
            rootSignatureErrorBlob.GetAddressOf());
        if (hr.FAILED)
        {
            string errors = Marshal.PtrToStringAnsi((nint)rootSignatureErrorBlob.Get()->GetBufferPointer())!;
            Log.Error($"D3D12SerializeVersionedRootSignature failed: {errors}");
            return;
        }

        hr = device.Device->CreateRootSignature(0,
            rootSignatureBlob.Get()->GetBufferPointer(),
            rootSignatureBlob.Get()->GetBufferSize(),
            __uuidof<ID3D12RootSignature>(),
            (void**)_handle.GetAddressOf());
        if (hr.FAILED)
        {
            return;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GPUDevice Device => _device;

    public ID3D12RootSignature* Handle => _handle;

    public uint PushConstantsIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _handle.Get()->SetName(newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    public bool DescriptorTableValidCbvUavSrv(int groupIndex) => _cbvUavSrvRootParameterIndex[groupIndex] != ~0u;
    public bool DescriptorTableValidSamplers(int groupIndex) => _samplerRootParameterIndex[groupIndex] != ~0u;
    public uint GetCbvUavSrvRootParameterIndex(int groupIndex) => _cbvUavSrvRootParameterIndex[groupIndex];
    public uint GetSamplerRootParameterIndex(int groupIndex) => _samplerRootParameterIndex[groupIndex];
}
