// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_FLAGS;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12DescriptorAllocatorNew : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12DescriptorHeap> _heap;
    private readonly ComPtr<ID3D12DescriptorHeap> _shaderVisibleHeap;
    private D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandle = default;
    private D3D12_CPU_DESCRIPTOR_HANDLE _startCpuHandleShaderVisible  = default;
    private D3D12_GPU_DESCRIPTOR_HANDLE _startGpuHandleShaderVisible = default;

    private bool[]? _allocatedDescriptors;

    public D3D12DescriptorAllocatorNew(D3D12GraphicsDevice device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint numDescriptors, bool shaderVisible)
    {
        _device = device;
        HeapType = type;
        NumDescriptors = numDescriptors;
        ShaderVisible = shaderVisible;
        Stride = device.Handle->GetDescriptorHandleIncrementSize(type);

        Guard.IsTrue(AllocateResources(numDescriptors));
    }

    public D3D12_DESCRIPTOR_HEAP_TYPE HeapType { get; }
    public uint NumDescriptors { get; private set; }
    public bool ShaderVisible { get;}

    public uint Stride { get; }

    public ID3D12DescriptorHeap* Heap => _heap;
    public ID3D12DescriptorHeap* ShaderVisibleHeap => _shaderVisibleHeap;

    /// <inheritdoc />
    public void Dispose()
    {
        _heap.Dispose();
        _shaderVisibleHeap.Dispose();
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

        HRESULT hr = _device.Handle->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), _heap.GetVoidAddressOf());
        if (hr.FAILED)
            return false;

        _startCpuHandle = _heap.Get()->GetCPUDescriptorHandleForHeapStart();
        Array.Resize(ref _allocatedDescriptors, (int)numDescriptors);

        if (ShaderVisible)
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            hr = _device.Handle->CreateDescriptorHeap(&heapDesc, __uuidof<ID3D12DescriptorHeap>(), _shaderVisibleHeap.GetVoidAddressOf());

            if (FAILED(hr))
                return false;

            _startCpuHandleShaderVisible = _shaderVisibleHeap.Get()->GetCPUDescriptorHandleForHeapStart();
            _startGpuHandleShaderVisible = _shaderVisibleHeap.Get()->GetGPUDescriptorHandleForHeapStart();
        }

        return true;
    }
}
