// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_VULKAN)
#include "alimer_internal.h"

#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
ALIMER_DISABLE_WARNINGS()
#include "third_party/vk_mem_alloc.h"
//#include "third_party/spirv_reflect.h"
ALIMER_ENABLE_WARNINGS()

#ifdef _WIN32
#   include <windows.h>
#else
#   include <dlfcn.h>
#endif

static struct {
    bool initialized;
#ifdef _WIN32
    HMODULE library;
#else
    void* library;
#endif
} state;

#endif /* defined(ALIMER_VULKAN) */
