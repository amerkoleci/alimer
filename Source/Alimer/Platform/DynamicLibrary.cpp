// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/DynamicLibrary.h"

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#elif defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

using namespace Alimer;
