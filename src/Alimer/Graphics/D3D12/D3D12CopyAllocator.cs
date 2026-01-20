// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CopyAllocator : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12CommandQueue> _queue;
    private readonly Lock _lock = new();
    private readonly List<D3D12UploadContext> _freelist = [];

    public D3D12CopyAllocator(D3D12GraphicsDevice device)
    {
        _device = device;

        D3D12_COMMAND_QUEUE_DESC desc = new();
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        desc.Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        HRESULT hr = device.Device->CreateCommandQueue(&desc,
            __uuidof<ID3D12CommandQueue>(),
            (void**)_queue.GetAddressOf()
            );
        if (hr.FAILED)
        {
            throw new GraphicsException("D3D12: CopyAllocator failed to create CommandQueue");
        }

        _queue.Get()->SetName("CopyAllocator");
    }

    public void Dispose()
    {
        _queue.Dispose();

        foreach (D3D12UploadContext context in _freelist)
        {
            context.CommandAllocator.Dispose();
            context.CommandList.Dispose();
            context.Fence.Dispose();

            context.UploadBuffer.Dispose();
            //context.uploadBufferData = nullptr;
        }
    }

    public D3D12UploadContext Allocate(ulong stagingSize)
    {
        D3D12UploadContext context = new();

        lock (_lock)
        {
            // Try to search for a staging buffer that can fit the request:
            for (int i = 0; i < _freelist.Count; ++i)
            {
                if (_freelist[i].UploadBufferSize >= stagingSize)
                {
                    context = _freelist[i];
                    D3D12UploadContext temp = _freelist[i];
                    _freelist[i] = _freelist[_freelist.Count - 1];
                    _freelist[_freelist.Count - 1] = temp;
                    _freelist.RemoveAt(_freelist.Count - 1);
                    break;
                }
            }
        }

        // If no buffer was found that fits the data, create one
        if (!context.IsValid)
        {
            HRESULT hr = _device.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
                __uuidof<ID3D12CommandAllocator>(),
                (void**)context.CommandAllocator.GetAddressOf()
                );
            ThrowIfFailed(hr);

            hr = _device.Device->CreateCommandList(0,
                D3D12_COMMAND_LIST_TYPE_COPY,
                context.CommandAllocator.Get(),
                null,
                __uuidof<ID3D12CommandList>(),
                (void**)context.CommandList.GetAddressOf()
                );
            ThrowIfFailed(hr);
            ThrowIfFailed(context.CommandList.Get()->Close());

            context.Fence = _device.Device->CreateFence();
            context.Fence.Get()->SetName("CopyAllocator::Fence");

            context.UploadBufferSize = Math.Max(BitOperations.RoundUpToPowerOf2(stagingSize), 65536);

            BufferDescriptor uploadBufferDesc = new(context.UploadBufferSize, BufferUsage.None, MemoryType.Upload, "CopyAllocator::UploadBuffer");
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
        ThrowIfFailed(context.Fence.Get()->Signal(0));

        ID3D12CommandList** commandLists = stackalloc ID3D12CommandList*[1]
        {
            (ID3D12CommandList*)context.CommandList.Get()
        };

        _queue.Get()->ExecuteCommandLists(1, commandLists);
        ThrowIfFailed(_queue.Get()->Signal(context.Fence.Get(), 1));
        ThrowIfFailed(context.Fence.Get()->SetEventOnCompletion(1, HANDLE.NULL));

        lock (_lock)
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
