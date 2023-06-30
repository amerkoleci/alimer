// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using Win32;
using Win32.Graphics.Direct3D12;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Win32.Graphics.Direct3D12.Apis;
using static Win32.Apis;
using D3DResourceStates = Win32.Graphics.Direct3D12.ResourceStates;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Pipeline : Pipeline
{
    private readonly ComPtr<ID3D12PipelineState> _handle;

    public D3D12Pipeline(D3D12GraphicsDevice device, in RenderPipelineDescription description)
        : base(device, PipelineType.Render, description.Label)
    {
        GraphicsPipelineStateDescription d3dDesc = new();
        HResult hr = device.Handle->CreateGraphicsPipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            Log.Error("D3D12: Failed to create Render Pipeline.");
            return;
        }
    }

    public D3D12Pipeline(D3D12GraphicsDevice device, in ComputePipelineDescription description)
        : base(device, PipelineType.Compute, description.Label)
    {
        ComputePipelineStateDescription d3dDesc = new();
        HResult hr = device.Handle->CreateComputePipelineState(&d3dDesc, __uuidof<ID3D12PipelineState>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            Log.Error("D3D12: Failed to create Compute Pipeline.");
            return;
        }
    }

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
