// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(_WIN32)
#   define VMA_CALL_PRE __declspec(dllexport)
#else
#   define VMA_CALL_PRE __attribute__((visibility("default")))
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wtautological-compare" // comparison of unsigned expression < 0 is always false
#   pragma clang diagnostic ignored "-Wunused-private-field"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wnullability-completeness"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wtautological-compare" // comparison of unsigned expression < 0 is always false
#   pragma GCC diagnostic ignored "-Wunused-private-field"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#   pragma GCC diagnostic ignored "-Wunused-variable"
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#   pragma GCC diagnostic ignored "-Wnullability-completeness"
#elif defined(_MSC_VER)
#   pragma warning(push, 4)
#   pragma warning(disable: 4127) // conditional expression is constant
#   pragma warning(disable: 4100) // unreferenced formal parameter
#   pragma warning(disable: 4189) // local variable is initialized but not referenced
#   pragma warning(disable: 4324) // structure was padded due to alignment specifier
#   pragma warning(disable: 4820) // 'X': 'N' bytes padding added after data member 'X'
#endif

#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#if defined(__ANDROID__)
#define VMA_NULLABLE
#define VMA_NOT_NULL
#endif

#include "vk_mem_alloc.h"

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
