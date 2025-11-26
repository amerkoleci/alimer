// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;

#pragma warning disable CS0649

namespace Alimer.Graphics.D3D12;

[Flags]
internal enum D3D12MA_ALLOCATOR_FLAGS
{
    D3D12MA_ALLOCATOR_FLAG_NONE = 0,
    D3D12MA_ALLOCATOR_FLAG_SINGLETHREADED = 0x1,
    D3D12MA_ALLOCATOR_FLAG_ALWAYS_COMMITTED = 0x2,
    D3D12MA_ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED = 0x4,
    D3D12MA_ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED = 0x8,
    D3D12MA_ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED = 0x10,
}

/// <summary>Bit flags to be used with <see cref="D3D12MA_ALLOCATION_DESC.Flags" />.</summary>
[Flags]
public enum D3D12MA_ALLOCATION_FLAGS
{
    /// <summary>Zero</summary>
    D3D12MA_ALLOCATION_FLAG_NONE = 0,

    /// <summary>Set this flag if the allocation should have its own dedicated memory allocation (committed resource with implicit heap).</summary>
    /// <remarks>
    ///   <para>Use it for special, big resources, like fullscreen textures used as render targets.</para>
    ///   <list type="bullet">
    ///     <item>
    ///       <description>When used with functions like <see cref="D3D12MA_Allocator.CreateResource" />, it will use <see cref="ID3D12Device.CreateCommittedResource" />, so the created allocation will contain a resource (<c>D3D12MA_Allocation.GetResource() != null</c>) but will not have a heap (<c>D3D12MA_Allocation.GetHeap() == null</c>), as the heap is implicit.</description>
    ///     </item>
    ///     <item>
    ///       <description>When used with raw memory allocation like <see cref="D3D12MA_Allocator.AllocateMemory" />, it will use <see cref="ID3D12Device.CreateHeap" />, so the created allocation will contain a heap (<c>D3D12MA_Allocation.GetHeap() != null</c>) and its offset will always be 0.</description>
    ///     </item>
    ///   </list>
    /// </remarks>
    D3D12MA_ALLOCATION_FLAG_COMMITTED = 0x1,

    /// <summary>Set this flag to only try to allocate from existing memory heaps and never create new such heap.</summary>
    /// <remarks>
    ///   <para>If new allocation cannot be placed in any of the existing heaps, allocation fails with <see cref="E_OUTOFMEMORY" /> error.</para>
    ///   <para>You should not use <see cref="D3D12MA_ALLOCATION_FLAG_COMMITTED" /> and <see cref="D3D12MA_ALLOCATION_FLAG_NEVER_ALLOCATE" /> at the same time. It makes no sense.</para>
    /// </remarks>
    D3D12MA_ALLOCATION_FLAG_NEVER_ALLOCATE = 0x2,

    /// <summary>Create allocation only if additional memory required for it, if any, won't exceed memory budget. Otherwise return <see cref="E_OUTOFMEMORY" />.</summary>
    D3D12MA_ALLOCATION_FLAG_WITHIN_BUDGET = 0x4,

    /// <summary>Allocation will be created from upper stack in a double stack pool.</summary>
    /// <remarks>This flag is only allowed for custom pools created with <see cref="D3D12MA_POOL_FLAG_ALGORITHM_LINEAR" /> flag.</remarks>
    D3D12MA_ALLOCATION_FLAG_UPPER_ADDRESS = 0x8,

    /// <summary>Set this flag if the allocated memory will have aliasing resources.</summary>
    /// <remarks>Use this when calling <see cref="D3D12MA_Allocator.CreateResource" /> and similar to guarantee creation of explicit heap for desired allocation and prevent it from using <see cref="ID3D12Device.CreateCommittedResource" />, so that new allocation object will always have <c>allocation->GetHeap() != null</c>.</remarks>
    D3D12MA_ALLOCATION_FLAG_CAN_ALIAS = 0x10,

    /// <summary>Allocation strategy that chooses smallest possible free range for the allocation to minimize memory usage and fragmentation, possibly at the expense of allocation time.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY = 0x00010000,

