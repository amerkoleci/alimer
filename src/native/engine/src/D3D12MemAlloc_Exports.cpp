// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12MemAlloc.h"
#include "D3D12MemAlloc_Exports.h"

#define STRINGIZE_HELPER(X) #X
#define STRINGIZE(X) STRINGIZE_HELPER(X)
#define ASSERT_MSG(ENGINE, C) "ABI changed, you must update the C structure for " STRINGIZE(#ENGINE) " to " STRINGIZE(#C) "."

#define ENSURE_SIZE_ALIGN(type0, type1) \
    static_assert(sizeof(type0) == sizeof(type1)); \
    static_assert(alignof(type0) == alignof(type1))

static_assert(sizeof(D3D12MA_Bool32) == sizeof(BOOL));
static_assert(sizeof(uint64_t) == sizeof(UINT64));
static_assert(sizeof(uint32_t) == sizeof(UINT));
static_assert(sizeof(uint32_t) == sizeof(ULONG));
static_assert(sizeof(D3D12MA_AllocHandle) == sizeof(D3D12MA::AllocHandle));

// Enums
static_assert(sizeof(D3D12MA_ALLOCATION_FLAGS) == sizeof(D3D12MA::ALLOCATION_FLAGS));
static_assert(D3D12MA_ALLOCATION_FLAG_NONE == D3D12MA::ALLOCATION_FLAG_NONE);
static_assert(D3D12MA_ALLOCATION_FLAG_COMMITTED == D3D12MA::ALLOCATION_FLAG_COMMITTED);
static_assert(D3D12MA_ALLOCATION_FLAG_NEVER_ALLOCATE == D3D12MA::ALLOCATION_FLAG_NEVER_ALLOCATE);
static_assert(D3D12MA_ALLOCATION_FLAG_WITHIN_BUDGET == D3D12MA::ALLOCATION_FLAG_WITHIN_BUDGET);
static_assert(D3D12MA_ALLOCATION_FLAG_UPPER_ADDRESS == D3D12MA::ALLOCATION_FLAG_UPPER_ADDRESS);
static_assert(D3D12MA_ALLOCATION_FLAG_CAN_ALIAS == D3D12MA::ALLOCATION_FLAG_CAN_ALIAS);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY == D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME == D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_TIME);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET == D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_OFFSET);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_BEST_FIT == D3D12MA::ALLOCATION_FLAG_STRATEGY_BEST_FIT);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_FIRST_FIT == D3D12MA::ALLOCATION_FLAG_STRATEGY_FIRST_FIT);
static_assert(D3D12MA_ALLOCATION_FLAG_STRATEGY_MASK == D3D12MA::ALLOCATION_FLAG_STRATEGY_MASK);

static_assert(sizeof(D3D12MA_POOL_FLAGS) == sizeof(D3D12MA::POOL_FLAGS));
static_assert(D3D12MA_POOL_FLAG_NONE == D3D12MA::POOL_FLAG_NONE);
static_assert(D3D12MA_POOL_FLAG_ALGORITHM_LINEAR == D3D12MA::POOL_FLAG_ALGORITHM_LINEAR);
static_assert(D3D12MA_POOL_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED == D3D12MA::POOL_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED);
static_assert(D3D12MA_POOL_FLAG_ALGORITHM_MASK == D3D12MA::POOL_FLAG_ALGORITHM_MASK);

static_assert(sizeof(D3D12MA_ALLOCATOR_FLAGS) == sizeof(D3D12MA::ALLOCATOR_FLAGS));
static_assert(D3D12MA_ALLOCATOR_FLAG_NONE == D3D12MA::ALLOCATOR_FLAG_NONE);
static_assert(D3D12MA_ALLOCATOR_FLAG_SINGLETHREADED == D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED);
static_assert(D3D12MA_ALLOCATOR_FLAG_ALWAYS_COMMITTED == D3D12MA::ALLOCATOR_FLAG_ALWAYS_COMMITTED);
static_assert(D3D12MA_ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED == D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED);
static_assert(D3D12MA_ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED == D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED);
static_assert(D3D12MA_ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED == D3D12MA::ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED);

static_assert(sizeof(D3D12MA_VIRTUAL_BLOCK_FLAGS) == sizeof(D3D12MA::VIRTUAL_BLOCK_FLAGS));
static_assert(D3D12MA_VIRTUAL_BLOCK_FLAG_NONE == D3D12MA::VIRTUAL_BLOCK_FLAG_NONE);
static_assert(D3D12MA_VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR == D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR);
static_assert(D3D12MA_VIRTUAL_BLOCK_FLAG_ALGORITHM_MASK == D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_MASK);

