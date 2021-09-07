// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Vector4.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    const Vector4 Vector4::Zero = { 0.f, 0.f, 0.f, 0.f };
    const Vector4 Vector4::One = { 1.f, 1.f, 1.f, 1.f };
    const Vector4 Vector4::UnitX = { 1.f, 0.f, 0.f, 0.f };
    const Vector4 Vector4::UnitY = { 0.f, 1.f, 0.f, 0.f };
    const Vector4 Vector4::UnitZ = { 0.f, 0.f, 1.f, 0.f };
    const Vector4 Vector4::UnitW = { 0.f, 0.f, 0.f, 1.f };

    std::string Vector4::ToString() const
    {
        return fmt::format("{} {} {} {}", x, y, z, w);
    }
}
