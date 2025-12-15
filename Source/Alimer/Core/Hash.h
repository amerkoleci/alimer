// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    /// Implements the FNV-1a hash algorithm
    /// @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    /// @param inData Data block of bytes
    /// @param inSize Number of bytes
    /// @param inSeed Seed of the hash (can be used to pass in the hash of a previous operation, otherwise leave default)
    /// @return Hash
    inline uint64_t HashBytes(const void* inData, uint32_t inSize, uint64_t inSeed = 0xcbf29ce484222325UL)
    {
        uint64_t hash = inSeed;
        for (const uint8_t* data = reinterpret_cast<const uint8_t*>(inData); data < reinterpret_cast<const uint8_t*>(inData) + inSize; ++data)
        {
            hash = hash ^ uint64_t(*data);
            hash = hash * 0x100000001b3UL;
        }
        return hash;
    }

    /// A 64 bit hash function by Thomas Wang, Jan 1997
    /// See: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
    /// @param value Value to hash
    /// @return Hash
    inline uint64_t Hash64(uint64_t value)
    {
        uint64_t hash = value;
        hash = (~hash) + (hash << 21); // hash = (hash << 21) - hash - 1;
        hash = hash ^ (hash >> 24);
        hash = (hash + (hash << 3)) + (hash << 8); // hash * 265
        hash = hash ^ (hash >> 14);
        hash = (hash + (hash << 2)) + (hash << 4); // hash * 21
        hash = hash ^ (hash >> 28);
        hash = hash + (hash << 31);
        return hash;
    }

    /// @brief Helper function that hashes a single value into ioSeed
   /// Taken from: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
    template <typename T>
    inline void HashCombine(std::size_t& ioSeed, const T& value)
    {
        std::hash<T> hasher;
        ioSeed ^= hasher(value) + 0x9e3779b9 + (ioSeed << 6) + (ioSeed >> 2);
    }

    template <typename T, typename... Rest>
    inline void HashCombine(size_t& seed, const T& v, Rest&&... rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        HashCombine(seed, std::forward<Rest>(rest)...);
    }

    constexpr size_t StringHash(const char* input)
    {
        // https://stackoverflow.com/questions/2111667/compile-time-string-hashing
        size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
        const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

        while (*input)
        {
            hash ^= static_cast<size_t>(*input);
            hash *= prime;
            ++input;
        }

        return hash;
    }
}

#undef ALIMER_MAKE_LIMITS
