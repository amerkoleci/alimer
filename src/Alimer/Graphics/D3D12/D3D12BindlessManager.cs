// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BINDING_TIER;
using static TerraFX.Interop.DirectX.D3D_SHADER_MODEL;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12;
using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindlessManager : IDisposable
{
    private readonly D3D12BindlessDescriptorHeap _resources;
    private readonly D3D12BindlessDescriptorHeap _samplers;

    public D3D12BindlessManager(D3D12GraphicsDevice device)
    {
        Device = device;
        // ResourceDescriptorHeap and SamplerDescriptorHeap access in shaders
        UltimateBindless = device.DxAdapter.Features.HighestShaderModel >= D3D_SHADER_MODEL_6_6;

        uint MaxNonSamplerDescriptors = 0;
        uint MaxSamplerDescriptors = 0;
        D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19 = default;
        if (FAILED(device.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, ref options19)))
        {
            if (device.DxAdapter.Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            }
            else if (device.DxAdapter.Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_2)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            else
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            MaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        }
        else
        {
            MaxNonSamplerDescriptors = options19.MaxViewDescriptorHeapSize;
            MaxSamplerDescriptors = options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers;
        }

        uint resourceCapacity = Math.Min(BindlessResourceCapacity, MaxNonSamplerDescriptors);
        uint samplersCapacity = Math.Min(BindlessSamplerCapacity, MaxSamplerDescriptors);

        uint nonBindlessResourcesCount = 1024;
        uint nonBindlessSamplerCount = 32;
        _resources = new D3D12BindlessDescriptorHeap(this, resourceCapacity - nonBindlessResourcesCount);
        _samplers = new D3D12BindlessDescriptorHeap(this, samplersCapacity - nonBindlessSamplerCount);
    }


    public D3D12GraphicsDevice Device { get; }
    public bool UltimateBindless { get; }

    public void Dispose()
    {
        //Samplers.Dispose();
        GC.SuppressFinalize(this);
    }

    public int AllocateSRV(ID3D12Resource* resource, in D3D12_SHADER_RESOURCE_VIEW_DESC desc)
    {
        int index = _resources.Allocate();

        if (index != InvalidBindlessIndex)
        {
            Debug.Assert(index < _resources.Capacity);

            uint descriptorIndex = Device.ShaderResourceViewHeap.AllocateBindlessDescriptor();
            D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = Device.ShaderResourceViewHeap.GetCpuHandle(descriptorIndex);

            fixed (D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc = &desc)
            {
                Device.Device->CreateShaderResourceView(resource, pDesc, descriptorHandle);
            }

            Device.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorIndex);
        }

        return index;
    }

    public int AllocateUAV(ID3D12Resource* resource, in D3D12_UNORDERED_ACCESS_VIEW_DESC desc)
    {
        int index = _resources.Allocate();

        if (index != InvalidBindlessIndex)
        {
            Debug.Assert(index < _resources.Capacity);

            uint descriptorIndex = Device.ShaderResourceViewHeap.AllocateBindlessDescriptor();
            D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = Device.ShaderResourceViewHeap.GetCpuHandle(descriptorIndex);

            fixed (D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc = &desc)
            {
                Device.Device->CreateUnorderedAccessView(resource, null, pDesc, descriptorHandle);
            }

            Device.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorIndex);
        }

        return index;
    }

    public int AllocateSampler(in D3D12_SAMPLER_DESC desc)
    {
        int index = _samplers.Allocate();

        if (index != InvalidBindlessIndex)
        {
            Debug.Assert(index < _samplers.Capacity);

            uint descriptorIndex = Device.SamplerHeap.AllocateBindlessDescriptor();
            D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = Device.SamplerHeap.GetCpuHandle(descriptorIndex);

            fixed (D3D12_SAMPLER_DESC* pDesc = &desc)
            {
                Device.Device->CreateSampler(pDesc, descriptorHandle);
            }

            Device.SamplerHeap.CopyToShaderVisibleHeap(descriptorIndex);
        }

        return index;
    }

    public void FreeSRV(int index)
    {
        if (index == InvalidBindlessIndex)
            return;

        Debug.Assert(index < _resources.Capacity);
        _resources.Free(index);
        Device.ShaderResourceViewHeap.ReleaseBindlessDescriptor((uint)index);
    }

    public void FreeUAV(int index)
    {
        if (index == InvalidBindlessIndex)
            return;

        Debug.Assert(index < _resources.Capacity);
        _resources.Free(index);
        Device.ShaderResourceViewHeap.ReleaseBindlessDescriptor((uint)index);
    }

    public void FreeSampler(int index)
    {
        if (index == InvalidBindlessIndex)
            return;

        Debug.Assert(index < _samplers.Capacity);
        _samplers.Free(index);
        Device.SamplerHeap.ReleaseBindlessDescriptor((uint)index);
    }

    class D3D12BindlessDescriptorHeap
    {
        private readonly Lock _lock = new();
        private readonly List<int> _freeList = [];

        public D3D12BindlessDescriptorHeap(D3D12BindlessManager parent, uint capacity)
        {
            ArgumentNullException.ThrowIfNull(parent);
            Parent = parent;

            Capacity = capacity;
            for (int i = 0; i < capacity; ++i)
            {
                _freeList.Add((int)capacity - i - 1);
            }
        }

        public D3D12BindlessManager Parent { get; }
        public uint Capacity { get; }

        public int Allocate()
        {
            lock (_lock)
            {
                if (_freeList.Count > 0)
                {
                    int index = _freeList[_freeList.Count - 1];
                    _freeList.RemoveAt(_freeList.Count - 1);
                    return index;
                }
            }

            return InvalidBindlessIndex;
        }

        public void Free(int index)
        {
            if (index == InvalidBindlessIndex)
                return;

            lock (_lock)
            {
                _freeList.Add(index);
            }
        }
    }
}
