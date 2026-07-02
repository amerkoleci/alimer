// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"

namespace Alimer
{
    /// 32-bit hash value for a string.
    class ALIMER_API StringId32
    {
    public:
        static constexpr uint32_t kInvalidId = 0xFFFFFFFF;

        /// Construct with zero value.
        StringId32() noexcept
            : hash(kInvalidId)
        {
        }

        /// Construct with an initial value.
        explicit StringId32(uint32_t value_) noexcept
            : hash(value_)
        {
        }

        /// Construct from a C string.
        StringId32(const char* str) noexcept;
        /// Construct from a string.
        StringId32(const String& str) noexcept;
        /// Construct from a string view.
        StringId32(StringView str) noexcept;
        /// Copy-construct from another hash.
        StringId32(const StringId32& other) noexcept;

        /// Assign from another hash.
        StringId32& operator=(const StringId32&) noexcept = default;

        /// Test for equality with another hash.
        bool operator==(const StringId32& rhs) const { return hash == rhs.hash; }

        /// Test for inequality with another hash.
        bool operator!=(const StringId32& rhs) const { return hash != rhs.hash; }

        /// Test if less than another hash.
        bool operator<(const StringId32& rhs) const { return hash < rhs.hash; }

        /// Test if greater than another hash.
        bool operator>(const StringId32& rhs) const { return hash > rhs.hash; }

        /// Return true if nonempty hash value.
        bool IsEmpty() const { return hash == kInvalidId; }

        /// Return true if nonempty hash value.
        explicit operator bool() const { return !IsEmpty(); }

        /// Return hash value.
        uint32_t GetHash() const { return hash; }

        /// Return as string.
        String ToString() const;

        /// Return original string.
        String GetString() const;

        /// Zero hash.
        static const StringId32 Empty;

    private:
        static uint32_t Hash(const char* str);

        /// Hash value.
        uint32_t hash;
    };

    static_assert(sizeof(StringId32) == sizeof(uint32_t), "Unexpected StringId32 size.");
}

template<> struct std::hash<Alimer::StringId32>
{
    std::size_t operator()(const Alimer::StringId32& value) const noexcept
    {
        return value.GetHash();
    }
};
