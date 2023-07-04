// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Pipeline : Pipeline
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12PipelineState> _handle;

    public D3D12Pipeline(D3D12GraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        GraphicsPipelineStateDescription d3dDesc = new();
        HResult hr = device.Handle->CreateGraphicsPipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            Log.Error("D3D12: Failed to create Render Pipeline.");
            return;
        }
    }

    public D3D12Pipeline(D3D12GraphicsDevice device, in ComputePipelineDescription description)
        : base(PipelineType.Compute, description.Label)
    {
        _device = device;
        ComputePipelineStateDescription d3dDesc = new();
        HResult hr = device.Handle->CreateComputePipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            Log.Error("D3D12: Failed to create Compute Pipeline.");
            return;
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12PipelineState* Handle => _handle;

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
