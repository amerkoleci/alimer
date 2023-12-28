// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Utilities.UnsafeUtilities;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_ROOT_SIGNATURE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12PipelineLayout : PipelineLayout
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12RootSignature> _handle;
    private readonly uint[] _cbvUavSrvRootParameterIndex;
    private readonly uint[] _samplerRootParameterIndex;

    public D3D12PipelineLayout(D3D12GraphicsDevice device, in PipelineLayoutDescription description)
        : base(description)
    {
        _device = device;

        PushConstantsBaseIndex = ~0u;

        // TODO: Handle dynamic constant buffers
        int setLayoutCount = description.BindGroupLayouts.Length;
        _cbvUavSrvRootParameterIndex = new uint[setLayoutCount];
        _samplerRootParameterIndex = new uint[setLayoutCount];

        List<D3D12_STATIC_SAMPLER_DESC> staticSamplers = new();

        // Count root parameter count first
        int rootParameterCount = 0;
        for (int i = 0; i < setLayoutCount; i++)
        {
            _cbvUavSrvRootParameterIndex[i] = ~0u;
            _samplerRootParameterIndex[i] = ~0u;

            D3D12BindGroupLayout bindGroupLayout = (D3D12BindGroupLayout)description.BindGroupLayouts[i];
            if (bindGroupLayout.DescriptorTableSizeCbvUavSrv > 0)
            {
                rootParameterCount++;
            }

            if (bindGroupLayout.DescriptorTableSizeSamplers > 0)
            {
                rootParameterCount++;
            }
        }

        int pushConstantRangeCount = description.PushConstantRanges.Length;
        uint rootParameterIndex = 0;
        D3D12_ROOT_PARAMETER1* rootParameters = stackalloc D3D12_ROOT_PARAMETER1[rootParameterCount];

        for (int i = 0; i < setLayoutCount; i++)
        {
            D3D12BindGroupLayout bindGroupLayout = (D3D12BindGroupLayout)description.BindGroupLayouts[i];
            if (bindGroupLayout.DescriptorTableSizeCbvUavSrv > 0)
            {
                _cbvUavSrvRootParameterIndex[i] = rootParameterIndex;

                Span<D3D12_DESCRIPTOR_RANGE1> cbvUavSrvDescriptorRanges = CollectionsMarshal.AsSpan(bindGroupLayout.CbvUavSrvDescriptorRanges);
                foreach (ref D3D12_DESCRIPTOR_RANGE1 range in cbvUavSrvDescriptorRanges)
                {
                    Debug.Assert(range.RegisterSpace == D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
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

            if (bindGroupLayout.StaticSamplers.Count > 0)
            {
                Span<D3D12_STATIC_SAMPLER_DESC> bindGroupLayoutStaticSamplers = CollectionsMarshal.AsSpan(bindGroupLayout.StaticSamplers);
                foreach (ref D3D12_STATIC_SAMPLER_DESC staticSampler in bindGroupLayoutStaticSamplers)
                {
                    Debug.Assert(staticSampler.RegisterSpace == D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
                    staticSampler.RegisterSpace = (uint)i;

                    staticSamplers.Add(staticSampler);
                }
            }
        }

        if (pushConstantRangeCount > 0)
        {
            PushConstantsBaseIndex = rootParameterIndex;

            for (int i = 0; i < pushConstantRangeCount; i++)
            {
                ref readonly PushConstantRange pushConstantRange = ref description.PushConstantRanges[i];

                rootParameters[rootParameterIndex].InitAsConstants(pushConstantRange.Size / 4, pushConstantRange.ShaderRegister);
                rootParameterIndex++;
            }
        }

        Span<D3D12_STATIC_SAMPLER_DESC> staticSamplersSpan = CollectionsMarshal.AsSpan(staticSamplers);

        fixed (D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = staticSamplersSpan)
        {
            D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = new(
                (uint)rootParameterCount,
                rootParameters,
                (uint)staticSamplers.Count,
                staticSamplerDescs,
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            );

            using ComPtr<ID3DBlob> rootSignatureBlob = default;
            using ComPtr<ID3DBlob> rootSignatureErrorBlob = default;

            HRESULT hr = D3DX12SerializeVersionedRootSignature(&versionedRootSignatureDesc,
                device.D3D12Features.RootSignatureHighestVersion,
                rootSignatureBlob.GetAddressOf(),
                rootSignatureErrorBlob.GetAddressOf());
            if (hr.FAILED)
            {
                string errors = Marshal.PtrToStringAnsi((nint)rootSignatureErrorBlob.Get()->GetBufferPointer())!;
                Log.Error($"D3D12SerializeVersionedRootSignature failed: {errors}");
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
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12PipelineLayout" /> class.
    /// </summary>
    ~D3D12PipelineLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12RootSignature* Handle => _handle;

    public uint PushConstantsBaseIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        fixed (char* pName = newLabel)
        {
            _ = _handle.Get()->SetName(pName);
        }
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
