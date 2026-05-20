// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    enum class AudioFormat : uint32_t
    {
        /// Signed 16-bit integer samples, range [-32768, 32767].
        Int16,
        /// Signed 32-bit integer samples, range [-2147483648, 2147483647].
        Int32,
        /// 32-bit floating point samples, range [-1, 1].
        Float32,
    };
}