static_assert(sizeof(D3D12MA_DEFRAGMENTATION_FLAGS) == sizeof(D3D12MA::DEFRAGMENTATION_FLAGS));
static_assert(D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST == D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_FAST);
static_assert(D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED == D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED);
static_assert(D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL == D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_FULL);
static_assert(D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_MASK == D3D12MA::DEFRAGMENTATION_FLAG_ALGORITHM_MASK);

static_assert(sizeof(D3D12MA_VIRTUAL_ALLOCATION_FLAGS) == sizeof(D3D12MA::VIRTUAL_ALLOCATION_FLAGS));
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_NONE == D3D12MA::VIRTUAL_ALLOCATION_FLAG_NONE);
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_UPPER_ADDRESS == D3D12MA::VIRTUAL_ALLOCATION_FLAG_UPPER_ADDRESS);
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY == D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME == D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME);
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET == D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET);
static_assert(D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MASK == D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MASK);

static_assert(sizeof(D3D12MA_DEFRAGMENTATION_MOVE_OPERATION) == sizeof(D3D12MA::DEFRAGMENTATION_MOVE_OPERATION));
static_assert(D3D12MA_DEFRAGMENTATION_MOVE_OPERATION_COPY == D3D12MA::DEFRAGMENTATION_MOVE_OPERATION_COPY);

// Structs
ENSURE_SIZE_ALIGN(D3D12MA_ALLOCATION_CALLBACKS, D3D12MA::ALLOCATION_CALLBACKS);
static_assert(offsetof(D3D12MA_ALLOCATION_CALLBACKS, pAllocate) == offsetof(D3D12MA::ALLOCATION_CALLBACKS, pAllocate), ASSERT_MSG(D3D12MA::ALLOCATION_CALLBACKS, D3D12MA_ALLOCATION_CALLBACKS));
static_assert(offsetof(D3D12MA_ALLOCATION_CALLBACKS, pFree) == offsetof(D3D12MA::ALLOCATION_CALLBACKS, pFree), ASSERT_MSG(D3D12MA::ALLOCATION_CALLBACKS, D3D12MA_ALLOCATION_CALLBACKS));
static_assert(offsetof(D3D12MA_ALLOCATION_CALLBACKS, pPrivateData) == offsetof(D3D12MA::ALLOCATION_CALLBACKS, pPrivateData), ASSERT_MSG(D3D12MA::ALLOCATION_CALLBACKS, D3D12MA_ALLOCATION_CALLBACKS));

