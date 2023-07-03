// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Win32;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12DescriptorAllocator : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly List<ComPtr<ID3D12DescriptorHeap>> _heaps = new();
    private readonly List<CpuDescriptorHandle> _freeList = new();

    public D3D12DescriptorAllocator(D3D12GraphicsDevice device, DescriptorHeapType type, uint descriptorsPerBlock)
    {
        _device = device;
        Type = type;
        DescriptorsPerBlock = descriptorsPerBlock;
        DescriptorSize = device.Handle->GetDescriptorHandleIncrementSize(type);
    }

    public DescriptorHeapType Type { get; }
    public uint DescriptorsPerBlock { get; }

    public uint DescriptorSize { get; }

    /// <inheritdoc />
    public void Dispose()
    {
        foreach (var heap in _heaps)
        {
            heap.Dispose();
        }

        _heaps.Clear();
    }

    public CpuDescriptorHandle Allocate()
    {
        lock (_freeList)
        {
            if (_freeList.Count == 0)
            {
                BlockAllocate();
            }
            Debug.Assert(_freeList.Count > 0);

            CpuDescriptorHandle handle = _freeList[_freeList.Count - 1];
            _freeList.RemoveAt(_freeList.Count - 1);
            return handle;
        }
    }

    public void Free(in CpuDescriptorHandle index)
    {
        lock (_freeList)
        {
            _freeList.Add(index);
        }
    }

    private void BlockAllocate()
    {
        ComPtr<ID3D12DescriptorHeap> heap = default;
        DescriptorHeapDescription heapDesc = new()
        {
            Type = Type,
            NumDescriptors = DescriptorsPerBlock,
            NodeMask = 0,
        };
        ThrowIfFailed(_device.Handle->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), heap.GetVoidAddressOf()));

        _heaps.Add(heap);

        CpuDescriptorHandle heapStart = heap.Get()->GetCPUDescriptorHandleForHeapStart();
        for (uint i = 0; i < heapDesc.NumDescriptors; ++i)
        {
            CpuDescriptorHandle handle = heapStart;
            handle.ptr += i * DescriptorSize;
            _freeList.Add(handle);
        }
    }
}
