// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_INTERNAL_H
#define ALIMER_INTERNAL_H

#include "alimer.h"
#include "alimer_platform.h"
#include <stdbool.h>
#include <string.h> // memset
#include <stdlib.h> 

#define ALIMER_UNUSED(x) (void)(x)

// Macro for determining size of arrays.
#if defined(_MSC_VER)
#   define ALIMER_ARRAYSIZE(arr) _countof(arr)
#else
#   define ALIMER_ARRAYSIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#ifndef ALIMER_ASSERT
#   include <assert.h>
#   define ALIMER_ASSERT(c) assert(c)
#endif

#define ALIMER_MIN(a, b) (a < b ? a : b)
#define ALIMER_MAX(a, b) (a > b ? a : b)
#define ALIMER_CLAMP(val, min, max) ALIMER_MAX(min, ALIMER_MIN(val, max))
#define ALIMER_DEF(val, def) (((val) == 0) ? (def) : (val))
#define ALIMER_DEF_FLOAT(val, def) (((val) == 0.0f) ? (def) : (val))
#define _check_flags(flags, flag) (((flags) & (flag)) != 0)

_ALIMER_EXTERN void* alimer_alloc(size_t size);
_ALIMER_EXTERN void* alimer_alloc_clear(size_t size);
_ALIMER_EXTERN void* alimer_realloc(void* ptr, size_t size);
_ALIMER_EXTERN void alimer_free(void* ptr);
_ALIMER_EXTERN void Alimer_clear(void* ptr, size_t size);

// Convenience macros for invoking custom memory allocation callbacks.
#define ALIMER_ALLOC(type)     ((type*)alimer_alloc_clear(sizeof(type)))
#define ALIMER_ALLOCN(type, n) ((type*)alimer_alloc_clear(sizeof(type) * n))
#define ALIMER_FREE(ptr)       (alimer_free((void*)(ptr)))

// Custom allocation callbacks.
typedef void* (*AlimerAllocateMemoryFunc)(size_t size, void* userData);
typedef void (*AlimerFreeMemoryFunc)(void* ptr, void* userData);

typedef struct AlimerMemoryAllocationCallbacks {
    AlimerAllocateMemoryFunc AllocateMemory;
    AlimerFreeMemoryFunc FreeMemory;
} AlimerMemoryAllocationCallbacks;

typedef struct AlimerState
{
    bool initialized;
    Config config;
} AlimerState;

_ALIMER_EXTERN AlimerState* GetState();
_ALIMER_EXTERN const AlimerMemoryAllocationCallbacks* MEMORY_ALLOC_CB;

_ALIMER_EXTERN void Alimer_SetAllocationCallbacks(const AlimerMemoryAllocationCallbacks* callback, void* userData);

#endif /* ALIMER_INTERNAL_H */
