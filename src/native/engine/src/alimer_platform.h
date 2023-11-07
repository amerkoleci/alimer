// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PLATFORM_H
#define ALIMER_PLATFORM_H

// Compilers
#define ALIMER_COMPILER_CLANG 0
#define ALIMER_COMPILER_CLANG_ANALYZER 0
#define ALIMER_COMPILER_CLANG_CL 0
#define ALIMER_COMPILER_GCC 0
#define ALIMER_COMPILER_MSVC 0

// Platform traits and groups
#define ALIMER_PLATFORM_APPLE 0
#define ALIMER_PLATFORM_POSIX 0

#define ALIMER_PLATFORM_FAMILY_MOBILE 0
#define ALIMER_PLATFORM_FAMILY_DESKTOP 0
#define ALIMER_PLATFORM_FAMILY_CONSOLE 0

// Platforms
#define ALIMER_PLATFORM_ANDROID 0
#define ALIMER_PLATFORM_LINUX 0
#define ALIMER_PLATFORM_IOS 0
#define ALIMER_PLATFORM_TVOS 0
#define ALIMER_PLATFORM_MACOS 0
#define ALIMER_PLATFORM_WINDOWS 0
#define ALIMER_PLATFORM_UWP 0
#define ALIMER_PLATFORM_XBOX_SCARLETT 0
#define ALIMER_PLATFORM_XBOX_ONE 0
#define ALIMER_PLATFORM_WEB 0

// CPU
#define ALIMER_ARCH_X64 0
#define ALIMER_ARCH_X86 0
#define ALIMER_ARCH_A64 0
#define ALIMER_ARCH_ARM 0
#define ALIMER_ARCH_PPC 0

// Architecture
#define ALIMER_ARCH_32BIT 0
#define ALIMER_ARCH_64BIT 0

#define ALIMER_STRINGIZE_HELPER(X) #X
#define ALIMER_STRINGIZE(X) ALIMER_STRINGIZE_HELPER(X)

#define ALIMER_CONCAT_HELPER(X, Y) X##Y
#define ALIMER_CONCAT(X, Y) ALIMER_CONCAT_HELPER(X, Y)

/* Detect compiler first */
#if defined(__clang__)
#    undef ALIMER_COMPILER_CLANG
#    define ALIMER_COMPILER_CLANG 1
#    define ALIMER_COMPILER_NAME "clang"
#    define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__clang_major__) "." ALIMER_STRINGIZE(__clang_minor__)
#    define ALIMER_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#    if defined(__clang_analyzer__)
#        undef ALIMER_COMPILER_CLANG_ANALYZER
#        define ALIMER_COMPILER_CLANG_ANALYZER 1
#    endif        // defined(__clang_analyzer__)
#    if defined(_MSC_VER)
#        undef ALIMER_COMPILER_MSVC
#        define ALIMER_COMPILER_MSVC 1
#        undef ALIMER_COMPILER_CLANG_CL
#        define ALIMER_COMPILER_CLANG_CL 1
#    endif
#elif defined(_MSC_VER)
#    undef ALIMER_COMPILER_MSVC
#    define ALIMER_COMPILER_MSVC 1
#    define ALIMER_COMPILER_NAME "msvc"
#    define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(_MSC_VER)
#elif defined(__GNUC__)
#    undef ALIMER_COMPILER_GCC
#    define ALIMER_COMPILER_GCC 1
#    define ALIMER_COMPILER_NAME "gcc"
#    define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__GNUC__) "." ALIMER_STRINGIZE(__GNUC_MINOR__)
#    define ALIMER_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#    error "Unknown compiler"
#endif

/* Detect platform*/
#ifdef _GAMING_XBOX_SCARLETT
#    undef ALIMER_PLATFORM_FAMILY_CONSOLE
#    define ALIMER_PLATFORM_FAMILY_CONSOLE 1
#    undef ALIMER_PLATFORM_XBOX_SCARLETT
#    define ALIMER_PLATFORM_XBOX_SCARLETT 1
#    define ALIMER_PLATFORM_NAME "XboxScarlett"
#elif (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
#    undef ALIMER_PLATFORM_FAMILY_CONSOLE
#    define ALIMER_PLATFORM_FAMILY_CONSOLE 1
#    undef ALIMER_PLATFORM_XBOX_ONE
#    define ALIMER_PLATFORM_XBOX_ONE 1
#    define ALIMER_PLATFORM_NAME "XboxOne"
#elif defined(_WIN32) || defined(_WIN64)
#   include <winapifamily.h>
#   if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#       undef ALIMER_PLATFORM_FAMILY_DESKTOP
#       define ALIMER_PLATFORM_FAMILY_DESKTOP 1
#       undef ALIMER_PLATFORM_WINDOWS
#       define ALIMER_PLATFORM_WINDOWS 1
#       define ALIMER_PLATFORM_NAME "Windows"
#   else
#       undef ALIMER_PLATFORM_FAMILY_CONSOLE
#       define ALIMER_PLATFORM_FAMILY_CONSOLE 1
#       undef ALIMER_PLATFORM_UWP
#       define ALIMER_PLATFORM_UWP 1
#       define ALIMER_PLATFORM_NAME "UWP"
#   endif