ENSURE_SIZE_ALIGN(D3D12MA_ALLOCATOR_DESC, D3D12MA::ALLOCATOR_DESC);
static_assert(offsetof(D3D12MA_ALLOCATOR_DESC, Flags) == offsetof(D3D12MA::ALLOCATOR_DESC, Flags), ASSERT_MSG(D3D12MA::ALLOCATOR_DESC, D3D12MA_ALLOCATOR_DESC));
static_assert(offsetof(D3D12MA_ALLOCATOR_DESC, pDevice) == offsetof(D3D12MA::ALLOCATOR_DESC, pDevice), ASSERT_MSG(D3D12MA::ALLOCATOR_DESC, D3D12MA_ALLOCATOR_DESC));
static_assert(offsetof(D3D12MA_ALLOCATOR_DESC, PreferredBlockSize) == offsetof(D3D12MA::ALLOCATOR_DESC, PreferredBlockSize), ASSERT_MSG(D3D12MA::ALLOCATOR_DESC, D3D12MA_ALLOCATOR_DESC));
static_assert(offsetof(D3D12MA_ALLOCATOR_DESC, pAllocationCallbacks) == offsetof(D3D12MA::ALLOCATOR_DESC, pAllocationCallbacks), ASSERT_MSG(D3D12MA::ALLOCATOR_DESC, D3D12MA_ALLOCATOR_DESC));
static_assert(offsetof(D3D12MA_ALLOCATOR_DESC, pAdapter) == offsetof(D3D12MA::ALLOCATOR_DESC, pAdapter), ASSERT_MSG(D3D12MA::ALLOCATOR_DESC, D3D12MA_ALLOCATOR_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_POOL_DESC, D3D12MA::POOL_DESC);
static_assert(offsetof(D3D12MA_POOL_DESC, Flags) == offsetof(D3D12MA::POOL_DESC, Flags), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, HeapProperties) == offsetof(D3D12MA::POOL_DESC, HeapProperties), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, HeapFlags) == offsetof(D3D12MA::POOL_DESC, HeapFlags), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, BlockSize) == offsetof(D3D12MA::POOL_DESC, BlockSize), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, MinBlockCount) == offsetof(D3D12MA::POOL_DESC, MinBlockCount), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, MaxBlockCount) == offsetof(D3D12MA::POOL_DESC, MaxBlockCount), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, MinAllocationAlignment) == offsetof(D3D12MA::POOL_DESC, MinAllocationAlignment), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, pProtectedSession) == offsetof(D3D12MA::POOL_DESC, pProtectedSession), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));
static_assert(offsetof(D3D12MA_POOL_DESC, ResidencyPriority) == offsetof(D3D12MA::POOL_DESC, ResidencyPriority), ASSERT_MSG(D3D12MA::POOL_DESC, D3D12MA_POOL_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_VIRTUAL_BLOCK_DESC, D3D12MA::VIRTUAL_BLOCK_DESC);
static_assert(offsetof(D3D12MA_VIRTUAL_BLOCK_DESC, Flags) == offsetof(D3D12MA::VIRTUAL_BLOCK_DESC, Flags), ASSERT_MSG(D3D12MA::VIRTUAL_BLOCK_DESC, D3D12MA_VIRTUAL_BLOCK_DESC));
static_assert(offsetof(D3D12MA_VIRTUAL_BLOCK_DESC, Size) == offsetof(D3D12MA::VIRTUAL_BLOCK_DESC, Size), ASSERT_MSG(D3D12MA::VIRTUAL_BLOCK_DESC, D3D12MA_VIRTUAL_BLOCK_DESC));
static_assert(offsetof(D3D12MA_VIRTUAL_BLOCK_DESC, pAllocationCallbacks) == offsetof(D3D12MA::VIRTUAL_BLOCK_DESC, pAllocationCallbacks), ASSERT_MSG(D3D12MA::VIRTUAL_BLOCK_DESC, D3D12MA_VIRTUAL_BLOCK_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_ALLOCATION_DESC, D3D12MA::ALLOCATION_DESC);
static_assert(offsetof(D3D12MA_ALLOCATION_DESC, Flags) == offsetof(D3D12MA::ALLOCATION_DESC, Flags), ASSERT_MSG(D3D12MA::ALLOCATION_DESC, D3D12MA_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_ALLOCATION_DESC, HeapType) == offsetof(D3D12MA::ALLOCATION_DESC, HeapType), ASSERT_MSG(D3D12MA::ALLOCATION_DESC, D3D12MA_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_ALLOCATION_DESC, ExtraHeapFlags) == offsetof(D3D12MA::ALLOCATION_DESC, ExtraHeapFlags), ASSERT_MSG(D3D12MA::ALLOCATION_DESC, D3D12MA_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_ALLOCATION_DESC, CustomPool) == offsetof(D3D12MA::ALLOCATION_DESC, CustomPool), ASSERT_MSG(D3D12MA::ALLOCATION_DESC, D3D12MA_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_ALLOCATION_DESC, pPrivateData) == offsetof(D3D12MA::ALLOCATION_DESC, pPrivateData), ASSERT_MSG(D3D12MA::ALLOCATION_DESC, D3D12MA_ALLOCATION_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_Statistics, D3D12MA::Statistics);
static_assert(offsetof(D3D12MA_Statistics, BlockCount) == offsetof(D3D12MA::Statistics, BlockCount), ASSERT_MSG(D3D12MA::Statistics, D3D12MA_Statistics));
static_assert(offsetof(D3D12MA_Statistics, AllocationCount) == offsetof(D3D12MA::Statistics, AllocationCount), ASSERT_MSG(D3D12MA::Statistics, D3D12MA_Statistics));
static_assert(offsetof(D3D12MA_Statistics, BlockBytes) == offsetof(D3D12MA::Statistics, BlockBytes), ASSERT_MSG(D3D12MA::Statistics, D3D12MA_Statistics));
static_assert(offsetof(D3D12MA_Statistics, AllocationBytes) == offsetof(D3D12MA::Statistics, AllocationBytes), ASSERT_MSG(D3D12MA::Statistics, D3D12MA_Statistics));

ENSURE_SIZE_ALIGN(D3D12MA_DetailedStatistics, D3D12MA::DetailedStatistics);
static_assert(offsetof(D3D12MA_DetailedStatistics, Stats) == offsetof(D3D12MA::DetailedStatistics, Stats), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));
static_assert(offsetof(D3D12MA_DetailedStatistics, UnusedRangeCount) == offsetof(D3D12MA::DetailedStatistics, UnusedRangeCount), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));
static_assert(offsetof(D3D12MA_DetailedStatistics, AllocationSizeMin) == offsetof(D3D12MA::DetailedStatistics, AllocationSizeMin), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));
static_assert(offsetof(D3D12MA_DetailedStatistics, AllocationSizeMax) == offsetof(D3D12MA::DetailedStatistics, AllocationSizeMax), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));
static_assert(offsetof(D3D12MA_DetailedStatistics, UnusedRangeSizeMin) == offsetof(D3D12MA::DetailedStatistics, UnusedRangeSizeMin), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));
static_assert(offsetof(D3D12MA_DetailedStatistics, UnusedRangeSizeMax) == offsetof(D3D12MA::DetailedStatistics, UnusedRangeSizeMax), ASSERT_MSG(D3D12MA::DetailedStatistics, D3D12MA_DetailedStatistics));

