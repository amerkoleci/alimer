// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/String.h"

namespace Alimer::Assert
{
    enum class FailBehavior
    {
        Halt,
        Continue,
    };

    typedef FailBehavior(*AssertHandlerFn)(const char* condition,
        const std::string& message,
        const char* file,
        uint32_t line);

    ALIMER_API AssertHandlerFn GetHandler();
    ALIMER_API void SetHandler(AssertHandlerFn newHandler);

    ALIMER_API FailBehavior ReportFailure(const char* condition, const char* file, int line, const std::string& message);
}

#ifndef ALIMER_ENABLE_ASSERT
#	if (defined(_DEBUG) || defined(PROFILE))
#		define ALIMER_ENABLE_ASSERT 1
#	else
#		define ALIMER_ENABLE_ASSERT 0
#	endif
#endif

#if ALIMER_ENABLE_ASSERT
#define ALIMER_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, Alimer::kEmptyString) == \
					Alimer::Assert::FailBehavior::Halt) \
					ALIMER_BREAKPOINT; \
			} \
		} while(0)

#define ALIMER_ASSERT_MSG(cond, ...) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, FMT::format(__VA_ARGS__)) == \
					Alimer::Assert::FailBehavior::Halt) \
					ALIMER_BREAKPOINT; \
			} \
		} while(0)

#define ALIMER_ASSERT_FAIL(...) \
		do \
		{ \
			if (Alimer::Assert::ReportFailure(0, __FILE__, __LINE__, FMT::format(__VA_ARGS__)) == \
				Alimer::Assert::FailBehavior::Halt) \
			    ALIMER_BREAKPOINT; \
		} while(0)

#define ALIMER_VERIFY(cond) ALIMER_ASSERT(cond)
#define ALIMER_VERIFY_MSG(cond, ...) ALIMER_ASSERT_MSG(cond, FMT::format(__VA_ARGS__))

// Assert that will be called very often, like inside data structures e.g. operator[].
// Making it non-empty can make program slow.
#define ALIMER_HEAVY_ASSERT(expr)   //ALIMER_ASSERT(expr)
#else
#   define ALIMER_ASSERT(condition) do { ALIMER_UNUSED(condition); } while(0)
#   define ALIMER_ASSERT_MSG(condition,...) do { ALIMER_UNUSED(condition); } while(0)
#   define ALIMER_ASSERT_FAIL(...)
#   define ALIMER_VERIFY(cond) (void)(cond)
#   define ALIMER_VERIFY_MSG(cond, ...) do { (void)(cond);} while(0)
#   define ALIMER_HEAVY_ASSERT(expr)
#endif
