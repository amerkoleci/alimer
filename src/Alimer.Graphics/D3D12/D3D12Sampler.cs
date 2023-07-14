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
    private readonly D3D12_CPU_DESCRIPTOR_HANDLE _handle;

    public D3D12Sampler(D3D12GraphicsDevice device, in SamplerDescription description)
        : base(description)
    {
        _device = device;
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        D3D12_SAMPLER_DESC d3dDesc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        d3dDesc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), 16u);
        if (d3dDesc.MaxAnisotropy > 1)
        {
            d3dDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
        }
        else
        {
            d3dDesc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reductionType);
        }

        d3dDesc.AddressU = description.AddressModeU.ToD3D12();
        d3dDesc.AddressV = description.AddressModeV.ToD3D12();
        d3dDesc.AddressW = description.AddressModeW.ToD3D12();
        d3dDesc.MipLODBias = 0.0f;
        if (description.Compare != CompareFunction.Never)
        {
            d3dDesc.ComparisonFunc = description.Compare.ToD3D12();
        }
        else
        {
            // Still set the function so it's not garbage.
            d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }
        d3dDesc.MinLOD = description.LodMinClamp;
        d3dDesc.MaxLOD = description.LodMaxClamp;

        _handle = device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device.Handle->CreateSampler(&d3dDesc, _handle);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public D3D12_CPU_DESCRIPTOR_HANDLE Handle => _handle;

    protected internal override void Destroy()
    {
        _device.FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _handle);
    }
}
