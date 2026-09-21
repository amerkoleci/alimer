// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PHYSICS_INTERNAL_H_
#define ALIMER_PHYSICS_INTERNAL_H_ 1

#include "alimer_physics.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <stdarg.h>

#define ALIMER_STRINGIZE_HELPER(X) #X
#define ALIMER_STRINGIZE(X) ALIMER_STRINGIZE_HELPER(X)
#define ALIMER_UNUSED(x) (void)(x)

// Macro for determining size of arrays.
#if defined(_MSC_VER)
#   define ALIMER_COUNT_OF(arr) _countof(arr)
#else
#   define ALIMER_COUNT_OF(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// Always turn on asserts in Debug mode
#if defined(_DEBUG) && !defined(ALIMER_ENABLE_ASSERTS)
#define ALIMER_ENABLE_ASSERTS
#endif

#ifdef ALIMER_ENABLE_ASSERTS
#   include <assert.h>
#   define ALIMER_ASSERT(c) assert(c)
#else
#   define ALIMER_ASSERT(...) ((void)0)
#endif

// ============================================================================
// Memory Allocation
// ============================================================================

// Internal allocator state
typedef struct PhysicsAllocator {
    PhysicsAllocCallback alloc;
    PhysicsFreeCallback free;
    void* userData;
} PhysicsAllocator;

_ALIMER_EXTERN PhysicsAllocator g_physics_allocator;

// Internal wrapper functions - use these instead of malloc/calloc/realloc/free
_ALIMER_EXTERN void* physics_malloc(size_t size)
{
    return g_physics_allocator.alloc(size, g_physics_allocator.userData);
}

_ALIMER_EXTERN void* physics_calloc(size_t count, size_t size)
{
    size_t total = count * size;
    void* ptr = g_physics_allocator.alloc(total, g_physics_allocator.userData);
    if (ptr)
        memset(ptr, 0, total);
    return ptr;
}

_ALIMER_EXTERN void  physics_free(void* ptr)
{
    if (ptr)
        g_physics_allocator.free(ptr, g_physics_allocator.userData);
}

_ALIMER_EXTERN char* physics_strdup(const char* str)
{
    if (!str)
        return nullptr;
    size_t len = strlen(str) + 1;
    char* copy = (char*)physics_malloc(len);
    if (copy)
        memcpy(copy, str, len);
    return copy;
}

// Convenience macros for invoking custom memory allocation callbacks.
#define PHYSICS_ALLOC(type)          ((type*)physics_calloc(1, sizeof(type)))
#define PHYSICS_ALLOCN(type, n)      ((type*)physics_calloc(n, sizeof(type)))
#define PHYSICS_MAX_LOG_MESSAGE_SIZE (1024)
#define PHYSICS_DEF(val, def) (((val) == 0) ? (def) : (val))
#define PHYSICS_DEF_FLT(val, def) (((val) == 0.0f) ? (def) : (val))

#ifdef __cplusplus
#include <new>
#include <atomic>

namespace
{
    template<typename T>
    static T* PhysicsAlloc()
    {
        T* object = (T*)physics_malloc(sizeof(T));
        new (object) T();
        return object;
    }

    template<typename T>
    static void PhysicsFree(T* object)
    {
        if (!object)
            return;

        if constexpr (!__is_trivially_copyable(T))
        {
            object->~T();
        }

        physics_free(object);
    }

    template <typename T>
    void SafeRelease(T& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    //---------------------------------------------
    // Basic comparisons
    //---------------------------------------------
    template <typename T> [[nodiscard]] constexpr T Abs(T v) { return (v >= 0) ? v : -v; }
    template <typename T> [[nodiscard]] constexpr T Min(T a, T b) { return (a < b) ? a : b; }
    template <typename T> [[nodiscard]] constexpr T Max(T a, T b) { return (a < b) ? b : a; }
    template <typename T> [[nodiscard]] constexpr T Clamp(T arg, T lo, T hi) { return (arg < lo) ? lo : (arg < hi) ? arg : hi; }
}

#endif /* __cplusplus */

#endif /* ALIMER_PHYSICS_INTERNAL_H_ */
