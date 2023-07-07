// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FENCE_FLAGS;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CopyAllocator : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly List<D3D12UploadContext> _freelist = new();

    public D3D12CopyAllocator(D3D12GraphicsDevice device)
    {
        _device = device;
    }

    public void Dispose()
    {
        foreach (D3D12UploadContext context in _freelist)
        {
            context.CommandAllocator.Dispose();
            context.CommandList.Dispose();
            context.Fence.Dispose();

            context.UploadBuffer.Dispose();
            //context.uploadBufferData = nullptr;
        }
    }

    public D3D12UploadContext Allocate(ulong size)
    {
        D3D12UploadContext context = new();

        lock (_freelist)
        {
            // Try to search for a staging buffer that can fit the request:
            for (int i = 0; i < _freelist.Count; ++i)
            {
                if (_freelist[i].UploadBufferSize >= size)
                {
                    if (_freelist[i].Fence.Get()->GetCompletedValue() == 1)
                    {
                        ThrowIfFailed(_freelist[i].Fence.Get()->Signal(0));

                        context = _freelist[i];
                        D3D12UploadContext temp = _freelist[i];
                        _freelist[i] = _freelist[_freelist.Count - 1];
                        _freelist[_freelist.Count - 1] = temp;
                        _freelist.RemoveAt(_freelist.Count - 1);
                        break;
                    }
                }
            }
        }

        // If no buffer was found that fits the data, create one
        if (!context.IsValid)
        {
            HRESULT hr = _device.Handle->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, __uuidof<ID3D12CommandAllocator>(), context.CommandAllocator.GetVoidAddressOf());
            ThrowIfFailed(hr);

            hr = _device.Handle->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY,
                context.CommandAllocator.Get(),
                null,
                __uuidof<ID3D12CommandList>(), context.CommandList.GetVoidAddressOf()
                );
            ThrowIfFailed(hr);

            ThrowIfFailed(context.CommandList.Get()->Close());
            context.Fence = _device.Handle->CreateFence();

            context.UploadBufferSize = BitOperations.RoundUpToPowerOf2(size);

            BufferDescription uploadBufferDesc = new(context.UploadBufferSize, BufferUsage.None, CpuAccessMode.Write, "CopyAllocator::UploadBuffer");
            context.UploadBuffer = (D3D12Buffer)_device.CreateBuffer(in uploadBufferDesc);
        }

        // Begin command list in valid state
        ThrowIfFailed(context.CommandAllocator.Get()->Reset());
        ThrowIfFailed(context.CommandList.Get()->Reset(context.CommandAllocator.Get(), null));

        return context;
    }

    public void Submit(in D3D12UploadContext context)
    {
        ThrowIfFailed(context.CommandList.Get()->Close());
        ID3D12CommandList** commandLists = stackalloc ID3D12CommandList*[1]
        {
            (ID3D12CommandList*)context.CommandList.Get()
        };

        _device.CopyQueue.Handle->ExecuteCommandLists(1, commandLists);
        ThrowIfFailed(_device.CopyQueue.Handle->Signal(context.Fence.Get(), 1));
        ThrowIfFailed(_device.GraphicsQueue.Handle->Wait(context.Fence.Get(), 1));
        ThrowIfFailed(_device.ComputeQueue.Handle->Wait(context.Fence.Get(), 1));
        if (_device.VideDecodeQueue is not null)
        {
            ThrowIfFailed(_device.VideDecodeQueue.Handle->Wait(context.Fence.Get(), 1));
        }

        lock (_freelist)
        {
            _freelist.Add(context);
        }
    }
}

internal unsafe struct D3D12UploadContext
{
    public ComPtr<ID3D12CommandAllocator> CommandAllocator;
    public ComPtr<ID3D12GraphicsCommandList> CommandList;
    public ComPtr<ID3D12Fence> Fence;
    public ulong UploadBufferSize;
    public D3D12Buffer UploadBuffer = null!;

    public readonly bool IsValid => CommandList.Get() != null;

    public D3D12UploadContext()
    {

    }
}
