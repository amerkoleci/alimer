// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/StringUtils.h"
#include <functional>

namespace Alimer::Assert
{
	enum class FailBehavior
	{
		Halt,
		Continue,
	};

	using AssertFn = std::function<FailBehavior(const char* condition, const std::string& message, const char* file, int line)>;

	ALIMER_API AssertFn GetHandler();
	ALIMER_API void SetHandler(AssertFn newHandler);

    ALIMER_API FailBehavior ReportFailure(const char* condition, const char* file, int line, const std::string& message);
}

#ifndef ALIMER_ENABLE_ASSERT
#	if (defined(_DEBUG) || defined(PROFILE))
#		define ALIMER_ENABLE_ASSERT 1
#	else
#		define ALIMER_ENABLE_ASSERT 0
#	endif
#endif

#ifdef ALIMER_ENABLE_ASSERT
#define ALIMER_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, Alimer::kEmptyString) == \
					Alimer::Assert::FailBehavior::Halt) \
					ALIMER_DEBUG_BREAK(); \
			} \
		} while(0)

#define ALIMER_ASSERT_MSG(cond, ...) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, fmt::format(__VA_ARGS__)) == \
					Alimer::Assert::FailBehavior::Halt) \
					ALIMER_DEBUG_BREAK(); \
			} \
		} while(0)

#define ALIMER_ASSERT_FAIL(...) \
		do \
		{ \
			if (Alimer::Assert::ReportFailure(0, __FILE__, __LINE__, fmt::format(__VA_ARGS__)) == \
				Alimer::Assert::FailBehavior::Halt) \
			ALIMER_DEBUG_BREAK(); \
		} while(0)

#define ALIMER_VERIFY(cond) ALIMER_ASSERT(cond)
#define ALIMER_VERIFY_MSG(cond, ...) ALIMER_ASSERT_MSG(cond, fmt::format(__VA_ARGS__))
#else
#   define ALIMER_ASSERT(condition) do { ALIMER_UNUSED(condition); } while(0)
#   define ALIMER_ASSERT_MSG(condition,...) do { ALIMER_UNUSED(condition); } while(0)
#   define ALIMER_ASSERT_FAIL(...)
#   define ALIMER_VERIFY(cond) (void)(cond)
#   define ALIMER_VERIFY_MSG(cond, ...) do { (void)(cond);} while(0)
#endif
