// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_LAYOUT;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindlessDescriptorSet : IDisposable
{
    private const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;
    private const uint BINDLESS_RESOURCE_CAPACITY = 500000;
    private const uint BINDLESS_SAMPLER_CAPACITY = 256; // it is chosen to be addressable by 8 bits

    public D3D12BindlessDescriptorSet(D3D12GraphicsDevice device)
    {
        Device = device;
        Sampler = new D3D12BindlessDescriptorHeap(this, BINDLESS_SAMPLER_CAPACITY);
    }


    public D3D12GraphicsDevice Device { get; }
    public D3D12BindlessDescriptorHeap Sampler { get; }

    public void Dispose()
    {
        //Samplers.Dispose();
        GC.SuppressFinalize(this);
    }

    internal unsafe class D3D12BindlessDescriptorHeap
    {
        private readonly Lock _lock = new();
        private readonly List<uint> _freeList = [];

        public D3D12BindlessDescriptorHeap(D3D12BindlessDescriptorSet parent, uint descriptorCount)
        {
            ArgumentNullException.ThrowIfNull(parent);
            Parent = parent;

            DescriptorCount = descriptorCount;
            for (uint i = 0; i < descriptorCount; ++i)
            {
                _freeList.Add(descriptorCount - i - 1);
            }
        }

        public D3D12BindlessDescriptorSet Parent { get; }
        public uint DescriptorCount { get; }

        public uint Allocate()
        {
            lock (_lock)
            {
                if (_freeList.Count > 0)
                {
                    uint index = _freeList[_freeList.Count - 1];
                    _freeList.RemoveAt(_freeList.Count - 1);
                    return index;
                }
            }

            return uint.MaxValue;
        }

        public void Free(uint index)
        {
            if (index == uint.MaxValue)
                return;

            lock (_lock)
            {
                _freeList.Add(index);
            }
        }
    }
}
