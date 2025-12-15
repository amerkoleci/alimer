// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/BitOperations.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

// TODO: Use std::countr_zero, etc
//#if ALIMER_CPP20
//#include <bit>
//#endif

using namespace Alimer;

uint32_t BitOperations::BitScanForward(uint32_t value)
{
    ALIMER_ASSERT(value != 0);

#if defined(_MSC_VER)
    unsigned long result;
    ::_BitScanForward(&result, value);
    return static_cast<uint32_t>(result);
#else
    int32_t result = __builtin_ctz(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::BitScanForward(uint64_t value)
{
    ALIMER_ASSERT(value != 0);

#if defined(_MSC_VER)
#if ALIMER_ARCH_64BIT
    unsigned long result;
    ::_BitScanForward64(&result, value);
    return static_cast<uint32_t>(result);
#else
    uint32_t lower = static_cast<uint32_t>(value);

    if (lower == 0)
    {
        uint32_t upper = static_cast<uint32_t>(value >> 32);
        return 32 + BitScanForward(upper);
    }

    return BitScanForward(lower);
#endif // ALIMER_ARCH_64BIT
#else
    int32_t result = __builtin_ctzll(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::BitScanReverse(uint32_t value)
{
    ALIMER_ASSERT(value != 0);

#if defined(_MSC_VER)
    unsigned long result;
    ::_BitScanReverse(&result, value);
    return static_cast<uint32_t>(result);
#else
    // LZCNT returns index starting from MSB, whereas BSR gives the index from LSB.
    // 31 ^ BSR here is equivalent to 31 - BSR since the BSR result is always between 0 and 31.
    // This saves an instruction, as subtraction from constant requires either MOV/SUB or NEG/ADD.

    int32_t result = __builtin_clz(value);
    return static_cast<uint32_t>(31 ^ result);
#endif
}

uint32_t BitOperations::BitScanReverse(uint64_t value)
{
    ALIMER_ASSERT(value != 0);

#if defined(_MSC_VER)
#if ALIMER_ARCH_64BIT
    unsigned long result;
    ::_BitScanReverse64(&result, value);
    return static_cast<uint32_t>(result);
#else
    uint32_t upper = static_cast<uint32_t>(value >> 32);

    if (upper == 0)
    {
        uint32_t lower = static_cast<uint32_t>(value);
        return BitScanReverse(lower);
    }

    return 32 + BitScanReverse(upper);
#endif // ALIMER_ARCH_64BIT
#else
    // LZCNT returns index starting from MSB, whereas BSR gives the index from LSB.
    // 63 ^ BSR here is equivalent to 63 - BSR since the BSR result is always between 0 and 63.
    // This saves an instruction, as subtraction from constant requires either MOV/SUB or NEG/ADD.

    int32_t result = __builtin_clzll(value);
    return static_cast<uint32_t>(63 ^ result);
#endif
}

uint32_t BitOperations::LeadingZeroCount(uint32_t value)
{
    if (value == 0)
    {
        return 32;
    }

#if defined(_MSC_VER)
    // LZCNT returns index starting from MSB, whereas BSR gives the index from LSB.
    // 31 ^ BSR here is equivalent to 31 - BSR since the BSR result is always between 0 and 31.
    // This saves an instruction, as subtraction from constant requires either MOV/SUB or NEG/ADD.

    uint32_t result = BitOperations::BitScanReverse(value);
    return 31 ^ result;
#else
    int32_t result = __builtin_clz(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::LeadingZeroCount(uint64_t value)
{
    if (value == 0)
    {
        return 64;
    }

#if defined(_MSC_VER)
    // LZCNT returns index starting from MSB, whereas BSR gives the index from LSB.
    // 63 ^ BSR here is equivalent to 63 - BSR since the BSR result is always between 0 and 63.
    // This saves an instruction, as subtraction from constant requires either MOV/SUB or NEG/ADD.

    uint32_t result = BitOperations::BitScanReverse(value);
    return 63 ^ result;
#else
    int32_t result = __builtin_clzll(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::Log2(uint32_t value)
{
    // The 0->0 contract is fulfilled by setting the LSB to 1.
    // Log(1) is 0, and setting the LSB for values > 1 does not change the log2 result.
    return 31 ^ BitOperations::LeadingZeroCount(value | 1);
}

uint32_t BitOperations::Log2(uint64_t value)
{
    // The 0->0 contract is fulfilled by setting the LSB to 1.
    // Log(1) is 0, and setting the LSB for values > 1 does not change the log2 result.
    return 63 ^ BitOperations::LeadingZeroCount(value | 1);
}

uint32_t BitOperations::PopCount(uint32_t value)
{
#if defined(_MSC_VER)
    // Inspired by the Stanford Bit Twiddling Hacks by Sean Eron Anderson:
    // http://graphics.stanford.edu/~seander/bithacks.html

    const uint32_t c1 = 0x55555555u;
    const uint32_t c2 = 0x33333333u;
    const uint32_t c3 = 0x0F0F0F0Fu;
    const uint32_t c4 = 0x01010101u;

    value -= (value >> 1) & c1;
    value = (value & c2) + ((value >> 2) & c2);
    value = (((value + (value >> 4)) & c3) * c4) >> 24;

    return value;
#else
    int32_t result = __builtin_popcount(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::PopCount(uint64_t value)
{
#if defined(_MSC_VER)
    // Inspired by the Stanford Bit Twiddling Hacks by Sean Eron Anderson:
    // http://graphics.stanford.edu/~seander/bithacks.html

    const uint64_t c1 = 0x5555555555555555ull;
    const uint64_t c2 = 0x3333333333333333ull;
    const uint64_t c3 = 0x0F0F0F0F0F0F0F0Full;
    const uint64_t c4 = 0x0101010101010101ull;

    value -= (value >> 1) & c1;
    value = (value & c2) + ((value >> 2) & c2);
    value = (((value + (value >> 4)) & c3) * c4) >> 56;

    return static_cast<uint32_t>(value);
#else
    int32_t result = __builtin_popcountll(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::ReverseBits(uint32_t value)
{
    // Inspired by the Stanford Bit Twiddling Hacks by Sean Eron Anderson:
    // http://graphics.stanford.edu/~seander/bithacks.html

    uint32_t result = value;

    // swap odd and even bits
    result = ((result >> 1) & 0x55555555) | ((result & 0x55555555) << 1);

    // swap consecutive pairs
    result = ((result >> 2) & 0x33333333) | ((result & 0x33333333) << 2);

    // swap nibbles ...
    result = ((result >> 4) & 0x0F0F0F0F) | ((result & 0x0F0F0F0F) << 4);

    // swap bytes
    result = ((result >> 8) & 0x00FF00FF) | ((result & 0x00FF00FF) << 8);

    // swap 2-byte pairs
    result = (result >> 16) | (result << 16);

    return result;
}

uint64_t BitOperations::ReverseBits(uint64_t value)
{
    // Inspired by the Stanford Bit Twiddling Hacks by Sean Eron Anderson:
    // http://graphics.stanford.edu/~seander/bithacks.html

    uint64_t result = value;

    // swap odd and even bits
    result = ((result >> 1) & 0x5555555555555555ull) | ((result & 0x5555555555555555ull) << 1);

    // swap consecutive pairs
    result = ((result >> 2) & 0x3333333333333333ull) | ((result & 0x3333333333333333ull) << 2);

    // swap nibbles ...
    result = ((result >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((result & 0x0F0F0F0F0F0F0F0Full) << 4);

    // swap bytes
    result = ((result >> 8) & 0x00FF00FF00FF00FFull) | ((result & 0x00FF00FF00FF00FFull) << 8);

    // swap 2-byte pairs
    result = ((result >> 16) & 0x0000FFFF0000FFFFull) | ((result & 0x0000FFFF0000FFFFull) << 16);

    // swap 4-byte pairs
    result = (result >> 32) | (result << 32);

    return result;
}

uint32_t BitOperations::RotateLeft(uint32_t value, uint32_t offset)
{
    // Mask the offset to ensure deterministic xplat behavior for overshifting
    return (value << (offset & 0x1F)) | (value >> ((32 - offset) & 0x1F));
}

uint64_t BitOperations::RotateLeft(uint64_t value, uint32_t offset)
{
    // Mask the offset to ensure deterministic xplat behavior for overshifting
    return (value << (offset & 0x3F)) | (value >> ((64 - offset) & 0x3F));
}

uint32_t BitOperations::RotateRight(uint32_t value, uint32_t offset)
{
    // Mask the offset to ensure deterministic xplat behavior for overshifting
    return (value >> (offset & 0x1F)) | (value << ((32 - offset) & 0x1F));
}

uint64_t BitOperations::RotateRight(uint64_t value, uint32_t offset)
{
    // Mask the offset to ensure deterministic xplat behavior for overshifting
    return (value >> (offset & 0x3F)) | (value << ((64 - offset) & 0x3F));
}

uint32_t BitOperations::TrailingZeroCount(uint32_t value)
{
    if (value == 0)
    {
        return 32;
    }

#if defined(_MSC_VER)
    return BitOperations::BitScanForward(value);
#else
    int32_t result = __builtin_ctz(value);
    return static_cast<uint32_t>(result);
#endif
}

uint32_t BitOperations::TrailingZeroCount(uint64_t value)
{
    if (value == 0)
    {
        return 64;
    }

#if defined(_MSC_VER)
    return BitOperations::BitScanForward(value);
#else
    int32_t result = __builtin_ctzll(value);
    return static_cast<uint32_t>(result);
#endif
}
