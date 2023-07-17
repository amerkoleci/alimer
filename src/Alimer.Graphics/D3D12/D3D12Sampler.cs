// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12_FILTER_REDUCTION_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMPARISON_FUNC;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12Sampler : Sampler
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12_SAMPLER_DESC _desc;

    public D3D12Sampler(D3D12GraphicsDevice device, in SamplerDescription description)
        : base(description)
    {
        _device = device;
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        _desc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        _desc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), 16u);
        if (_desc.MaxAnisotropy > 1)
        {
            _desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
        }
        else
        {
            _desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reductionType);
        }

        _desc.AddressU = description.AddressModeU.ToD3D12();
        _desc.AddressV = description.AddressModeV.ToD3D12();
        _desc.AddressW = description.AddressModeW.ToD3D12();
        _desc.MipLODBias = 0.0f;
        if (description.Compare != CompareFunction.Never)
        {
            _desc.ComparisonFunc = description.Compare.ToD3D12();
        }
        else
        {
            // Still set the function so it's not garbage.
            _desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }
        _desc.MinLOD = description.LodMinClamp;
        _desc.MaxLOD = description.LodMaxClamp;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public void CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SAMPLER_DESC desc = _desc;
        _device.Handle->CreateSampler(&desc, handle);
    }

    protected internal override void Destroy()
    {
    }
}
