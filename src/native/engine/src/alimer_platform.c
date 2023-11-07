// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include <stdarg.h>
#include <stdio.h>

/* Memory */
static void* _default_alloc(size_t size, void* user_data)
{
    ALIMER_UNUSED(user_data);
    void* ptr = malloc(size);
    ALIMER_ASSERT(ptr);
    return ptr;
}

static void _default_free(void* ptr, void* user_data)
{
    ALIMER_UNUSED(user_data);
    free(ptr);
}

const AlimerMemoryAllocationCallbacks DEFAULT_MEMORY_ALLOC_CB = { _default_alloc, _default_free };
const AlimerMemoryAllocationCallbacks* MEMORY_ALLOC_CB = &DEFAULT_MEMORY_ALLOC_CB;
void* s_memoryUserData = NULL;

void* alimer_alloc(size_t size)
{
    return MEMORY_ALLOC_CB->AllocateMemory(size, s_memoryUserData);
}

void* alimer_alloc_clear(size_t size)
{
    void* ptr = alimer_alloc(size);
    Alimer_clear(ptr, size);
    return ptr;
}

void* alimer_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void alimer_free(void* ptr)
{
    MEMORY_ALLOC_CB->FreeMemory(ptr, s_memoryUserData);
}

void Alimer_clear(void* ptr, size_t size)
{
    ALIMER_ASSERT(ptr && (size > 0));
    memset(ptr, 0, size);
}

void Alimer_SetAllocationCallbacks(const AlimerMemoryAllocationCallbacks* callback, void* userData)
{
    if (callback == NULL)
    {
        MEMORY_ALLOC_CB = &DEFAULT_MEMORY_ALLOC_CB;
    }
    else {
        MEMORY_ALLOC_CB = callback;
    }

    s_memoryUserData = userData;
}

/* Other */
void Alimer_GetVersion(int* major, int* minor, int* patch)
{
    if (major) *major = ALIMER_VERSION_MAJOR;
    if (minor) *minor = ALIMER_VERSION_MINOR;
    if (patch) *patch = ALIMER_VERSION_PATCH;
}
