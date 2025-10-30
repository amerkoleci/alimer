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

    public D3D12QueryHeap(D3D12GraphicsDevice device, in QueryHeapDescription descriptor)
        : base(descriptor)
    {
        _device = device;
        D3D12_QUERY_HEAP_DESC heapDesc = new()
        {
            Type = descriptor.Type.ToD3D12(),
            Count = (uint)descriptor.Count,
            NodeMask = 0u
        };
        HRESULT hr = device.Device->CreateQueryHeap(&heapDesc, __uuidof<ID3D12QueryHeap>(), (void**)_handle.GetAddressOf());
        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create QueryHeap.");
            return;
        }

        if (string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        switch(descriptor.Type)
        {
            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
            case QueryType.Timestamp:
                QueryResultSize = sizeof(ulong);
                break;

            case QueryType.PipelineStatistics:
                QueryResultSize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
                break;
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12QueryHeap* Handle => _handle;
    public int QueryResultSize { get; }

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
    protected override void OnLabelChanged(string? newLabel)
    {
        _handle.Get()->SetName(newLabel);
    }
}
