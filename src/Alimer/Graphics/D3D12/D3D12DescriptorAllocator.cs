// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_FLAGS;
using static TerraFX.Interop.Windows.Windows;
using DescriptorIndex = System.UInt32;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12DescriptorAllocator : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12DescriptorHeap> _heap;
    private readonly ComPtr<ID3D12DescriptorHeap> _shaderVisibleHeap;
    private D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandle = default;
    private D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandleShaderVisible = default;
    private D3D12_GPU_DESCRIPTOR_HANDLE _startGpuHandleShaderVisible = default;
    private readonly Lock _lock = new();
    private bool[] _allocatedDescriptors = [];
    private DescriptorIndex _searchStart;
    private DescriptorIndex _searchBindlessStart;

    private const DescriptorIndex InvalidDescriptorIndex = ~0u;

    public D3D12DescriptorAllocator(D3D12GraphicsDevice device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint numDescriptors, bool shaderVisible, uint nonBindlessCount = 0)
    {
        _device = device;
        HeapType = type;
        NumDescriptors = numDescriptors;
        ShaderVisible = shaderVisible;
        Stride = device.Device->GetDescriptorHandleIncrementSize(type);
        if (ShaderVisible)
        {
            _searchBindlessStart = 0;
            _searchStart = numDescriptors - nonBindlessCount;
        }
        _ = AllocateResources(numDescriptors);
    }

    public D3D12_DESCRIPTOR_HEAP_TYPE HeapType { get; }
    public uint NumDescriptors { get; private set; }
    public uint NumAllocatedDescriptors { get; private set; }
    public bool ShaderVisible { get; }
    public uint Stride { get; }

    public ID3D12DescriptorHeap* Heap => _heap;
    public ID3D12DescriptorHeap* ShaderVisibleHeap => _shaderVisibleHeap;

    /// <inheritdoc />
    public void Dispose()
    {
        _heap.Dispose();
        _shaderVisibleHeap.Dispose();
    }

    public DescriptorIndex AllocateBindlessDescriptor()
    {
        return AllocateDescriptors(true, 1);
    }

    public DescriptorIndex AllocateDescriptors(uint count = 1)
    {
        return AllocateDescriptors(false, count);
    }

    private DescriptorIndex AllocateDescriptors(bool bindless, uint count)
    {
        lock (_lock)
        {
            DescriptorIndex foundIndex = 0;
            uint freeCount = 0;
            bool found = false;
            DescriptorIndex startIndex = bindless ? _searchBindlessStart : _searchStart;

            // Find a contiguous range of 'count' indices for which _allocatedDescriptors[index] is false
            for (DescriptorIndex index = startIndex; index < NumDescriptors; index++)
            {
                if (_allocatedDescriptors[index])
                    freeCount = 0;
                else
                    freeCount += 1;

                if (freeCount >= count)
                {
                    int foundIndexInt = (int)index - (int)count + 1;

                    foundIndex = (uint)foundIndexInt;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                foundIndex = NumDescriptors;

                if (!Grow(NumDescriptors + count))
                {
                    Log.Error("Failed to grow a descriptor heap!");
                    return InvalidDescriptorIndex;
                }
            }

            for (DescriptorIndex index = foundIndex; index < foundIndex + count; index++)
            {
                _allocatedDescriptors[index] = true;
            }

            NumAllocatedDescriptors += count;
            if (bindless && ShaderVisible)
            {
                _searchBindlessStart = foundIndex + count;
            }
            else
            {
                _searchStart = foundIndex + count;
            }
            return foundIndex;
        }
    }

    public void ReleaseBindlessDescriptor(DescriptorIndex index)
    {
        ReleaseDescriptors(true, index, 1);
    }

    public void ReleaseDescriptors(DescriptorIndex baseIndex, uint count = 1)
    {
        ReleaseDescriptors(false, baseIndex, count);
    }

    private void ReleaseDescriptors(bool bindless, DescriptorIndex baseIndex, uint count = 1)
    {
        if (count == 0)
            return;

        lock (_lock)
        {
            for (DescriptorIndex index = baseIndex; index < baseIndex + count; index++)
            {
#if DEBUG
                if (!_allocatedDescriptors[index])
                {
                    Log.Error("Attempted to release an un-allocated descriptor");
                }
#endif

                _allocatedDescriptors[index] = false;
            }

            NumAllocatedDescriptors -= count;

            if (bindless)
            {
                if (_searchBindlessStart > baseIndex)
                    _searchBindlessStart = baseIndex;
            }
            else
            {
                if (_searchStart > baseIndex)
                    _searchStart = baseIndex;
            }
        }
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(DescriptorIndex index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = _startCpuHandle;
        handle.ptr += index * Stride;
        return handle;
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleShaderVisible(DescriptorIndex index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = _startCpuHandleShaderVisible;
        handle.ptr += index * Stride;
        return handle;
    }

    public D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(DescriptorIndex index)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = _startGpuHandleShaderVisible;
        handle.ptr += index * Stride;
        return handle;
    }

    public void CopyToShaderVisibleHeap(DescriptorIndex index, uint count = 1u)
    {
        _device.Device->CopyDescriptorsSimple(count, GetCpuHandleShaderVisible(index), GetCpuHandle(index), HeapType);
    }

    private bool AllocateResources(uint numDescriptors)
    {
        NumDescriptors = numDescriptors;
        _heap.Dispose();
        _shaderVisibleHeap.Dispose();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = new()
        {
            Type = HeapType,
            NumDescriptors = numDescriptors,
            Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            NodeMask = 0
        };

        HRESULT hr = _device.Device->CreateDescriptorHeap(&heapDesc,
            __uuidof<ID3D12DescriptorHeap>(),
            (void**)_heap.GetAddressOf()
            );
        if (hr.FAILED)
            return false;

        _startCpuHandle = _heap.Get()->GetCPUDescriptorHandleForHeapStart();
        Array.Resize(ref _allocatedDescriptors, (int)numDescriptors);

        if (ShaderVisible)
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            hr = _device.Device->CreateDescriptorHeap(&heapDesc,
                __uuidof<ID3D12DescriptorHeap>(),
                (void**)_shaderVisibleHeap.GetAddressOf()
                );

            if (FAILED(hr))
                return false;

            _startCpuHandleShaderVisible = _shaderVisibleHeap.Get()->GetCPUDescriptorHandleForHeapStart();
            _startGpuHandleShaderVisible = _shaderVisibleHeap.Get()->GetGPUDescriptorHandleForHeapStart();
        }

        return true;
    }

    private bool Grow(uint minRequiredSize)
    {
        uint oldSize = NumDescriptors;
        uint newSize = BitOperations.RoundUpToPowerOf2(minRequiredSize);

        ComPtr<ID3D12DescriptorHeap> oldHeap = _heap;

        if (!AllocateResources(newSize))
            return false;

        _device.Device->CopyDescriptorsSimple(oldSize, _startCpuHandle, oldHeap.Get()->GetCPUDescriptorHandleForHeapStart(), HeapType);

        if (_shaderVisibleHeap.Get() is not null)
        {
            _device.Device->CopyDescriptorsSimple(oldSize, _startCpuHandleShaderVisible, oldHeap.Get()->GetCPUDescriptorHandleForHeapStart(), HeapType);
        }

        return true;
    }
}
