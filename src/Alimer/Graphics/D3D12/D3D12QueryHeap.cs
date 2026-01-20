// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static TerraFX.Interop.DirectX.D3D12_QUERY_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_QUERY_TYPE;
using static TerraFX.Interop.Windows.Windows;
using TerraFX.Interop.Windows;
using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12QueryHeap : QueryHeap
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12QueryHeap> _handle;

    public D3D12QueryHeap(D3D12GraphicsDevice device, in QueryHeapDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        D3D12_QUERY_HEAP_DESC heapDesc = new()
        {
            Count = descriptor.Count,
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

        switch (descriptor.Type)
        {
            case QueryType.Occlusion:
                heapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
                BackendQueryType = D3D12_QUERY_TYPE_OCCLUSION;
                QueryResultSize = sizeof(ulong);
                break;

            case QueryType.Timestamp:
                heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                BackendQueryType = D3D12_QUERY_TYPE_TIMESTAMP;
                QueryResultSize = sizeof(ulong);
                break;

            case QueryType.PipelineStatistics:
                if (_device.DxAdapter.Features.MeshShaderPipelineStatsSupported)
                {
                    heapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS1;
                    QueryResultSize = (uint)sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS1);
                    BackendQueryType = D3D12_QUERY_TYPE_PIPELINE_STATISTICS1;
                }
                else
                {
                    heapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
                    QueryResultSize = (uint)sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
                    BackendQueryType = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
                }
                break;
        }

    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12QueryHeap* Handle => _handle;

    public D3D12_QUERY_TYPE BackendQueryType { get; }

    /// <inheritdoc />
    public override uint QueryResultSize { get; }

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
