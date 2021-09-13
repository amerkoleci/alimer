// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Color.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    const Color Color::CornflowerBlue = { 0.392156899f, 0.584313750f, 0.929411829f, 1.0f };

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
