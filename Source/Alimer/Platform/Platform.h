// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"

namespace Alimer::Platform
{
    ALIMER_API PlatformID GetID();
    ALIMER_API PlatformFamily GetFamily();
    ALIMER_API uint32_t GetNumPhysicalCPUs();
}
