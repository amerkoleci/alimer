// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12Sampler : Sampler
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12_SAMPLER_DESC _desc;

    public D3D12Sampler(D3D12GraphicsDevice device, in SamplerDescriptor description)
        : base(description)
    {
        _device = device;
        _desc = D3D12Utils.ToD3D12SamplerDesc(in description);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public void CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SAMPLER_DESC desc = _desc;
        _device.Device->CreateSampler(&desc, handle);
    }

    protected internal override void Destroy()
    {
    }
}
