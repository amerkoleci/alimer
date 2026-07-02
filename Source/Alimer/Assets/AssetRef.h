// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/StringId.h"
#include "Alimer/Core/Containers.h"

namespace Alimer
{
	class Stream;

	/// Typed asset reference for serialization.
	struct ALIMER_API AssetRef
	{
		/// Asset type.
		StringId32 type;
		/// Asset name.
		String name;

		/// Constructor.
        AssetRef() = default;

		/// Construct with type only and empty id.
		explicit AssetRef(StringId32 type_)
			: type(type_)
		{
		}

		/// Construct with type and asset name.
        AssetRef(StringId32 type, const String& name_)
			: type(type)
			, name(name_)
		{
		}

		AssetRef(const AssetRef&) = default;
		AssetRef& operator=(const AssetRef&) = default;

		AssetRef(AssetRef&&) = default;
		AssetRef& operator=(AssetRef&&) = default;

        /// Parse asset reference from a string. Return Empty on failure.
        [[nodiscard]] static AssetRef Parse(const String& str);

        /// Try parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(const String& str, AssetRef* result);

		/// Deserialize from a binary stream.
		void FromBinary(Stream& source);

		/// Return as a string.
        String ToString() const;
		/// Serialize to a binary stream.
		void ToBinary(Stream& dest) const;

		/// Test for equality with another reference.
		bool operator == (const AssetRef& rhs) const { return type == rhs.type && name == rhs.name; }
		/// Test for inequality with another reference.
		bool operator != (const AssetRef& rhs) const { return !(*this == rhs); }

        /// Empty asset reference.
        static const AssetRef Empty;
	};

    /// List of typed resource references for serialization.
    struct AssetRefList
    {
        /// Asset type.
        StringId32 type;
        /// List of resource names.
        StringVector names;

        /// Constructor.
        AssetRefList() = default;

        /// Copy-construct.
        AssetRefList(const AssetRefList& rhs)
            : type(rhs.type)
            , names(rhs.names)
        {
        }

        /// Construct with type and name list.
        AssetRefList(StringId32 type_, const StringVector& names_ = {})
            : type(type_)
            , names(names_)
        {
        }

        /// Parse asset reference list from a string. Return Empty on failure.
        [[nodiscard]] static AssetRefList Parse(const String& str);

        /// Try parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(const String& str, AssetRefList* result);

        /// Deserialize from a binary stream.
        void FromBinary(Stream& source);

        /// Return as a string.
        String ToString() const;
        /// Deserialize from a binary stream.
        void ToBinary(Stream& dest) const;

        /// Test for equality with another reference list.
        bool operator == (const AssetRefList& rhs) const { return type == rhs.type && names == rhs.names; }
        /// Test for inequality with another reference list.
        bool operator != (const AssetRefList& rhs) const { return !(*this == rhs); }

        /// Empty asset reference list.
        static const AssetRefList Empty;
    };
}