ENSURE_SIZE_ALIGN(D3D12MA_TotalStatistics, D3D12MA::TotalStatistics);
static_assert(offsetof(D3D12MA_TotalStatistics, HeapType) == offsetof(D3D12MA::TotalStatistics, HeapType), ASSERT_MSG(D3D12MA::TotalStatistics, D3D12MA_TotalStatistics));
static_assert(offsetof(D3D12MA_TotalStatistics, MemorySegmentGroup) == offsetof(D3D12MA::TotalStatistics, MemorySegmentGroup), ASSERT_MSG(D3D12MA::TotalStatistics, D3D12MA_TotalStatistics));
static_assert(offsetof(D3D12MA_TotalStatistics, Total) == offsetof(D3D12MA::TotalStatistics, Total), ASSERT_MSG(D3D12MA::TotalStatistics, D3D12MA_TotalStatistics));

ENSURE_SIZE_ALIGN(D3D12MA_Budget, D3D12MA::Budget);
static_assert(offsetof(D3D12MA_Budget, Stats) == offsetof(D3D12MA::Budget, Stats), ASSERT_MSG(D3D12MA::Budget, D3D12MA_Budget));
static_assert(offsetof(D3D12MA_Budget, UsageBytes) == offsetof(D3D12MA::Budget, UsageBytes), ASSERT_MSG(D3D12MA::Budget, D3D12MA_Budget));
static_assert(offsetof(D3D12MA_Budget, BudgetBytes) == offsetof(D3D12MA::Budget, BudgetBytes), ASSERT_MSG(D3D12MA::Budget, D3D12MA_Budget));

ENSURE_SIZE_ALIGN(D3D12MA_DEFRAGMENTATION_MOVE, D3D12MA::DEFRAGMENTATION_MOVE);
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_MOVE, Operation) == offsetof(D3D12MA::DEFRAGMENTATION_MOVE, Operation), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_MOVE, D3D12MA_DEFRAGMENTATION_MOVE));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_MOVE, pSrcAllocation) == offsetof(D3D12MA::DEFRAGMENTATION_MOVE, pSrcAllocation), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_MOVE, D3D12MA_DEFRAGMENTATION_MOVE));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_MOVE, pDstTmpAllocation) == offsetof(D3D12MA::DEFRAGMENTATION_MOVE, pDstTmpAllocation), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_MOVE, D3D12MA_DEFRAGMENTATION_MOVE));

ENSURE_SIZE_ALIGN(D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO, D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO);
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO, MoveCount) == offsetof(D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO, MoveCount), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO, pMoves) == offsetof(D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO, pMoves), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO));

ENSURE_SIZE_ALIGN(D3D12MA_DEFRAGMENTATION_STATS, D3D12MA::DEFRAGMENTATION_STATS);
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_STATS, BytesMoved) == offsetof(D3D12MA::DEFRAGMENTATION_STATS, BytesMoved), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_STATS, D3D12MA_DEFRAGMENTATION_STATS));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_STATS, BytesFreed) == offsetof(D3D12MA::DEFRAGMENTATION_STATS, BytesFreed), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_STATS, D3D12MA_DEFRAGMENTATION_STATS));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_STATS, AllocationsMoved) == offsetof(D3D12MA::DEFRAGMENTATION_STATS, AllocationsMoved), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_STATS, D3D12MA_DEFRAGMENTATION_STATS));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_STATS, HeapsFreed) == offsetof(D3D12MA::DEFRAGMENTATION_STATS, HeapsFreed), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_STATS, D3D12MA_DEFRAGMENTATION_STATS));

ENSURE_SIZE_ALIGN(D3D12MA_DEFRAGMENTATION_DESC, D3D12MA::DEFRAGMENTATION_DESC);
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_DESC, Flags) == offsetof(D3D12MA::DEFRAGMENTATION_DESC, Flags), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_DESC, D3D12MA_DEFRAGMENTATION_DESC));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_DESC, MaxBytesPerPass) == offsetof(D3D12MA::DEFRAGMENTATION_DESC, MaxBytesPerPass), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_DESC, D3D12MA_DEFRAGMENTATION_DESC));
static_assert(offsetof(D3D12MA_DEFRAGMENTATION_DESC, MaxAllocationsPerPass) == offsetof(D3D12MA::DEFRAGMENTATION_DESC, MaxAllocationsPerPass), ASSERT_MSG(D3D12MA::DEFRAGMENTATION_DESC, D3D12MA_DEFRAGMENTATION_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_VIRTUAL_ALLOCATION_DESC, D3D12MA::VIRTUAL_ALLOCATION_DESC);
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_DESC, Flags) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_DESC, Flags), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_DESC, D3D12MA_VIRTUAL_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_DESC, Size) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_DESC, Size), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_DESC, D3D12MA_VIRTUAL_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_DESC, Alignment) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_DESC, Alignment), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_DESC, D3D12MA_VIRTUAL_ALLOCATION_DESC));
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_DESC, pPrivateData) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_DESC, pPrivateData), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_DESC, D3D12MA_VIRTUAL_ALLOCATION_DESC));

