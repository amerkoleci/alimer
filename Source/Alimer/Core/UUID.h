// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    /// 32-bit hash value for a string.
    class ALIMER_API UUID
    {
    public:
        UUID();
        UUID(uint64_t value);

        UUID(const UUID&) = default;
        UUID& operator=(const UUID&) = default;
        UUID(UUID&&) = default;
        UUID& operator=(UUID&&) = default;

        /// Return value.
        [[nodiscard]] uint64_t GetValue() const { return _value; }

        /// Return value.
        [[nodiscard]] operator uint64_t() const { return _value; }

        /// Test for equality with another hash.
        bool operator==(const UUID& rhs) const { return _value == rhs._value; }

        /// Test for inequality with another hash.
        bool operator!=(const UUID& rhs) const { return _value != rhs._value; }

        /// Test if less than another hash.
        bool operator<(const UUID& rhs) const { return _value < rhs._value; }

        /// Test if greater than another hash.
        bool operator>(const UUID& rhs) const { return _value > rhs._value; }

    private:
        uint64_t _value;
    };

    static_assert(sizeof(UUID) == sizeof(uint64_t), "Unexpected UUID size.");
}

template<> struct std::hash<Alimer::UUID>
{
    std::size_t operator()(const Alimer::UUID& value) const noexcept
    {
        return static_cast<std::size_t>(value.GetValue());
    }
};
