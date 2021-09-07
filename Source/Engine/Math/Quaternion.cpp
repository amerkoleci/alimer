// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Quaternion.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    const Quaternion Quaternion::Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
    const Quaternion Quaternion::One = { 1.0f, 1.0f, 1.0f, 1.0f };
    const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };

    std::string Quaternion::ToString() const
    {
        return fmt::format("{} {} {} {}", x, y, z, w);
    }

    size_t Quaternion::ToHash() const
    {
        size_t hash = 0;
        Alimer::HashCombine(hash, x, y, z, w);
        return hash;
    }
}
