// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_TYPES_H_
#define ALIMER_TYPES_H_ 1

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

#if !defined(ALIMER_NULLABLE)
#define ALIMER_NULLABLE
#endif

#ifdef __cplusplus
#   define DEFAULT_INITIALIZER(x) = x
#else
#   define DEFAULT_INITIALIZER(x)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#endif /* ALIMER_TYPES_H_ */
