// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

#if defined(ALIMER_STD_CONTAINERS)
#include <unordered_set>
#else
#include <ankerl/unordered_dense.h>
#endif

namespace Alimer
{
    template<typename Key>
#if defined(ALIMER_STD_CONTAINERS)
    using UnorderedSet = std::unordered_set<Key, HashType<Key>>;
#else
    using UnorderedSet = ankerl::unordered_dense::set<Key>;
#endif
}