ENSURE_SIZE_ALIGN(D3D12MA_VIRTUAL_ALLOCATION_INFO, D3D12MA::VIRTUAL_ALLOCATION_INFO);
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_INFO, Offset) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_INFO, Offset), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_INFO, D3D12MA_VIRTUAL_ALLOCATION_INFO));
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_INFO, Size) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_INFO, Size), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_INFO, D3D12MA_VIRTUAL_ALLOCATION_INFO));
static_assert(offsetof(D3D12MA_VIRTUAL_ALLOCATION_INFO, pPrivateData) == offsetof(D3D12MA::VIRTUAL_ALLOCATION_INFO, pPrivateData), ASSERT_MSG(D3D12MA::VIRTUAL_ALLOCATION_INFO, D3D12MA_VIRTUAL_ALLOCATION_INFO));

ENSURE_SIZE_ALIGN(D3D12MA_VirtualAllocation, D3D12MA::VirtualAllocation);
static_assert(offsetof(D3D12MA_VirtualAllocation, AllocHandle) == offsetof(D3D12MA::VirtualAllocation, AllocHandle), ASSERT_MSG(D3D12MA::VirtualAllocation, D3D12MA_VirtualAllocation));

/* Top level API */
HRESULT D3D12MA_CreateAllocator(const D3D12MA_ALLOCATOR_DESC* pDesc, D3D12MA_Allocator** ppAllocator)
{
    const D3D12MA::ALLOCATOR_DESC* desc = reinterpret_cast<const D3D12MA::ALLOCATOR_DESC*>(pDesc);
    D3D12MA::Allocator** allocator = reinterpret_cast<D3D12MA::Allocator**>(ppAllocator);
    HRESULT result = D3D12MA::CreateAllocator(desc, allocator);
    return result;
}

HRESULT D3D12MA_CreateVirtualBlock(const D3D12MA_VIRTUAL_BLOCK_DESC* pDesc, D3D12MA_VirtualBlock** ppVirtualBlock)
{
    const D3D12MA::VIRTUAL_BLOCK_DESC* desc = reinterpret_cast<const D3D12MA::VIRTUAL_BLOCK_DESC*>(pDesc);
    D3D12MA::VirtualBlock** virtual_block = reinterpret_cast<D3D12MA::VirtualBlock**>(ppVirtualBlock);
    HRESULT result = D3D12MA::CreateVirtualBlock(desc, virtual_block);
    return result;
}

/* D3D12MA_Allocator */
uint32_t D3D12MA_Allocator_AddRef(D3D12MA_Allocator* pSelf)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->AddRef();
}

uint32_t D3D12MA_Allocator_Release(D3D12MA_Allocator* pSelf)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->Release();
}

D3D12MA_Bool32 D3D12MA_Allocator_IsUMA(D3D12MA_Allocator* pSelf)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->IsUMA();
}

D3D12MA_Bool32 D3D12MA_Allocator_IsCacheCoherentUMA(D3D12MA_Allocator* pSelf)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->IsCacheCoherentUMA();
}

D3D12MA_Bool32 D3D12MA_Allocator_IsGPUUploadHeapSupported(D3D12MA_Allocator* pSelf)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->IsGPUUploadHeapSupported();
}

uint64_t D3D12MA_Allocator_GetMemoryCapacity(D3D12MA_Allocator* pSelf, uint32_t memorySegmentGroup)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    return self->GetMemoryCapacity(memorySegmentGroup);
}

HRESULT D3D12MA_Allocator_CreateResource(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::ALLOCATION_DESC* alloc_desc = reinterpret_cast<const D3D12MA::ALLOCATION_DESC*>(pAllocDesc);
    D3D12MA::Allocation** allocation = reinterpret_cast<D3D12MA::Allocation**>(ppAllocation);
    HRESULT result = self->CreateResource(alloc_desc, pResourceDesc, InitialResourceState, pOptimizedClearValue, allocation, riidResource, ppvResource);
    return result;
}

