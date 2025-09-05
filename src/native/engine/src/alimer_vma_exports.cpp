// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"

#if defined(_WIN32)
#   define VMA_CALL_PRE __declspec(dllexport)
#else
#   define VMA_CALL_PRE __attribute__((visibility("default")))
#endif

#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#if defined(__ANDROID__)
#define VMA_NULLABLE
#define VMA_NOT_NULL
#endif

ALIMER_DISABLE_WARNINGS()
#include "vk_mem_alloc.h"
ALIMER_ENABLE_WARNINGS()
