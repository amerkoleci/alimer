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
    public const string LibraryName = AlimerApi.LibraryName;

    [Flags]
    internal enum ALLOCATOR_FLAGS
    {
        ALLOCATOR_FLAG_NONE = 0,
        ALLOCATOR_FLAG_SINGLETHREADED = 0x1,
        ALLOCATOR_FLAG_ALWAYS_COMMITTED = 0x2,
        ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED = 0x4,
        ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED = 0x8,
        ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED = 0x10,
    }

    [Flags]
    internal enum ALLOCATION_FLAGS
    {
        ALLOCATION_FLAG_NONE = 0,
        ALLOCATION_FLAG_COMMITTED = 0x1,
        ALLOCATION_FLAG_NEVER_ALLOCATE = 0x2,
        ALLOCATION_FLAG_WITHIN_BUDGET = 0x4,
        ALLOCATION_FLAG_UPPER_ADDRESS = 0x8,
        ALLOCATION_FLAG_CAN_ALIAS = 0x10,
        ALLOCATION_FLAG_STRATEGY_MIN_MEMORY = 0x00010000,
        ALLOCATION_FLAG_STRATEGY_MIN_TIME = 0x00020000,
        ALLOCATION_FLAG_STRATEGY_MIN_OFFSET = 0x0004000,
        ALLOCATION_FLAG_STRATEGY_BEST_FIT = ALLOCATION_FLAG_STRATEGY_MIN_MEMORY,
        ALLOCATION_FLAG_STRATEGY_FIRST_FIT = ALLOCATION_FLAG_STRATEGY_MIN_TIME,
        /// A bit mask to extract only `STRATEGY` bits from entire set of flags.
        ALLOCATION_FLAG_STRATEGY_MASK =
            ALLOCATION_FLAG_STRATEGY_MIN_MEMORY |
            ALLOCATION_FLAG_STRATEGY_MIN_TIME |
            ALLOCATION_FLAG_STRATEGY_MIN_OFFSET,
    }

    internal struct ALLOCATION_CALLBACKS
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

    internal struct ALLOCATOR_DESC
    {
        public ALLOCATOR_FLAGS Flags;
        public ID3D12Device* pDevice;
        public ulong PreferredBlockSize;
        public ALLOCATION_CALLBACKS* pAllocationCallbacks;
        public IDXGIAdapter* pAdapter;
    }

    internal struct ALLOCATION_DESC
    {
        public ALLOCATION_FLAGS Flags;
        public D3D12_HEAP_TYPE HeapType;
        public D3D12_HEAP_FLAGS ExtraHeapFlags;
        public /*Pool**/nint CustomPool;
        /// Custom general-purpose pointer that will be stored in D3D12MA::Allocation.
        public void* pPrivateData;
    }

    internal struct Statistics
    {
        public uint BlockCount;
        public uint AllocationCount;
        public ulong BlockBytes;
        public ulong AllocationBytes;
    }

    internal struct DetailedStatistics
    {
        public Statistics Stats;
        public uint UnusedRangeCount;
        public ulong AllocationSizeMin;
        public ulong AllocationSizeMax;
        public ulong UnusedRangeSizeMin;
        public ulong UnusedRangeSizeMax;
    }

    internal struct TotalStatistics
    {
        public _HeapType_FixedBuffer HeapType;
        public _MemorySegmentGroup_FixedBuffer MemorySegmentGroup;
        public DetailedStatistics Total;

        [InlineArray(5)]
        public partial struct _HeapType_FixedBuffer
        {
            public DetailedStatistics e0;
        }

        [InlineArray(2)]
        public partial struct _MemorySegmentGroup_FixedBuffer
        {
            public DetailedStatistics e0;
        }
    }

    [SkipLocalsInit]
    public static HRESULT CreateAllocator(in ALLOCATOR_DESC desc, out nint allocator)
    {
        Unsafe.SkipInit(out allocator);

        fixed (ALLOCATOR_DESC* descPtr = &desc)
        {
            fixed (nint* allocatorPtr = &allocator)
            {
                return CreateAllocator(descPtr, allocatorPtr);
            }
        }
    }

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_CreateAllocator")]
    private static partial HRESULT CreateAllocator(ALLOCATOR_DESC* pDesc, nint* ppAllocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_AddRef")]
    public static partial uint Allocator_AddRef(nint allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_Release")]
    public static partial uint Allocator_Release(nint allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CalculateStatistics")]
    public static partial void Allocator_CalculateStatistics(nint allocator, TotalStatistics* pStats);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource2")]
    public static partial HRESULT Allocator_CreateResource2(
        nint allocator,
        ALLOCATION_DESC* pAllocationDesc,
        D3D12_RESOURCE_DESC1* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        D3D12_CLEAR_VALUE* pOptimizedClearValue,
        out nint allocation,
        Guid* riidResource, void** ppvResource
        );

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource3")]
    public static partial HRESULT Allocator_CreateResource3(
        nint allocator,
        ALLOCATION_DESC* pAllocationDesc,
        D3D12_RESOURCE_DESC1* pResourceDesc,
        D3D12_BARRIER_LAYOUT InitialLayout,
        D3D12_CLEAR_VALUE* pOptimizedClearValue,
        uint NumCastableFormats,
        DXGI_FORMAT* pCastableFormats,
        out nint allocation,
        Guid* riidResource, void** ppvResource
        );

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocation_AddRef")]
    public static partial uint Allocation_AddRef(nint allocation);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocation_Release")]
    public static partial uint Allocation_Release(nint allocation);
}