HRESULT D3D12MA_Allocator_CreateResource2(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::ALLOCATION_DESC* alloc_desc = reinterpret_cast<const D3D12MA::ALLOCATION_DESC*>(pAllocDesc);
    D3D12MA::Allocation** allocation = reinterpret_cast<D3D12MA::Allocation**>(ppAllocation);
    HRESULT result = self->CreateResource2(alloc_desc, pResourceDesc, InitialResourceState, pOptimizedClearValue, allocation, riidResource, ppvResource);
    return result;
}

HRESULT D3D12MA_Allocator_CreateResource3(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, uint32_t NumCastableFormats, DXGI_FORMAT* pCastableFormats, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::ALLOCATION_DESC* alloc_desc = reinterpret_cast<const D3D12MA::ALLOCATION_DESC*>(pAllocDesc);
    D3D12MA::Allocation** allocation = reinterpret_cast<D3D12MA::Allocation**>(ppAllocation);
    HRESULT result = self->CreateResource3(alloc_desc, pResourceDesc, InitialLayout, pOptimizedClearValue, NumCastableFormats, pCastableFormats, allocation, riidResource, ppvResource);
    return result;
}

HRESULT D3D12MA_Allocator_AllocateMemory(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_ALLOCATION_INFO* pAllocInfo, D3D12MA_Allocation** ppAllocation)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::ALLOCATION_DESC* alloc_desc = reinterpret_cast<const D3D12MA::ALLOCATION_DESC*>(pAllocDesc);
    D3D12MA::Allocation** allocation = reinterpret_cast<D3D12MA::Allocation**>(ppAllocation);
    return self->AllocateMemory(alloc_desc, pAllocInfo, allocation);
}

HRESULT D3D12MA_Allocator_CreateAliasingResource(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    D3D12MA::Allocation* allocation = reinterpret_cast<D3D12MA::Allocation*>(pAllocation);
    return self->CreateAliasingResource(allocation, AllocationLocalOffset, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
}

HRESULT D3D12MA_Allocator_CreateAliasingResource1(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    D3D12MA::Allocation* allocation = reinterpret_cast<D3D12MA::Allocation*>(pAllocation);
    return self->CreateAliasingResource1(allocation, AllocationLocalOffset, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
}

HRESULT D3D12MA_Allocator_CreateAliasingResource2(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, uint32_t NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riidResource, void** ppvResource)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    D3D12MA::Allocation* allocation = reinterpret_cast<D3D12MA::Allocation*>(pAllocation);
    return self->CreateAliasingResource2(allocation, AllocationLocalOffset, pResourceDesc, InitialLayout, pOptimizedClearValue, NumCastableFormats, pCastableFormats, riidResource, ppvResource);
}

HRESULT D3D12MA_Allocator_CreatePool(D3D12MA_Allocator* pSelf, const D3D12MA_POOL_DESC* pPoolDesc, D3D12MA_Pool** ppPool)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::POOL_DESC* pool_desc = reinterpret_cast<const D3D12MA::POOL_DESC*>(pPoolDesc);
    D3D12MA::Pool** pool = reinterpret_cast<D3D12MA::Pool**>(ppPool);
    HRESULT result = self->CreatePool(pool_desc, pool);
    return result;
}

void D3D12MA_Allocator_SetCurrentFrameIndex(D3D12MA_Allocator* pSelf, uint32_t frameIndex)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    self->SetCurrentFrameIndex(frameIndex);
}

void D3D12MA_Allocator_GetBudget(D3D12MA_Allocator* pSelf, D3D12MA_Budget* pLocalBudget, D3D12MA_Budget* pNonLocalBudget)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    D3D12MA::Budget* local_budget = reinterpret_cast<D3D12MA::Budget*>(pLocalBudget);
    D3D12MA::Budget* non_local_budget = reinterpret_cast<D3D12MA::Budget*>(pNonLocalBudget);
    self->GetBudget(local_budget, non_local_budget);
}

void D3D12MA_Allocator_CalculateStatistics(D3D12MA_Allocator* pSelf, D3D12MA_TotalStatistics* pStats)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    D3D12MA::TotalStatistics* stats = reinterpret_cast<D3D12MA::TotalStatistics*>(pStats);
    self->CalculateStatistics(stats);
}

void D3D12MA_Allocator_BuildStatsString(D3D12MA_Allocator* pSelf, wchar_t** ppStatsString, D3D12MA_Bool32 DetailedMap)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    self->BuildStatsString(ppStatsString, DetailedMap);
}

void D3D12MA_Allocator_FreeStatsString(D3D12MA_Allocator* pSelf, wchar_t* pStatsString)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    self->FreeStatsString(pStatsString);
}

