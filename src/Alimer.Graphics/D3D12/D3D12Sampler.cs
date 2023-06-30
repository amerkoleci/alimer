// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32.Graphics.Direct3D12;
using static Win32.Graphics.Direct3D12.Apis;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12Sampler : Sampler
{
    private readonly CpuDescriptorHandle _handle;

    public D3D12Sampler(D3D12GraphicsDevice device, in SamplerDescription description)
        : base(device, description)
    {
        FilterType minFilter = description.MinFilter.ToD3D12();
        FilterType magFilter = description.MagFilter.ToD3D12();
        FilterType mipmapFilter = description.MipFilter.ToD3D12();

        FilterReductionType reduction = description.Compare != CompareFunction.Never ? FilterReductionType.Comparison : FilterReductionType.Standard;

        Win32.Graphics.Direct3D12.SamplerDescription d3dDesc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        d3dDesc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), 16u);
        if (d3dDesc.MaxAnisotropy > 1)
        {
            d3dDesc.Filter = EncodeAnisotropicFilter(reduction);
        }
        else
        {
            d3dDesc.Filter = EncodeBasicFilter(minFilter, magFilter, mipmapFilter, reduction);
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
            d3dDesc.ComparisonFunc = ComparisonFunction.Never;
        }
        d3dDesc.MinLOD = description.LodMinClamp;
        d3dDesc.MaxLOD = description.LodMaxClamp;

        //device.NativeDevice->CreateSampler(&d3dDesc, _handle);
    }

    public CpuDescriptorHandle Handle => _handle;

    protected internal override void Destroy()
    {

    }
}