    /// <summary>Allocation strategy that chooses first suitable free range for the allocation - not necessarily in terms of the smallest offset but the one that is easiest and fastest to find to minimize allocation time, possibly at the expense of allocation quality.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME = 0x00020000,

    /// <summary>Allocation strategy that chooses always the lowest offset in available space. This is not the most efficient strategy but achieves highly packed data. Used internally by defragmentation, not recomended in typical usage.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET = 0x0004000,

    /// <summary>Alias to <see cref="D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY" />.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_BEST_FIT = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY,

    /// <summary>Alias to <see cref="D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME" />.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_FIRST_FIT = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME,

    /// <summary>A bit mask to extract only <c>STRATEGY</c> bits from entire set of flags.</summary>
    D3D12MA_ALLOCATION_FLAG_STRATEGY_MASK = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME | D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET,
}

internal unsafe struct D3D12MA_ALLOCATION_CALLBACKS
{
    public delegate* unmanaged<nuint, nuint, void*, void*> pAllocate;
    public delegate* unmanaged<void*, void*, void> pFree;
    public void* pPrivateData;
}

internal unsafe partial struct D3D12MA_ALLOCATOR_DESC
{
    public D3D12MA_ALLOCATOR_FLAGS Flags;
    public ID3D12Device* pDevice;
    public ulong PreferredBlockSize;
    public D3D12MA_ALLOCATION_CALLBACKS* pAllocationCallbacks;
    public IDXGIAdapter* pAdapter;
}

internal unsafe struct D3D12MA_ALLOCATION_DESC
{
    public D3D12MA_ALLOCATION_FLAGS Flags;
    public D3D12_HEAP_TYPE HeapType;
    public D3D12_HEAP_FLAGS ExtraHeapFlags;
    public /*Pool**/nint CustomPool;
    public void* pPrivateData;
}

internal struct D3D12MA_Statistics
{
    public uint BlockCount;
    public uint AllocationCount;
    public ulong BlockBytes;
    public ulong AllocationBytes;
}

internal struct D3D12MA_DetailedStatistics
{
    public D3D12MA_Statistics Stats;
    public uint UnusedRangeCount;
    public ulong AllocationSizeMin;
    public ulong AllocationSizeMax;
    public ulong UnusedRangeSizeMin;
    public ulong UnusedRangeSizeMax;
}

internal struct D3D12MA_TotalStatistics
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

internal unsafe partial class D3D12MA
{
    public const string LibraryName = AlimerApi.LibraryName;

    [SkipLocalsInit]
    public static HRESULT CreateAllocator(D3D12MA_ALLOCATOR_DESC* desc, out nint allocator)
    {
        Unsafe.SkipInit(out allocator);

        fixed (nint* allocatorPtr = &allocator)
        {
            return CreateAllocator(desc, allocatorPtr);
        }
    }

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_CreateAllocator")]
    private static partial HRESULT CreateAllocator(D3D12MA_ALLOCATOR_DESC* pDesc, nint* ppAllocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_AddRef")]
    public static partial uint Allocator_AddRef(nint allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_Release")]
    public static partial uint Allocator_Release(nint allocator);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CalculateStatistics")]
    public static partial void Allocator_CalculateStatistics(nint allocator, D3D12MA_TotalStatistics* pStats);

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource2")]
    public static partial HRESULT Allocator_CreateResource2(
        nint allocator,
        D3D12MA_ALLOCATION_DESC* pAllocationDesc,
        D3D12_RESOURCE_DESC1* pResourceDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        D3D12_CLEAR_VALUE* pOptimizedClearValue,
        out nint allocation,
        Guid* riidResource, void** ppvResource
        );

    [LibraryImport(LibraryName, EntryPoint = "D3D12MA_Allocator_CreateResource3")]
    public static partial HRESULT Allocator_CreateResource3(
        nint allocator,
        D3D12MA_ALLOCATION_DESC* pAllocationDesc,
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
