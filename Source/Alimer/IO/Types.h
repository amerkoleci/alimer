// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"
#include <filesystem>

namespace Alimer
{
    /// File open mode.
    enum class FileMode : uint32_t
    {
        Read,
        Write,
        ReadWrite,
    };

    using FileSystemPath = std::filesystem::path;

    static constexpr uint32_t kInvalidObjectRefId = 0xFFFFFFFF;

    /// Reference to an object with id for serialization.
    struct ALIMER_API ObjectRef final
    {
        /// Object id.
        uint32_t id;

        /// Construct with no reference.
        ObjectRef() noexcept
            : id(kInvalidObjectRefId)
        {

        }

        // Copy-construct.
        ObjectRef(const ObjectRef& rhs) = default;

        /// Construct with object id.
        constexpr explicit ObjectRef(uint32_t id_) noexcept
            : id(id_)
        {
        }

        /// Test for equality with another reference.
        bool operator == (const ObjectRef& rhs) const noexcept { return id == rhs.id; }
        /// Test for inequality with another reference.
        bool operator != (const ObjectRef& rhs) const noexcept { return id != rhs.id; }

        /// Return true if nonempty hash value.
        bool IsValid() const noexcept { return id != kInvalidObjectRefId; }

        /// Return true if nonempty hash value.
        explicit operator bool() const noexcept { return id != kInvalidObjectRefId; }

        /// Invalid object reference.
        static const ObjectRef Invalid;
    };

    static_assert(sizeof(ObjectRef) == sizeof(uint32_t), "Unexpected ObjectRef size.");
}

template<> struct std::hash<Alimer::ObjectRef>
{
    std::size_t operator()(const Alimer::ObjectRef& value) const noexcept
    {
        return std::hash<uint32_t>()(value.id);
    }
};
