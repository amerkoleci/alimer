// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer::BitOperations
{
    ALIMER_API uint32_t BitScanForward(uint32_t value);
    ALIMER_API uint32_t BitScanForward(uint64_t value);
    ALIMER_API uint32_t BitScanReverse(uint32_t value);
    ALIMER_API uint32_t BitScanReverse(uint64_t value);

    ALIMER_API uint32_t LeadingZeroCount(uint32_t value);
    ALIMER_API uint32_t LeadingZeroCount(uint64_t  value);

    ALIMER_API uint32_t Log2(uint32_t value);
    ALIMER_API uint32_t Log2(uint64_t value);

    ALIMER_API uint32_t PopCount(uint32_t value);
    ALIMER_API uint32_t PopCount(uint64_t value);

    /// Reverses the bits in an integer value
    ALIMER_API uint32_t ReverseBits(uint32_t value);

    /// Reverses the bits in an integer value
    ALIMER_API uint64_t ReverseBits(uint64_t value);

    ALIMER_API uint32_t RotateLeft(uint32_t value, uint32_t offset);
    ALIMER_API uint64_t RotateLeft(uint64_t value, uint32_t offset);
    ALIMER_API uint32_t RotateRight(uint32_t value, uint32_t offset);
    ALIMER_API uint64_t RotateRight(uint64_t value, uint32_t offset);

    ALIMER_API uint32_t TrailingZeroCount(uint32_t value);
    ALIMER_API uint32_t TrailingZeroCount(uint64_t value);
}
