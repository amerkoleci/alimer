// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D11;
using static Win32.Apis;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11QueryHeap : QueryHeap
{
    // ID3D11Predicate?
    private readonly ComPtr<ID3D11Query> _handle;

    public D3D11QueryHeap(D3D11GraphicsDevice device, in QueryHeapDescriptor descriptor)
        : base(device, descriptor)
    {
        QueryDescription queryDesc = new()
        {
            Query = descriptor.Type.ToD3D11(),
            MiscFlags = QueryMiscFlags.None,
        };
        HResult hr = device.Handle->CreateQuery(&queryDesc, _handle.GetAddressOf());
        if (hr.Failure)
        {
            //return nullptr;
        }

        if (string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public ID3D11Query* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D11QueryHeap" /> class.
    /// </summary>
    ~D3D11QueryHeap() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _handle.Get()->SetDebugName(newLabel);
    }
}
