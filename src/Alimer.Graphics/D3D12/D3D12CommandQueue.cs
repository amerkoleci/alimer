// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandQueue : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12CommandQueue> _handle;
    private readonly ComPtr<ID3D12Fence> _fence;

    public D3D12CommandQueue(D3D12GraphicsDevice device, QueueType type)
    {
        _device = device;

        CommandQueueDescription d3dDesc = new(type.ToD3D12(), CommandQueuePriority.Normal);
        HResult hr = device.Handle->CreateCommandQueue(&d3dDesc, __uuidof<ID3D12CommandQueue>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            throw new GraphicsException("D3D12: Failed to create CommandQueue");
        }

        hr = device.Handle->CreateFence(0, FenceFlags.Shared, __uuidof<ID3D12Fence>(), _fence.GetVoidAddressOf());
        if (hr.Failure)
        {
            throw new GraphicsException("D3D12: Failed to create Fence");
        }

        switch (type)
        {
            case QueueType.Graphics:
                _handle.Get()->SetName("Graphics Queue");
                _fence.Get()->SetName("GraphicsQueue - Fence");
                break;
            case QueueType.Compute:
                _handle.Get()->SetName("Compute Queue");
                _fence.Get()->SetName("ComputeQueue - Fence");
                break;
            case QueueType.Copy:
                _handle.Get()->SetName("CopyQueue");
                _fence.Get()->SetName("CopyQueue - Fence");
                break;
        }
    }


    public ID3D12CommandQueue* Handle => _handle;

    /// <inheritdoc />
    public void Dispose()
    {
        _fence.Dispose();
        _handle.Dispose();
    }
}
