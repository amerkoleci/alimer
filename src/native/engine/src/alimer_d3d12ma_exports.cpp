// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12MemAlloc.h"
using namespace D3D12MA;

#ifdef __cplusplus
#   define D3D12MA_EXPORT_API extern "C" __declspec(dllexport)
#else
#   define D3D12MA_EXPORT_API extern __declspec(dllexport)
#endif

D3D12MA_EXPORT_API HRESULT D3D12MA_CreateAllocator(const ALLOCATOR_DESC* pDesc, Allocator** ppAllocator)
{
    return CreateAllocator(pDesc, ppAllocator);
}

D3D12MA_EXPORT_API uint32_t D3D12MA_Allocator_AddRef(Allocator* allocator)
{
    return allocator->AddRef();
}

D3D12MA_EXPORT_API uint32_t D3D12MA_Allocator_Release(Allocator* allocator)
{
    return allocator->Release();
}

D3D12MA_EXPORT_API HRESULT D3D12MA_Allocator_CreateResource2(
    Allocator* allocator,
    const ALLOCATION_DESC* pAllocationDesc,
    const D3D12_RESOURCE_DESC1* pResourceDesc,
    D3D12_RESOURCE_STATES InitialResourceState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    Allocation** ppAllocation,
    const IID* riidResource, void** ppvResource)
{
    return allocator->CreateResource2(
        pAllocationDesc,
        pResourceDesc,
        InitialResourceState,
        pOptimizedClearValue,
        ppAllocation,
        *riidResource,
        ppvResource);
}

D3D12MA_EXPORT_API HRESULT D3D12MA_Allocator_CreateResource3(
    Allocator* allocator,
    const ALLOCATION_DESC* pAllocationDesc,
    const D3D12_RESOURCE_DESC1* pResourceDesc,
    D3D12_BARRIER_LAYOUT InitialLayout,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    UINT32 NumCastableFormats,
    DXGI_FORMAT* pCastableFormats,
    Allocation** ppAllocation,
    const IID* riidResource, void** ppvResource)
{
    return allocator->CreateResource3(
        pAllocationDesc,
        pResourceDesc,
        InitialLayout,
        pOptimizedClearValue,
        NumCastableFormats,
        pCastableFormats,
        ppAllocation,
        *riidResource,
        ppvResource);
}

D3D12MA_EXPORT_API uint32_t D3D12MA_Allocation_AddRef(Allocation* allocation)
{
    return allocation->AddRef();
}

D3D12MA_EXPORT_API uint32_t D3D12MA_Allocation_Release(Allocation* allocation)
{
    return allocation->Release();
}