void D3D12MA_Allocator_BeginDefragmentation(void* pSelf, const D3D12MA_DEFRAGMENTATION_DESC* pDesc, D3D12MA_DefragmentationContext** ppContext)
{
    D3D12MA::Allocator* self = reinterpret_cast<D3D12MA::Allocator*>(pSelf);
    const D3D12MA::DEFRAGMENTATION_DESC* desc = reinterpret_cast<const D3D12MA::DEFRAGMENTATION_DESC*>(pDesc);
    D3D12MA::DefragmentationContext** context = reinterpret_cast<D3D12MA::DefragmentationContext**>(ppContext);
    self->BeginDefragmentation(desc, context);
}

/* D3D12MA_Allocation */
uint32_t D3D12MA_Allocation_AddRef(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->AddRef();
}

uint32_t D3D12MA_Allocation_Release(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->Release();
}

uint64_t D3D12MA_Allocation_GetOffset(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetOffset();
}

uint64_t D3D12MA_Allocation_GetAlignment(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetAlignment();
}

uint64_t D3D12MA_Allocation_GetSize(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetSize();
}

ID3D12Resource* D3D12MA_Allocation_GetResource(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetResource();
}

void D3D12MA_Allocation_SetResource(D3D12MA_Allocation* pSelf, ID3D12Resource* pResource)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    self->SetResource(pResource);
}

ID3D12Heap* D3D12MA_Allocation_GetHeap(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetHeap();
}

void D3D12MA_Allocation_SetPrivateData(D3D12MA_Allocation* pSelf, void* pPrivateData)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    self->SetPrivateData(pPrivateData);
}

void* D3D12MA_Allocation_GetPrivateData(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetPrivateData();
}

void D3D12MA_Allocation_SetName(D3D12MA_Allocation* pSelf, const wchar_t* Name)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    self->SetName(Name);
}

const wchar_t* D3D12MA_Allocation_GetName(D3D12MA_Allocation* pSelf)
{
    D3D12MA::Allocation* self = reinterpret_cast<D3D12MA::Allocation*>(pSelf);
    return self->GetName();
}

/* D3D12MA_Pool */
uint32_t D3D12MA_Pool_AddRef(D3D12MA_Pool* pSelf)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    return self->AddRef();
}

uint32_t D3D12MA_Pool_Release(D3D12MA_Pool* pSelf)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    return self->Release();
}

void D3D12MA_Pool_GetDesc(D3D12MA_Pool* pSelf, D3D12MA_POOL_DESC* pDesc)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    const D3D12MA::POOL_DESC desc = self->GetDesc();
    memcpy(pDesc, &desc, sizeof(D3D12MA_POOL_DESC));
}

void D3D12MA_Pool_GetStatistics(D3D12MA_Pool* pSelf, D3D12MA_Statistics* pStats)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    D3D12MA::Statistics* stats = reinterpret_cast<D3D12MA::Statistics*>(pStats);
    self->GetStatistics(stats);
}

void D3D12MA_Pool_CalculateStatistics(D3D12MA_Pool* pSelf, D3D12MA_DetailedStatistics* pStats)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    D3D12MA::DetailedStatistics* stats = reinterpret_cast<D3D12MA::DetailedStatistics*>(pStats);
    self->CalculateStatistics(stats);
}

void D3D12MA_Pool_SetName(D3D12MA_Pool* pSelf, const wchar_t* Name)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    self->SetName(Name);
}

const wchar_t* D3D12MA_Pool_GetName(D3D12MA_Pool* pSelf)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    return self->GetName();
}

HRESULT D3D12MA_Pool_BeginDefragmentation(D3D12MA_Pool* pSelf, const D3D12MA_DEFRAGMENTATION_DESC* pDesc, D3D12MA_DefragmentationContext** ppContext)
{
    D3D12MA::Pool* self = reinterpret_cast<D3D12MA::Pool*>(pSelf);
    const D3D12MA::DEFRAGMENTATION_DESC* desc = reinterpret_cast<const D3D12MA::DEFRAGMENTATION_DESC*>(pDesc);
    D3D12MA::DefragmentationContext** context = reinterpret_cast<D3D12MA::DefragmentationContext**>(ppContext);
    HRESULT result = self->BeginDefragmentation(desc, context);
    return result;
}

/* D3D12MA_VirtualBlock */
uint32_t D3D12MA_VirtualBlock_AddRef(D3D12MA_VirtualBlock* pSelf)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    return self->AddRef();
}

uint32_t D3D12MA_VirtualBlock_Release(D3D12MA_VirtualBlock* pSelf)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    return self->Release();
}

D3D12MA_Bool32 D3D12MAVirtualBlock_IsEmpty(D3D12MA_VirtualBlock* pSelf)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    return self->IsEmpty();
}

