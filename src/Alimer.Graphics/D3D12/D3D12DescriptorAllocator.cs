// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12DescriptorAllocator : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly List<ComPtr<ID3D12DescriptorHeap>> _heaps = new();
    private readonly List<D3D12_CPU_DESCRIPTOR_HANDLE> _freeList = new();

    public D3D12DescriptorAllocator(D3D12GraphicsDevice device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint descriptorsPerBlock)
    {
        _device = device;
        Type = type;
        DescriptorsPerBlock = descriptorsPerBlock;
        DescriptorSize = device.Handle->GetDescriptorHandleIncrementSize(type);
    }

    public D3D12_DESCRIPTOR_HEAP_TYPE Type { get; }
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

    public D3D12_CPU_DESCRIPTOR_HANDLE Allocate()
    {
        lock (_freeList)
        {
            if (_freeList.Count == 0)
            {
                BlockAllocate();
            }
            Debug.Assert(_freeList.Count > 0);

            D3D12_CPU_DESCRIPTOR_HANDLE handle = _freeList[_freeList.Count - 1];
            _freeList.RemoveAt(_freeList.Count - 1);
            return handle;
        }
    }

    public void Free(in D3D12_CPU_DESCRIPTOR_HANDLE index)
    {
        lock (_freeList)
        {
            _freeList.Add(index);
        }
    }

    private void BlockAllocate()
    {
        ComPtr<ID3D12DescriptorHeap> heap = default;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = new()
        {
            Type = Type,
            NumDescriptors = DescriptorsPerBlock,
            NodeMask = 0,
        };
        ThrowIfFailed(_device.Handle->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), heap.GetVoidAddressOf()));

        _heaps.Add(heap);

        D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heap.Get()->GetCPUDescriptorHandleForHeapStart();
        for (uint i = 0; i < heapDesc.NumDescriptors; ++i)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = heapStart;
            handle.ptr += i * DescriptorSize;
            _freeList.Add(handle);
        }
    }
}
