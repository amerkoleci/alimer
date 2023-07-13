// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindGroup : BindGroup
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12BindGroupLayout _layout;

    public D3D12BindGroup(D3D12GraphicsDevice device, BindGroupLayout layout, in BindGroupDescription description)
        : base(description)
    {
        _device = device;
        _layout = (D3D12BindGroupLayout)layout;
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12BindGroup" /> class.
    /// </summary>
    ~D3D12BindGroup() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override BindGroupLayout Layout => _layout;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
