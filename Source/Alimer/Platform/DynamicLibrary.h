// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"

namespace Alimer
{
    class ALIMER_API DynamicLibrary final
    {
    public:
        DynamicLibrary() = default;
        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary(DynamicLibrary&&) noexcept = default;
        ~DynamicLibrary() = default;

        DynamicLibrary& operator=(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(DynamicLibrary&& lib) noexcept = default;
    };
}
