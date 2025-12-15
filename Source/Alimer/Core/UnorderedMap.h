// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

#if defined(ALIMER_STD_CONTAINERS)
#include <unordered_map>
#else
#include <ankerl/unordered_dense.h>
#endif

namespace Alimer
{
    template<class TKey, class TValue>
#if defined(ALIMER_STD_CONTAINERS)
    using UnorderedMap = std::unordered_map<TKey, TValue>;
#else
    using UnorderedMap = ankerl::unordered_dense::map<TKey, TValue>;
#endif
}
