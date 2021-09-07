// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Matrix3x4.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    const Matrix3x4 Matrix3x4::Zero = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };
    const Matrix3x4 Matrix3x4::Identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f };

    Matrix3x4::Matrix3x4(const float* data) noexcept
    {
        ALIMER_ASSERT(data != nullptr);

        m11 = data[0];
        m12 = data[1];
        m13 = data[2];
        m14 = data[3];

        m21 = data[4];
        m22 = data[5];
        m23 = data[6];
        m24 = data[7];

        m31 = data[8];
        m32 = data[9];
        m33 = data[10];
        m34 = data[11];
    }

    std::string Matrix3x4::ToString() const
    {
        return fmt::format("{} {} {} {} {} {} {} {} {} {} {} {}",
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34);
    }

    size_t Matrix3x4::ToHash() const
    {
        size_t hash = 0;
        Alimer::HashCombine(hash,
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34);
        return hash;
    }
}
