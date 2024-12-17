// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PLATFORM_H_
#define ALIMER_PLATFORM_H_ 1

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __declspec(dllexport)
#        else
#            define _ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define _ALIMER_EXPORT
#        endif
#    endif
#else
#    define _ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_API _ALIMER_EXTERN _ALIMER_EXPORT

#if !defined(ALIMER_OBJECT_ATTRIBUTE)
#define ALIMER_OBJECT_ATTRIBUTE
#endif
#if !defined(ALIMER_ENUM_ATTRIBUTE)
#define ALIMER_ENUM_ATTRIBUTE
#endif
#if !defined(ALIMER_STRUCT_ATTRIBUTE)
#define ALIMER_STRUCT_ATTRIBUTE
#endif
#if !defined(ALIMER_FUNC_ATTRIBUTE)
#define ALIMER_FUNC_ATTRIBUTE
#endif
#if !defined(ALIMER_NULLABLE)
#define ALIMER_NULLABLE
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Types */
typedef uint32_t Flags;
typedef uint32_t Bool32;

#endif /* ALIMER_PLATFORM_H_ */
