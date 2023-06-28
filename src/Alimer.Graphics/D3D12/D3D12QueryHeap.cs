// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;
using D3DQueryHeapDescription = Win32.Graphics.Direct3D12.QueryHeapDescription;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12QueryHeap : QueryHeap
{
    private readonly ComPtr<ID3D12QueryHeap> _handle;

    public D3D12QueryHeap(D3D12GraphicsDevice device, in QueryHeapDescription description)
        : base(device, description)
    {
        D3DQueryHeapDescription heapDesc = new()
        {
            Type = description.Type.ToD3D12(),
            Count = (uint)description.Count,
            NodeMask = 0u
        };
        HResult hr = device.Handle->CreateQueryHeap(&heapDesc, __uuidof<ID3D12QueryHeap>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            //return nullptr;
        }

        if (string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

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