void D3D12MAVirtualBlock_GetAllocationInfo(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation, D3D12MA_VIRTUAL_ALLOCATION_INFO* pInfo)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    D3D12MA::VirtualAllocation* allocation = reinterpret_cast<D3D12MA::VirtualAllocation*>(&Allocation);
    D3D12MA::VIRTUAL_ALLOCATION_INFO* info = reinterpret_cast<D3D12MA::VIRTUAL_ALLOCATION_INFO*>(pInfo);
    self->GetAllocationInfo(*allocation, info);
}

HRESULT D3D12MAVirtualBlock_Allocate(D3D12MA_VirtualBlock* pSelf, const D3D12MA_VIRTUAL_ALLOCATION_DESC* pDesc, D3D12MA_VirtualAllocation* pAllocation, uint64_t* pOffset)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    const D3D12MA::VIRTUAL_ALLOCATION_DESC* desc = reinterpret_cast<const D3D12MA::VIRTUAL_ALLOCATION_DESC*>(pDesc);
    D3D12MA::VirtualAllocation* allocation = reinterpret_cast<D3D12MA::VirtualAllocation*>(pAllocation);
    return self->Allocate(desc, allocation, pOffset);
}

void D3D12MAVirtualBlock_FreeAllocation(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    D3D12MA::VirtualAllocation* allocation = reinterpret_cast<D3D12MA::VirtualAllocation*>(&Allocation);
    self->FreeAllocation(*allocation);
}

void D3D12MAVirtualBlock_Clear(D3D12MA_VirtualBlock* pSelf)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    self->Clear();
}

void D3D12MAVirtualBlock_SetAllocationPrivateData(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation, void* pPrivateData)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    D3D12MA::VirtualAllocation* allocation = reinterpret_cast<D3D12MA::VirtualAllocation*>(&Allocation);
    self->SetAllocationPrivateData(*allocation, pPrivateData);
}

void D3D12MAVirtualBlock_GetStatistics(D3D12MA_VirtualBlock* pSelf, D3D12MA_Statistics* pStats)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    D3D12MA::Statistics* stats = reinterpret_cast<D3D12MA::Statistics*>(pStats);
    self->GetStatistics(stats);
}

void D3D12MAVirtualBlock_CalculateStatistics(D3D12MA_VirtualBlock* pSelf, D3D12MA_DetailedStatistics* pStats)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    D3D12MA::DetailedStatistics* stats = reinterpret_cast<D3D12MA::DetailedStatistics*>(pStats);
    self->CalculateStatistics(stats);
}

void D3D12MAVirtualBlock_BuildStatsString(D3D12MA_VirtualBlock* pSelf, wchar_t** ppStatsString)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    self->BuildStatsString(ppStatsString);
}

void D3D12MAVirtualBlock_FreeStatsString(D3D12MA_VirtualBlock* pSelf, wchar_t* pStatsString)
{
    D3D12MA::VirtualBlock* self = reinterpret_cast<D3D12MA::VirtualBlock*>(pSelf);
    self->FreeStatsString(pStatsString);
}

/* D3D12MA_DefragmentationContext */
uint32_t D3D12MA_DefragmentationContext_AddRef(D3D12MA_DefragmentationContext* pSelf)
{
    D3D12MA::DefragmentationContext* self = reinterpret_cast<D3D12MA::DefragmentationContext*>(pSelf);
    return self->AddRef();
}

uint32_t D3D12MA_DefragmentationContext_Release(D3D12MA_DefragmentationContext* pSelf)
{
    D3D12MA::DefragmentationContext* self = reinterpret_cast<D3D12MA::DefragmentationContext*>(pSelf);
    return self->Release();
}

HRESULT D3D12MA_DefragmentationContext_BeginPass(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO* pPassInfo)
{
    D3D12MA::DefragmentationContext* self = reinterpret_cast<D3D12MA::DefragmentationContext*>(pSelf);
    D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO* pass_info = reinterpret_cast<D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO*>(pPassInfo);
    return self->BeginPass(pass_info);
}

HRESULT D3D12MA_DefragmentationContext_EndPass(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO* pPassInfo)
{
    D3D12MA::DefragmentationContext* self = reinterpret_cast<D3D12MA::DefragmentationContext*>(pSelf);
    D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO* pass_info = reinterpret_cast<D3D12MA::DEFRAGMENTATION_PASS_MOVE_INFO*>(pPassInfo);
    return self->EndPass(pass_info);
}

void D3D12MA_DefragmentationContext_GetStats(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_STATS* pStats)
{
    D3D12MA::DefragmentationContext* self = reinterpret_cast<D3D12MA::DefragmentationContext*>(pSelf);
    D3D12MA::DEFRAGMENTATION_STATS* stats = reinterpret_cast<D3D12MA::DEFRAGMENTATION_STATS*>(pStats);
    self->GetStats(stats);
}
