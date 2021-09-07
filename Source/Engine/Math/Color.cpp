// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Color.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    std::string Color::ToString() const
    {
        return fmt::format("{} {} {} {}", r, g, b, a);
    }

    size_t Color::ToHash() const
    {
        size_t hash = 0;
        HashCombine(hash, r, g, b, a);
        return hash;
    }
}