#elif defined(__ANDROID__)
#    undef ALIMER_PLATFORM_POSIX
#    define ALIMER_PLATFORM_POSIX 1
#    undef ALIMER_PLATFORM_FAMILY_MOBILE
#    define ALIMER_PLATFORM_FAMILY_MOBILE 1
#    undef ALIMER_PLATFORM_ANDROID
#    define ALIMER_PLATFORM_ANDROID 1
#    define ALIMER_PLATFORM_NAME "Android"
#elif defined(__EMSCRIPTEN__)
#    undef ALIMER_PLATFORM_POSIX
#    define ALIMER_PLATFORM_POSIX 1
#    undef ALIMER_PLATFORM_LINUX
#    define ALIMER_PLATFORM_LINUX 1
#    undef ALIMER_PLATFORM_WEB
#    define ALIMER_PLATFORM_WEB 1
#    define ALIMER_PLATFORM_NAME "Web"
#elif defined(__linux__)        // note: __ANDROID__/__EMSCRIPTEN__ implies __linux__
#    undef ALIMER_PLATFORM_POSIX
#    define ALIMER_PLATFORM_POSIX 1
#    undef ALIMER_PLATFORM_LINUX
#    define ALIMER_PLATFORM_LINUX 1
#    define ALIMER_PLATFORM_NAME "Linux"
#elif defined(__APPLE__)        // macOS, iOS, tvOS
#    include <TargetConditionals.h>
#    undef ALIMER_PLATFORM_APPLE
#    define ALIMER_PLATFORM_APPLE 1
#    undef ALIMER_PLATFORM_POSIX
#    define ALIMER_PLATFORM_POSIX 1
#    if TARGET_OS_WATCH        // watchOS
#        error "Apple Watch is not supported"
#    elif TARGET_OS_IOS        // iOS
#        undef ALIMER_PLATFORM_FAMILY_MOBILE
#        define ALIMER_PLATFORM_FAMILY_MOBILE 1
#        undef ALIMER_PLATFORM_IOS
#        define ALIMER_PLATFORM_IOS 1
#        define ALIMER_PLATFORM_NAME "iOS"
#    elif TARGET_OS_TV        // tvOS
#        undef ALIMER_PLATFORM_FAMILY_CONSOLE
#        define ALIMER_PLATFORM_FAMILY_CONSOLE 1
#        undef ALIMER_PLATFORM_TVOS
#        define ALIMER_PLATFORM_TVOS 1
#        define ALIMER_PLATFORM_NAME "tvOS"
#    elif TARGET_OS_MAC
#        undef ALIMER_PLATFORM_FAMILY_DESKTOP
#        define ALIMER_PLATFORM_FAMILY_DESKTOP 1
#        undef ALIMER_PLATFORM_MACOS
#        define ALIMER_PLATFORM_MACOS 1
#        define ALIMER_PLATFORM_NAME "macOS"
#    endif
#else
#    error "Unknown operating system"
#endif

/**
Architecture defines, see http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
*/
#if defined(__x86_64__) || defined(_M_X64)
#    undef ALIMER_ARCH_X64
#    define ALIMER_ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86) || defined(__EMSCRIPTEN__)
#    undef ALIMER_ARCH_X86
#    define ALIMER_ARCH_X86 1
#elif defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)
#    undef ALIMER_ARCH_A64
#    define ALIMER_ARCH_A64 1
#elif defined(__arm__) || defined(_M_ARM)
#    undef ALIMER_ARCH_ARM
#    define ALIMER_ARCH_ARM 1
#elif defined(__ppc__) || defined(_M_PPC) || defined(__CELLOS_LV2__)
#    undef ALIMER_ARCH_PPC
#    define ALIMER_ARCH_PPC 1
#else
#    error "Unknown architecture"
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__64BIT__) || defined(__mips64) || defined(__powerpc64__) || \
	defined(__ppc64__) || defined(__LP64__)
