// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Memory.h"
#include "Alimer/Math/MathHelper.h"
#include "Alimer/Core/Log.h"
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <malloc.h>
#endif /* defined(_WIN32) */

using namespace Alimer;

void* Memory::AlignedAlloc(size_t size, size_t alignment)
{
    ALIMER_ASSERT(IsPow2(alignment));

#if defined(_WIN32)
    // Microsoft doesn't implement C++17 std::aligned_alloc
    void* result = _aligned_malloc(size, alignment);
#else
    void* result = aligned_alloc(alignment, AlignUp(size, alignment));
#endif

    ALIMER_ASSERT(result);
    return result;
}

void Memory::AlignedFree(void* ptr)
{
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void* Memory::Alloc(size_t size)
{
    void* result = malloc(size);
    ALIMER_ASSERT(result);
    return result;
}

void* Memory::Alloc(size_t elementCount, size_t elementSize)
{
    size_t byteCount = GetByteCount(elementCount, elementSize);
    return Alloc(byteCount);
}

void* Memory::AllocZeroed(size_t size)
{
    return AllocZeroed(size, 1);
}

void* Memory::AllocZeroed(size_t elementCount, size_t elementSize)
{
#ifndef _WIN32
    void* result;
    if ((elementCount != 0) && (elementSize != 0))
    {
        result = calloc(elementCount, elementSize);
    }
    else
    {
        // The C standard does not define what happens when num == 0 or size == 0, we want an "empty" allocation
        result = malloc(1);
    }
#else
    // The Windows implementation handles elementCount == 0 && elementSize == 0 as we expect
    void* result = calloc(elementCount, elementSize);
#endif
    ALIMER_ASSERT(result);
    return result;
}

void Memory::Free(void* ptr)
{
    free(ptr);
}

void* Memory::Realloc(void* ptr, size_t size)
{
    void* result = realloc(ptr, (size != 0) ? size : 1);
    ALIMER_ASSERT(result);
    return result;
}

void Memory::Clear(void* ptr, size_t size)
{
    if (ptr != nullptr && size > 0)
    {
        memset(ptr, 0, size);
    }
}

size_t Memory::GetByteCount(size_t elementCount, size_t elementSize)
{
    // This is based on the `mi_count_size_overflow` and `mi_mul_overflow` methods from microsoft/mimalloc.
    // Original source is Copyright (c) 2019 Microsoft Corporation, Daan Leijen. Licensed under the MIT license

    // sqrt(nuint.MaxValue)
    size_t multiplyNoOverflow = (size_t)1 << (4 * sizeof(size_t));

    return ((elementSize >= multiplyNoOverflow) || (elementCount >= multiplyNoOverflow)) && (elementSize > 0) && ((SIZE_MAX / elementSize) < elementCount) ? SIZE_MAX : (elementCount * elementSize);
}
