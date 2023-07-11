// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindGroupLayout : BindGroupLayout
{
    private readonly D3D12GraphicsDevice _device;

    public D3D12BindGroupLayout(D3D12GraphicsDevice device, in BindGroupLayoutDescription description)
        : base(description)
    {
        _device = device;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12BindGroupLayout" /> class.
    /// </summary>
    ~D3D12BindGroupLayout() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
