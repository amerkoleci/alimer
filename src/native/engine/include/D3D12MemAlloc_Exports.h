// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef D3D12MA_C_H_
#define D3D12MA_C_H_

#ifdef __cplusplus
#   define D3D12MA_C_API extern "C" __declspec(dllexport)
#else
#   define D3D12MA_C_API extern __declspec(dllexport)
#endif

#include <stddef.h>
#include <stdint.h>
#include <directx/d3d12.h>
#include <dxgi1_4.h>

typedef uint32_t D3D12MA_Bool32;
typedef uint64_t D3D12MA_AllocHandle;

typedef void* (*D3D12MA_AllocateFunctionType)(size_t Size, size_t Alignment, void* pPrivateData);
typedef void (*D3D12MA_FreeFunctionType)(void* pMemory, void* pPrivateData);

typedef struct D3D12MA_Allocator                D3D12MA_Allocator;
typedef struct D3D12MA_Allocation               D3D12MA_Allocation;
typedef struct D3D12MA_Pool                     D3D12MA_Pool;
typedef struct D3D12MA_VirtualBlock             D3D12MA_VirtualBlock;
typedef struct D3D12MA_DefragmentationContext   D3D12MA_DefragmentationContext;

typedef enum D3D12MA_ALLOCATION_FLAGS {
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

    _D3D12MA_ALLOCATION_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_ALLOCATION_FLAGS;

typedef enum D3D12MA_POOL_FLAGS {
    D3D12MA_POOL_FLAG_NONE = 0x0,
    D3D12MA_POOL_FLAG_ALGORITHM_LINEAR = 0x1,
    D3D12MA_POOL_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED = 0x2,
    D3D12MA_POOL_FLAG_ALGORITHM_MASK = D3D12MA_POOL_FLAG_ALGORITHM_LINEAR,

    _D3D12MA_POOL_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_POOL_FLAGS;

typedef enum D3D12MA_ALLOCATOR_FLAGS {
    D3D12MA_ALLOCATOR_FLAG_NONE = 0x00,
    D3D12MA_ALLOCATOR_FLAG_SINGLETHREADED = 0x01,
    D3D12MA_ALLOCATOR_FLAG_ALWAYS_COMMITTED = 0x02,
    D3D12MA_ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED = 0x04,
    D3D12MA_ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED = 0x08,
    D3D12MA_ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED = 0x10,

    _D3D12MA_ALLOCATOR_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_ALLOCATOR_FLAGS;

typedef enum D3D12MA_VIRTUAL_BLOCK_FLAGS {
    D3D12MA_VIRTUAL_BLOCK_FLAG_NONE = 0,
    D3D12MA_VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR = D3D12MA_POOL_FLAG_ALGORITHM_LINEAR,
    D3D12MA_VIRTUAL_BLOCK_FLAG_ALGORITHM_MASK = D3D12MA_POOL_FLAG_ALGORITHM_MASK,

    _D3D12MA_VIRTUAL_BLOCK_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_VIRTUAL_BLOCK_FLAGS;

typedef enum D3D12MA_DEFRAGMENTATION_FLAGS {
    D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST = 0x1,
    D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED = 0x2,
    D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL = 0x4,
    D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_MASK = D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST | D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED | D3D12MA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL,

    _D3D12MA_DEFRAGMENTATION_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_DEFRAGMENTATION_FLAGS;

typedef enum D3D12MA_DEFRAGMENTATION_MOVE_OPERATION {
    D3D12MA_DEFRAGMENTATION_MOVE_OPERATION_COPY = 0,
    D3D12MA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE = 1,
    D3D12MA_DEFRAGMENTATION_MOVE_OPERATION_DESTROY = 2,

    _D3D12MA_DEFRAGMENTATION_MOVE_OPERATION_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_DEFRAGMENTATION_MOVE_OPERATION;

typedef enum D3D12MA_VIRTUAL_ALLOCATION_FLAGS {
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_NONE = 0,
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_UPPER_ADDRESS = D3D12MA_ALLOCATION_FLAG_UPPER_ADDRESS,
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY,
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_TIME,
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET = D3D12MA_ALLOCATION_FLAG_STRATEGY_MIN_OFFSET,
    D3D12MA_VIRTUAL_ALLOCATION_FLAG_STRATEGY_MASK = D3D12MA_ALLOCATION_FLAG_STRATEGY_MASK,

    _D3D12MA_VIRTUAL_ALLOCATION_FLAG_FORCE_U32 = 0x7FFFFFFF
} D3D12MA_VIRTUAL_ALLOCATION_FLAGS;

typedef struct D3D12MA_ALLOCATION_CALLBACKS {
    D3D12MA_AllocateFunctionType* pAllocate;
    D3D12MA_FreeFunctionType* pFree;
    void* pPrivateData;
} D3D12MA_ALLOCATION_CALLBACKS;

typedef struct D3D12MA_ALLOCATOR_DESC {
    D3D12MA_ALLOCATOR_FLAGS Flags;
    ID3D12Device* pDevice;
    uint64_t PreferredBlockSize;
    const D3D12MA_ALLOCATION_CALLBACKS* pAllocationCallbacks;
    IDXGIAdapter* pAdapter;
} D3D12MA_ALLOCATOR_DESC;

typedef struct D3D12MA_POOL_DESC
{
    D3D12MA_POOL_FLAGS Flags;
    D3D12_HEAP_PROPERTIES HeapProperties;
    D3D12_HEAP_FLAGS HeapFlags;
    uint64_t BlockSize;
    uint32_t MinBlockCount;
    uint32_t MaxBlockCount;
    uint64_t MinAllocationAlignment;
    ID3D12ProtectedResourceSession* pProtectedSession;
    D3D12_RESIDENCY_PRIORITY ResidencyPriority;
} D3D12MA_POOL_DESC;

typedef struct D3D12MA_VIRTUAL_BLOCK_DESC
{
    D3D12MA_VIRTUAL_BLOCK_FLAGS Flags;
    uint64_t Size;
    const D3D12MA_ALLOCATION_CALLBACKS* pAllocationCallbacks;
} D3D12MA_VIRTUAL_BLOCK_DESC;

typedef struct D3D12MA_ALLOCATION_DESC
{
    D3D12MA_ALLOCATION_FLAGS Flags;
    D3D12_HEAP_TYPE HeapType;
    D3D12_HEAP_FLAGS ExtraHeapFlags;
    D3D12MA_Pool* CustomPool;
    void* pPrivateData;
} D3D12MA_ALLOCATION_DESC;

typedef struct D3D12MA_VIRTUAL_ALLOCATION_DESC
{
    D3D12MA_VIRTUAL_ALLOCATION_FLAGS Flags;
    uint64_t Size;
    uint64_t Alignment;
    void* pPrivateData;
} D3D12MA_VIRTUAL_ALLOCATION_DESC;

typedef struct D3D12MA_VIRTUAL_ALLOCATION_INFO {
    uint64_t Offset;
    uint64_t Size;
    void* pPrivateData;
} D3D12MA_VIRTUAL_ALLOCATION_INFO;

typedef struct D3D12MA_Statistics {
    uint32_t BlockCount;
    uint32_t AllocationCount;
    uint64_t BlockBytes;
    uint64_t AllocationBytes;
} D3D12MA_Statistics;

typedef struct D3D12MA_DetailedStatistics {
    D3D12MA_Statistics Stats;
    uint32_t UnusedRangeCount;
    uint64_t AllocationSizeMin;
    uint64_t AllocationSizeMax;
    uint64_t UnusedRangeSizeMin;
    uint64_t UnusedRangeSizeMax;
} D3D12MA_DetailedStatistics;

typedef struct D3D12MA_TotalStatistics {
    D3D12MA_DetailedStatistics HeapType[5];
    D3D12MA_DetailedStatistics MemorySegmentGroup[2];
    D3D12MA_DetailedStatistics Total;
} D3D12MA_TotalStatistics;

typedef struct D3D12MA_Budget {
    D3D12MA_Statistics Stats;
    uint64_t UsageBytes;
    uint64_t BudgetBytes;
} D3D12MA_Budget;

typedef struct D3D12MA_DEFRAGMENTATION_MOVE
{
    D3D12MA_DEFRAGMENTATION_MOVE_OPERATION Operation;
    D3D12MA_Allocation* pSrcAllocation;
    D3D12MA_Allocation* pDstTmpAllocation;
} D3D12MA_DEFRAGMENTATION_MOVE;

typedef struct D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO
{
    uint32_t MoveCount;
    D3D12MA_DEFRAGMENTATION_MOVE* pMoves;
} D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO;

typedef struct D3D12MA_DEFRAGMENTATION_STATS
{
    uint64_t BytesMoved;
    uint64_t BytesFreed;
    uint32_t AllocationsMoved;
    uint32_t HeapsFreed;
} D3D12MA_DEFRAGMENTATION_STATS;

typedef struct D3D12MA_DEFRAGMENTATION_DESC
{
    D3D12MA_DEFRAGMENTATION_FLAGS Flags;
    uint64_t MaxBytesPerPass;
    uint32_t MaxAllocationsPerPass;
} D3D12MA_DEFRAGMENTATION_DESC;


typedef struct D3D12MA_VirtualAllocation {
    D3D12MA_AllocHandle AllocHandle;
} D3D12MA_VirtualAllocation;

/* Top level API */
D3D12MA_C_API HRESULT D3D12MA_CreateAllocator(const D3D12MA_ALLOCATOR_DESC* pDesc, D3D12MA_Allocator** ppAllocator);
D3D12MA_C_API HRESULT D3D12MA_CreateVirtualBlock(const D3D12MA_VIRTUAL_BLOCK_DESC* pDesc, D3D12MA_VirtualBlock** ppVirtualBlock);

/* D3D12MA_Allocator */
D3D12MA_C_API uint32_t D3D12MA_Allocator_AddRef(D3D12MA_Allocator* pSelf);
D3D12MA_C_API uint32_t D3D12MA_Allocator_Release(D3D12MA_Allocator* pSelf);
D3D12MA_C_API D3D12MA_Bool32 D3D12MA_Allocator_IsUMA(D3D12MA_Allocator* pSelf);
D3D12MA_C_API D3D12MA_Bool32 D3D12MA_Allocator_IsCacheCoherentUMA(D3D12MA_Allocator* pSelf);
D3D12MA_C_API D3D12MA_Bool32 D3D12MA_Allocator_IsGPUUploadHeapSupported(D3D12MA_Allocator* pSelf);
D3D12MA_C_API uint64_t D3D12MA_Allocator_GetMemoryCapacity(D3D12MA_Allocator* pSelf, uint32_t memorySegmentGroup);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateResource(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateResource2(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateResource3(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, uint32_t NumCastableFormats, DXGI_FORMAT* pCastableFormats, D3D12MA_Allocation** ppAllocation, REFIID riidResource, void** ppvResource);
D3D12MA_C_API HRESULT D3D12MA_Allocator_AllocateMemory(D3D12MA_Allocator* pSelf, const D3D12MA_ALLOCATION_DESC* pAllocDesc, const D3D12_RESOURCE_ALLOCATION_INFO* pAllocInfo, D3D12MA_Allocation** ppAllocation);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateAliasingResource(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateAliasingResource1(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource);
D3D12MA_C_API HRESULT D3D12MA_Allocator_CreateAliasingResource2(D3D12MA_Allocator* pSelf, D3D12MA_Allocation* pAllocation, uint64_t AllocationLocalOffset, const D3D12_RESOURCE_DESC1* pResourceDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, uint32_t NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riidResource, void** ppvResource);

D3D12MA_C_API HRESULT D3D12MA_Allocator_CreatePool(D3D12MA_Allocator* pSelf, const D3D12MA_POOL_DESC* pPoolDesc, D3D12MA_Pool** ppPool);
D3D12MA_C_API void D3D12MA_Allocator_SetCurrentFrameIndex(D3D12MA_Allocator* pSelf, uint32_t frameIndex);
D3D12MA_C_API void D3D12MA_Allocator_GetBudget(D3D12MA_Allocator* pSelf, D3D12MA_Budget* pLocalBudget, D3D12MA_Budget* pNonLocalBudget);
D3D12MA_C_API void D3D12MA_Allocator_CalculateStatistics(D3D12MA_Allocator* pSelf, D3D12MA_TotalStatistics* pStats);
D3D12MA_C_API void D3D12MA_Allocator_BuildStatsString(D3D12MA_Allocator* pSelf, wchar_t** ppStatsString, D3D12MA_Bool32 DetailedMap);
D3D12MA_C_API void D3D12MA_Allocator_FreeStatsString(D3D12MA_Allocator* pSelf, wchar_t* pStatsString);
D3D12MA_C_API void D3D12MA_Allocator_BeginDefragmentation(void* pSelf, const D3D12MA_DEFRAGMENTATION_DESC* pDesc, D3D12MA_DefragmentationContext** ppContext);

/* D3D12MA_Allocation */
D3D12MA_C_API uint32_t D3D12MA_Allocation_AddRef(D3D12MA_Allocation* pSelf);
D3D12MA_C_API uint32_t D3D12MA_Allocation_Release(D3D12MA_Allocation* pSelf);
D3D12MA_C_API uint64_t D3D12MA_Allocation_GetOffset(D3D12MA_Allocation* pSelf);
D3D12MA_C_API uint64_t D3D12MA_Allocation_GetAlignment(D3D12MA_Allocation* pSelf);
D3D12MA_C_API uint64_t D3D12MA_Allocation_GetSize(D3D12MA_Allocation* pSelf);
D3D12MA_C_API ID3D12Resource* D3D12MA_Allocation_GetResource(D3D12MA_Allocation* pSelf);
D3D12MA_C_API void D3D12MA_Allocation_SetResource(D3D12MA_Allocation* pSelf, ID3D12Resource* pResource);
D3D12MA_C_API ID3D12Heap* D3D12MA_Allocation_GetHeap(D3D12MA_Allocation* pSelf);
D3D12MA_C_API void D3D12MA_Allocation_SetPrivateData(D3D12MA_Allocation* pSelf, void* pPrivateData);
D3D12MA_C_API void* D3D12MA_Allocation_GetPrivateData(D3D12MA_Allocation* pSelf);
D3D12MA_C_API void D3D12MA_Allocation_SetName(D3D12MA_Allocation* pSelf, const wchar_t* Name);
D3D12MA_C_API const wchar_t* D3D12MA_Allocation_GetName(D3D12MA_Allocation* pSelf);

/* D3D12MA_Pool */
D3D12MA_C_API uint32_t D3D12MA_Pool_AddRef(D3D12MA_Pool* pSelf);
D3D12MA_C_API uint32_t D3D12MA_Pool_Release(D3D12MA_Pool* pSelf);
D3D12MA_C_API void D3D12MA_Pool_GetDesc(D3D12MA_Pool* pSelf, D3D12MA_POOL_DESC* pDesc);
D3D12MA_C_API void D3D12MA_Pool_GetStatistics(D3D12MA_Pool* pSelf, D3D12MA_Statistics* pStats);
D3D12MA_C_API void D3D12MA_Pool_CalculateStatistics(D3D12MA_Pool* pSelf, D3D12MA_DetailedStatistics* pStats);
D3D12MA_C_API void D3D12MA_Pool_SetName(D3D12MA_Pool* pSelf, const wchar_t* Name);
D3D12MA_C_API const wchar_t* D3D12MA_Pool_GetName(D3D12MA_Pool* pSelf);
D3D12MA_C_API HRESULT D3D12MA_Pool_BeginDefragmentation(D3D12MA_Pool* pSelf, const D3D12MA_DEFRAGMENTATION_DESC* pDesc, D3D12MA_DefragmentationContext** ppContext);

/* D3D12MA_VirtualBlock */
D3D12MA_C_API uint32_t D3D12MA_VirtualBlock_AddRef(D3D12MA_VirtualBlock* pSelf);
D3D12MA_C_API uint32_t D3D12MA_VirtualBlock_Release(D3D12MA_VirtualBlock* pSelf);
D3D12MA_C_API D3D12MA_Bool32 D3D12MAVirtualBlock_IsEmpty(D3D12MA_VirtualBlock* pSelf);
D3D12MA_C_API void D3D12MAVirtualBlock_GetAllocationInfo(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation, D3D12MA_VIRTUAL_ALLOCATION_INFO* pInfo);
D3D12MA_C_API HRESULT D3D12MAVirtualBlock_Allocate(D3D12MA_VirtualBlock* pSelf, const D3D12MA_VIRTUAL_ALLOCATION_DESC* pDesc, D3D12MA_VirtualAllocation* pAllocation, uint64_t* pOffset);
D3D12MA_C_API void D3D12MAVirtualBlock_FreeAllocation(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation);
D3D12MA_C_API void D3D12MAVirtualBlock_Clear(D3D12MA_VirtualBlock* pSelf);
D3D12MA_C_API void D3D12MAVirtualBlock_SetAllocationPrivateData(D3D12MA_VirtualBlock* pSelf, D3D12MA_VirtualAllocation Allocation, void* pPrivateData);
D3D12MA_C_API void D3D12MAVirtualBlock_GetStatistics(D3D12MA_VirtualBlock* pSelf, D3D12MA_Statistics* pStats);
D3D12MA_C_API void D3D12MAVirtualBlock_CalculateStatistics(D3D12MA_VirtualBlock* pSelf, D3D12MA_DetailedStatistics* pStats);
D3D12MA_C_API void D3D12MAVirtualBlock_BuildStatsString(D3D12MA_VirtualBlock* pSelf, wchar_t** ppStatsString);
D3D12MA_C_API void D3D12MAVirtualBlock_FreeStatsString(D3D12MA_VirtualBlock* pSelf, wchar_t* pStatsString);

/* D3D12MA_DefragmentationContext */
D3D12MA_C_API uint32_t D3D12MA_DefragmentationContext_AddRef(D3D12MA_DefragmentationContext* pSelf);
D3D12MA_C_API uint32_t D3D12MA_DefragmentationContext_Release(D3D12MA_DefragmentationContext* pSelf);
D3D12MA_C_API HRESULT D3D12MA_DefragmentationContext_BeginPass(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO* pPassInfo);
D3D12MA_C_API HRESULT D3D12MA_DefragmentationContext_EndPass(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_PASS_MOVE_INFO* pPassInfo);
D3D12MA_C_API void D3D12MA_DefragmentationContext_GetStats(D3D12MA_DefragmentationContext* pSelf, D3D12MA_DEFRAGMENTATION_STATS* pStats);

#endif /* D3D12MA_C_H_ */
