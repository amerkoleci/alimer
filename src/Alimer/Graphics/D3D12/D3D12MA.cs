// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;

#pragma warning disable CS0649

namespace Alimer.Graphics.D3D12;

internal unsafe partial class D3D12MA
{
    public const string LibraryName = "d3d12ma";

    #region Handles
    public readonly record struct D3D12MA_Allocator(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static D3D12MA_Allocator Null => default;
    }

    public readonly record struct D3D12MA_Allocation(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static D3D12MA_Allocation Null => default;
    }
    public readonly record struct D3D12MA_Pool(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static D3D12MA_Pool Null => default;
    }
    #endregion

    [Flags]
    public enum D3D12MA_ALLOCATOR_FLAGS
    {
        D3D12MA_ALLOCATOR_FLAG_NONE = 0x00,
        D3D12MA_ALLOCATOR_FLAG_SINGLETHREADED = 0x01,
        D3D12MA_ALLOCATOR_FLAG_ALWAYS_COMMITTED = 0x02,
        D3D12MA_ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED = 0x04,
        D3D12MA_ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED = 0x08,
        D3D12MA_ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED = 0x10,
    }

    [Flags]
    public enum D3D12MA_ALLOCATION_FLAGS
    {
        D3D12MA_ALLOCATION_FLAG_NONE = 0x00000000,
        D3D12MA_ALLOCATION_FLAG_COMMITTED = 0x00000001,
        D3D12MA_ALLOCATION_FLAG_NEVER_ALLOCATE = 0x00000002,
        D3D12MA_ALLOCATION_FLAG_WITHIN_BUDGET = 0x00000004,
        D3D12MA_ALLOCATION_FLAG_UPPER_ADDRESS = 0x00000008,
        D3D12MA_ALLOCATION_FLAG_CAN_ALIAS = 0x00000010,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY = 0x00010000,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME = 0x00020000,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET = 0x0004000,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_BEST_FIT = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_FIRST_FIT = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME,
        D3D12MA_ALLOCATION_FLAG_STRATEGY_MASK = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME | D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET,
    }

    public struct D3D12MA_ALLOCATION_CALLBACKS
    {
        /// <summary>Allocation function.</summary>
        //[NativeTypeName("ALLOCATE_FUNC_PTR")]
        public delegate* unmanaged<nuint, nuint, void*, void*> pAllocate;

        /// <summary>Dellocation function.</summary>
        //[NativeTypeName("FREE_FUNC_PTR")]
        public delegate* unmanaged<void*, void*, void> pFree;

        /// <summary>
        /// Custom data that will be passed to allocation and deallocation functions as `pUserData` parameter.
        /// </summary>
        public void* pPrivateData;
    }

    public struct D3D12MA_ALLOCATOR_DESC
    {
        public D3D12MA_ALLOCATOR_FLAGS Flags;
        public ID3D12Device* pDevice;
        public ulong PreferredBlockSize;
        public D3D12MA_ALLOCATION_CALLBACKS* pAllocationCallbacks;
        public IDXGIAdapter* pAdapter;
    }

    public struct D3D12MA_ALLOCATION_DESC
    {
        public D3D12MA_ALLOCATION_FLAGS Flags;
        public D3D12_HEAP_TYPE HeapType;
        public D3D12_HEAP_FLAGS ExtraHeapFlags;
        public D3D12MA_Pool CustomPool;
        /// Custom general-purpose pointer that will be stored in D3D12MA::Allocation.
        public void* pPrivateData;
    }

    public struct D3D12MA_Statistics
    {
        public uint BlockCount;
        public uint AllocationCount;
        public ulong BlockBytes;
        public ulong AllocationBytes;
    }

    public struct D3D12MA_DetailedStatistics
    {
        public D3D12MA_Statistics Stats;
        public uint UnusedRangeCount;
        public ulong AllocationSizeMin;
        public ulong AllocationSizeMax;
        public ulong UnusedRangeSizeMin;
        public ulong UnusedRangeSizeMax;
    }

    public struct D3D12MA_TotalStatistics
    {
        public _HeapType_FixedBuffer HeapType;
        public _MemorySegmentGroup_FixedBuffer MemorySegmentGroup;
        public D3D12MA_DetailedStatistics Total;

        [InlineArray(5)]
        public partial struct _HeapType_FixedBuffer
        {
            public D3D12MA_DetailedStatistics e0;
        }

        [InlineArray(2)]
        public partial struct _MemorySegmentGroup_FixedBuffer
        {
            public D3D12MA_DetailedStatistics e0;
        }
    }

    [SkipLocalsInit]
    public static HRESULT D3D12MA_CreateAllocator(D3D12MA_ALLOCATOR_DESC* desc, out D3D12MA_Allocator allocator)
    {
        Unsafe.SkipInit(out allocator);

        fixed (D3D12MA_Allocator* allocatorPtr = &allocator)
            return D3D12MA_CreateAllocator(desc, allocatorPtr);
    }

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_CreateAllocator")]
    private static partial HRESULT D3D12MA_CreateAllocator(D3D12MA_ALLOCATOR_DESC* pDesc, D3D12MA_Allocator* ppAllocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_AddRef")]
    public static partial uint D3D12MA_Allocator_AddRef(D3D12MA_Allocator allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_Release")]
    public static partial uint D3D12MA_Allocator_Release(D3D12MA_Allocator allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CalculateStatistics")]
    public static partial void D3D12MA_Allocator_CalculateStatistics(D3D12MA_Allocator allocator, D3D12MA_TotalStatistics* pStats);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource2")]
    public static partial HRESULT D3D12MA_Allocator_CreateResource2(
        D3D12MA_Allocator allocator,
        D3D12MA_ALLOCATION_DESC* pAllocationDesc,
        D3D12_RESOURCE_DESC1* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        D3D12_CLEAR_VALUE* pOptimizedClearValue,
        out D3D12MA_Allocation allocation,
        Guid* riidResource, void** ppvResource
        );

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource3")]
    public static partial HRESULT D3D12MA_Allocator_CreateResource3(
        D3D12MA_Allocator allocator,
        D3D12MA_ALLOCATION_DESC* pAllocationDesc,
        D3D12_RESOURCE_DESC1* pResourceDesc,
        D3D12_BARRIER_LAYOUT InitialLayout,
        D3D12_CLEAR_VALUE* pOptimizedClearValue,
        uint NumCastableFormats,
        DXGI_FORMAT* pCastableFormats,
        out D3D12MA_Allocation allocation,
        Guid* riidResource, void** ppvResource
        );

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocation_AddRef")]
    public static partial uint D3D12MA_Allocation_AddRef(D3D12MA_Allocation allocation);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocation_Release")]
    public static partial uint D3D12MA_Allocation_Release(D3D12MA_Allocation allocation);
}
