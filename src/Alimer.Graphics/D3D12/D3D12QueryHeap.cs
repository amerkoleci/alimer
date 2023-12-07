// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;
using TerraFX.Interop.Windows;
using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12QueryHeap : QueryHeap
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12QueryHeap> _handle;

    public D3D12QueryHeap(D3D12GraphicsDevice device, in QueryHeapDescription description)
        : base(description)
    {
        _device = device;
        D3D12_QUERY_HEAP_DESC heapDesc = new()
        {
            Type = description.Type.ToD3D12(),
            Count = (uint)description.Count,
            NodeMask = 0u
        };
        HRESULT hr = device.Handle->CreateQueryHeap(&heapDesc, __uuidof<ID3D12QueryHeap>(), _handle.GetVoidAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create QueryHeap.");
            return;
        }

        if (string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12QueryHeap* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12QueryHeap" /> class.
    /// </summary>
    ~D3D12QueryHeap() => Dispose(disposing: false);

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
