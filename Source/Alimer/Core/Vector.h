// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

#if defined(ALIMER_STD_CONTAINERS)
#include <vector>
#else
#include <vector>
#endif

namespace Alimer
{
    template<typename T, typename Allocator = std::allocator<T>>
#if defined(ALIMER_STD_CONTAINERS)
    using Vector = std::vector<T, Allocator>;
#else
    using Vector = std::vector<T, Allocator>;
#endif
}
