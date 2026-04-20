// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Log.h"
#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif

#ifndef RAPIDJSON_PARSE_DEFAULT_FLAGS
#define RAPIDJSON_PARSE_DEFAULT_FLAGS (kParseIterativeFlag)
#endif

#include <exception>
#undef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) do { \
	if (!(x)) { \
        LOGE("Rapidjson assert: {}", #x); \
		std::terminate(); \
    } \
} while(0)

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"
