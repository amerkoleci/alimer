// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BINDING_TIER;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindlessDescriptorSet : IDisposable
{
    public D3D12BindlessDescriptorSet(D3D12GraphicsDevice device)
    {
        Device = device;

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
        Resources = new D3D12BindlessDescriptorHeap(this, resourceCapacity - nonBindlessResourcesCount);
        Sampler = new D3D12BindlessDescriptorHeap(this, samplersCapacity - nonBindlessSamplerCount);
    }


    public D3D12GraphicsDevice Device { get; }
    public D3D12BindlessDescriptorHeap Resources { get; }
    public D3D12BindlessDescriptorHeap Sampler { get; }

    public void Dispose()
    {
        //Samplers.Dispose();
        GC.SuppressFinalize(this);
    }

    internal class D3D12BindlessDescriptorHeap
    {
        private readonly Lock _lock = new();
        private readonly List<int> _freeList = [];

        public D3D12BindlessDescriptorHeap(D3D12BindlessDescriptorSet parent, uint capacity)
        {
            ArgumentNullException.ThrowIfNull(parent);
            Parent = parent;

            Capacity = capacity;
            for (int i = 0; i < capacity; ++i)
            {
                _freeList.Add((int)capacity - i - 1);
            }
        }

        public D3D12BindlessDescriptorSet Parent { get; }
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