#    undef ALIMER_ARCH_64BIT
#    define ALIMER_ARCH_64BIT 1
#else
#    undef ALIMER_ARCH_32BIT
#    define ALIMER_ARCH_32BIT 1
#endif

/**
* SIMD defines
 */
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86) || defined(__EMSCRIPTEN__)
#   define ALIMER_USE_SSE
 // Detect enabled instruction sets
#   if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && !defined(ALIMER_USE_AVX512)
#       define ALIMER_USE_AVX512
#   endif

#   if (defined(__AVX2__) || defined(ALIMER_USE_AVX512)) && !defined(ALIMER_USE_AVX2)
#       define ALIMER_USE_AVX2
#   endif

#   if (defined(__AVX__) || defined(ALIMER_USE_AVX2)) && !defined(ALIMER_USE_AVX)
#       define ALIMER_USE_AVX
#   endif

#   if (defined(__SSE4_2__) || defined(ALIMER_USE_AVX)) && !defined(ALIMER_USE_SSE4_2)
#       define ALIMER_USE_SSE4_2
#   endif

#   if (defined(__SSE4_1__) || defined(ALIMER_USE_SSE4_2)) && !defined(ALIMER_USE_SSE4_1)
#       define ALIMER_USE_SSE4_1
#   endif

#   if (defined(__F16C__) || defined(ALIMER_USE_AVX2)) && !defined(ALIMER_USE_F16C)
#       define ALIMER_USE_F16C
#   endif

#   if (defined(__LZCNT__) || defined(ALIMER_USE_AVX2)) && !defined(ALIMER_USE_LZCNT)
#       define ALIMER_USE_LZCNT
#   endif

#   if (defined(__BMI__) || defined(ALIMER_USE_AVX2)) && !defined(ALIMER_USE_TZCNT)
#       define ALIMER_USE_TZCNT
#   endif

#   if ALIMER_COMPILER_CLANG || ALIMER_COMPILER_GCC
#       if defined(__FMA__) && !defined(ALIMER_USE_FMADD)
#           define ALIMER_USE_FMADD
#       endif
#   elif ALIMER_COMPILER_MSVC
#       if defined(__AVX2__) && !defined(ALIMER_USE_FMADD) // AVX2 also enables fused multiply add
#           define ALIMER_USE_FMADD
#   endif
#   else
#       error Undefined compiler
#   endif

#elif defined(_M_ARM) || defined(__ARM_NEON__) || defined(__ARM_NEON) || defined(__aarch64__) || defined(_M_ARM64)
#    define ALIMER_USE_NEON
#endif

 /* Compiler defines */
#if defined(__clang__)
#    define ALIMER_THREADLOCAL _Thread_local
#    define ALIMER_DEPRECATED __attribute__(deprecated)
#    define ALIMER_FORCE_INLINE inline __attribute__((__always_inline__))
#    define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#    define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#    define ALIMER_UNREACHABLE() __builtin_unreachable()
#    define ALIMER_DEBUG_BREAK() __builtin_trap()
#define ALIMER_DISABLE_WARNINGS() \
    _Pragma("clang diagnostic push")\
	_Pragma("clang diagnostic ignored \"-Wall\"") \
	_Pragma("clang diagnostic ignored \"-Wextra\"") \
	_Pragma("clang diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-completeness\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-extension\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-function\"")

#define ALIMER_ENABLE_WARNINGS() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#    define ALIMER_THREADLOCAL __thread
#    define ALIMER_DEPRECATED __attribute__(deprecated)
#    define ALIMER_FORCE_INLINE inline __attribute__((__always_inline__))
#    define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#    define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#    define ALIMER_UNREACHABLE() __builtin_unreachable()
#    define ALIMER_DEBUG_BREAK() __builtin_trap()
#	define ALIMER_DISABLE_WARNINGS() \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wall\"") \
	_Pragma("GCC diagnostic ignored \"-Wextra\"") \
	_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"")

#define ALIMER_ENABLE_WARNINGS() _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#    define ALIMER_THREADLOCAL __declspec(thread)
#    define ALIMER_DEPRECATED __declspec(deprecated)
#    define ALIMER_FORCE_INLINE __forceinline
#    define ALIMER_LIKELY(x) (x)
#    define ALIMER_UNLIKELY(x) (x)
#    define ALIMER_UNREACHABLE() __assume(false)
#    define ALIMER_DEBUG_BREAK() __debugbreak()
#   define ALIMER_DISABLE_WARNINGS() __pragma(warning(push, 0))
#   define ALIMER_ENABLE_WARNINGS() __pragma(warning(pop))
#endif

#endif /* ALIMER_PLATFORM_H */
